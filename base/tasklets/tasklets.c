/**
 * @ingroup tasklets
 * @file
 *
 * Implementation of the @ref tasklets "RTAI tasklets module".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2006 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @defgroup tasklets module
 *
 * The tasklets module adds an interesting feature along the line, pioneered
 * by RTAI, of a symmetric usage of all its services inter-intra kernel and
 * user space, both for soft and hard real time applications.   In such a way
 * you have opened a whole spectrum of development and implementation
 * lanes, allowing maximum flexibility with uncompromized performances.
 *
 * The new services provided can be useful when you have many tasks, both
 * in kernel and user space, that must execute simple, often ripetitive,
 * functions, both in soft and hard real time, asynchronously within their
 * parent application. Such tasks are here called tasklets and can be of
 * two kinds: normal tasklets and timed tasklets (timers).
 *
 * It must be noted that only timers should need to be made available both in
 * user and kernel space.   In fact normal tasklets in kernel space are nothing
 * but standard functions that can be directly executed by calling them, so
 * there would be no need for any special treatment.   However to maintain full
 * usage symmetry and to ease any possible porting from one address space to
 * the other, plain tasklets can be used in the same way from whatever address
 * space.
 *
 * Tasklets should be used where and whenever the standard hard real time
 * RTAI tasks are used.  Instances of such applications are timed polling and
 * simple Programmable Logic Controllers (PLC) like sequences of services.
 * Obviously there are many others instances that can make it sufficient the
 * use of tasklets, either normal or timers.   In general such an approach can
 * be a very useful complement to fully featured tasks in controlling complex
 * machines and systems, both for basic and support services.
 *
 * It is remarked that the implementation found here for timed tasklets rely on
 * server support tasks, one per cpu, that execute the related timer functions,
 * either in oneshot or periodic mode, on the base of their time deadline and
 * according to their, user assigned, priority. Instead, as told above, plain
 * tasklets are just functions executed from kernel space; their execution
 * needs no server and is simply triggered by calling a given service function
 * at due time, either from a kernel task or interrupt handler requiring, or in
 * charge of, their execution whenever they are needed.
 *
 * Note that in user space you run within the memory of the process owning the
 * tasklet function so you MUST lock all of your tasks memory in core, by
 * using mlockall, to prevent it being swapped out.   Pre grow also your stack
 * to the largest size needed during the execution of your application, see
 * mlockall usage in Linux mans.
 *
 * The RTAI distribution contains many useful examples that demonstrate the use
 * of most tasklets services, both in kernel and user space.
 *
 *@{*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/rtai_sched.h>
#include <rtai_tasklets.h>
#include <rtai_lxrt.h>
#include <rtai_malloc.h>
#include <rtai_schedcore.h>

MODULE_LICENSE("GPL");

extern struct epoch_struct boot_epoch;

DEFINE_LINUX_CR0

#ifdef CONFIG_SMP
#define NUM_CPUS	   RTAI_NR_CPUS
#define TIMED_TIMER_CPUID  (timed_timer->cpuid)
#define TIMER_CPUID	(timer->cpuid)
#define LIST_CPUID	 (cpuid)
#else
#define NUM_CPUS	   1
#define TIMED_TIMER_CPUID  (0)
#define TIMER_CPUID	(0)
#define LIST_CPUID	 (0)
#endif


static struct rt_tasklet_struct timers_list[NUM_CPUS] =
{ { &timers_list[0], &timers_list[0], RT_SCHED_LOWEST_PRIORITY, 0, 0, RT_TIME_END, 0LL, NULL, 0UL, 0UL, 0, NULL, NULL, 0,
#ifdef  CONFIG_RTAI_LONG_TIMED_LIST
{ NULL }
#endif
}, };

static struct rt_tasklet_struct tasklets_list =
{ &tasklets_list, &tasklets_list, };

// static spinlock_t timers_lock[NUM_CPUS] = { SPIN_LOCK_UNLOCKED, };
static spinlock_t timers_lock[NUM_CPUS] = { __SPIN_LOCK_UNLOCKED(timers_lock[0]), };
static DEFINE_SPINLOCK(tasklets_lock);

static struct rt_fun_entry rt_tasklet_fun[]  __attribute__ ((__unused__));

static struct rt_fun_entry rt_tasklet_fun[] = {
	{ 0, rt_init_tasklet },    		//   0
	{ 0, rt_delete_tasklet },    		//   1
	{ 0, rt_insert_tasklet },    		//   2
	{ 0, rt_remove_tasklet },    		//   3
	{ 0, rt_tasklet_use_fpu },   		//   4
	{ 0, rt_insert_timer },    		//   5
	{ 0, rt_remove_timer },    		//   6
	{ 0, rt_set_timer_priority },  		//   7
	{ 0, rt_set_timer_firing_time },   	//   8
	{ 0, rt_set_timer_period },   		//   9
	{ 0, rt_set_tasklet_handler },  	//  10
	{ 0, rt_set_tasklet_data },   		//  11
	{ 0, rt_exec_tasklet },   		//  12
	{ 0, rt_wait_tasklet_is_hard },	   	//  13
	{ 0, rt_set_tasklet_priority },  	//  14
	{ 0, rt_register_task },	  	//  15
	{ 0, rt_get_timer_times },		//  16
	{ 0, rt_get_timer_overrun },		//  17

/* Posix timers support */

	{ 0, rt_ptimer_create },		//  18
	{ 0, rt_ptimer_settime },		//  19
	{ 0, rt_ptimer_overrun },		//  20
	{ 0, rt_ptimer_gettime },		//  21
	{ 0, rt_ptimer_delete }			//  22

