/*
 * Copyright (C) 1999-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
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


#ifndef _RTAI_SCHEDCORE_H
#define _RTAI_SCHEDCORE_H

#ifdef __KERNEL__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/timex.h>
#include <linux/sched.h>
#include <asm/param.h>
#include <asm/io.h>
#include <linux/oom.h>
#include <rtai_version.h>
#include <rtai_lxrt.h>
#include <rtai_sched.h>
#include <rtai_malloc.h>
#include <rtai_trace.h>
#include <rtai_sem.h>
#include <rtai_rwl.h>
#include <rtai_spl.h>
#include <rtai_scb.h>
#include <rtai_mbx.h>
#include <rtai_msg.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>


#ifdef OOM_DISABLE
#define RTAI_OOM_DISABLE() \
	do { current->signal->oom_score_adj = OOM_DISABLE; } while (0)
#else
#define RTAI_OOM_DISABLE()
#endif

#define NON_RTAI_TASK_SUSPEND(task) \
	do { (task->lnxtsk)->state = TASK_SOFTREALTIME; } while (0)

#define NON_RTAI_TASK_RESUME(ready_task) \
	do { pend_wake_up_srq(ready_task->lnxtsk, rtai_cpuid()); } while (0)

#define REQUEST_RESUME_SRQs_STUFF() \
do { \
	if (!(wake_up_srq[0].srq = hal_alloc_irq())) { \
		printk("*** ABORT, NO VIRQ AVAILABLE FOR THE WAKING UP SRQ. ***\n"); \
		return -1; \
	} \
	ipipe_request_irq(hal_root_domain, wake_up_srq[0].srq, (void *)wake_up_srq_handler, NULL, NULL); \
} while (0)

#define RELEASE_RESUME_SRQs_STUFF() \
do { \
	ipipe_free_irq(hal_root_domain, wake_up_srq[0].srq); \
	hal_free_irq(wake_up_srq[0].srq); \
} while (0)

extern RT_TASK rt_smp_linux_task[];

extern RT_TASK *rt_smp_current[];

extern RTIME rt_smp_time_h[];

extern volatile int rt_sched_timed;

RT_TASK *rt_get_base_linux_task(RT_TASK **base_linux_task);

RT_TASK *rt_alloc_dynamic_task(void);

void rt_enq_ready_edf_task(RT_TASK *ready_task);

void rt_enq_ready_task(RT_TASK *ready_task);

int rt_renq_ready_task(RT_TASK *ready_task,
		       int priority);

void rt_rem_ready_task(RT_TASK *task);

void rt_rem_ready_current(RT_TASK *rt_current);

void rt_enq_timed_task(RT_TASK *timed_task);

void rt_rem_timed_task(RT_TASK *task);

void rt_dequeue_blocked(RT_TASK *task);

#ifdef CONFIG_RTAI_MALLOC
#ifdef CONFIG_RTAI_MALLOC_BUILTIN
#define sched_mem_init() \
	{ if(__rtai_heap_init() != 0) { \
                return(-ENOMEM); \
        } }
#define sched_mem_end()	 __rtai_heap_exit()
#else  /* CONFIG_RTAI_MALLOC_BUILTIN */
#define sched_mem_init()
#define sched_mem_end()
#endif /* !CONFIG_RTAI_MALLOC_BUILTIN */
#define call_exit_handlers(task)	        __call_exit_handlers(task)
#define set_exit_handler(task, fun, arg1, arg2)	__set_exit_handler(task, fun, arg1, arg2)
#else  /* !CONFIG_RTAI_MALLOC */
#define sched_mem_init()
#define sched_mem_end()
#define call_exit_handlers(task)
#define set_exit_handler(task, fun, arg1, arg2)
#endif /* CONFIG_RTAI_MALLOC */

#define SEMHLF 0x0000FFFF
#define RPCHLF 0xFFFF0000
#define RPCINC 0x00010000

#define DECLARE_RT_CURRENT int cpuid; RT_TASK *rt_current
#define ASSIGN_RT_CURRENT rt_current = rt_smp_current[cpuid = rtai_cpuid()]
#define RT_CURRENT rt_smp_current[rtai_cpuid()]

#define MAX_LINUX_RTPRIO  99
#define MIN_LINUX_RTPRIO   1

#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
void rtai_handle_isched_lock(int nesting);
#endif /* CONFIG_RTAI_SCHED_ISR_LOCK */

