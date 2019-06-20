/**
 * @ingroup tasklets
 * @file
 *
 * Interface of the @ref tasklets "mini LXRT RTAI tasklets module".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#ifndef _RTAI_TASKLETS_H
#define _RTAI_TASKLETS_H

/**
 * @addtogroup tasklets
 *@{*/

#include <rtai_types.h>
#include <rtai_sched.h>

#define TASKLETS_IDX  1

#define INIT	 	 0
#define DELETE		 1
#define TASK_INSERT 	 2
#define TASK_REMOVE	 3
#define USE_FPU	 	 4
#define TIMER_INSERT 	 5
#define TIMER_REMOVE	 6
#define SET_TASKLETS_PRI 7
#define SET_FIR_TIM	 8	
#define SET_PER	 	 9
#define SET_HDL		10
#define SET_DAT	 	11
#define EXEC_TASKLET    12
#define WAIT_IS_HARD	13
#define SET_TSK_PRI	14
#define REG_TASK   	15
#define GET_TMR_TIM	16
#define GET_TMR_OVRN	17

/* Posix timers support */

#define PTIMER_CREATE   18
#define PTIMER_SETTIME  19
#define PTIMER_OVERRUN  20
#define PTIMER_GETTIME  21
#define PTIMER_DELETE   22

#define POSIX_TIMERS    128

/* End Posix timers support */

struct rt_task_struct;

#define TASKLET_STACK_SIZE  8196

struct rt_usp_tasklet_struct {
	struct rt_tasklet_struct *next, *prev;
	int priority, uses_fpu, cpuid;
	RTIME firing_time, period;
	void (*handler)(unsigned long);
	unsigned long data, id;
	long thread;
	struct rt_task_struct *task;
	struct rt_tasklet_struct *usptasklet;
	int overrun;
};

#ifdef __KERNEL__

struct rt_tasklet_struct {
	struct rt_tasklet_struct *next, *prev;
	int priority, uses_fpu, cpuid;
	RTIME firing_time, period;
	void (*handler)(unsigned long);
	unsigned long data, id;
	long thread;
	struct rt_task_struct *task;
	struct rt_tasklet_struct *usptasklet;
	int overrun;
#ifdef  CONFIG_RTAI_LONG_TIMED_LIST
	rb_root_t rbr;
	rb_node_t rbn;
#endif
};

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

int __rtai_tasklets_init(void);

void __rtai_tasklets_exit(void);

struct rt_tasklet_struct *rt_init_tasklet(void);

RTAI_SYSCALL_MODE void rt_delete_tasklet(struct rt_tasklet_struct *tasklet);

RTAI_SYSCALL_MODE int rt_insert_tasklet(struct rt_tasklet_struct *tasklet, int priority, void (*handler)(unsigned long), unsigned long data, unsigned long id, int pid);

RTAI_SYSCALL_MODE void rt_remove_tasklet(struct rt_tasklet_struct *tasklet);

struct rt_tasklet_struct *rt_find_tasklet_by_id(unsigned long id);

RTAI_SYSCALL_MODE int rt_exec_tasklet(struct rt_tasklet_struct *tasklet);

RTAI_SYSCALL_MODE void rt_set_tasklet_priority(struct rt_tasklet_struct *tasklet, int priority);

RTAI_SYSCALL_MODE int rt_set_tasklet_handler(struct rt_tasklet_struct *tasklet, void (*handler)(unsigned long));

#define rt_fast_set_tasklet_handler(t, h) do { (t)->handler = (h); } while (0)

RTAI_SYSCALL_MODE void rt_set_tasklet_data(struct rt_tasklet_struct *tasklet, unsigned long data);

#define rt_fast_set_tasklet_data(t, d) \
do { \
   (t)->data = (d); \
} while (0)

/**
 * Notify the use of floating point operations within any tasklet/timer.
 *
 * rt_tasklets_use_fpu notifies that there is at least one tasklet/timer using
 * floating point calculations within its handler function.
 *
 * @param use_fpu set/resets the use of floating point calculations:
 * - a value different from 0 sets the use of floating point calculations ;
 * - a 0 value resets the no floating calculations state.
 *
 * Note that the use of floating calculations is assigned once for all and is
 * valid for all tasklets/timers. If just one handler needs it all of them
 * will have floating point support. An optimized floating point support,
 * i.e. on a per tasklet/timer base will add an unnoticeable performance
 * improvement on most CPUs. However such an optimization is not rule out a
 * priori, if anybody can prove it is really important.
 *
 * This function and macro can be used within the timer handler.
 *
 */