/* End Posix timers support */

};

#ifdef CONFIG_RTAI_LONG_TIMED_LIST

/* BINARY TREE */
static inline void enq_timer(struct rt_tasklet_struct *timed_timer)
{
	struct rt_tasklet_struct *timerh, *tmrnxt, *timer;
	rb_node_t **rbtn, *rbtpn = NULL;
	timer = timerh = &timers_list[TIMED_TIMER_CPUID];
	rbtn = &timerh->rbr.rb_node;

	while (*rbtn) {
		rbtpn = *rbtn;
		tmrnxt = rb_entry(rbtpn, struct rt_tasklet_struct, rbn);
		if (timer->firing_time > tmrnxt->firing_time) {
			rbtn = &(rbtpn)->rb_right;
		} else {
			rbtn = &(rbtpn)->rb_left;
			timer = tmrnxt;
		}
	}
	rb_link_node(&timed_timer->rbn, rbtpn, rbtn);
	rb_insert_color(&timed_timer->rbn, &timerh->rbr);
	timer->prev = (timed_timer->prev = timer->prev)->next = timed_timer;
	timed_timer->next = timer;
}

#define rb_erase_timer(timer) \
rb_erase(&(timer)->rbn, &timers_list[NUM_CPUS > 1 ? (timer)->cpuid : 0].rbr)

#else /* !CONFIG_RTAI_LONG_TIMED_LIST */

/* LINEAR */
static inline void enq_timer(struct rt_tasklet_struct *timed_timer)
{
	struct rt_tasklet_struct *timer;
	timer = &timers_list[TIMED_TIMER_CPUID];
	while (timed_timer->firing_time > (timer = timer->next)->firing_time);
	timer->prev = (timed_timer->prev = timer->prev)->next = timed_timer;
	timed_timer->next = timer;
}

#define rb_erase_timer(timer)

#endif /* CONFIG_RTAI_LONG_TIMED_LIST */

static inline void rem_timer(struct rt_tasklet_struct *timer)
{
	(timer->next)->prev = timer->prev;
	(timer->prev)->next = timer->next;
	timer->next = timer->prev = timer;
	rb_erase_timer(timer);
}

/**
 * Insert a tasklet in the list of tasklets to be processed.
 *
 * rt_insert_tasklet insert a tasklet in the list of tasklets to be processed.
 *
 * @param tasklet is the pointer to the tasklet structure to be used to manage
 * the tasklet at hand.
 *
 * @param handler is the tasklet function to be executed.
 *
 * @param data is an unsigned long to be passed to the handler.   Clearly by an
 * appropriate type casting one can pass a pointer to whatever data structure
 * and type is needed.
 *
 * @param id is a unique unsigned number to be used to identify the tasklet
 * tasklet. It is typically required by the kernel space service, interrupt
 * handler ot task, in charge of executing a user space tasklet.   The support
 * functions nam2num() and num2nam() can be used for setting up id from a six
 * character string.
 *
 * @param pid is an integer that marks a tasklet either as being a kernel or
 * user space one. Despite its name you need not to know the pid of the tasklet
 * parent process in user space.   Simple use 0 for kernel space and 1 for user
 * space.
 *
 * @retval 0 on success
 * @return a negative number to indicate that an invalid handler address has
 * been passed.
 *
 */