#ifdef CONFIG_SMP
#define rt_time_h (rt_smp_time_h[cpuid])
#define rt_linux_task (rt_smp_linux_task[cpuid])
#else
#define rt_time_h (rt_smp_time_h[0])
#define rt_linux_task (rt_smp_linux_task[0])
#endif

/*
 * WATCH OUT for the max expected number of arguments of rtai funs and 
 * their scattered around different calling ways.
 */

#define RTAI_MAX_FUN_ARGS  9
struct fun_args { unsigned long a[RTAI_MAX_FUN_ARGS]; RTAI_SYSCALL_MODE long long (*fun)(unsigned long, ...); };
//used in sys.c
#define RTAI_FUN_ARGS  arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[RTAI_MAX_FUN_ARGS - 1]
//used in sched.c (generalised calls from soft threads)
#define RTAI_FUNARGS   funarg->a[0],funarg->a[1],funarg->a[2],funarg->a[3],funarg->a[4],funarg->a[5],funarg->a[6],funarg->a[7],funarg->a[RTAI_MAX_FUN_ARGS - 1]

#ifdef CONFIG_SMP

static inline void send_sched_ipi(unsigned long dest)
{
	_send_sched_ipi(dest);
}

#define RT_SCHEDULE_MAP(schedmap) \
	do { if (schedmap) send_sched_ipi(schedmap); } while (0)

#define RT_SCHEDULE_MAP_BOTH(schedmap) \
	do { if (schedmap) send_sched_ipi(schedmap); rt_schedule(); } while (0)

#define RT_SCHEDULE(task, cpuid) \
	do { \
		if ((task)->runnable_on_cpus != (cpuid)) { \
			send_sched_ipi(1 << (task)->runnable_on_cpus); \
		} else { \
			rt_schedule(); \
		} \
	} while (0)

#define RT_SCHEDULE_BOTH(task, cpuid) \
	{ \
		if ((task)->runnable_on_cpus != (cpuid)) { \
			send_sched_ipi(1 << (task)->runnable_on_cpus); \
		} \
		rt_schedule(); \
	}

#else /* !CONFIG_SMP */

#define send_sched_ipi(dest)

#define RT_SCHEDULE_MAP_BOTH(schedmap)  rt_schedule()

#define RT_SCHEDULE_MAP(schedmap)       rt_schedule()

#define RT_SCHEDULE(task, cpuid)        rt_schedule()

#define RT_SCHEDULE_BOTH(task, cpuid)   rt_schedule()

#endif /* CONFIG_SMP */

#define BASE_SOFT_PRIORITY 1000000000

#ifndef TASK_NOWAKEUP
#define TASK_NOWAKEUP  TASK_UNINTERRUPTIBLE
#endif

#define TASK_HARDREALTIME  (TASK_INTERRUPTIBLE) // | TASK_NOWAKEUP)
#define TASK_RTAISRVSLEEP  (TASK_INTERRUPTIBLE) // | TASK_NOWAKEUP)
#define TASK_SOFTREALTIME  TASK_INTERRUPTIBLE

static inline void enq_ready_edf_task(RT_TASK *ready_task)
{
	RT_TASK *task;
#ifdef CONFIG_SMP
	task = rt_smp_linux_task[ready_task->runnable_on_cpus].rnext;
#else
	task = rt_smp_linux_task[0].rnext;
#endif
	while (task->policy < 0 && ready_task->period >= task->period) {
		task = task->rnext;
	}
	task->rprev = (ready_task->rprev = task->rprev)->rnext = ready_task;
	ready_task->rnext = task;
}

struct epoch_struct { spinlock_t lock; volatile int touse; volatile RTIME time[2][2]; };

#ifdef CONFIG_RTAI_CLOCK_REALTIME
#define REALTIME2COUNT(rtime) \
	if (rtime > boot_epoch.time[boot_epoch.touse][0]) { \
		rtime -= boot_epoch.time[boot_epoch.touse][0]; \
	}
#else 
#define REALTIME2COUNT(rtime)
#endif

#define MAX_WAKEUP_SRQ (1 << 6)

struct klist_t { int srq; volatile unsigned long in, out; void *task[MAX_WAKEUP_SRQ]; };
extern struct klist_t wake_up_srq[];