RTAI_SYSCALL_MODE struct rt_task_struct *rt_tasklet_use_fpu(struct rt_tasklet_struct *tasklet, int use_fpu);

/**
 * Init, in kernel space, a timed tasklet, simply called timer, structure
 * to be used in user space.
 *
 * rt_timer_init allocate a timer tasklet structure (struct rt_tasklet_struct)
 * in kernel space to be used for the management of a user space timer.
 *
 * This function is to be used only for user space timers. In kernel space
 * it is just an empty macro, as the user can, and must allocate the related
 * structure directly, either statically or dynamically.
 *
 * @return the pointer to the timer structure the user space application must
 * use to access all its related services.
 *
 */
#define rt_init_timer rt_init_tasklet 

/**
 * Delete, in kernel space, a timed tasklet, simply called timer, structure
 * to be used in user space.
 *
 * rt_timer_delete free a timer tasklet structure (struct rt_tasklet_struct) in
 * kernel space that was allocated by rt_timer_init.
 *
 * @param timer is a pointer to a timer tasklet structure (struct
 * rt_tasklet_struct).
 *
 * This function is to be used only for user space timers. In kernel space
 * it is just an empty macro, as the user can, and must allocate the related
 * structure directly, either statically or dynamically.
 *
 */
#define rt_delete_timer rt_delete_tasklet

RTAI_SYSCALL_MODE int rt_insert_timer(struct rt_tasklet_struct *timer, int priority, RTIME firing_time, RTIME period, void (*handler)(unsigned long), unsigned long data, int pid);

RTAI_SYSCALL_MODE void rt_remove_timer(struct rt_tasklet_struct *timer);

RTAI_SYSCALL_MODE void rt_set_timer_priority(struct rt_tasklet_struct *timer, int priority);

RTAI_SYSCALL_MODE void rt_set_timer_firing_time(struct rt_tasklet_struct *timer, RTIME firing_time);

RTAI_SYSCALL_MODE void rt_set_timer_period(struct rt_tasklet_struct *timer, RTIME period);

RTAI_SYSCALL_MODE void rt_get_timer_times(struct rt_tasklet_struct *timer, RTIME timer_times[]);

RTAI_SYSCALL_MODE RTIME rt_get_timer_overrun(struct rt_tasklet_struct *timer);

/* Posix timers support */

RTAI_SYSCALL_MODE timer_t rt_kptimer_create(struct rt_tasklet_struct *timer, void (*handler)(unsigned long), unsigned long data, long pid, long thread);
RTAI_SYSCALL_MODE timer_t rt_ptimer_create(struct rt_tasklet_struct **timer);

RTAI_SYSCALL_MODE void rt_ptimer_settime(timer_t timer, const struct itimerspec *value, unsigned long data, long flags);

RTAI_SYSCALL_MODE int rt_ptimer_overrun(timer_t timer);

RTAI_SYSCALL_MODE void rt_ptimer_gettime(timer_t timer, RTIME timer_times[]);

RTAI_SYSCALL_MODE int rt_ptimer_delete(timer_t timer, long space);

/* End Posix timers support */

#define rt_fast_set_timer_period(t, p) \
do { \
   (t)->period = (p); \
} while (0)

/**
 * Change the timer handler.
 *
 * rt_set_timer_handler changes the timer handler function overloading any
 * existing value, so that at the next timer firing the new handler will be
 * used.   Note that if a oneshot timer has its handler changed after it has
 * already expired this function has no effect. You should reinsert it in the
 * timer list with the new handler.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param handler is the new handler.
 *
 * The macro rt_fast_set_timer_handler can safely be used to substitute the
 * corresponding function in kernel space.
 *
 * This function and macro can be used within the timer handler.
 *
 * @retval 0 on success.
 *
 */
#define rt_set_timer_handler rt_set_tasklet_handler

#define rt_fast_set_timer_handler(t, h) do { (t)->handler = (h); } while (0)

/**
 * Change the data passed to a timer.
 *
 * rt_set_timer_data changes the timer data, overloading any existing value, so
 * that at the next timer firing the new data will be used.   Note that if a
 * oneshot timer has its data changed after it is already expired this function
 * has no effect.   You should reinsert it in the timer list with the new data.
 *
 * @param timer is the pointer to the timer structure to be used to manage the
 * timer at hand.
 *
 * @param data is the new data.
 *
 * The macro rt_fast_set_timer_data can safely be used substitute the
 * corresponding function in kernel space.
 *
 *  This function and macro can be used within the timer handler.
 *
 * @retval 0 on success.
 *
 */