RTAI_SYSCALL_MODE int rt_insert_tasklet(struct rt_tasklet_struct *tasklet, int priority, void (*handler)(unsigned long), unsigned long data, unsigned long id, int pid)
{
	unsigned long flags;

// tasklet initialization
	if (!handler || !id) {
		return -EINVAL;
	}
	tasklet->uses_fpu = 0;
	tasklet->priority = priority;
	tasklet->handler  = handler;
	tasklet->data     = data;
	tasklet->id       = id;
	if (!pid) {
		tasklet->task = 0;
	} else {
		(tasklet->task)->priority = priority;
		rt_copy_to_user(tasklet->usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
	}
// tasklet insertion tasklets_list
	flags = rt_spin_lock_irqsave(&tasklets_lock);
	tasklet->next	     = &tasklets_list;
	tasklet->prev	     = tasklets_list.prev;
	(tasklets_list.prev)->next = tasklet;
	tasklets_list.prev	 = tasklet;
	rt_spin_unlock_irqrestore(flags, &tasklets_lock);
	return 0;
}

/**
 * Remove a tasklet in the list of tasklets to be processed.
 *
 * rt_remove_tasklet remove a tasklet from the list of tasklets to be processed.
 *
 * @param tasklet is the pointer to the tasklet structure to be used to manage
 * the tasklet at hand.
 *
 */

RTAI_SYSCALL_MODE void rt_remove_tasklet(struct rt_tasklet_struct *tasklet)
{
	if (tasklet->next && tasklet->prev && tasklet->next != tasklet && tasklet->prev != tasklet) {
		unsigned long flags;
		flags = rt_spin_lock_irqsave(&tasklets_lock);
		(tasklet->next)->prev = tasklet->prev;
		(tasklet->prev)->next = tasklet->next;
		tasklet->next = tasklet->prev = tasklet;
		rt_spin_unlock_irqrestore(flags, &tasklets_lock);
	}
}

/**
 * Find a tasklet identified by its id.
 *
 * @param id is the unique unsigned long to be used to identify the tasklet.
 *
 * The support functions nam2num() and num2nam() can be used for setting up id
 * from a six character string.
 *
 * @return the pointer to a tasklet handler on success
 * @retval 0 to indicate that @a id is not a valid identifier so that the
 * related tasklet was not found.
 *
 */

struct rt_tasklet_struct *rt_find_tasklet_by_id(unsigned long id)
{
	struct rt_tasklet_struct *tasklet;

	tasklet = &tasklets_list;
	while ((tasklet = tasklet->next) != &tasklets_list) {
		if (id == tasklet->id) {
			return tasklet;
		}
	}
	return 0;
}

/**
 * Exec a tasklet.
 *
 * rt_exec_tasklet execute a tasklet from the list of tasklets to be processed.
 *
 * @param tasklet is the pointer to the tasklet structure to be used to manage
 * the tasklet @a tasklet.
 *
 * Kernel space tasklets addresses are usually available directly and can be
 * easily be used in calling rt_tasklet_exec.   In fact one can call the related
 * handler directly without using such a support  function, which is mainly
 * supplied for symmetry and to ease the porting of applications from one space
 * to the other.
 *
 * User space tasklets instead must be first found within the tasklet list by
 * calling rt_find_tasklet_by_id() to get the tasklet address to be used
 * in rt_tasklet_exec().
 *
 */

RTAI_SYSCALL_MODE int rt_exec_tasklet(struct rt_tasklet_struct *tasklet)
{
	if (tasklet && tasklet->next != tasklet && tasklet->prev != tasklet) {
		if (!tasklet->task) {
			tasklet->handler(tasklet->data);
		} else {
			rt_task_resume(tasklet->task);
		}
		return 0;
	}
	return -EINVAL;
}

RTAI_SYSCALL_MODE void rt_set_tasklet_priority(struct rt_tasklet_struct *tasklet, int priority)
{
	tasklet->priority = priority;
	if (tasklet->task) {
		(tasklet->task)->priority = priority;
	}
}

RTAI_SYSCALL_MODE int rt_set_tasklet_handler(struct rt_tasklet_struct *tasklet, void (*handler)(unsigned long))
{
	if (!handler) {
		return -EINVAL;
	}
	tasklet->handler = handler;
	if (tasklet->task) {
		rt_copy_to_user(tasklet->usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
	}
	return 0;
}

RTAI_SYSCALL_MODE void rt_set_tasklet_data(struct rt_tasklet_struct *tasklet, unsigned long data)
{
	tasklet->data = data;
	if (tasklet->task) {
		rt_copy_to_user(tasklet->usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
	}
}

RTAI_SYSCALL_MODE RT_TASK *rt_tasklet_use_fpu(struct rt_tasklet_struct *tasklet, int use_fpu)
{
	tasklet->uses_fpu = use_fpu ? 1 : 0;
	return tasklet->task;
}

static RT_TASK timers_manager[NUM_CPUS];

static inline void asgn_min_prio(int cpuid)
{
// find minimum priority in timers_struct
	RT_TASK *timer_manager;
	struct rt_tasklet_struct *timer, *timerl;
	spinlock_t *lock;
	unsigned long flags;
	int priority;

	priority = (timer = (timerl = &timers_list[LIST_CPUID])->next)->priority;
	flags = rt_spin_lock_irqsave(lock = &timers_lock[LIST_CPUID]);
	while ((timer = timer->next) != timerl) {
		if (timer->priority < priority) {
			priority = timer->priority;
		}
		rt_spin_unlock_irqrestore(flags, lock);
		flags = rt_spin_lock_irqsave(lock);
	}
	rt_spin_unlock_irqrestore(flags, lock);
	flags = rt_global_save_flags_and_cli();
	if ((timer_manager = &timers_manager[LIST_CPUID])->priority > priority) {
		timer_manager->priority = priority;
		if (timer_manager->state == RT_SCHED_READY) {
			rem_ready_task(timer_manager);
			enq_ready_task(timer_manager);
		}
	}
	rt_global_restore_flags(flags);
}

static inline void set_timer_firing_time(struct rt_tasklet_struct *timer, RTIME firing_time)
{
	if (timer->next != timer && timer->prev != timer) {
		spinlock_t *lock;
		unsigned long flags;

		timer->firing_time = firing_time;
		flags = rt_spin_lock_irqsave(lock = &timers_lock[TIMER_CPUID]);
		rem_timer(timer);
		enq_timer(timer);
		rt_spin_unlock_irqrestore(flags, lock);
	}
}

/**
 * Insert a timer in the list of timers to be processed.
 *
 * rt_insert_timer insert a timer in the list of timers to be processed.  Timers
 * can be either periodic or oneshot.   A periodic timer is reloaded at each
 * expiration so that it executes with the assigned periodicity.   A oneshot
 * timer is fired just once and then removed from the timers list. Timers can be
 * reinserted or modified within their handlers functions.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param priority is the priority to be used to execute timers handlers when
 * more than one timer has to be fired at the same time.It can be assigned any
 * value such that: 0 < priority < RT_LOWEST_PRIORITY.
 *
 * @param firing_time is the time of the first timer expiration.
 *
 * @param period is the period of a periodic timer. A periodic timer keeps
 * calling its handler at  firing_time + k*period k = 0, 1.  To define a oneshot
 * timer simply use a null period.
 *
 * @param handler is the timer function to be executed at each timer expiration.
 *
 * @param data is an unsigned long to be passed to the handler.   Clearly by a
 * appropriate type casting one can pass a pointer to whatever data structure
 * and type is needed.
 *
 * @param pid is an integer that marks a timer either as being a kernel or user
 * space one. Despite its name you need not to know the pid of the timer parent
 * process in user space. Simple use 0 for kernel space and 1 for user space.
 *
 * @retval 0 on success
 * @retval EINVAL if @a handler is an invalid handler address
 *
 */

RTAI_SYSCALL_MODE int rt_insert_timer(struct rt_tasklet_struct *timer, int priority, RTIME firing_time, RTIME period, void (*handler)(unsigned long), unsigned long data, int pid)
{
	spinlock_t *lock;
	unsigned long flags, cpuid;
	RT_TASK *timer_manager;

// timer initialization
	timer->uses_fpu    = 0;

	if (pid >= 0) {
		if (!handler) {
			return -EINVAL;
		}
		timer->handler   = handler;
		timer->data 			 = data;
	} else {
		if (timer->handler != NULL || timer->handler == (void *)1) {
			timer->handler = (void *)1;
			timer->data    = data;
		}
	}

	timer->priority    = priority;
	REALTIME2COUNT(firing_time)
	timer->firing_time = firing_time;
	timer->period      = period;

	if (!pid) {
		timer->task = 0;
		timer->cpuid = cpuid = NUM_CPUS > 1 ? rtai_cpuid() : 0;
	} else {
		timer->cpuid = cpuid = NUM_CPUS > 1 ? (timer->task)->runnable_on_cpus : 0;
		(timer->task)->priority = priority;
		rt_copy_to_user(timer->usptasklet, timer, sizeof(struct rt_usp_tasklet_struct));
	}
// timer insertion in timers_list
	flags = rt_spin_lock_irqsave(lock = &timers_lock[LIST_CPUID]);
	enq_timer(timer);
	rt_spin_unlock_irqrestore(flags, lock);
// timers_manager priority inheritance
	if (timer->priority < (timer_manager = &timers_manager[LIST_CPUID])->priority) {
		timer_manager->priority = timer->priority;
	}
// timers_task deadline inheritance
	flags = rt_global_save_flags_and_cli();
	if (timers_list[LIST_CPUID].next == timer && (timer_manager->state & RT_SCHED_DELAYED) && firing_time < timer_manager->resume_time) {
		timer_manager->resume_time = firing_time;
		rem_timed_task(timer_manager);
		enq_timed_task(timer_manager);
		rt_schedule();
	}
	rt_global_restore_flags(flags);
	return 0;
}

/**
 * Remove a timer in the list of timers to be processed.
 *
 * rt_remove_timer remove a timer from the list of the timers to be processed.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 */

RTAI_SYSCALL_MODE void rt_remove_timer(struct rt_tasklet_struct *timer)
{
	if (timer->next && timer->prev && timer->next != timer && timer->prev != timer) {
		spinlock_t *lock;
		unsigned long flags;
		flags = rt_spin_lock_irqsave(lock = &timers_lock[TIMER_CPUID]);
		rem_timer(timer);
		rt_spin_unlock_irqrestore(flags, lock);
		asgn_min_prio(TIMER_CPUID);
	}
}

/**
 * Change the priority of an existing timer.
 *
 * rt_set_timer_priority change the priority of an existing timer.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param priority is the priority to be used to execute timers handlers when
 * more than one timer has to be fired at the same time. It can be assigned any
 * value such that: 0 < priority < RT_LOWEST_PRIORITY.
 *
 * This function can be used within the timer handler.
 *
 */

RTAI_SYSCALL_MODE void rt_set_timer_priority(struct rt_tasklet_struct *timer, int priority)
{
	timer->priority = priority;
	if (timer->task) {
		(timer->task)->priority = priority;
	}
	asgn_min_prio(TIMER_CPUID);
}

/**
 * Change the firing time of a timer.
 *
 * rt_set_timer_firing_time changes the firing time of a periodic timer
 * overloading any existing value, so that the timer next shoot will take place
 * at the new firing time. Note that if a oneshot timer has its firing time
 * changed after it has already expired this function has no effect. You
 * should reinsert it in the timer list with the new firing time.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param firing_time is the new time of the first timer expiration.
 *
 * This function can be used within the timer handler.
 *
 * @retval 0 on success.
 *
 */

RTAI_SYSCALL_MODE void rt_set_timer_firing_time(struct rt_tasklet_struct *timer, RTIME firing_time)
{
	unsigned long flags;
	RT_TASK *timer_manager;

	set_timer_firing_time(timer, firing_time);
	flags = rt_global_save_flags_and_cli();
	if (timers_list[TIMER_CPUID].next == timer && ((timer_manager = &timers_manager[TIMER_CPUID])->state & RT_SCHED_DELAYED) && firing_time < timer_manager->resume_time) {
		timer_manager->resume_time = firing_time;
		rem_timed_task(timer_manager);
		enq_timed_task(timer_manager);
		rt_schedule();
	}
	rt_global_restore_flags(flags);
}

/**
 * Change the period of a timer.
 *
 * rt_set_timer_period changes the period of a periodic timer. Note that the new
 * period will be used to pace the timer only after the expiration of the firing
 * time already in place. Using this function with a period different from zero
 * for a oneshot timer, that has not expired yet, will transform it into a
 * periodic timer.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param period is the new period of a periodic timer.
 *
 * The macro #rt_fast_set_timer_period  can substitute the corresponding
 * function in kernel space if both the existing timer period and the new one
 * fit into an 32 bits integer.
 *
 * This function an be used within the timer handler.
 *
 * @retval 0 on success.
 *
 */

RTAI_SYSCALL_MODE void rt_set_timer_period(struct rt_tasklet_struct *timer, RTIME period)
{
	spinlock_t *lock;
	unsigned long flags;
	flags = rt_spin_lock_irqsave(lock = &timers_lock[TIMER_CPUID]);
	timer->period = period;
	rt_spin_unlock_irqrestore(flags, lock);
}

RTAI_SYSCALL_MODE void rt_get_timer_times(struct rt_tasklet_struct *timer, RTIME timer_times[])
{
	RTIME firing;

	firing = -rt_get_time();
	firing += timer->firing_time;

	timer_times[0] = firing > 0 ? firing : -1;
	timer_times[1] = timer->period;
}

RTAI_SYSCALL_MODE RTIME rt_get_timer_overrun(struct rt_tasklet_struct *timer)
{
	return timer->overrun;
}

static int TimersManagerPrio = 0;
RTAI_MODULE_PARM(TimersManagerPrio, int);

// the timers_manager task function

static void rt_timers_manager(long cpuid)
{
	RTIME now;
	RT_TASK *timer_manager;
	struct rt_tasklet_struct *tmr, *timer, *timerl;
	spinlock_t *lock;
	unsigned long flags, timer_tol;
	int priority, used_fpu;

	timer_manager = &timers_manager[LIST_CPUID];
	timerl = &timers_list[LIST_CPUID];
	lock = &timers_lock[LIST_CPUID];
	timer_tol = tuned.timers_tol[LIST_CPUID];

	while (1) {
		int retval;
		retval = rt_sleep_until((timerl->next)->firing_time);
//		now = timer_manager->resume_time + timer_tol;
		now = rt_get_time() + timer_tol;
// find all the timers to be fired, in priority order
		while (1) {
			used_fpu = 0;
			tmr = timer = timerl;
			priority = RT_SCHED_LOWEST_PRIORITY;
			flags = rt_spin_lock_irqsave(lock);
			while ((tmr = tmr->next)->firing_time <= now) {
				if (tmr->priority < priority) {
					priority = (timer = tmr)->priority;
				}
			}
			rt_spin_unlock_irqrestore(flags, lock);
			if (timer == timerl) {
				if (timer_manager->priority > TimersManagerPrio) {
					timer_manager->priority = TimersManagerPrio;
				}
				break;
			}
			timer_manager->priority = priority;
#if 1
			flags = rt_spin_lock_irqsave(lock);
			rem_timer(timer);
			if (timer->period) {
				timer->firing_time += timer->period;
				enq_timer(timer);
			}
			rt_spin_unlock_irqrestore(flags, lock);
#else
			if (!timer->period) {
				flags = rt_spin_lock_irqsave(lock);
				rem_timer(timer);
				rt_spin_unlock_irqrestore(flags, lock);
			} else {
				set_timer_firing_time(timer, timer->firing_time + timer->period);
			}
#endif
	//	if (retval != RTE_TMROVRN) {
			tmr->overrun = 0;
			if (!timer->task) {
				if (!used_fpu && timer->uses_fpu) {
					used_fpu = 1;
					save_fpcr_and_enable_fpu(linux_cr0);
					save_fpenv(timer_manager->fpu_reg);
				}
				timer->handler(timer->data);
			} else {
				rt_task_resume(timer->task);
			}
	//	} else {
	//		tmr->overrun++;
	//	}
		}
		if (used_fpu) {
			restore_fpenv(timer_manager->fpu_reg);
			restore_fpcr(linux_cr0);
		}
// set next timers_manager priority according to the highest priority timer
		asgn_min_prio(LIST_CPUID);
// if no more timers in timers_struct remove timers_manager from tasks list
	}
}

/**
 * Init, in kernel space, a tasklet structure to be used in user space.
 *
 * rt_tasklet_init allocate a tasklet structure (struct rt_tasklet_struct) in
 * kernel space to be used for the management of a user space tasklet.
 *
 * This function is to be used only for user space tasklets. In kernel space
 * it is just an empty macro, as the user can, and must  allocate the related
 * structure directly, either statically or dynamically.
 *
 * @return the pointer to the tasklet structure the user space application must
 * use to access all its related services.
 */
struct rt_tasklet_struct *rt_init_tasklet(void)
{
	struct rt_tasklet_struct *tasklet;
	if ((tasklet = rt_malloc(sizeof(struct rt_tasklet_struct)))) {
		memset(tasklet, 0, sizeof(struct rt_tasklet_struct));
	}
	return tasklet;
}

RTAI_SYSCALL_MODE void rt_register_task(struct rt_tasklet_struct *tasklet, struct rt_tasklet_struct *usptasklet, RT_TASK *task)
{
	tasklet->task = task;
	tasklet->usptasklet = usptasklet;
	rt_copy_to_user(usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
}

RTAI_SYSCALL_MODE int rt_wait_tasklet_is_hard(struct rt_tasklet_struct *tasklet, long thread)
{
#define POLLS_PER_SEC 100
	int i;
	tasklet->thread = thread;
	for (i = 0; i < POLLS_PER_SEC/5; i++) {
		if (!tasklet->task || !((tasklet->task)->state & RT_SCHED_SUSPENDED)) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(HZ/POLLS_PER_SEC);
		} else {
			return 0;
		}
	}
	return 1;
#undef POLLS_PER_SEC
}

/**
 * Delete, in kernel space, a tasklet structure to be used in user space.
 *
 * rt_tasklet_delete free a tasklet structure (struct rt_tasklet_struct) in
 * kernel space that was allocated by rt_tasklet_init.
 *
 * @param tasklet is the pointer to the tasklet structure (struct
 * rt_tasklet_struct) returned by rt_tasklet_init.
 *
 * This function is to be used only for user space tasklets. In kernel space
 * it is just an empty macro, as the user can, and must  allocate the related
 * structure directly, either statically or dynamically.
 *
 */

RTAI_SYSCALL_MODE int rt_delete_tasklet(struct rt_tasklet_struct *tasklet)
{
	int thread;

	rt_remove_tasklet(tasklet);
	tasklet->handler = 0;
	rt_copy_to_user(tasklet->usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
	rt_task_resume(tasklet->task);
	thread = tasklet->thread;
	rt_free(tasklet);
	return thread;
}

/*
 * Posix Timers support function
 */


static int PosixTimers = POSIX_TIMERS;
RTAI_MODULE_PARM(PosixTimers, int);

static DEFINE_SPINLOCK(ptimer_lock);
static volatile int ptimer_index;
struct ptimer_list { int t_indx, p_idx; struct ptimer_list *p_ptr; struct rt_tasklet_struct *timer;} *posix_timer;

static int init_ptimers(void)
{
	int i;

	if (!(posix_timer = (struct ptimer_list *)kmalloc((PosixTimers)*sizeof(struct ptimer_list), GFP_KERNEL))) {
		printk("Init MODULE no memory for Posix Timer's list.\n");
		return -ENOMEM;
	}
	for (i = 0; i < PosixTimers; i++) {
		posix_timer[i].t_indx = posix_timer[i].p_idx = i;
		posix_timer[i].p_ptr = posix_timer + i;
	}
	return 0;

}

static void cleanup_ptimers(void)
{
	kfree(posix_timer);
}

static inline int get_ptimer_indx(struct rt_tasklet_struct *timer)
{
	unsigned long flags;

	flags = rt_spin_lock_irqsave(&ptimer_lock);
	if (ptimer_index < PosixTimers) {
		struct ptimer_list *p;
		p = posix_timer[ptimer_index++].p_ptr;
		p->timer = timer;
		rt_spin_unlock_irqrestore(flags, &ptimer_lock);
		return p->t_indx;
	}
	rt_spin_unlock_irqrestore(flags, &ptimer_lock);
	return 0;
}

static inline int gvb_ptimer_indx(int itimer)
{
	unsigned long flags;

	flags = rt_spin_lock_irqsave(&ptimer_lock);
	if (itimer < PosixTimers) {
		struct ptimer_list *tmp_p;
		int tmp_place;
		tmp_p = posix_timer[--ptimer_index].p_ptr;
		tmp_place = posix_timer[itimer].p_idx;
		posix_timer[itimer].p_idx = ptimer_index;
		posix_timer[ptimer_index].p_ptr = &posix_timer[itimer];
		tmp_p->p_idx = tmp_place;
		posix_timer[tmp_place].p_ptr = tmp_p;
		rt_spin_unlock_irqrestore(flags, &ptimer_lock);
		return 0;
	}
	rt_spin_unlock_irqrestore(flags, &ptimer_lock);
	return -EINVAL;
}

RTAI_SYSCALL_MODE timer_t rt_ptimer_create(struct rt_tasklet_struct *timer, void (*handler)(unsigned long), unsigned long data, long pid, long thread)
{
	if (thread) {
		rt_wait_tasklet_is_hard(timer, thread);
	}
	timer->next = timer;
	timer->prev = timer;
	timer->data = data;
	timer->handler = handler;
	return get_ptimer_indx(timer);
}
EXPORT_SYMBOL(rt_ptimer_create);

RTAI_SYSCALL_MODE void rt_ptimer_settime(timer_t timer, const struct itimerspec *value, unsigned long data, long flags)
{
	struct rt_tasklet_struct *tasklet;
	RTIME now;

	tasklet = posix_timer[timer].timer;
	rt_remove_timer(tasklet);
	now = rt_get_time();
	if (flags == TIMER_ABSTIME)	{
		if (timespec2count(&(value->it_value)) < now) {
			now -= timespec2count (&(value->it_value));
		}else {
			now = 0;
		}
	}
	if (timespec2count ( &(value->it_value)) > 0) {
		if (data) {
			rt_insert_timer(tasklet, 0, now + timespec2count ( &(value->it_value) ), timespec2count ( &(value->it_interval) ), NULL, data, -1);
		} else {
			rt_insert_timer(tasklet, 0, now + timespec2count ( &(value->it_value) ), timespec2count ( &(value->it_interval) ), tasklet->handler, tasklet->data, 0);
		}
	}
}
EXPORT_SYMBOL(rt_ptimer_settime);

RTAI_SYSCALL_MODE int rt_ptimer_overrun(timer_t timer)
{
	return rt_get_timer_overrun(posix_timer[timer].timer);
}
EXPORT_SYMBOL(rt_ptimer_overrun);

RTAI_SYSCALL_MODE void rt_ptimer_gettime(timer_t timer, RTIME timer_times[])
{
	rt_get_timer_times(posix_timer[timer].timer, timer_times);
}
EXPORT_SYMBOL(rt_ptimer_gettime);

RTAI_SYSCALL_MODE int rt_ptimer_delete(timer_t timer, long space)
{
	struct rt_tasklet_struct *tasklet;
	int rtn = 0;

	tasklet = posix_timer[timer].timer;
	gvb_ptimer_indx(timer);
	rt_remove_tasklet(tasklet);
	if (space) {
		tasklet->handler = 0;
		rt_copy_to_user(tasklet->usptasklet, tasklet, sizeof(struct rt_usp_tasklet_struct));
		rt_task_resume(tasklet->task);
		rtn = tasklet->thread;
	}
	rt_free(tasklet);
	return rtn;
}
EXPORT_SYMBOL(rt_ptimer_delete);

 /*
 * End Posix timers support function
 */

static int TaskletsStacksize = TASKLET_STACK_SIZE;
RTAI_MODULE_PARM(TaskletsStacksize, int);

int __rtai_tasklets_init(void)
{
	int cpuid;

	if(set_rt_fun_ext_index(rt_tasklet_fun, TASKLETS_IDX)) {
		printk("Recompile your module with a different index\n");
		return -EACCES;
	}
	if (init_ptimers()) {
		return -ENOMEM;
	}
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		timers_lock[cpuid] = timers_lock[0];
		timers_list[cpuid] = timers_list[0];
		timers_list[cpuid].cpuid = cpuid;
		timers_list[cpuid].next = timers_list[cpuid].prev = &timers_list[cpuid];
		rt_task_init_cpuid(&timers_manager[cpuid], rt_timers_manager, cpuid, TaskletsStacksize, TimersManagerPrio, 0, 0, cpuid);
		rt_task_resume(&timers_manager[cpuid]);
	}
	printk(KERN_INFO "RTAI[tasklets]: loaded.\n");
	return 0;
}

void __rtai_tasklets_exit(void)
{
	int cpuid;
 	reset_rt_fun_ext_index(rt_tasklet_fun, TASKLETS_IDX);
	cleanup_ptimers();
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		rt_task_delete(&timers_manager[cpuid]);
	}
	printk(KERN_INFO "RTAI[tasklets]: unloaded.\n");
}

/*@}*/

#ifndef CONFIG_RTAI_TASKLETS_BUILTIN
module_init(__rtai_tasklets_init);
module_exit(__rtai_tasklets_exit);
#endif /* !CONFIG_RTAI_TASKLETS_BUILTIN */

#ifdef CONFIG_KBUILD
EXPORT_SYMBOL(rt_insert_tasklet);
EXPORT_SYMBOL(rt_remove_tasklet);
EXPORT_SYMBOL(rt_find_tasklet_by_id);
EXPORT_SYMBOL(rt_exec_tasklet);
EXPORT_SYMBOL(rt_set_tasklet_priority);
EXPORT_SYMBOL(rt_set_tasklet_handler);
EXPORT_SYMBOL(rt_set_tasklet_data);
EXPORT_SYMBOL(rt_tasklet_use_fpu);
EXPORT_SYMBOL(rt_insert_timer);
EXPORT_SYMBOL(rt_remove_timer);
EXPORT_SYMBOL(rt_set_timer_priority);
EXPORT_SYMBOL(rt_set_timer_firing_time);
EXPORT_SYMBOL(rt_set_timer_period);
EXPORT_SYMBOL(rt_init_tasklet);
EXPORT_SYMBOL(rt_register_task);
EXPORT_SYMBOL(rt_wait_tasklet_is_hard);
EXPORT_SYMBOL(rt_delete_tasklet);
EXPORT_SYMBOL(rt_get_timer_times);
EXPORT_SYMBOL(rt_get_timer_overrun);
#endif /* CONFIG_KBUILD */