#define pend_wake_up_srq(lnxtsk, cpuid) \
do { \
	wake_up_srq[cpuid].task[wake_up_srq[cpuid].in++ & (MAX_WAKEUP_SRQ - 1)] = lnxtsk; \
	hal_pend_uncond(wake_up_srq[0].srq, cpuid); \
} while (0)

static inline void enq_ready_task(RT_TASK *ready_task)
{
	RT_TASK *task;
	if (ready_task->is_hard) {
#ifdef CONFIG_SMP
		task = rt_smp_linux_task[ready_task->runnable_on_cpus].rnext;
#else
		task = rt_smp_linux_task[0].rnext;
#endif
		while (ready_task->priority >= task->priority) {
			if ((task = task->rnext)->priority < 0) break;
		}
		task->rprev = (ready_task->rprev = task->rprev)->rnext = ready_task;
		ready_task->rnext = task;
	} else {
		ready_task->state |= RT_SCHED_SFTRDY;
		NON_RTAI_TASK_RESUME(ready_task);
	}
}

static inline int renq_ready_task(RT_TASK *ready_task, int priority)
{
	int retval;
	if ((retval = ready_task->priority != priority)) {
		ready_task->priority = priority;
		if (ready_task->state == RT_SCHED_READY) {
			(ready_task->rprev)->rnext = ready_task->rnext;
			(ready_task->rnext)->rprev = ready_task->rprev;
			enq_ready_task(ready_task);
		}
	}
	return retval;
}

static inline void rem_ready_task(RT_TASK *task)
{
	if (task->state == RT_SCHED_READY) {
		if (!task->is_hard) {
			NON_RTAI_TASK_SUSPEND(task);
		}
//		task->unblocked = 0;
		(task->rprev)->rnext = task->rnext;
		(task->rnext)->rprev = task->rprev;
	}
}

static inline void rem_ready_current(RT_TASK *rt_current)
{
	if (!rt_current->is_hard) {
		NON_RTAI_TASK_SUSPEND(rt_current);
	}
//	rt_current->unblocked = 0;
	(rt_current->rprev)->rnext = rt_current->rnext;
	(rt_current->rnext)->rprev = rt_current->rprev;
}

#ifdef CONFIG_RTAI_LONG_TIMED_LIST

/* BINARY TREE */
static inline void enq_timed_task(RT_TASK *timed_task)
{
	RT_TASK *taskh, *tsknxt, *task;
	rb_node_t **rbtn, *rbtpn = NULL;
#ifdef CONFIG_SMP
	task = taskh = &rt_smp_linux_task[timed_task->runnable_on_cpus];
#else
	task = taskh = &rt_smp_linux_task[0];
#endif
	rbtn = &taskh->rbr.rb_node;

	while (*rbtn) {
		rbtpn = *rbtn;
		tsknxt = rb_entry(rbtpn, RT_TASK, rbn);
		if (timed_task->resume_time > tsknxt->resume_time) {
			rbtn = &(rbtpn)->rb_right;
		} else {
			rbtn = &(rbtpn)->rb_left;
			task = tsknxt;
		}
	}
	rb_link_node(&timed_task->rbn, rbtpn, rbtn);
	rb_insert_color(&timed_task->rbn, &taskh->rbr);
	task->tprev = (timed_task->tprev = task->tprev)->tnext = timed_task;
	timed_task->tnext = task;
}

#define	rb_erase_task(task, cpuid) \
	rb_erase(&(task)->rbn, &rt_smp_linux_task[cpuid].rbr);

#else /* !CONFIG_RTAI_LONG_TIMED_LIST */

/* LINEAR */
static inline void enq_timed_task(RT_TASK *timed_task)
{
	RT_TASK *task;
#ifdef CONFIG_SMP
	task = rt_smp_linux_task[timed_task->runnable_on_cpus].tnext;
#else
	task = rt_smp_linux_task[0].tnext;
#endif
	while (timed_task->resume_time > task->resume_time) {
		task = task->tnext;
	}
	task->tprev = (timed_task->tprev = task->tprev)->tnext = timed_task;
	timed_task->tnext = task;
}

#define	rb_erase_task(task, cpuid)

#endif /* !CONFIG_RTAI_LONG_TIMED_LIST */