#define rt_set_timer_data rt_set_tasklet_data

#define rt_fast_set_timer_data(t, d) do { (t)->data = (d); } while (0)

#define rt_timer_use_fpu rt_tasklet_use_fpu

RTAI_SYSCALL_MODE int rt_wait_tasklet_is_hard(struct rt_tasklet_struct *tasklet, long thread);

RTAI_SYSCALL_MODE void rt_register_task(struct rt_tasklet_struct *tasklet, struct rt_tasklet_struct *usptasklet, struct rt_task_struct *task);
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdarg.h>

#include <rtai_usi.h>
#include <rtai_lxrt.h>

#define rt_tasklet_struct  rt_usp_tasklet_struct
#if 0
struct rt_tasklet_struct {
	struct rt_tasklet_struct *next, *prev;
	int priority, uses_fpu, cpuid;
	RTIME firing_time, period;
	void (*handler)(unsigned long);
	unsigned long data, id;
	long thread;
	struct rt_task_struct *task;
	struct rt_tasklet_struct *usptasklet;
	int overrun;
#ifdef  CONFIG_RTAI_LONG_TIMED_LIST
	struct { void *rb_parent; int rb_color; void *rb_right, *rb_left; } rbn;
	struct { void *rb_node; } rbr;
#endif
};
#endif

#ifndef __SUPPORT_TASKLET__
#define __SUPPORT_TASKLET__

struct support_tasklet_s { struct rt_tasklet_struct *tasklet; pthread_t thread; volatile int done; };

static int support_tasklet(struct support_tasklet_s *args)
{
	RT_TASK *task;
	struct rt_tasklet_struct usptasklet;

	if ((task = rt_thread_init((unsigned long)args->tasklet, 98, 0, SCHED_FIFO, 0xF))) {
	{
		struct { struct rt_tasklet_struct *tasklet, *usptasklet; RT_TASK *task; } reg = { args->tasklet, &usptasklet, task };
		rtai_lxrt(TASKLETS_IDX, sizeof(reg), REG_TASK, &reg);
	}
		rt_grow_and_lock_stack(TASKLET_STACK_SIZE/2);
		mlockall(MCL_CURRENT | MCL_FUTURE);
		rt_make_hard_real_time();
		args->done = 1;
		while (1) {
			rt_task_suspend(task);
			if (usptasklet.handler) {
				usptasklet.handler(usptasklet.data);
			} else {
				break;
			}
		}
		rtai_sti();
		rt_make_soft_real_time();
		rt_task_delete(task);
		return 0;
	}
	printf("CANNOT INIT SUPPORT TASKLET\n");
	return -1;
	
}
#endif /* __SUPPORT_TASKLET__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(void, rt_delete_tasklet, (struct rt_tasklet_struct *tasklet));

RTAI_PROTO(struct rt_tasklet_struct *, rt_init_tasklet, (void))
{
	int is_hard;
	struct support_tasklet_s arg;

	if ((arg.tasklet = (struct rt_tasklet_struct*)rtai_lxrt(TASKLETS_IDX, SIZARG, INIT, &arg).v[LOW])) {
		if ((is_hard = rt_is_hard_real_time(NULL))) {
			rt_make_soft_real_time();
		}
		arg.done = 0;
		if ((arg.thread = rt_thread_create((void *)support_tasklet, &arg.tasklet, TASKLET_STACK_SIZE))) {
			int i;
#define POLLS_PER_SEC 100
		        for (i = 0; i < POLLS_PER_SEC/5 && !arg.done; i++) {
				struct timespec delay = { 0, 1000000000/POLLS_PER_SEC };
				nanosleep(&delay, NULL);
       			}
#undef POLLS_PER_SEC
			if (!arg.done || rtai_lxrt(TASKLETS_IDX, SIZARG, WAIT_IS_HARD, &arg).i[LOW]) {
				goto notdone;
			}
		} else {
notdone:
			rt_delete_tasklet(arg.tasklet);
			arg.tasklet = NULL;
		}
		if (is_hard) {
			rt_make_hard_real_time();
		}
	}
	return arg.tasklet;
}

