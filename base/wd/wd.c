/*
 * Development of this (watchdog) module was sponsored by Alcatel, Strasbourg
 * as part of general debugging enhancements to RTAI.
 *
 * Copyright (©) 2001 Ian Soanes <ians@lineo.com>, All rights reserved
 *
 * Copyright (©) 2009: Kenneth Jacker   <khj@cs.appstate.edu>
 * Copyright (©) 2009: Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*******************************************************************************
 *
 *                           RTAI Watchdog Module
 *                           --------------------
 *
 * Module to provide various watchdog protection services to RTAI thereby 
 * protecting it (and the host Linux OS) against programming errors in RTAI
 * applications.
 *
 * Services provided...
 *
 * 1. Detection of RT tasks that exceed their alloted time period. This will
 *    detect tasks that have gone into infinite loops or are regularly
 *    overunning. Normally such tasks would prevent other tasks (and Linux)
 *    from being scheduled and possibly lock and/or crash the system.
 *
 * 2. The ability to adjust important parameters when inserting the watchdog 
 *    module and from other RT modules via a simple API.
 *
 * 3. Configurable policy to use on bad tasks. Currently available policies 
 *    are...
 *
 *    o Do nothing, other than log some messages and keep a record of the bad 
 *      task. In reality you will probably never get the chance to see these 
 *      messages if the task is locking out the Linux task. This policy is not 
 *      usually recommended.
 *
 *    o Resynchronise the task's frame time and nothing more. This is good for
 *      tasks that occasionally overrun. Doing this should prevent the system 
 *      from locking up and crashing as the scheduler tries to catch up with the
 *      missed deadlines. The maximum (if any) number of times to resynchronise 
 *      a task before permanently suspending it is configurable. 
 *
 *    o Debug policy, this is a special case of the above resync policy. It is 
 *      recommended when step and trace debugging RT tasks. 
 *      (See README.WATCHDOG for full details)
 *
 *    o Stretch (increase) the period of the offending task until it no longer
 *      overruns. The percentage increment (of the original period) is 
 *      configurable, as is the maximum (if any) number of times to increase 
 *      the period before permanently suspending the task. When a task's period 
 *      is increased in this way the scheduler is asked to resynchronise the 
 *      task's frame time in order to prevent the system locking up and crashing
 *      as it tries to catch up with the missed deadlines. This policy could be 
 *      a useful development aid if you are not sure what period to use for a 
 *      task.
 *
 *    o Slip the offending task by forcibly suspending it for a percentage of
 *      its period. The percentage slip is configurable, as is the maximum (if
 *      any) number of times to slip the task before it is permanently 
 *      suspended. By slipping the task, other tasks (including Linux) are 
 *      given the oppurtunity to run and the system doesn't lock up.
 *
 *    o Suspend the offending task so that it no longer poses any threat to 
 *      the system. The task will still be known to the scheduler so it could 
 *      possibly be resumed sometime later.
 *
 *    o Kill the offending task and remove all trace of it.
 *
 * 4. A safety limit that will suspend any task that overruns excessively. The
 *    definition of 'excessive' is configurable and can also be disabled. This 
 *    is designed to deal with infinite loops no matter what the current policy.
 *    The safety limit needs to be set sufficiently high so that it doesn't 
 *    interfere with the prevailing watchdog policy. This limit is automatically
 *    disabled when the policy is set to 'Debug' in order not to suspend RT 
 *    tasks being step and trace debugged.
 *
 * 5. Keeps a record of bad tasks (apart from those that have been killed) that 
 *    can be examined via a /proc interface. (/proc/rtai/watchdog)
 * 
 * ID: @(#)$Id: wd.c,v 1.18 2017/03/19 15:55:22 mante Exp $
 *
 *******************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#ifdef CONFIG_PROC_FS
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>
static struct proc_dir_entry *wd_proc;
static int PROC_READ_FUN(wdog_read_proc);
#endif

#include <rtai_sched.h>
#include <rtai_wd.h>

// Switches on LED heartbeat and extra logging
//#define WDBUG
#ifdef WDBUG
#define LPT_PORT 0x378
#define DBUG WDLOG
#else
#define DBUG(x...)
#endif

// Leave this defined if you don't want to use RTAI dynamic memory management
#define MY_ALLOC
#ifndef MY_ALLOC
#ifdef CONFIG_RTAI_MALLOC
#include <rtai_malloc.h>
#else
#define MY_ALLOC		// Not configured so we must use our own
#endif
#endif
#ifdef MY_ALLOC
#define BAD_TASK_MAX 100	// Feel free to change this

static DEFINE_SPINLOCK(alloc_lock);
static BAD_RT_TASK bad_task_pool[BAD_TASK_MAX];
#endif

// The current version number
static char version[] = "$Revision: 1.18 $";
static char ver[10];

// User friendly policy names
static char *policy_name[] = 
	{"Nothing", "Resync", "Debug", "Stretch", "Slip", "Suspend", "Kill"};

// Private data
static int	    num_wdogs;		// Number of watchdogs (and task lists)
static RT_TASK 	    wdog[RTAI_NR_CPUS];	// Watchdog tasks (1 per CPU)
static RT_TASK 	   *tlists[RTAI_NR_CPUS];	// Scheduler's RT task lists
static BAD_RT_TASK *bad_tl[RTAI_NR_CPUS]; // Bad task lists (1 per watchdog)

// -------------------------- CONFIGURABLE PARAMETERS --------------------------
// Module parameters
static int TickPeriod = 10000000;	// Task period in nano seconds
RTAI_MODULE_PARM(TickPeriod, int);	// (should be shorter than all others)

static int wd_OneShot = 1;		// One shot timer mode (the only one in use now)
//RTAI_MODULE_PARM(wd_OneShot, int);	// (should be the same as other tasks) 

static int Grace = 3;			// How much a task can be overdue
RTAI_MODULE_PARM(Grace, int);		// (in periods, always 1 in some modes)

static int GraceDiv = 1;		// Divisor to allow Gracevalues < 1
RTAI_MODULE_PARM(GraceDiv, int);	// overrun = period * Grace / Gracediv

static int Safety = 100;		// Safety net to suspend infinite loops
RTAI_MODULE_PARM(Safety, int);		// (overrides policy, -ve disables)

static int Policy = WD_SUSPEND;		// How to punish misbehavers
RTAI_MODULE_PARM(Policy, int);		// (see above and header for details)

static int Stretch = 10;		// %ge to increase period by
RTAI_MODULE_PARM(Stretch, int);		// (can be over 100%, 100% is doubling)

static int Slip = 10;			// %ge of period to slip a task
RTAI_MODULE_PARM(Slip, int);		// (can be over 100%)

static int Limit = 100;			// Maximum number of offences
RTAI_MODULE_PARM(Limit, int);		// (-ve means disabled ie. no limit)

static int LooperTimeLimit = 100;	// Maximum looper time ms
RTAI_MODULE_PARM(LooperTimeLimit, int);	// (care it combines with wd period)
static int LooperLimit;

// Parameter configuring API
RTAI_SYSCALL_MODE int rt_wdset_grace(int new) // How much a task can be overdue
{
    int old = Grace;

    if (Policy <= WD_STRETCH && new != 1)	return -EINVAL;
    if (new < 1) 		       	return -EINVAL;
    Grace = new;
    return old;
}

RTAI_SYSCALL_MODE int rt_wdset_gracediv(int new) // Divisor for Gracevalues < 1
{
    int old = GraceDiv;

    if (Policy <= WD_STRETCH && new != 1)	return -EINVAL;
    if (new < 1) 		       	return -EINVAL;
    GraceDiv = new;
    return old;
}

RTAI_SYSCALL_MODE int rt_wdset_safety(int new) // Safety net to suspend infinite loops
{
    int old = Safety;

    if (new >= 0 && new < Grace)       	return -EINVAL;
    Safety = new;
    return old;
}

RTAI_SYSCALL_MODE wd_policy rt_wdset_policy(wd_policy new) // How to punish misbehavers
{
    wd_policy old = Policy;

    if (new < WD_NOTHING || new > WD_KILL)   return -EINVAL;
    if (new <= WD_STRETCH)                Grace  = GraceDiv = 1;
    if (new == WD_DEBUG)                  Safety = Limit = -1;
    Policy = new;
    return old;
}

RTAI_SYSCALL_MODE int rt_wdset_slip(int new)  // %ge of period to slip a task
{
    int old = Slip;

    if (new < 0) 			return -EINVAL;
    Slip = new;
    return old;
}

RTAI_SYSCALL_MODE int rt_wdset_stretch(int new)	 // %ge to increase period by
{
    int old = Stretch;

    if (new < 0) 			return -EINVAL;
    Stretch = new;
    return old;
}

RTAI_SYSCALL_MODE int rt_wdset_limit(int new)  // Maximum number of offences
{
    int old = Limit;

    Limit = new;
    return old;
}

// ----------------------------- MEMORY MANAGEMENT -----------------------------
static BAD_RT_TASK *new_bad_task(void)
{
#ifdef MY_ALLOC
    int		bt;

    spin_lock(&alloc_lock);
    for (bt = 0; bt < BAD_TASK_MAX; bt++) {
	if (!(bad_task_pool[bt].in_use)) {
	    bad_task_pool[bt].in_use = 1;
	    spin_unlock(&alloc_lock);
	    return &bad_task_pool[bt];
	}
    }
    spin_unlock(&alloc_lock);
    return NULL;
#else
    return rt_malloc(sizeof(BAD_RT_TASK));
#endif
}

static void free_bad_task(BAD_RT_TASK *bt)
{
#ifdef MY_ALLOC
    bt->in_use = 0;
#else
    rt_free(bt);
#endif
}

// -------------------------- LINKED LIST FUNCTIONS ----------------------------
static void append_bad_task(BAD_RT_TASK **list, BAD_RT_TASK *new)
{
    BAD_RT_TASK	*end = *list;

    if (!end) {
	*list = new;
    } else {
	while (end->next) end = end->next;
	end->next = new;
    }
}

static BAD_RT_TASK *delete_bad_task(BAD_RT_TASK **list, BAD_RT_TASK *del)
{
    BAD_RT_TASK	*rtn, *bt = *list;

    if (bt == del) {
	rtn = *list = NULL;
    } else {
	while (bt->next != del) bt = bt->next;
	rtn = bt->next = del->next;
    }
    free_bad_task(del);
    return rtn;		// Next in list
}

static BAD_RT_TASK *find_bad_task(BAD_RT_TASK *list, RT_TASK *t)
{
    BAD_RT_TASK *bt = list;

    while (bt) {
	if (bt->task == t) break;
	bt = bt->next;
    }
    return bt;
}

// ------------------------- WHICH CPU IS A TASK ON? ---------------------------
static inline int which_cpu(RT_TASK *t)
{
	return t->runnable_on_cpus;
}

// ------------------------ MP PROOF SUSPEND AND DELETE ------------------------
static void smpproof_task_suspend(RT_TASK *t)
{
    int cpuid;

    rt_task_suspend(t);
    if ((cpuid = which_cpu(t)) != rtai_cpuid()) {	// Not really suspended
	DBUG("Resuming dummy watchdog %d\n", cpuid);
	rt_task_resume(&wdog[cpuid]);			// ...until we do this!
    }
}

static void smpproof_task_delete(RT_TASK *t)
{
    int cpuid;

    rt_task_delete(t);
    if ((cpuid = which_cpu(t)) != rtai_cpuid()) {	// Not really suspended
	DBUG("Resuming dummy watchdog %d\n", cpuid);
	rt_task_resume(&wdog[cpuid]);			// ...until we do this!
    }
}

// ----------------------------- POLICY FUNCTIONS ------------------------------
static void stretch_badtask(RT_TASK *t, BAD_RT_TASK *bt, int cpuid)
{
    // Stretch the task's period and ask scheduler to resync frame time
    t->period      += rtai_llimd(bt->orig_period, Stretch, 100);
    t->resync_frame = 1;
    DBUG( "...by %d%% to %uns\n", 
	  Stretch, (int)count2nano_cpuid(t->period, cpuid));
}

static void start_slipping_badtask(RT_TASK *t, BAD_RT_TASK *bt, int cpuid)
{
    // Mark task as slipping and work out how many watchdog ticks to suspend it
    bt->slipping  = 1;
    bt->countdown = rtai_llimd(rtai_llimd(count2nano_cpuid(t->period, cpuid), Slip, 100), 
	    		   1, 
			   TickPeriod);
    DBUG( "Suspending task 0x%X for %d ticks (slip %d)\n", 
	  t, bt->countdown, bt->count);

    // Suspend task - it will get resumed later
    smpproof_task_suspend(t);
}

static void check_slipping_badtask(BAD_RT_TASK *bt)
{
    // Resume task if it's been suspended long enough
    if (--(bt->countdown) <= 0) {
	bt->slipping = 0;
	rt_task_resume(bt->task);
	DBUG("Finished slip %d of task 0x%X, resuming\n", bt->count, bt->task);
    }
}

// ------------------------- FUNCTION TO DECIDE POLICY -------------------------
static void handle_badtask(int wd, RT_TASK *t, BAD_RT_TASK *bt, RTIME overrun)
{
    // Start 'criminal record' for first time offenders
    if (!bt) {
	bt = new_bad_task();
	if (!bt) return;
	bt->task        = t;
	bt->next        = NULL;
	bt->slipping    = 0;
	bt->count       = 0;
	bt->countdown   = 0;
	bt->valid       = 1;
	bt->forced      = 0;
	bt->orig_period = t->period;
	append_bad_task(&bad_tl[wd], bt);
    }

    // Increment offence count and note current policy
    (bt->count)++;
    bt->policy = Policy;

    // Pure loopers must be suspend always
    if (!overrun) {
	bt->count = - LooperTimeLimit;
	bt->forced = 1;
        bt->policy = WD_SUSPEND;
	bt->orig_period = 0;
	WDLOG("Suspending task %p\n", t);
	smpproof_task_suspend(t);  
	return;
    }

    // In severe cases we must suspend regardless of current policy
    if ((overrun >= (Safety * bt->orig_period)) && (Safety >= 0)) {
	WDLOG("Forcing suspension of severely overrun task %p\n", t);
	bt->forced = 1;
	smpproof_task_suspend(t);  
	return;
    }

    // Has it pushed its luck too far?
    if ((bt->count >= Limit) && (Limit >= 0)) {
	WDLOG("Task %p reached offence limit, suspending\n", t);
	bt->forced = 1;
	smpproof_task_suspend(t);
	return;
    }

    // What to do depends on current policy
    switch (Policy) {

	case WD_NOTHING: 	// Hope for the best
       	    break;

	case WD_RESYNC:	// Resynchronise frame time
	case WD_DEBUG:	// Same thing for debugging
	    WDLOG("Resynchronising task %p\n", t);
	    t->resync_frame = 1;
	    break;

	case WD_STRETCH: 	// Stretch the task's period
	    WDLOG("Stretching period of task %p\n", t);
	    stretch_badtask(t, bt, wd);
	    break;

	case WD_SLIP:    	// Suspend but arrange to resume later
	    WDLOG("Slipping task %p\n", t);
	    start_slipping_badtask(t, bt, wd);
	    break;

	case WD_SUSPEND: 	// Suspend the task
	    WDLOG("Suspending task %p\n", t);
	    smpproof_task_suspend(t);  
	    break;

	case WD_KILL:    	// Delete the task
	    WDLOG("Killing task %p\n", t);
	    smpproof_task_delete(t);   
	    break;

	default:      	// Invalid
	    WDLOG("Invalid policy (%d)\n", Policy);
	    break;
    }
}

static void watch_looper(int cpuid, void *self, BAD_RT_TASK *bt)
{
	extern RT_TASK rt_smp_linux_task[];
	extern RT_TASK *lxrt_prev_task[];
	static RT_TASK *prev_task[RTAI_NR_CPUS];     // task we preempted
	static int     prev_task_cnt[RTAI_NR_CPUS];  // count the same preempted
	RT_TASK *prev = lxrt_prev_task[cpuid];
	if (prev == prev_task[cpuid] && prev != &rt_smp_linux_task[cpuid] && prev != self && !prev->resync_frame && !prev->period) {
		if (++prev_task_cnt[cpuid] == LooperLimit) {
			WDLOG("Found looper task %p (list %d)\n", prev, cpuid);
			handle_badtask(cpuid, prev, bt, 0);
		}
	} else {
		prev_task[cpuid] = prev;
		prev_task_cnt[cpuid] = 0;
	}
}

// -------------------------- THE MAIN WATCHDOG TASK ---------------------------
static void watchdog(long wd)
{
#ifdef WDBUG
    int 	 led    = 0;
    static int 	 output = 0x0000;
#endif
    RT_TASK 	*task, *self = rt_whoami();
    BAD_RT_TASK *bt;
    RTIME 	 now, overrun;
    int		 another, dog;

    while (1) {
#ifdef WDBUG
	// LED heartbeat visible on parallel port
	led = !led;
	if (led) output |=  (1 << wd); 
	else     output &= ~(1 << wd);
	outb(output, LPT_PORT);
#endif
	// Fix any overrun of our own (step and trace debug)
	now = rt_get_time_cpuid(wd);
	if (now - self->resume_time >= self->period) {
	    self->resync_frame = 1;
	    rt_task_wait_period();
	    DBUG("Resynchronised watchdog %d\n", wd);
	    continue;
	}

	// Mark all this watchdog's bad tasks as invalid ready for check
	for (bt = bad_tl[wd]; bt; bt->valid = 0, bt = bt->next);

	// Loop through all the RT tasks in this watchdog's list
	task = tlists[wd];
	while ((task = task->next)) {

	    // Ignore ourself but note another watchdog
	    if (task == self) continue;
	    for (another = dog = 0; dog < num_wdogs; dog++) {
		if (task == &wdog[dog]) {
		    another = 1 + dog;
		    break;
		}
	    }

	    // Search for any criminal record and check slipping tasks
	    if ((bt = find_bad_task(bad_tl[wd], task))) {
		bt->valid = 1;
		if (bt->slipping) {
		    check_slipping_badtask(bt);
		    continue;
		}
	    }

	    // Ignore non-periodic, resyncing, suspended or blocked tasks
	    if (!task->period || task->resync_frame || task->state & 
		    (RT_SCHED_SUSPENDED|RT_SCHED_DELAYED|RT_SCHED_SEMAPHORE|RT_SCHED_SEND|RT_SCHED_RECEIVE|RT_SCHED_RPC|RT_SCHED_RETURN)) {
		continue;
	    }

	    // Check for overrun and decide what to do (ignore other watchdogs)
	    overrun = now - task->resume_time;
	    if (overrun >= rtai_llimd(task->period, Grace, GraceDiv)) {
		if (another--) {
		    WDLOG("WARNING: Watchdog %d is overrunning\n", another);
		} else {
		    WDLOG("Found overrunning task %p (list %d)\n", task, wd);
		    handle_badtask(wd, task, bt, overrun);
		}
	    }
	}

	watch_looper(wd, self, bt);

	// Clean up any bad tasks still marked invalid (their RT task has gone)
	for (bt = bad_tl[wd]; bt;) {
	    if (!(bt->valid)) {
		bt = delete_bad_task(&bad_tl[wd], bt);
	    } else {
		bt = bt->next;
	    }
	}

	// Wait for next watchdog 'tick'
	rt_task_wait_period();
    }
}

// -------------------------- DUMMY WATCHDOG TASK ------------------------------
static void dummy(long wd)
{
    // Go straight back to sleep - old legacy SMP proof suspend and delete
    while (1) {
	rt_task_suspend(&wdog[wd]);
    }
}

// ----------------------------- PROC INTERFACE --------------------------------
#ifdef CONFIG_PROC_FS
static int PROC_READ_FUN(wdog_read_proc)
{
    RT_TASK	*task;
    BAD_RT_TASK	*bt;
    long 	 onsec, osec;
    long 	 ansec, asec;
    int		 cpuid, tl, id = 1;
    char	 action[10];
    PROC_PRINT_VARS;

    // Heading and parameters
    PROC_PRINT("\nRTAI Watchdog Status\n");
    PROC_PRINT(  "--------------------\n");
    PROC_PRINT("%d Watchdog task%s running at %d Hz in oneshot mode\n", 
	       num_wdogs, num_wdogs > 1 ? "s" : "",
	       (int)rtai_imuldiv(NSECS_PER_SEC, 1, TickPeriod));
#ifdef MY_ALLOC
    PROC_PRINT("Using static memory management (%d entries)\n", BAD_TASK_MAX);
#else
    PROC_PRINT("Using dynamic memory management\n");
#endif
    PROC_PRINT("Policy         : '%s'\n", policy_name[Policy]);
    PROC_PRINT("Grace periods  : %d%s\n", Grace, 
	       (Policy <= WD_STRETCH) ? " (forced)" : "");
    PROC_PRINT("Grace divisor  : %d%s\n", GraceDiv, 
	       (Policy <= WD_STRETCH) ? " (forced)" : "");
    PROC_PRINT("Safety limit   : ");
    if (Safety < 0) {
	PROC_PRINT("(disabled)\n");
    } else {
	PROC_PRINT("%d period%s\n", Safety, Safety > 1 ? "s" : "");
    }
    PROC_PRINT("Slip factor    : %d%%\n", Slip);
    PROC_PRINT("Stretch factor : %d%%\n", Stretch);
    PROC_PRINT("Offense limit  : ");
    if (Limit < 0) {
	PROC_PRINT("(disabled)\n");
    } else {
	PROC_PRINT("%d\n", Limit);
    }
    PROC_PRINT("Loopers limit  : %d ms\n", LooperTimeLimit);

    // List criminal records
    PROC_PRINT("\nBad tasks...\n\n");
    PROC_PRINT("RT Task    ID "
	       "CPU%s "
	       "Priority State Count "
	       "Original period Adjusted period "
	       "Action\n",
	       "");
    PROC_PRINT("---------- -- "
	       "---%s "
	       "-------- ----- ----- "
	       "--------------- --------------- "
	       "---------\n",
	       "");
    for (tl = 0; tl < num_wdogs; tl++) {
	task = tlists[tl];
	while ((task = task->next)) {
	    if ((bt = find_bad_task(bad_tl[tl], task))) {
		if (bt->forced) {
		    sprintf(action, "%s *", policy_name[WD_SUSPEND]);
		} else {
		    strcpy(action, policy_name[bt->policy]);
		}
		cpuid = task->runnable_on_cpus;
		osec  = rtai_ulldiv( count2nano_cpuid(bt->orig_period, cpuid),
			        NSECS_PER_SEC, 
			        &onsec);
		asec  = rtai_ulldiv( count2nano_cpuid(task->period, cpuid),
			        NSECS_PER_SEC, 
			        &ansec);
		PROC_PRINT( "0x%08lx %-2d "
			    "%s%-2d%s "
			    "%-8d 0x%-3x %-5d "
			    "%02ds %09dns %02ds %09dns "
			    "%s\n",
			    (long)task, id, 
			    "",
			    (int)task->runnable_on_cpus,
			    " ",
			    task->priority, task->state, bt->count,
			    (int)osec, (int)onsec, (int)asec, (int)ansec, 
			    action);
	    }
	    id++;
	}
    }
    PROC_PRINT_DONE;
}
#endif

// ------------------------ WATCHDOG ENTRIES TABLE -----------------------------

#include <rtai_lxrt.h>

static struct rt_fun_entry rt_watchdog_fun[]  __attribute__ ((__unused__));

static struct rt_fun_entry rt_watchdog_fun[] = {
        [WD_SET_GRACE]	  = { 0, rt_wdset_grace },
        [WD_SET_GRACEDIV] = { 0, rt_wdset_gracediv },
        [WD_SET_SAFETY]   = { 0, rt_wdset_safety },
        [WD_SET_POLICY]   = { 0, rt_wdset_policy },
        [WD_SET_SLIP]     = { 0, rt_wdset_slip },
        [WD_SET_STRETCH]  = { 0, rt_wdset_stretch },
        [WD_SET_LIMIT]    = { 0, rt_wdset_limit }
};

PROC_READ_OPEN_OPS(rtai_wdog_fops, wdog_read_proc);

// ----------------------------- MODULE CONTROL --------------------------------
int __rtai_wd_init(void)
{
    RTIME 	 period;
    int		 dog;
    RT_TASK	*lnx0;
    char	*c;

    if(set_rt_fun_ext_index(rt_watchdog_fun, WD_INDX)) {
	printk("Recompile your module with a different index\n");
	return -EACCES;
    }
    // Some parameters have to be forced
    if (Policy <= WD_STRETCH) Grace  = GraceDiv = 1;
    if (Policy == WD_DEBUG)   Safety = Limit = -1;
    LooperLimit = rtai_llimd(LooperTimeLimit, 1000000, TickPeriod);

    // Deduce number of watchdogs needed from scheduler type
    num_wdogs = num_online_cpus();

    // Fill array of pointers to scheduler's task lists
    lnx0 = rt_get_base_linux_task(tlists);

    // Register watchdogs with scheduler
    for (dog = 0; dog < num_online_cpus(); dog++) {
	if (rt_register_watchdog(&wdog[dog], dog) < 0) {
	    WDLOG("Failed to register watchdog %d with RTAI scheduler\n", dog);
	    for (dog--; dog >= 0; dog--) rt_deregister_watchdog(&wdog[dog], dog);
	    return -EBUSY;
	}
    }

    // Set up chosen timer mode and hard timing
    if (wd_OneShot) {
	start_rt_timer(0);
    } else {
	rt_set_periodic_mode();
	start_rt_timer((int)nano2count(TickPeriod));
    }

    // Set up and start watchdog tasks (on separate CPUs if MP). We run as 
    // many real watchdogs as there are CPUs.
    for (dog = 0; dog < num_online_cpus(); dog++) {
	rt_task_init_cpuid(&wdog[dog], 
			   (dog < num_wdogs) ? watchdog : dummy, 
			    dog, 2000, RT_SCHED_HIGHEST_PRIORITY, 0, 0, dog);
    }
    for (dog = 0; dog < num_wdogs; dog++) {
	period = nano2count_cpuid(TickPeriod, dog);
	rt_task_make_periodic(&wdog[dog], 
			       rt_get_time_cpuid(dog) + period, 
			       period);
    }

    // Tidy up version number
    if ((c = strchr(version, ' '))) {
	*(strchr(c, '$')) = '\0';
	strcpy(ver, c + 1);
    } else {
	strcpy(ver, "? ");
    }

    // Log initial parameters
    WDLOG( "loaded.\n");
    WDLOG( "%d Watchdog task%s running at %d Hz in oneshot mode\n", 
	   num_wdogs, num_wdogs > 1 ? "s" : "",
	   rtai_imuldiv(NSECS_PER_SEC, 1, TickPeriod));
#ifdef MY_ALLOC
    WDLOG( "Using static memory management (%d entries)\n", BAD_TASK_MAX);
#else
    WDLOG( "Using dynamic memory management\n");
#endif
    WDLOG( "Policy         : '%s'\n", policy_name[Policy]);
    WDLOG( "Grace periods  : %d%s\n", Grace, 
	   (Policy <= WD_STRETCH) ? " (forced)" : "");
    WDLOG( "Grace divisor  : %d%s\n", GraceDiv, 
	   (Policy <= WD_STRETCH) ? " (forced)" : "");
    WDLOG( "Safety limit   : ");
    if (Safety < 0) {
	rt_printk("(disabled)\n");
    } else {
	rt_printk("%d period%s\n", Safety, Safety > 1 ? "s" : " ");
    }
    WDLOG( "Slip factor    : %d%%\n", Slip);
    WDLOG( "Stretch factor : %d%%\n", Stretch);
    WDLOG( "Offense limit  : ");
    if (Limit < 0) {
	rt_printk("(disabled)\n");
    } else {
	rt_printk("%d\n", Limit);
    }
    WDLOG( "Loopers limit  : %d ms\n", LooperTimeLimit);

#ifdef CONFIG_PROC_FS
    // Register /proc interface
    wd_proc = CREATE_PROC_ENTRY("watchdog", 0, rtai_proc_root, &rtai_wdog_fops);
    SET_PROC_READ_ENTRY(wd_proc, wdog_read_proc);
#endif
    return 0;
}

void __rtai_wd_exit(void)
{
    BAD_RT_TASK	*bt;
    int		 dog;

#ifdef CONFIG_PROC_FS
    // Remove /proc interface
    remove_proc_entry("watchdog", rtai_proc_root);
#endif
    // Deregister all watchdogs and shutdown the timer
    for (dog = 0; dog < num_online_cpus(); dog++) {
	rt_deregister_watchdog(&wdog[dog], dog);
    }
    stop_rt_timer();
    rt_busy_sleep(TickPeriod);

    // Cleanup and remove all watchdogs and bad task lists
    for (dog = 0; dog < num_online_cpus(); dog++) {
	rt_task_delete(&wdog[dog]);
	if (dog < num_wdogs) {
	    for (bt = bad_tl[dog]; bt;) {
		bt = delete_bad_task(&bad_tl[dog], bt);
	    }
	}
    }

    reset_rt_fun_ext_index(rt_watchdog_fun, WD_INDX);

    // It's all over :(
    WDLOG("unloaded.\n");
}

module_init(__rtai_wd_init);
module_exit(__rtai_wd_exit);

EXPORT_SYMBOL(rt_wdset_grace);
EXPORT_SYMBOL(rt_wdset_gracediv);
EXPORT_SYMBOL(rt_wdset_safety);
EXPORT_SYMBOL(rt_wdset_policy);
EXPORT_SYMBOL(rt_wdset_slip);
EXPORT_SYMBOL(rt_wdset_stretch);
EXPORT_SYMBOL(rt_wdset_limit);