static inline void rem_timed_task(RT_TASK *task)
{
	if ((task->state & RT_SCHED_DELAYED)) {
                (task->tprev)->tnext = task->tnext;
                (task->tnext)->tprev = task->tprev;
#ifdef CONFIG_SMP
		rb_erase_task(task, task->runnable_on_cpus);
#else
		rb_erase_task(task, 0);
#endif
	}
}

static inline void wake_up_timed_tasks(int cpuid)
{
	RT_TASK *taskh, *task;
#ifdef CONFIG_SMP
	task = (taskh = &rt_smp_linux_task[cpuid])->tnext;
#else
	task = (taskh = &rt_smp_linux_task[0])->tnext;
#endif
	if (task->resume_time - task->schedlat <= rt_time_h) {
		do {
			if ((task->state & RT_SCHED_SUSPENDED) && task->suspdepth > 0) {
				task->suspdepth = 0;
			}
        	        if ((task->state &= ~(RT_SCHED_DELAYED | RT_SCHED_SUSPENDED | RT_SCHED_SEMAPHORE | RT_SCHED_RECEIVE | RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_RETURN | RT_SCHED_MBXSUSP | RT_SCHED_POLL)) == RT_SCHED_READY) {
                	        if (task->policy < 0) {
                        	        enq_ready_edf_task(task);
	                        } else {
        	                        enq_ready_task(task);
                	        }
#if ((CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY > 0 || CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY > 0))
				task->busy_time_align = 1;
#endif
        	        }
			rb_erase_task(task, cpuid);
			task = task->tnext;
		} while (task->resume_time <= rt_time_h);
#ifdef CONFIG_SMP
		rt_smp_linux_task[cpuid].tnext = task;
		task->tprev = &rt_smp_linux_task[cpuid];
#else
		rt_smp_linux_task[0].tnext = task;
		task->tprev = &rt_smp_linux_task[0];
#endif
	}
}

#define get_time() rt_get_time()

static inline void enqueue_blocked(RT_TASK *task, QUEUE *queue, int qtype)
{
        QUEUE *q;
        task->blocked_on = (q = queue);
        if (!qtype) {
                while ((q = q->next) != queue && (q->task)->priority <= task->priority);
        }
        q->prev = (task->queue.prev = q->prev)->next  = &(task->queue);
        task->queue.next = q;
}


static inline void dequeue_blocked(RT_TASK *task)
{
        task->prio_passed_to     = NULL;
        (task->queue.prev)->next = task->queue.next;
        (task->queue.next)->prev = task->queue.prev;
        task->blocked_on         = NULL;
}

static inline unsigned long pass_prio(RT_TASK *to, RT_TASK *from)
{
        QUEUE *q, *blocked_on;
#ifdef CONFIG_SMP
	RT_TASK *rhead;
        unsigned long schedmap;
        schedmap = 0;
#endif
//	from->prio_passed_to = to;
        while (to && to->priority > from->priority) {
                to->priority = from->priority;
		if (to->state == RT_SCHED_READY) {
			if ((to->rprev)->priority > to->priority || (to->rnext)->priority < to->priority) {
#ifdef CONFIG_SMP
				rhead = rt_smp_linux_task[to->runnable_on_cpus].rnext;
#endif
				(to->rprev)->rnext = to->rnext;
				(to->rnext)->rprev = to->rprev;
				enq_ready_task(to);
#ifdef CONFIG_SMP
				if (rhead != rt_smp_linux_task[to->runnable_on_cpus].rnext)  {
					__set_bit(to->runnable_on_cpus & 0x1F, &schedmap);
				}
#endif
			}
			break;
//		} else if ((void *)(q = to->blocked_on) > RTE_HIGERR && !((to->state & RT_SCHED_SEMAPHORE) && ((SEM *)q)->qtype)) {
		} else if ((unsigned long)(blocked_on = to->blocked_on) > RTE_HIGERR && (((to->state & RT_SCHED_SEMAPHORE) && ((SEM *)blocked_on)->type > 0) || (to->state & (RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_RETURN)))) {
			if (to->queue.prev != blocked_on) {
				q = blocked_on;
				(to->queue.prev)->next = to->queue.next;
				(to->queue.next)->prev = to->queue.prev;
				while ((q = q->next) != blocked_on && (q->task)->priority <= to->priority);
				q->prev = (to->queue.prev = q->prev)->next  = &(to->queue);
				to->queue.next = q;
				if (to->queue.prev != blocked_on) {
					break;
				}
			}
			to = (to->state & RT_SCHED_SEMAPHORE) ? ((SEM *)blocked_on)->owndby : blocked_on->task;
                }
//		to = to->prio_passed_to;
	}
#ifdef CONFIG_SMP
	return schedmap;
#else
	return 0;
#endif
}