#define rt_init_timer rt_init_tasklet

RTAI_PROTO(void, rt_delete_tasklet, (struct rt_tasklet_struct *tasklet))
{
	struct { struct rt_tasklet_struct *tasklet; } arg = { tasklet };
	rtai_lxrt(TASKLETS_IDX, SIZARG, DELETE, &arg);
	return;
}

#define rt_delete_timer rt_delete_tasklet

RTAI_PROTO(int, rt_insert_timer,(struct rt_tasklet_struct *timer,
				 int priority,
				 RTIME firing_time,
				 RTIME period,
				 void (*handler)(unsigned long),
				 unsigned long data,
				 int pid))
{
	struct { struct rt_tasklet_struct *timer; long priority; RTIME firing_time; RTIME period; void (*handler)(unsigned long); unsigned long data; long pid; } arg = { timer, priority, firing_time, period, handler, data, pid };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, TIMER_INSERT, &arg).i[LOW];
}

RTAI_PROTO(void, rt_remove_timer, (struct rt_tasklet_struct *timer))
{
	struct { struct rt_tasklet_struct *timer; } arg = { timer };
	rtai_lxrt(TASKLETS_IDX, SIZARG, TIMER_REMOVE, &arg);
}

RTAI_PROTO(void, rt_set_timer_priority, (struct rt_tasklet_struct *timer, int priority))
{
	struct { struct rt_tasklet_struct *timer; long priority; } arg = { timer, priority };
	rtai_lxrt(TASKLETS_IDX, SIZARG, SET_TASKLETS_PRI, &arg);
}

RTAI_PROTO(void, rt_set_timer_firing_time, (struct rt_tasklet_struct *timer, RTIME firing_time))
{
	struct { struct rt_tasklet_struct *timer; RTIME firing_time; } arg = { timer, firing_time };
	rtai_lxrt(TASKLETS_IDX, SIZARG, SET_FIR_TIM, &arg);
}

RTAI_PROTO(void, rt_set_timer_period, (struct rt_tasklet_struct *timer, RTIME period))
{
	struct { struct rt_tasklet_struct *timer; RTIME period; } arg = { timer, period };
	rtai_lxrt(TASKLETS_IDX, SIZARG, SET_PER, &arg);
}

RTAI_PROTO(void, rt_get_timer_times, (struct rt_tasklet_struct *timer, RTIME timer_times[]))
{
	if (timer_times) {
		struct { struct rt_tasklet_struct *timer; RTIME *timer_times; } arg = { timer, timer_times };
		rtai_lxrt(TASKLETS_IDX, SIZARG, GET_TMR_TIM, &arg);
	}
}

RTAI_PROTO(RTIME, rt_get_timer_overrun, (struct rt_tasklet_struct *timer ))
{
	struct { struct rt_tasklet_struct *timer; } arg = { timer };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, GET_TMR_OVRN, &arg).rt;
}

RTAI_PROTO(int, rt_set_tasklet_handler, (struct rt_tasklet_struct *tasklet, void (*handler)(unsigned long)))
{
	struct { struct rt_tasklet_struct *tasklet; void (*handler)(unsigned long); } arg = { tasklet, handler };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, SET_HDL, &arg).i[LOW];
}

#define rt_set_timer_handler rt_set_tasklet_handler

RTAI_PROTO(void, rt_set_tasklet_data, (struct rt_tasklet_struct *tasklet, unsigned long data))
{
	struct { struct rt_tasklet_struct *tasklet; unsigned long data; } arg = { tasklet, data };
	rtai_lxrt(TASKLETS_IDX, SIZARG, SET_DAT, &arg);
}

#define rt_set_timer_data rt_set_tasklet_data

RTAI_PROTO(RT_TASK *, rt_tasklet_use_fpu, (struct rt_tasklet_struct *tasklet, int use_fpu))
{
	RT_TASK *task;
	struct { struct rt_tasklet_struct *tasklet; long use_fpu; } arg = { tasklet, use_fpu };
	if ((task = (RT_TASK*)rtai_lxrt(TASKLETS_IDX, SIZARG, USE_FPU, &arg).v[LOW])) {
		rt_task_use_fpu(task, use_fpu);
	}
	return task;
}

#define rt_timer_use_fpu rt_tasklet_use_fpu