static inline RT_TASK *_rt_whoami(void)
{
#ifdef CONFIG_SMP
        RT_TASK *rt_current;
        unsigned long flags;
        flags = rt_global_save_flags_and_cli();
        rt_current = RT_CURRENT;
        rt_global_restore_flags(flags);
        return rt_current;
#else
        return rt_smp_current[0];
#endif
}

static inline void __call_exit_handlers(RT_TASK *task)
{
	XHDL *pt, *tmp;

	pt = task->ExitHook; // Initialise ExitHook in rt_task_init()
	while ( pt ) {
		(*pt->fun) (pt->arg1, pt->arg2);
		tmp = pt;
		pt  = pt->nxt;
		rt_free(tmp);
	}
	task->ExitHook = 0;
}

static inline XHDL *__set_exit_handler(RT_TASK *task, void (*fun) (void *, int), void *arg1, int arg2)
{
	XHDL *p;

	// exit handler functions are automatically executed at terminattion time by rt_task_delete()
	// in the reverse order they were created (like C++ destructors behave).
	if (task->magic != RT_TASK_MAGIC) return 0;
	if (!(p = (XHDL *) rt_malloc (sizeof(XHDL)))) return 0;
	p->fun  = fun;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->nxt  = task->ExitHook;
	return (task->ExitHook = p);
}

static inline int rtai_init_features (void)

{
#ifdef CONFIG_RTAI_SEM_BUILTIN
    __rtai_sem_init();
#endif /* CONFIG_RTAI_SEM_BUILTIN */
#ifdef CONFIG_RTAI_MSG_BUILTIN
    __rtai_msg_init();
#endif /* CONFIG_RTAI_MSG_BUILTIN */
#ifdef CONFIG_RTAI_MBX_BUILTIN
    __rtai_mbx_init();
#endif /* CONFIG_RTAI_MBX_BUILTIN */
#ifdef CONFIG_RTAI_FIFOS_BUILTIN
    __rtai_fifos_init();
#endif /* CONFIG_RTAI_FIFOS_BUILTIN */
#ifdef CONFIG_RTAI_SHM_BUILTIN
    __rtai_shm_init();
#endif /* CONFIG_RTAI_SHM_BUILTIN */
#ifdef CONFIG_RTAI_MATH_BUILTIN
    __rtai_math_init();
#endif /* CONFIG_RTAI_MATH_BUILTIN */

	return 0;
}

static inline void rtai_cleanup_features (void) {

#ifdef CONFIG_RTAI_MATH_BUILTIN
    __rtai_math_exit();
#endif /* CONFIG_RTAI_MATH_BUILTIN */
#ifdef CONFIG_RTAI_SHM_BUILTIN
    __rtai_shm_exit();
#endif /* CONFIG_RTAI_SHM_BUILTIN */
#ifdef CONFIG_RTAI_FIFOS_BUILTIN
    __rtai_fifos_exit();
#endif /* CONFIG_RTAI_FIFOS_BUILTIN */
#ifdef CONFIG_RTAI_MBX_BUILTIN
    __rtai_mbx_exit();
#endif /* CONFIG_RTAI_MBX_BUILTIN */
#ifdef CONFIG_RTAI_MSG_BUILTIN
    __rtai_msg_exit();
#endif /* CONFIG_RTAI_MSG_BUILTIN */
#ifdef CONFIG_RTAI_SEM_BUILTIN
    __rtai_sem_exit();
#endif /* CONFIG_RTAI_SEM_BUILTIN */
}

int rt_check_current_stack(void);

int rt_kthread_init_old(RT_TASK *task,
		    void (*rt_thread)(long),
		    long data,
		    int stack_size,
		    int priority,
		    int uses_fpu,
		    void(*signal)(void));

int rt_kthread_init_cpuid(RT_TASK *task,
			  void (*rt_thread)(long),
			  long data,
			  int stack_size,
			  int priority,
			  int uses_fpu,
			  void(*signal)(void),
			  unsigned int cpuid);

#else /* !__KERNEL__ */

#endif /* __KERNEL__ */

#endif /* !_RTAI_SCHEDCORE_H */