RTAI_PROTO(int, rt_insert_tasklet,(struct rt_tasklet_struct *tasklet,
				   int priority,
				   void (*handler)(unsigned long),
				   unsigned long data,
				   unsigned long id,
				   int pid))
{
	struct { struct rt_tasklet_struct *tasklet; long priority; void (*handler)(unsigned long); unsigned long data; unsigned long id; long pid; } arg = { tasklet, priority, handler, data, id, pid };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, TASK_INSERT, &arg).i[LOW];
}

RTAI_PROTO(void, rt_set_tasklet_priority, (struct rt_tasklet_struct *tasklet, int priority))
{
	struct { struct rt_tasklet_struct *tasklet; long priority; } arg = { tasklet, priority };
	rtai_lxrt(TASKLETS_IDX, SIZARG, SET_TSK_PRI, &arg);
}

RTAI_PROTO(void, rt_remove_tasklet, (struct rt_tasklet_struct *tasklet))
{
	struct { struct rt_tasklet_struct *tasklet; } arg = { tasklet };
	rtai_lxrt(TASKLETS_IDX, SIZARG, TASK_REMOVE, &arg);
}

RTAI_PROTO(int, rt_exec_tasklet, (struct rt_tasklet_struct *tasklet))
{
	struct { struct rt_tasklet_struct *tasklet; } arg = { tasklet };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, EXEC_TASKLET, &arg).i[LOW];
}

#include <stdlib.h>

struct rt_tasklets_struct { volatile int in, out, avb, ntasklets; struct rt_tasklet_struct **tasklets; unsigned long lock; };

RTAI_PROTO(struct rt_tasklets_struct *, rt_create_tasklets, (int ntasklets))
{
	struct rt_tasklets_struct *tasklets;
	if ((tasklets = (struct rt_tasklets_struct *)malloc(sizeof(struct rt_tasklets_struct)))) {
		if ((tasklets->tasklets = (struct rt_tasklet_struct **)malloc(ntasklets*sizeof(struct rt_tasklet_struct *)))) {
			int i;
			for (i = 0; i < ntasklets; i++) {
				if (!(tasklets->tasklets[i] = rt_init_tasklet())) {
					int k;
					for (k = 0; k < i; k++) {
						rt_delete_tasklet(tasklets->tasklets[k]);
					}
					free(tasklets->tasklets);
					goto free_tasklets;
				}
			}
			tasklets->lock = 0;
			tasklets->ntasklets = tasklets->avb = tasklets->in = tasklets->out = ntasklets;
			return tasklets;
		} else {
free_tasklets:
			free(tasklets);
		}
	}
	return NULL;
}

#define rt_create_timers  rt_create_tasklets

RTAI_PROTO(void, rt_destroy_tasklets, (struct rt_tasklets_struct *tasklets))
{
	int i;
	for (i = 0; i < tasklets->ntasklets; i++) {
		rt_delete_tasklet(tasklets->tasklets[i]);
	}
	free(tasklets->tasklets);
	free(tasklets);
}

#define rt_destroy_timers  rt_destroy_tasklets

#include <asm/rtai_atomic.h>

RTAI_PROTO(struct rt_tasklet_struct *, rt_get_tasklet, (struct rt_tasklets_struct *tasklets))
{
	struct rt_tasklet_struct *tasklet;
	while (atomic_cmpxchg((void *)&tasklets->lock, 0, 1));
	if (tasklets->avb > 0) {
		if (tasklets->out >= tasklets->ntasklets) {
			tasklets->out = 0;
		}
		tasklets->avb--;
		tasklet = tasklets->tasklets[tasklets->out++];
		tasklets->lock = 0;
		return tasklet;
	}
	tasklets->lock = 0;
	return NULL;
}

#define rt_get_timer  rt_get_tasklet

RTAI_PROTO(int, rt_gvb_tasklet, (struct rt_tasklet_struct *tasklet, struct rt_tasklets_struct *tasklets))
{
	while (atomic_cmpxchg((void *)&tasklets->lock, 0, 1));
	if (tasklets->avb < tasklets->ntasklets) {
		if (tasklets->in >= tasklets->ntasklets) {
			tasklets->in = 0;
		}
		tasklets->avb++;
		tasklets->tasklets[tasklets->in++] = tasklet;
		tasklets->lock = 0;
		return 0;
	}
	tasklets->lock = 0;
	return EINVAL;
}

#define rt_gvb_timer  rt_gvb_tasklet

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

/*@}*/

#endif /* !_RTAI_TASKLETS_H */
