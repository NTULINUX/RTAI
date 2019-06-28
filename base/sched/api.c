/**
 * @ingroup lxrt
 * @file
 * Common scheduling function 
 * @author Paolo Mantegazza
 *
 * This file is part of the RTAI project.
 *
 * @note Copyright &copy; 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * @note Copyright &copy; 2019 Alec Ari <neotheuser@ymail.com>
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


#include <linux/module.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>

#include <rtai_schedcore.h>
#include <rtai_prinher.h>
#include <rtai_registry.h>

/* ++++++++++++++++++++++++ COMMON FUNCTIONALITIES ++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++ PRIORITY MANAGEMENT ++++++++++++++++++++++++++++ */

RTAI_SYSCALL_MODE void rt_set_sched_policy(RT_TASK *task, int policy, int rr_quantum_ns)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if ((task->policy = policy ? 1 : 0)) {
		task->rr_quantum = nano2count_cpuid(rr_quantum_ns, task->runnable_on_cpus);
		if ((task->rr_quantum & 0xF0000000) || !task->rr_quantum) {
#ifdef CONFIG_SMP
			task->rr_quantum = rt_smp_times[task->runnable_on_cpus].linux_tick;
#else
			task->rr_quantum =  rt_smp_times[0].linux_tick;
#endif
		}
		task->rr_remaining = task->rr_quantum;
		task->yield_time = 0;
	}
}


/**
 * @anchor rt_get_prio
 * @brief Check a task priority.
 * 
 * rt_get_prio returns the base priority of task @e task.
 *
 * Recall that a task has a base native priority, assigned at its
 * birth or by @ref rt_change_prio(), and an actual, inherited,
 * priority. They can be different because of priority inheritance.
 *
 * @param task is the affected task.
 *
 * @return rt_get_prio returns the priority of task @e task.
 *
 */
int rt_get_prio(RT_TASK *task)
{
	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	return task->base_priority;
}


/**
 * @anchor rt_get_inher_prio
 * @brief Check a task priority.
 * 
 * rt_get_prio returns the base priority task @e task has inherited
 * from other tasks, either blocked on resources owned by or waiting
 * to pass a message to task @e task.
 *
 * Recall that a task has a base native priority, assigned at its
 * birth or by @ref rt_change_prio(), and an actual, inherited,
 * priority. They can be different because of priority inheritance.
 *
 * @param task is the affected task.
 *
 * @return rt_get_inher_prio returns the priority of task @e task.
 *
 */
int rt_get_inher_prio(RT_TASK *task)
{
	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	return task->priority;
}


/**
 * @anchor rt_get_priorities
 * @brief Check inheredited and base priority.
 * 
 * rt_get_priorities returns the base and inherited priorities of a task.
 *
 * Recall that a task has a base native priority, assigned at its
 * birth or by @ref rt_change_prio(), and an actual, inherited,
 * priority. They can be different because of priority inheritance.
 *
 * @param task is the affected task.
 *
 * @param priority the actual, e.e. inherited priority.
 *
 * @param base_priority the base priority.
 *
 * @return rt_get_priority returns 0 if non NULL priority addresses
 * are given, EINVAL if addresses are NULL or task is not a valid object.
 *
 */

RTAI_SYSCALL_MODE int rt_get_priorities(RT_TASK *task, int *priority, int *base_priority)
{
	if (!task) {
		task = RT_CURRENT;
	}
	if (task->magic != RT_TASK_MAGIC || !priority || !base_priority) {
		return -EINVAL;
	}
	*priority      = task->priority;
	*base_priority = task->base_priority;
	return 0;
}

/**
 * @anchor rt_task_get_info
 * @brief Get task task data listed in RT_TASK_INFO type.
 * 
 * rt_task_get_info returns task data listed in RT_TASK_INFO type.
 *
 * @param task is the task of interest, NULL can be used for the current task.
 * @param task_info a pointer to RT_TASK_INFO.
 *
 * @return -EINVAL if task is not valid or task_info is NULL, 0 if OK.
 *
 */

RTAI_SYSCALL_MODE int rt_task_get_info(RT_TASK *task, RT_TASK_INFO *task_info)
{
	if (!task) {
		task = RT_CURRENT;
	} 
	if (task->magic != RT_TASK_MAGIC || task_info == NULL) {
		return -EINVAL;
	}
	task_info->period        = task->period;
	task_info->base_priority = task->base_priority;
	task_info->priority      = task->priority;
	return 0;
}

RTAI_SYSCALL_MODE int rt_task_get_info_user(RT_TASK *task, RT_TASK_INFO *task_info)
{
	RT_TASK_INFO ltask_info;
	if (!rt_task_get_info(task, &ltask_info)) {
		rt_copy_to_user(task_info, &ltask_info, sizeof(RT_TASK_INFO));
		return 0;
	}
	return -EINVAL;
}

/**
 * @anchor rt_change_prio
 * @brief Change a task priority.
 * 
 * rt_change_prio changes the base priority of task @e task to @e
 * prio. 
 *
 * Recall that a task has a base native priority, assigned at its
 * birth or by @ref rt_change_prio(), and an actual, inherited,
 * priority. They can be different because of priority inheritance.
 *
 * @param task is the affected task.
 *
 * @param priority is the new priority, it can range within 0 < prio < 
 * RT_SCHED_LOWEST_PRIORITY. 
 *
 * @return rt_change_prio returns the base priority task @e task had
 * before the change.
 *
 */
RTAI_SYSCALL_MODE int rt_change_prio(RT_TASK *task, int priority)
{
	unsigned long flags;
	int prio, base_priority;
	RT_TASK *rhead;

	if (task->magic != RT_TASK_MAGIC || priority < 0) {
		return -EINVAL;
	}

	prio = task->base_priority;
	flags = rt_global_save_flags_and_cli();
	if (!task->is_hard && priority < BASE_SOFT_PRIORITY) {
		priority += BASE_SOFT_PRIORITY;
	}
	base_priority = task->base_priority;
	task->base_priority = priority;
	if (base_priority == task->priority || priority < task->priority) {
		QUEUE *q, *blocked_on;
		unsigned long schedmap = 0;
		do {
			task->priority = priority;
			if (task->state == RT_SCHED_READY) {
				if ((task->rprev)->priority > task->priority || (task->rnext)->priority < task->priority) {
					rhead = rt_smp_linux_task[task->runnable_on_cpus].rnext;
					(task->rprev)->rnext = task->rnext;
					(task->rnext)->rprev = task->rprev;
					enq_ready_task(task);
					if (rhead != rt_smp_linux_task[task->runnable_on_cpus].rnext) {
#ifdef CONFIG_SMP
						__set_bit(task->runnable_on_cpus & 0x1F, &schedmap);
#else
						schedmap = 1;
#endif
					}
				}
				break;
//			 } else if ((task->state & (RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_RETURN | RT_SCHED_SEMAPHORE))) {
			} else if ((unsigned long)(blocked_on = task->blocked_on) > RTE_HIGERR && (((task->state & RT_SCHED_SEMAPHORE) && ((SEM *)blocked_on)->type > 0) || (task->state & (RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_RETURN)))) {
				if (task->queue.prev != (blocked_on = task->blocked_on)) {
					q = blocked_on;
					(task->queue.prev)->next = task->queue.next;
					(task->queue.next)->prev = task->queue.prev;
					while ((q = q->next) != blocked_on && (q->task)->priority <= priority);
					q->prev = (task->queue.prev = q->prev)->next  = &(task->queue);
					task->queue.next = q;
					if (task->queue.prev != blocked_on) {
						break;
					}
				}
				task = (task->state & RT_SCHED_SEMAPHORE) ? ((SEM *)blocked_on)->owndby : blocked_on->task;
			}
		} while (task && task->priority > priority);
		if (schedmap) {
#ifdef CONFIG_SMP
			if (test_and_clear_bit(rtai_cpuid(), &schedmap)) {
				RT_SCHEDULE_MAP_BOTH(schedmap);
			} else {
				RT_SCHEDULE_MAP(schedmap);
			}
#else
			rt_schedule();
#endif
		}
	}
	rt_global_restore_flags(flags);
	return prio;
}

/* +++++++++++++++++++++ TASK RELATED SCHEDULER SERVICES ++++++++++++++++++++ */


/**
 * @anchor rt_whoami
 * @brief Get the task pointer of the current task.
 *
 * Calling rt_whoami from a task can get a pointer to its own task
 * structure.
 * 
 * @return The pointer to the current task.
 */
RT_TASK *rt_whoami(void)
{
	return _rt_whoami();
}


/**
 * @anchor rt_task_yield
 * Yield the current task.
 *
 * @ref rt_task_yield() stops the current task and takes it at the end
 * of the list of ready tasks having its same priority. The scheduler
 * makes the next ready task of the same priority active.
 * If the current task has the highest priority no more then it results
 * in an immediate rescheduling.
 *
 * Recall that RTAI schedulers allow only higher priority tasks to
 * preempt the execution of lower priority ones. So equal priority
 * tasks cannot preempt each other and @ref rt_task_yield() should be
 * used if a user needs a cooperative time slicing among equal
 * priority tasks. The implementation of the related policy is wholly
 * in the hand of the user. It is believed that time slicing is too
 * much an overhead for the most demanding real time applications, so
 * it is left up to you.
 */
void rt_task_yield(void)
{
	RT_TASK *rt_current, *task;
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	rt_current = RT_CURRENT;
	if (rt_smp_linux_task[rt_current->runnable_on_cpus].rnext == rt_current) {
		task = rt_current->rnext;
		while (rt_current->priority == task->priority) {
			task = task->rnext;
		}
		if (task != rt_current->rnext) {
			(rt_current->rprev)->rnext = rt_current->rnext;
			(rt_current->rnext)->rprev = rt_current->rprev;
			task->rprev = (rt_current->rprev = task->rprev)->rnext = rt_current;
			rt_current->rnext = task;
			rt_schedule();
		}
	} else {
		rt_schedule();
	}
	rt_global_restore_flags(flags);
}


/**
 * @anchor rt_task_suspend
 * rt_task_suspend suspends execution of the task task.
 *
 * It will not be executed until a call to @ref rt_task_resume() or
 * @ref rt_task_make_periodic() is made. Multiple suspends and require as 
 * many @ref rt_task_resume() as the rt_task_suspends placed on a task.
 *
 * @param task pointer to a task structure.
 *
 * @return the task suspend depth. An abnormal termination returns as 
 * described below:
 * - @b -EINVAL: task does not refer to a valid task.
 * - @b RTE_UNBLKD:  the task was unblocked while suspended;
 *
 */
RTAI_SYSCALL_MODE int rt_task_suspend(RT_TASK *task)
{
	unsigned long flags;

	if (!task) {
		task = RT_CURRENT;
	} else if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	if (!task_owns_sems(task) && !task->suspdepth) {
		task->suspdepth = 1;
		if (task == RT_CURRENT) {
			task->blocked_on = (void *)task;
			rem_ready_current(task);
			task->state |= RT_SCHED_SUSPENDED;
			rt_schedule();
			if (unlikely(task->blocked_on != NULL)) {
				rt_global_restore_flags(flags);
				return RTE_UNBLKD;
			}
		} else {
			rem_ready_task(task);
			rem_timed_task(task);
			task->state |= RT_SCHED_SUSPENDED;
			if (task->runnable_on_cpus != rtai_cpuid()) {
				send_sched_ipi(1 << task->runnable_on_cpus);
			}
		}
	} else {
		task->suspdepth++;
	}
	rt_global_restore_flags(flags);
	return task->suspdepth;
}


RTAI_SYSCALL_MODE int rt_task_suspend_if(RT_TASK *task)
{
	unsigned long flags;

	if (!task) {
		task = RT_CURRENT;
	} else if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	if (task->suspdepth < 0) {
		task->suspdepth++;
	}
	rt_global_restore_flags(flags);
	return task->suspdepth;
}


RTAI_SYSCALL_MODE int rt_task_suspend_until(RT_TASK *task, RTIME time)
{
	unsigned long flags;

	if (!task) {
		task = RT_CURRENT;
	} else if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	if (!task_owns_sems(task) && !task->suspdepth) {
#ifdef CONFIG_SMP
		int cpuid = rtai_cpuid();
#endif
		if ((task->resume_time = time) > rt_time_h) {
			task->suspdepth = 1;
			if (task == RT_CURRENT) {
				int retval = 0;
				task->blocked_on = (void *)task;
				rem_ready_current(task);
				enq_timed_task(task);
				task->state |= (RT_SCHED_SUSPENDED | RT_SCHED_DELAYED);
				rt_schedule();
				if ((void *)task->blocked_on > RTP_HIGERR) {
					retval = RTE_TIMOUT;
				} else if (unlikely(task->blocked_on != NULL)) {
					retval = RTE_UNBLKD;
				}
				rt_global_restore_flags(flags);
				return retval ? retval : task->suspdepth;
			} else {
				rem_ready_task(task);
				if (!(task->state & RT_SCHED_DELAYED)) {
					enq_timed_task(task);
				}
				task->state |= (RT_SCHED_SUSPENDED | RT_SCHED_DELAYED);
				if (task->runnable_on_cpus != rtai_cpuid()) {
					send_sched_ipi(1 << task->runnable_on_cpus);
				}
			}
		} else {
			rt_global_restore_flags(flags);
			return RTE_TMROVRN;
		}
	} else {
		task->suspdepth++;
	}
	rt_global_restore_flags(flags);
	return task->suspdepth;
}


RTAI_SYSCALL_MODE int rt_task_suspend_timed(RT_TASK *task, RTIME delay)
{
	return rt_task_suspend_until(task, get_time() + delay);
}


/**
 * @anchor rt_task_resume
 * Resume a task.
 *
 * rt_task_resume resumes execution of the task @e task previously
 * suspended by @ref rt_task_suspend(), or makes a newly created task
 * ready to run, if it makes the task ready. Since no account is made
 * for multiple suspend rt_task_resume unconditionally resumes any
 * task it makes ready.
 *
 * @param task pointer to a task structure.
 *
 * @return 0 on success. A negative value on failure as described below:
 * - @b EINVAL: task does not refer to a valid task.
 *
 */
RTAI_SYSCALL_MODE int rt_task_resume(RT_TASK *task)
{
	unsigned long flags;

	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	if (!(--task->suspdepth)) {
		rem_timed_task(task);
		if ((task->state &= ~(RT_SCHED_SUSPENDED | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
			task->blocked_on = NULL;
			enq_ready_task(task);
			RT_SCHEDULE(task, rtai_cpuid());
		}
	}
	rt_global_restore_flags(flags);
	return 0;
}


/**
 * @anchor rt_get_task_state
 * Query task state
 *
 * rt_get_task_state returns the state of a real time task.
 *
 * @param task is a pointer to the task structure.
 *
 * Task state is formed by the bitwise OR of one or more of the
 * following flags:
 *
 * @retval READY Task @e task is ready to run (i.e. unblocked).
 * Note that on a UniProcessor machine the currently running task is
 * just in READY state, while on MultiProcessors can be (READY |
 * RUNNING), see below. 
 * @retval SUSPENDED Task @e task blocked waiting for a resume.
 * @retval DELAYED Task @e task blocked waiting for its next running
 * period or expiration of a timeout.
 * @retval SEMAPHORE Task @e task blocked on a semaphore, waiting for
 * the semaphore to be signaled.
 * @retval SEND Task @e task blocked on sending a message, receiver
 * was not in RECEIVE state.
 * @retval RECEIVE Task @e task blocked waiting for incoming messages,
 * sends or rpcs. 
 * @retval RPC Task @e task blocked on a Remote Procedure Call,
 * receiver was not in RECEIVE state.
 * @retval RETURN Task @e task blocked waiting for a return from a
 * Remote Procedure Call, receiver got the RPC but has not replied
 * yet. 
 * @retval RUNNING Task @e task is running, used only for SMP
 * schedulers. 
 *
 * The returned task state is just an approximate information. Timer
 * and other hardware interrupts may cause a change in the state of
 * the queried task before the caller could evaluate the returned
 * value. Caller should disable interrupts if it wants reliable info
 * about an other task.  rt_get_task_state does not perform any check
 * on pointer task.
 */
int rt_get_task_state(RT_TASK *task)
{
	return task->state;
}


/**
 * @anchor rt_linux_use_fpu
 * @brief Set indication of FPU usage.
 *
 * rt_linux_use_fpu informs the scheduler that floating point
 * arithmetic operations will be used also by foreground Linux
 * processes, i.e. the Linux kernel itself (unlikely) and any of its
 * processes. 
 *
 * @param use_fpu_flag If this parameter has a nonzero value, the
 * Floating Point Unit (FPU) context is also switched when @e task or
 * the kernel becomes active.
 * This makes task switching slower, negligibly, on all 32 bits CPUs
 * but 386s and the oldest 486s. 
 * This flag can be set also by rt_task_init when the real time task
 * is created. With UP and MUP schedulers care is taken to avoid
 * useless saves/ restores of the FPU environment. 
 * Under SMP tasks can be moved from CPU to CPU so saves/restores for
 * tasks using the FPU are always carried out. 
 * Note that by default Linux has this flag cleared. Beside by using
 * rt_linux_use_fpu you can change the Linux FPU flag when you insmod
 * any RTAI scheduler module by setting the LinuxFpu command line
 * parameter of the rtai_sched module itself.
 *
 * @return 0 on success. A negative value on failure as described below:
 * - @b EINVAL: task does not refer to a valid task.
 *
 * See also: @ref rt_linux_use_fpu().
 */
void rt_linux_use_fpu(int use_fpu_flag)
{
	int cpuid;
	for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
		rt_linux_task.uses_fpu = use_fpu_flag ? 1 : 0;
	}
}


/**
 * @anchor rt_task_use_fpu
 * @brief 
 *
 * rt_task_use_fpu informs the scheduler that floating point
 * arithmetic operations will be used by the real time task @e task.
 *
 * @param task is a pointer to the real time task.
 * 
 * @param use_fpu_flag If this parameter has a nonzero value, the
 * Floating Point Unit (FPU) context is also switched when @e task or
 * the kernel becomes active.
 * This makes task switching slower, negligibly, on all 32 bits CPUs
 * but 386s and the oldest 486s.
 * This flag can be set also by @ref rt_task_init() when the real time
 * task is created. With UP and MUP schedulers care is taken to avoid
 * useless saves/restores of the FPU environment.
 * Under SMP tasks can be moved from CPU to CPU so saves/restores for
 * tasks using the FPU are always carried out. 
 *
 * @return 0 on success. A negative value on failure as described below:
 * - @b EINVAL: task does not refer to a valid task.
 *
 * See also: @ref rt_linux_use_fpu().
 */
RTAI_SYSCALL_MODE int rt_task_use_fpu(RT_TASK *task, int use_fpu_flag)
{
	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	task->uses_fpu = use_fpu_flag ? 1 : 0;
	return 0;
}


/**
 * @anchor rt_task_signal_handler
 * @brief Set the signal handler of a task.
 *
 * rt_task_signal_handler installs, or changes, the signal function
 * of a real time task.
 *
 * @param task is a pointer to the real time task.
 *
 * @param handler is the entry point of the signal function.
 *
 * A signal handler function can be set also when the task is newly
 * created with @ref rt_task_init(). The signal handler is a function
 * called within the task environment and with interrupts disabled,
 * when the task becomes the current running task after a context
 * switch, except at its very first scheduling. It allows you to
 * implement whatever signal management policy you think useful, and
 * many other things as well (FIXME).
 *
 * @return 0 on success.A negative value on failure as described below:
 * - @b EINVAL: task does not refer to a valid task.
 */
RTAI_SYSCALL_MODE int rt_task_signal_handler(RT_TASK *task, void (*handler)(void))
{
	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	task->signal = handler;
	return 0;
}

/* ++++++++++++++++++++++++++++ MEASURING TIME ++++++++++++++++++++++++++++++ */

struct epoch_struct boot_epoch = { __SPIN_LOCK_UNLOCKED(boot_epoch.lock), 0, };
EXPORT_SYMBOL(boot_epoch);

static inline void _rt_get_boot_epoch(volatile RTIME time_orig[])
{
	unsigned long flags;
	struct timeval tv;
	RTIME t;

	flags = rt_spin_lock_irqsave(&boot_epoch.lock);
	do_gettimeofday(&tv);
	t = rtai_rdtsc();
	rt_spin_unlock_irqrestore(flags, &boot_epoch.lock);

	time_orig[0] = tv.tv_sec*(RTIME)rtai_tunables.clock_freq + rtai_imuldiv(tv.tv_usec, rtai_tunables.clock_freq, 1000000) - t;
	time_orig[1] = tv.tv_sec*1000000000ULL + tv.tv_usec*1000ULL - rtai_llimd(t, 1000000000, rtai_tunables.clock_freq);
}

void rt_get_boot_epoch(void)
{
	int use;
	_rt_get_boot_epoch(boot_epoch.time[use = 1 - boot_epoch.touse]);
	boot_epoch.touse = use;
}

void rt_gettimeorig(RTIME time_orig[])
{
	if (time_orig == NULL) {
		rt_get_boot_epoch();
	} else {
		_rt_get_boot_epoch(time_orig);
	}
}

/* +++++++++++++++++++++++++++ CONTROLLING TIME ++++++++++++++++++++++++++++++ */

/**
 * @anchor rt_task_make_periodic_relative_ns
 * Make a task run periodically.
 *
 * rt_task_make_periodic_relative_ns mark the task @e task, previously
 * created with @ref rt_task_init(), as suitable for a periodic
 * execution, with period @e period, when @ref rt_task_wait_period()
 * is called.
 *
 * The time of first execution is defined through @e start_time or @e
 * start_delay. @e start_time is an absolute value measured in clock
 * ticks. @e start_delay is relative to the current time and measured
 * in nanoseconds. 
 *
 * @param task is a pointer to the task you want to make periodic.
 *
 * @param start_delay is the time, to wait before the task start
 *	  running, in nanoseconds.
 *
 * @param period corresponds to the period of the task, in nanoseconds.
 *
 * @retval 0 on success. A negative value on failure as described below:
 * - @b EINVAL: task does not refer to a valid task.
 *
 * Recall that the term clock ticks depends on the mode in which the 
 * hard timer runs. So, since only oneshot is used, a clock tick will
 * last as the inverse of the running frequency of the hard timer in
 * use, irrespective of any period used in the call to start_rt_timer.
 */
RTAI_SYSCALL_MODE int rt_task_make_periodic_relative_ns(RT_TASK *task, RTIME start_delay, RTIME period)
{
	unsigned long flags;

	if (!task) {
		task = RT_CURRENT;
	} else if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	start_delay = nano2count_cpuid(start_delay, task->runnable_on_cpus);
	period = nano2count_cpuid(period, task->runnable_on_cpus);
	flags = rt_global_save_flags_and_cli();
	task->periodic_resume_time = task->resume_time = rt_get_time_cpuid(task->runnable_on_cpus) + start_delay;
	task->period = period;
	task->suspdepth = 0;
        if (!(task->state & RT_SCHED_DELAYED)) {
		rem_ready_task(task);
		task->state = (task->state & ~RT_SCHED_SUSPENDED) | RT_SCHED_DELAYED;
		enq_timed_task(task);
}
	RT_SCHEDULE(task, rtai_cpuid());
	rt_global_restore_flags(flags);
	return 0;
}


/**
 * @anchor rt_task_make_periodic
 * Make a task run periodically
 *
 * rt_task_make_periodic mark the task @e task, previously created
 * with @ref rt_task_init(), as suitable for a periodic execution, with
 * period @e period, when @ref rt_task_wait_period() is called.
 *
 * The time of first execution is defined through @e start_time or @e
 * start_delay. @e start_time is an absolute value measured in clock
 * ticks.  @e start_delay is relative to the current time and measured
 * in nanoseconds.
 *
 * @param task is a pointer to the task you want to make periodic.
 *
 * @param start_time is the absolute time to wait before the task start
 *	  running, in clock ticks.
 *
 * @param period corresponds to the period of the task, in clock ticks.
 *
 * @retval 0 on success. A negative value on failure as described
 * below: 
 * - @b EINVAL: task does not refer to a valid task.
 *
 * See also: @ref rt_task_make_periodic_relative_ns().
 * Recall that the term clock ticks depends on the mode in which the 
 * hard timer runs. So, since only oneshot is used, a clock tick will
 * last as the inverse of the running frequency of the hard timer in
 * use, irrespective of any period used in the call to start_rt_timer.
 *
 */
RTAI_SYSCALL_MODE int rt_task_make_periodic(RT_TASK *task, RTIME start_time, RTIME period)
{
	unsigned long flags;

	if (!task) {
		task = RT_CURRENT;
	} else if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	REALTIME2COUNT(start_time);
	flags = rt_global_save_flags_and_cli();
	task->periodic_resume_time = task->resume_time = start_time;
	task->period = period;
	task->suspdepth = 0;
        if (!(task->state & RT_SCHED_DELAYED)) {
		rem_ready_task(task);
		task->state = (task->state & ~RT_SCHED_SUSPENDED) | RT_SCHED_DELAYED;
		enq_timed_task(task);
	}
	RT_SCHEDULE(task, rtai_cpuid());
	rt_global_restore_flags(flags);
	return 0;
}


/**
 * @anchor rt_task_wait_period
 * Wait till next period.
 *
 * rt_task_wait_period suspends the execution of the currently running
 * real time task until the next period is reached.
 * The task must have
 * been previously marked for a periodic execution by calling
 * @ref rt_task_make_periodic() or @ref rt_task_make_periodic_relative_ns().
 *
 * @return 0 if the period expires as expected. An abnormal termination 
 * returns as described below:
 * - @b RTE_UNBLKD:  the task was unblocked while sleeping;
 * - @b RTE_TMROVRN: an immediate return was taken because the next period
 *   has already expired.
 *
 * @note The task is suspended only temporarily, i.e. it simply gives
 * up control until the next time period.
 */
int rt_task_wait_period(void)
{
	DECLARE_RT_CURRENT;
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	ASSIGN_RT_CURRENT;
	if (rt_current->resync_frame) { // Request from watchdog
	    	rt_current->resync_frame = 0;
		rt_current->periodic_resume_time = rt_current->resume_time = rtai_rdtsc();
	} else if ((rt_current->periodic_resume_time += rt_current->period) > rt_time_h) {
		void *blocked_on;
		rt_current->resume_time = rt_current->periodic_resume_time;
		rt_current->blocked_on = NULL;
		rt_current->state |= RT_SCHED_DELAYED;
		rem_ready_current(rt_current);
		enq_timed_task(rt_current);
		rt_schedule();
		blocked_on = rt_current->blocked_on;
		rt_global_restore_flags(flags);
		return likely(!blocked_on) ? 0 : RTE_UNBLKD;
	}
	rt_global_restore_flags(flags);
	return RTE_TMROVRN;
}

RTAI_SYSCALL_MODE void rt_task_set_resume_end_times(RTIME resume, RTIME end)
{
	RT_TASK *rt_current;
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	rt_current = RT_CURRENT;
	rt_current->policy   = -1;
	rt_current->priority =  0;
	if (resume > 0) {
		rt_current->resume_time = resume;
	} else {
		rt_current->resume_time -= resume;
	}
	if (end > 0) {
		rt_current->period = end;
	} else {
		rt_current->period = rt_current->resume_time - end;
	}
	rt_current->state |= RT_SCHED_DELAYED;
	rem_ready_current(rt_current);
	enq_timed_task(rt_current);
	rt_schedule();
	rt_global_restore_flags(flags);
}

RTAI_SYSCALL_MODE int rt_set_resume_time(RT_TASK *task, RTIME new_resume_time)
{
	unsigned long flags;

	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	if (task->state & RT_SCHED_DELAYED) {
		if (((task->resume_time = new_resume_time) - (task->tnext)->resume_time) > 0) {
			rem_timed_task(task);
			enq_timed_task(task);
			rt_global_restore_flags(flags);
			return 0;
        	}
        }
	rt_global_restore_flags(flags);
	return -ETIME;
}

RTAI_SYSCALL_MODE int rt_set_period(RT_TASK *task, RTIME new_period)
{
	unsigned long flags;

	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}
	flags = rt_global_save_flags_and_cli();
	task->period = new_period;
	rt_global_restore_flags(flags);
	return 0;
}

/**
 * @anchor next_period
 * @brief Get the time a periodic task will be resumed after calling
 *  rt_task_wait_period.
 *
 * this function returns the time when the caller task will run
 * next. Combined with the appropriate @ref rt_get_time function() it
 * can be used for checking the fraction of period used or any period
 * overrun.
 *
 * @return Next period time in internal count units.
 */
RTIME next_period(void)
{
	RT_TASK *rt_current;
	unsigned long flags;
	flags = rt_global_save_flags_and_cli();
	rt_current = RT_CURRENT;
	rt_global_restore_flags(flags);
	return rt_current->periodic_resume_time + rt_current->period;
}

/**
 * @anchor rt_busy_sleep
 * @brief Delay/suspend execution for a while.
 *
 * rt_busy_sleep delays the execution of the caller task without
 * giving back the control to the scheduler. This function burns away
 * CPU cycles in a busy wait loop so it should be used only for very
 * short synchronization delays. On machine not having a TSC clock it
 * can lead to many microseconds uncertain busy sleeps because of the
 * need of reading the 8254 timer.
 *
 * @param ns is the number of nanoseconds to wait.
 * 
 * See also: @ref rt_sleep(), @ref rt_sleep_until().
 *
 * @note A higher priority task or interrupt handler can run before
 *	 the task goes to sleep, so the actual time spent in these
 *	 functions may be longer than that specified.
 */
RTAI_SYSCALL_MODE void rt_busy_sleep(int ns)
{
	RTIME end_time;
	end_time = rtai_rdtsc() + rtai_llimd(ns, rtai_tunables.clock_freq, 1000000000);
	while (rtai_rdtsc() < end_time);
}

/**
 * @anchor rt_sleep
 * @brief Delay/suspend execution for a while.
 *
 * rt_sleep suspends execution of the caller task for a time of delay
 * internal count units. During this time the CPU is used by other
 * tasks.
 * 
 * @param delay Corresponds to the time the task is going to be suspended.
 *
 * See also: @ref rt_busy_sleep(), @ref rt_sleep_until().
 *
 * @return 0 if the delay expires as expected. An abnormal termination returns
 *  as described below:
 * - @b RTE_UNBLKD:  the task was unblocked while sleeping;
 * - @b RTE_TMROVRN: an immediate return was taken because the delay is too
 *   short to be honoured.
 *
 * @note A higher priority task or interrupt handler can run before
 *	 the task goes to sleep, so the actual time spent in these
 *	 functions may be longer than the the one specified.
 */
RTAI_SYSCALL_MODE int rt_sleep(RTIME delay)
{
	DECLARE_RT_CURRENT;
	unsigned long flags;
	flags = rt_global_save_flags_and_cli();
	ASSIGN_RT_CURRENT;
	if ((rt_current->resume_time = get_time() + delay) > rt_time_h) {
		void *blocked_on;
		rt_current->blocked_on = NULL;
		rt_current->state |= RT_SCHED_DELAYED;
		rem_ready_current(rt_current);
		enq_timed_task(rt_current);
		rt_schedule();
		blocked_on = rt_current->blocked_on;
		rt_global_restore_flags(flags);
		return likely(!blocked_on) ? 0 : RTE_UNBLKD;
	}
	rt_global_restore_flags(flags);
	return RTE_TMROVRN;
}

/**
 * @anchor rt_sleep_until
 * @brief Delay/suspend execution for a while.
 *
 * rt_sleep_until is similar to @ref rt_sleep() but the parameter time
 * is the absolute time till the task have to be suspended. If the
 * given time is already passed this call has no effect.
 * 
 * @param time Absolute time till the task have to be suspended
 *
 * See also: @ref rt_busy_sleep(), @ref rt_sleep_until().
 *
 * @return 0 if the sleeping expires as expected. An abnormal termination 
 * returns as described below:
 * - @b RTE_UNBLKD:  the task was unblocked while sleeping;
 * - @b RTE_TMROVRN: an immediate return was taken because the time deadline
 *   has already expired.
 *
 * @note A higher priority task or interrupt handler can run before
 *	 the task goes to sleep, so the actual time spent in these
 *	 functions may be longer than the the one specified.
 */
RTAI_SYSCALL_MODE int rt_sleep_until(RTIME time)
{
	DECLARE_RT_CURRENT;
	unsigned long flags;
	REALTIME2COUNT(time);
	flags = rt_global_save_flags_and_cli();
	ASSIGN_RT_CURRENT;
	if ((rt_current->resume_time = time) > rt_time_h) {
		void *blocked_on;
		rt_current->blocked_on = NULL;
		rt_current->state |= RT_SCHED_DELAYED;
		rem_ready_current(rt_current);
		enq_timed_task(rt_current);
		rt_schedule();
		blocked_on = rt_current->blocked_on;
		rt_global_restore_flags(flags);
		return likely(!blocked_on) ? 0 : RTE_UNBLKD;
	}
	rt_global_restore_flags(flags);
	return RTE_TMROVRN;
}

int _rt_task_masked_unblock(RT_TASK *task, unsigned long mask)
{
	if (task->state && task->state != RT_SCHED_READY) {
		if (mask & RT_SCHED_DELAYED) {
			rem_timed_task(task);
		}
		if (task->state != RT_SCHED_READY) {
			if ((mask & task->state & RT_SCHED_SUSPENDED) && task->suspdepth > 0) {
				task->suspdepth = 0;
			}
			if ((task->state &= ~mask) == RT_SCHED_READY) {
				task->blocked_on = RTP_UNBLKD;
				enq_ready_task(task);
				RT_SCHEDULE(task, rtai_cpuid());
			}
		}
		return RTE_UNBLKD;
	}
	return 0;
}

RTAI_SYSCALL_MODE int rt_task_masked_unblock(RT_TASK *task, unsigned long mask)
{
	unsigned long flags;
	int ret;

	if (task->magic != RT_TASK_MAGIC) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	ret = _rt_task_masked_unblock(task, mask);
	rt_global_restore_flags(flags);
	return ret;
}

int rt_nanosleep(struct timespec *rqtp, struct timespec *rmtp)
{
	RTIME expire;

	if (rqtp->tv_nsec >= 1000000000L || rqtp->tv_nsec < 0 || rqtp->tv_sec < 0) {
		return -EINVAL;
	}
	rt_sleep_until(expire = rt_get_time() + timespec2count(rqtp));
	if ((expire -= rt_get_time()) > 0) {
		if (rmtp) {
			count2timespec(expire, rmtp);
		}
		return -EINTR;
	}
	return 0;
}

/* +++++++++++++++++++ READY AND TIMED QUEUE MANIPULATION +++++++++++++++++++ */

void rt_enq_ready_edf_task(RT_TASK *ready_task)
{
	enq_ready_edf_task(ready_task);
}

void rt_enq_ready_task(RT_TASK *ready_task)
{
	enq_ready_task(ready_task);
}

int rt_renq_ready_task(RT_TASK *ready_task, int priority)
{
	return renq_ready_task(ready_task, priority);
}

void rt_rem_ready_task(RT_TASK *task)
{
	rem_ready_task(task);
}

void rt_rem_ready_current(RT_TASK *rt_current)
{
	rem_ready_current(rt_current);
}

void rt_enq_timed_task(RT_TASK *timed_task)
{
	enq_timed_task(timed_task);
}

void rt_wake_up_timed_tasks(int cpuid)
{
#ifdef CONFIG_SMP
	wake_up_timed_tasks(cpuid);
#else
        wake_up_timed_tasks(0);
#endif
}

void rt_rem_timed_task(RT_TASK *task)
{
	rem_timed_task(task);
}

void rt_enqueue_blocked(RT_TASK *task, QUEUE *queue, int qtype)
{
	enqueue_blocked(task, queue, qtype);
}

void rt_dequeue_blocked(RT_TASK *task)
{
	dequeue_blocked(task);
}

int rt_renq_current(RT_TASK *rt_current, int priority)
{
	return renq_ready_task(rt_current, priority);
}

/* ++++++++++++++++++++++++ NAMED TASK INIT/DELETE ++++++++++++++++++++++++++ */

RTAI_SYSCALL_MODE RT_TASK *rt_named_task_init(const char *task_name, void (*thread)(long), long data, int stack_size, int prio, int uses_fpu, void(*signal)(void))
{
	RT_TASK *task;
	unsigned long name;

	if ((task = rt_get_adr(name = nam2num(task_name)))) {
		return task;
	}
        if ((task = rt_malloc(sizeof(RT_TASK))) && !rt_task_init(task, thread, data, stack_size, prio, uses_fpu, signal)) {
		if (rt_register(name, task, IS_TASK, 0)) {
			return task;
		}
		rt_task_delete(task);
	}
	rt_free(task);
	return (RT_TASK *)0;
}

RTAI_SYSCALL_MODE RT_TASK *rt_named_task_init_cpuid(const char *task_name, void (*thread)(long), long data, int stack_size, int prio, int uses_fpu, void(*signal)(void), unsigned int run_on_cpu)
{
	RT_TASK *task;
	unsigned long name;

	if ((task = rt_get_adr(name = nam2num(task_name)))) {
		return task;
	}
        if ((task = rt_malloc(sizeof(RT_TASK))) && !rt_task_init_cpuid(task, thread, data, stack_size, prio, uses_fpu, signal, run_on_cpu)) {
		if (rt_register(name, task, IS_TASK, 0)) {
			return task;
		}
		rt_task_delete(task);
	}
	rt_free(task);
	return (RT_TASK *)0;
}

RTAI_SYSCALL_MODE int rt_named_task_delete(RT_TASK *task)
{
	if (!rt_task_delete(task)) {
		rt_free(task);
	}
	return rt_drg_on_adr(task);
}

/* +++++++++++++++++++++++++++++++ REGISTRY +++++++++++++++++++++++++++++++++ */

#define HASHED_REGISTRY

#ifdef HASHED_REGISTRY

int max_slots;
static struct rt_registry_entry *lxrt_list;
static DEFINE_SPINLOCK(list_lock);

#define COLLISION_COUNT() do { col++; } while(0)
static unsigned long long col;
#ifndef COLLISION_COUNT
#define COLLISION_COUNT()
#endif

#define NONAME  (1UL)
#define NOADR   ((void *)1)

#define PRIMES_TAB_GRANULARITY  100

static unsigned short primes[ ] = { 1, 103, 211, 307, 401, 503, 601, 701, 809, 907, 1009, 1103, 1201, 1301, 1409, 1511, 1601, 1709, 1801, 1901, 2003, 2111, 2203, 2309, 2411, 2503, 2609, 2707, 2801, 2903, 3001, 3109, 3203, 3301, 3407, 3511,
3607, 3701, 3803, 3907, 4001, 4111, 4201, 4327, 4409, 4507, 4603, 4703, 4801, 4903, 5003, 5101, 5209, 5303, 5407, 5501, 5623, 5701, 5801, 5903, 6007, 6101, 6203, 6301, 6421, 6521, 6607, 6703, 6803, 6907, 7001, 7103, 7207, 7307, 7411, 7507,
7603, 7703, 7817, 7901, 8009, 8101, 8209, 8311, 8419, 8501, 8609, 8707, 8803, 8923, 9001, 9103, 9203, 9311, 9403, 9511, 9601, 9719, 9803, 9901, 10007, 10103, 10211, 10301, 10427, 10501, 10601, 10709, 10831, 10903, 11003, 11113, 11213, 11311, 11411, 11503, 11597, 11617, 11701, 11801, 11903, 12007, 12101, 12203, 12301, 12401, 12503, 12601, 12703, 12809, 12907, 13001, 13103, 13217, 13309, 13411, 13513, 13613, 13709, 13807, 13901, 14009, 14107, 14207, 14303, 14401, 14503, 14621,
14713, 14813, 14923, 15013, 15101, 15217, 15307, 15401, 15511, 15601, 15727, 15803, 15901, 16001, 16103, 16217, 16301, 16411, 16519, 16603, 16703, 16811, 16901, 17011, 17107, 17203, 17317, 17401, 17509, 17609, 17707, 17807, 17903, 18013, 18119, 18211, 18301, 18401, 18503, 18617, 18701, 18803, 18911, 19001, 19121, 19207, 19301, 19403, 19501, 19603, 19709, 19801, 19913, 20011, 20101 };

#define hash_fun(m, n) ((m)%(n) + 1)

static inline int hash_ins_adr(void *adr, struct rt_registry_entry *list, int lstlen, int nlink)
{
	int i, k;
	unsigned long flags;

	i = hash_fun((unsigned long)adr, lstlen);
	while (1) {
		k = i;
		while (list[k].adr > NOADR && list[k].adr != adr) {
COLLISION_COUNT();
			if (++k > lstlen) {
				k = 1;
			}
			if (k == i) {
				return 0;
			}
		}
		flags = rt_spin_lock_irqsave(&list_lock);
		if (list[k].adr == adr) {
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return -k;
		} else if (list[k].adr <= NOADR) {
			list[k].adr       = adr;
			list[k].nlink     = nlink;
			list[nlink].alink = k;
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return k;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
}

static int hash_ins_name(unsigned long name, void *adr, int type, struct task_struct *lnxtsk, struct rt_registry_entry *list, int lstlen, int inc)
{
	int i, k;
	unsigned long flags;

	i = hash_fun(name, lstlen);
	while (1) {
		k = i;
		while (list[k].name > NONAME && list[k].name != name) {
COLLISION_COUNT();
			if (++k > lstlen) {
				k = 1;
			}
			if (k == i) {
				return 0;
			}
		}
		flags = rt_spin_lock_irqsave(&list_lock);
		if (list[k].name == name) {
			if (inc) {
				list[k].count++;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return -k;
		} else if (list[k].name <= NONAME) {
			list[k].name  = name;
			list[k].type  = type;
			list[k].tsk   = lnxtsk;
			list[k].count = 1;
			list[k].alink = 0;
			rt_spin_unlock_irqrestore(flags, &list_lock);
	                if (hash_ins_adr(adr, list, lstlen, k) <= 0) {
        	                return 0;
                	}
			return k;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
}

static void *hash_find_name(unsigned long name, struct rt_registry_entry *list, long lstlen, int inc, int *slot)
{
	int i, k;
	unsigned long flags;

	i = hash_fun(name, lstlen);
	while (1) {
		k = i;
		while (list[k].name && list[k].name != name) {
COLLISION_COUNT();
			if (++k > lstlen) {
				k = 1;
			}
			if (k == i) {
				return NULL;
			}
		}
		flags = rt_spin_lock_irqsave(&list_lock);
		if (list[k].name == name) {
			if (inc) {
				list[k].count++;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			if (slot) {
				*slot = k;
			}
			return list[list[k].alink].adr;
		} else if (list[k].name <= NONAME) {
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return NULL;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
}

static unsigned long hash_find_adr(void *adr, struct rt_registry_entry *list, long lstlen, int inc)
{
	int i, k;
	unsigned long flags;

	i = hash_fun((unsigned long)adr, lstlen);
	while (1) {
		k = i;
		while (list[k].adr && list[k].adr != adr) {
COLLISION_COUNT();
			if (++k > lstlen) {
				k = 1;
			}
			if (k == i) {
				return 0;
			}
		}
		flags = rt_spin_lock_irqsave(&list_lock);
		if (list[k].adr == adr) {
			if (inc) {
				list[list[k].nlink].count++;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return list[list[k].nlink].name;
		} else if (list[k].adr <= NOADR) {
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return 0;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
}

static int hash_rem_name(unsigned long name, struct rt_registry_entry *list, long lstlen, int dec)
{
	int i, k;
	unsigned long flags;

	k = i = hash_fun(name, lstlen);
	while (list[k].name && list[k].name != name) {
COLLISION_COUNT();
		if (++k > lstlen) {
			k = 1;
		}
		if (k == i) {
			return 0;
		}
	}
	flags = rt_spin_lock_irqsave(&list_lock);
	if (list[k].name == name) {
		if (!dec || (list[k].count && !--list[k].count)) {
			int j;
			if ((i = k + 1) > lstlen) {
				i = 1;
			}
			list[k].name = !list[i].name ? 0UL : NONAME;
			if ((j = list[k].alink)) {
				if ((i = j + 1) > lstlen) {
					i = 1;
				}
				list[j].adr = !list[i].adr ? NULL : NOADR;
			}
		}
		if (dec) {
			k = list[k].count;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
		return k;
	}
	rt_spin_unlock_irqrestore(flags, &list_lock);
	return dec;
}

static int hash_rem_adr(void *adr, struct rt_registry_entry *list, long lstlen, int dec)
{
	int i, k;
	unsigned long flags;

	k = i = hash_fun((unsigned long)adr, lstlen);
	while (list[k].adr && list[k].adr != adr) {
COLLISION_COUNT();
		if (++k > lstlen) {
			k = 1;
		}
		if (k == i) {
			return 0;
		}
	}
	flags = rt_spin_lock_irqsave(&list_lock);
	if (list[k].adr == adr) {
		if (!dec || (list[list[k].nlink].count && !--list[list[k].nlink].count)) {
			int j;
			if ((i = k + 1) > lstlen) {
				i = 1;
			}
			list[k].adr = !list[i].adr ? NULL : NOADR;
			j = list[k].nlink;
			if ((i = j + 1) > lstlen) {
				i = 1;
			}
			list[j].name = !list[i].name ? 0UL : NONAME;
		}
		if (dec) {
			k = list[list[k].nlink].count;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
		return k;
	}
	rt_spin_unlock_irqrestore(flags, &list_lock);
	return dec;
}

static inline int registr(unsigned long name, void *adr, int type, struct task_struct *lnxtsk)
{
	return abs(hash_ins_name(name, adr, type, lnxtsk, lxrt_list, max_slots, 1));
}

static inline int drg_on_name(unsigned long name)
{
	return hash_rem_name(name, lxrt_list, max_slots, 0);
} 

static inline int drg_on_name_cnt(unsigned long name)
{
	return hash_rem_name(name, lxrt_list, max_slots, -EFAULT);
} 

static inline int drg_on_adr(void *adr)
{
	return hash_rem_adr(adr, lxrt_list, max_slots, 0);
} 

static inline int drg_on_adr_cnt(void *adr)
{
	return hash_rem_adr(adr, lxrt_list, max_slots, -EFAULT);
} 

static inline unsigned long get_name(void *adr)
{
	if (!adr) {
		unsigned long flags, name;
		int i;
		for (i = 0; i < 10; i++) {
#if 0
			static unsigned long nameseed = MAX_NAM2NUM;
			flags = rt_spin_lock_irqsave(&list_lock);
			if ((name = ++nameseed) == 0xFFFFFFFFUL) {
				name = nameseed = MAX_NAM2NUM;
			}
#else
			flags = rt_spin_lock_irqsave(&list_lock);
			name = MAX_NAM2NUM + irandu(0xFFFFFFFFUL - MAX_NAM2NUM - 2);
#endif
			rt_spin_unlock_irqrestore(flags, &list_lock);
			if (!hash_find_name(name, lxrt_list, max_slots, 0, NULL)) {
				return name;
			}
		}
		return 0;
	} else {
		return hash_find_adr(adr, lxrt_list, max_slots, 0);
	}
	return 0;
} 

static inline void *get_adr(unsigned long name)
{
	return hash_find_name(name, lxrt_list, max_slots, 0, NULL);
} 

static inline void *get_adr_cnt(unsigned long name)
{
	return hash_find_name(name, lxrt_list, max_slots, 1, NULL);
} 

static inline int get_type(unsigned long name)
{
	int slot;

	if (hash_find_name(name, lxrt_list, max_slots, 0, &slot)) {
		return lxrt_list[slot].type;
	}
        return -EINVAL;
}

unsigned long is_process_registered(struct task_struct *lnxtsk)
{
	void *adr = rtai_tskext(lnxtsk, TSKEXT0);
	return adr ? hash_find_adr(adr, lxrt_list, max_slots, 0) : 0;
}

int rt_get_registry_slot(int slot, struct rt_registry_entry *entry)
{
	unsigned long flags;
       	flags = rt_spin_lock_irqsave(&list_lock);
	if (lxrt_list[slot].name > NONAME) {
		*entry = lxrt_list[slot];
		entry->adr = lxrt_list[entry->alink].adr;
		rt_spin_unlock_irqrestore(flags, &list_lock);
		return slot;
       	}
        rt_spin_unlock_irqrestore(flags, &list_lock);
        return 0;
}

int rt_registry_alloc(void)
{
	if ((max_slots = (MAX_SLOTS + PRIMES_TAB_GRANULARITY - 1)/(PRIMES_TAB_GRANULARITY)) >= sizeof(primes)/sizeof(primes[0])) {
		printk("REGISTRY TABLE TOO LARGE FOR AVAILABLE PRIMES\n");
                return -ENOMEM;
        }
	max_slots = primes[max_slots];
	if (!(lxrt_list = vmalloc((max_slots + 1)*sizeof(struct rt_registry_entry)))) {
		printk("NO MEMORY FOR REGISTRY TABLE\n");
                return -ENOMEM;
	}
	memset(lxrt_list, 0, (max_slots + 1)*sizeof(struct rt_registry_entry));
	return 0;
}

void rt_registry_free(void)
{
	if (lxrt_list) {
		vfree(lxrt_list);
		lxrt_list = NULL;
	}
}
#else
volatile int max_slots;
static struct rt_registry_entry *lxrt_list;
static DEFINE_SPINLOCK(list_lock);

int rt_registry_alloc(void)
{
	if (!(lxrt_list = vmalloc((MAX_SLOTS + 1)*sizeof(struct rt_registry_entry)))) {
                printk("NO MEMORY FOR REGISTRY TABLE\n");
                return -ENOMEM;
        }
        memset(lxrt_list, 0, (MAX_SLOTS + 1)*sizeof(struct rt_registry_entry));
        return 0;
}

void rt_registry_free(void)
{
	if (lxrt_list) {
		vfree(lxrt_list);
	}
}

static inline int registr(unsigned long name, void *adr, int type, struct task_struct *tsk)
{
        unsigned long flags;
        int i, slot;
/*
 * Register a resource. This allows other programs (RTAI and/or user space)
 * to use the same resource because they can find the address from the name.
*/
        // index 0 is reserved for the null slot.
	while ((slot = max_slots) < MAX_SLOTS) {
        	for (i = 1; i <= max_slots; i++) {
                	if (lxrt_list[i].name == name) {
				return 0;
			}
		}
        	flags = rt_spin_lock_irqsave(&list_lock);
                if (slot == max_slots && max_slots < MAX_SLOTS) {
			slot = ++max_slots;
                        lxrt_list[slot].name  = name;
                        lxrt_list[slot].adr   = adr;
                        lxrt_list[slot].tsk   = tsk;
                        lxrt_list[slot].type  = type;
                        lxrt_list[slot].count = 1;
                        rt_spin_unlock_irqrestore(flags, &list_lock);
                        return slot;
                }
        	rt_spin_unlock_irqrestore(flags, &list_lock);
        }
        return 0;
}

static inline int drg_on_name(unsigned long name)
{
	unsigned long flags;
	int slot;
	for (slot = 1; slot <= max_slots; slot++) {
		flags = rt_spin_lock_irqsave(&list_lock);
		if (lxrt_list[slot].name == name) {
			if (slot < max_slots) {
				lxrt_list[slot] = lxrt_list[max_slots];
			}
			if (max_slots > 0) {
				max_slots--;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return slot;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
	return 0;
} 

static inline int drg_on_name_cnt(unsigned long name)
{
	unsigned long flags;
	int slot, count;
	for (slot = 1; slot <= max_slots; slot++) {
		flags = rt_spin_lock_irqsave(&list_lock);
		if (lxrt_list[slot].name == name && lxrt_list[slot].count > 0 && !(count = --lxrt_list[slot].count)) {
			if (slot < max_slots) {
				lxrt_list[slot] = lxrt_list[max_slots];
			}
			if (max_slots > 0) {
				max_slots--;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return count;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
	return -EFAULT;
} 

static inline int drg_on_adr(void *adr)
{
	unsigned long flags;
	int slot;
	for (slot = 1; slot <= max_slots; slot++) {
		flags = rt_spin_lock_irqsave(&list_lock);
		if (lxrt_list[slot].adr == adr) {
			if (slot < max_slots) {
				lxrt_list[slot] = lxrt_list[max_slots];
			}
			if (max_slots > 0) {
				max_slots--;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return slot;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
	return 0;
} 

static inline int drg_on_adr_cnt(void *adr)
{
	unsigned long flags;
	int slot, count;
	for (slot = 1; slot <= max_slots; slot++) {
		flags = rt_spin_lock_irqsave(&list_lock);
		if (lxrt_list[slot].adr == adr && lxrt_list[slot].count > 0 && !(count = --lxrt_list[slot].count)) {
			if (slot < max_slots) {
				lxrt_list[slot] = lxrt_list[max_slots];
			}
			if (max_slots > 0) {
				max_slots--;
			}
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return count;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
	return -EFAULT;
} 

static inline unsigned long get_name(void *adr)
{
	static unsigned long nameseed = 3518743764UL;
	int slot;
        if (!adr) {
		unsigned long flags;
		unsigned long name;
		flags = rt_spin_lock_irqsave(&list_lock);
		if ((name = ++nameseed) == 0xFFFFFFFFUL) {
			nameseed = 3518743764UL;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
		return name;
        }
	for (slot = 1; slot <= max_slots; slot++) {
		if (lxrt_list[slot].adr == adr) {
			return lxrt_list[slot].name;
		}
	}
	return 0;
} 

static inline void *get_adr(unsigned long name)
{
	int slot;
	for (slot = 1; slot <= max_slots; slot++) {
		if (lxrt_list[slot].name == name) {
			return lxrt_list[slot].adr;
		}
	}
	return 0;
} 

static inline void *get_adr_cnt(unsigned long name)
{
	unsigned long flags;
	int slot;
	for (slot = 1; slot <= max_slots; slot++) {
		flags = rt_spin_lock_irqsave(&list_lock);
		if (lxrt_list[slot].name == name) {
			++lxrt_list[slot].count;
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return lxrt_list[slot].adr;
		}
		rt_spin_unlock_irqrestore(flags, &list_lock);
	}
	return 0;
} 

static inline int get_type(unsigned long name)
{
        int slot;
        for (slot = 1; slot <= max_slots; slot++) {
                if (lxrt_list[slot].name == name) {
                        return lxrt_list[slot].type;
                }
        }
        return -EINVAL;
}

unsigned long is_process_registered(struct task_struct *tsk)
{
        void *adr;

	if ((adr = rtai_tskext(tsk, TSKEXT0))) {
		int slot;
		for (slot = 1; slot <= max_slots; slot++) {
			if (lxrt_list[slot].adr == adr) {
				return lxrt_list[slot].name;
			}
                }
        }
        return 0;
}

int rt_get_registry_slot(int slot, struct rt_registry_entry *entry)
{
	unsigned long flags;

	if(entry == 0) {
		return 0;
	}
	flags = rt_spin_lock_irqsave(&list_lock);
	if (slot > 0 && slot <= max_slots ) {
		if (lxrt_list[slot].name != 0) {
			*entry = lxrt_list[slot];
			rt_spin_unlock_irqrestore(flags, &list_lock);
			return slot;
		}
	}
	rt_spin_unlock_irqrestore(flags, &list_lock);

	return 0;
}
#endif

/**
 * @ingroup lxrt
 * Register an object.
 *
 * rt_register registers the object to be identified with @a name, which is
 * pointed by @a adr.
 *
 * @return a positive number on success, 0 on failure.
 */
int rt_register(unsigned long name, void *adr, int type, struct task_struct *t)
{
/*
 * Register a resource. This function provides the service to all RTAI tasks.
*/
	return get_adr(name) ? 0 : registr(name, adr, type, t );
}


/**
 * @ingroup lxrt
 * Deregister an object by its name.
 *
 * rt_drg_on_name deregisters the object identified by its @a name.
 *
 * @return a positive number on success, 0 on failure.
 */
int rt_drg_on_name(unsigned long name)
{
	return drg_on_name(name);
} 

/**
 * @ingroup lxrt
 * Deregister an object by its address.
 *
 * rt_drg_on_adr deregisters the object identified by its @a adr.
 *
 * @return a positive number on success, 0 on failure.
 */
int rt_drg_on_adr(void *adr)
{
	return drg_on_adr(adr);
} 

RTAI_SYSCALL_MODE unsigned long rt_get_name(void *adr)
{
	return get_name(adr);
} 

RTAI_SYSCALL_MODE void *rt_get_adr(unsigned long name)
{
	return get_adr(name);
}

int rt_get_type(unsigned long name)
{
	return get_type(name);
}

int rt_drg_on_name_cnt(unsigned long name)
{
	return drg_on_name_cnt(name);
}

int rt_drg_on_adr_cnt(void *adr)
{
	return drg_on_adr_cnt(adr);
}

void *rt_get_adr_cnt(unsigned long name)
{
	return get_adr_cnt(name);
}

#include <rtai_lxrt.h>

extern struct rt_fun_entry rt_fun_lxrt[];

void krtai_objects_release(void)
{
	int slot;
        struct rt_registry_entry entry;
	char name[8], *type;

	for (slot = 1; slot <= max_slots; slot++) {
                if (rt_get_registry_slot(slot, &entry)) {
			switch (entry.type) {
	                       	case IS_TASK:
					type = "TASK";
					rt_named_task_delete(entry.adr);
					break;
				case IS_SEM:
					type = "SEM ";
					((void (*)(void *))rt_fun_lxrt[NAMED_SEM_DELETE].fun)(entry.adr);
					break;
				case IS_RWL:
					type = "RWL ";
					((void (*)(void *))rt_fun_lxrt[NAMED_RWL_DELETE].fun)(entry.adr);
					break;
				case IS_SPL:
					type = "SPL ";
					((void (*)(void *))rt_fun_lxrt[NAMED_SPL_DELETE].fun)(entry.adr);
					break;
				case IS_MBX:
					type = "MBX ";
					((void (*)(void *))rt_fun_lxrt[NAMED_MBX_DELETE].fun)(entry.adr);
	                       		break;	
				case IS_PRX:
					type = "PRX ";
					((void (*)(void *))rt_fun_lxrt[PROXY_DETACH].fun)(entry.adr);
					rt_drg_on_adr(entry.adr); 
					break;
	                       	default:
					type = "ALIEN";
					break;
			}
			num2nam(entry.name, name);
			rt_printk("SCHED releases registered named %s %s\n", type, name);
		}
	}
}

/* +++++++++++++++++ SUPPORT FOR THE LINUX SYSCALL SERVER +++++++++++++++++++ */

RTAI_SYSCALL_MODE int rt_set_linux_syscall_mode(long mode, void (*cbfun)(long, long))
{
	RT_TASK *server;
	struct linux_syscalls_list *syscalls;
	if ((server = RT_CURRENT->linux_syscall_server) == NULL || mode != SYNC_LINUX_SYSCALL || mode != ASYNC_LINUX_SYSCALL) {
		return EINVAL;
	}
	syscalls = server->linux_syscall_server;
	rt_put_user(mode, &syscalls->mode);
	rt_put_user(cbfun, &syscalls->cbfun);
	return 0;
}

#ifndef CONFIG_RTAI_USE_STACK_ARGS

void rt_exec_linux_syscall(RT_TASK *rt_current, struct linux_syscalls_list *syscalls, struct pt_regs *regs)
{
	int in, id;
	struct { int in, out, nr, id, mode; void (*cbfun)(long, long); RT_TASK *serv; } from;

	rt_copy_from_user(&from, syscalls, sizeof(from));
	in = from.in;
	if (++from.in >= from.nr) {
		from.in = 0;
	}
	if (from.mode == ASYNC_LINUX_SYSCALL && from.in == from.out) {
		regs->LINUX_SYSCALL_RETREG = -1;
		return;
	}

#if defined( __NR_socketcall)
	if (regs->LINUX_SYSCALL_NR == __NR_socketcall) {
		int i;
		for (i = 0; i < NSYSCALL_PACARGS; i++) {
			long pacarg;
			rt_get_user(pacarg, (long *)regs->LINUX_SYSCALL_REG2 + i);
			rt_put_user(pacarg, &syscalls->syscall[in].pacargs[i]);
		}
		rt_put_user((long)syscalls->syscall[in].pacargs, &syscalls->syscall[in].args[2]);
	} else
#endif
	{
		rt_put_user(regs->LINUX_SYSCALL_REG2, &syscalls->syscall[in].args[2]); 
	}
	rt_put_user(regs->LINUX_SYSCALL_NR,   &syscalls->syscall[in].args[0]); 
	rt_put_user(regs->LINUX_SYSCALL_REG1, &syscalls->syscall[in].args[1]); 
	rt_put_user(regs->LINUX_SYSCALL_REG3, &syscalls->syscall[in].args[3]); 
	rt_put_user(regs->LINUX_SYSCALL_REG4, &syscalls->syscall[in].args[4]); 
	rt_put_user(regs->LINUX_SYSCALL_REG5, &syscalls->syscall[in].args[5]); 
	rt_put_user(regs->LINUX_SYSCALL_REG6, &syscalls->syscall[in].args[6]); 
	rt_put_user(from.id,    &syscalls->syscall[in].id); 
	rt_put_user(from.mode,  &syscalls->syscall[in].mode); 
	rt_put_user(from.cbfun, &syscalls->syscall[in].cbfun); 
	id = from.id;
	if (++from.id < 0) {
		from.id = 0;
	}
	rt_put_user(from.id, &syscalls->id);
	rt_put_user(from.in, &syscalls->in);
	if (from.serv->suspdepth >= -from.nr) {
		from.serv->priority = rt_current->priority + BASE_SOFT_PRIORITY;
		rt_task_resume(from.serv);
	}
	if (from.mode == SYNC_LINUX_SYSCALL) {
		rt_task_suspend(rt_current);
		rt_get_user(regs->LINUX_SYSCALL_RETREG, &syscalls->syscall[in].retval);
	} else {
		regs->LINUX_SYSCALL_RETREG = id;
	}
	return;
}

#else

void rt_exec_linux_syscall(RT_TASK *rt_current, struct linux_syscalls_list *syscalls, struct pt_regs *regs)
{
	int in, id;
	struct linux_syscall syscall;
	struct { int in, out, nr, id, mode; void (*cbfun)(long, long); RT_TASK *serv; } from;

	rt_copy_from_user(&from, syscalls, sizeof(from));
	in = from.in;
	if (++from.in >= from.nr) {
		from.in = 0;
	}
	if (from.mode == ASYNC_LINUX_SYSCALL && from.in == from.out) {
		regs->LINUX_SYSCALL_RETREG = -1;
		return;
	}

#if defined( __NR_socketcall)
	if (regs->LINUX_SYSCALL_NR == __NR_socketcall) {
		if ((void *)regs->LINUX_SYSCALL_REG2 > PAGE_OFFSET) {
			memcpy(syscall.pacargs, (void *)regs->LINUX_SYSCALL_REG2, sizeof(syscall.pacargs));
		} else {
			rt_copy_from_user(syscall.pacargs, (void *)regs->LINUX_SYSCALL_REG2, sizeof(syscall.pacargs));
		}
		syscall.args[2] = (long)(&syscalls->syscall[in].pacargs);
		id = offsetof(struct linux_syscall, retval);
	} else
#endif
	{
		syscall.args[2] = regs->LINUX_SYSCALL_REG2;
		id = offsetof(struct linux_syscall, pacargs);
	}
        syscall.args[0] = regs->LINUX_SYSCALL_NR;
        syscall.args[1] = regs->LINUX_SYSCALL_REG1;
        syscall.args[3] = regs->LINUX_SYSCALL_REG3;
        syscall.args[4] = regs->LINUX_SYSCALL_REG4;
        syscall.args[5] = regs->LINUX_SYSCALL_REG5;
        syscall.args[6] = regs->LINUX_SYSCALL_REG6;
        syscall.id      = from.id;
        syscall.mode    = from.mode;
        syscall.cbfun   = from.cbfun;
        rt_copy_to_user(&syscalls->syscall[in].args, &syscall, id);
	id = from.id;
	if (++from.id < 0) {
		from.id = 0;
	}
	rt_put_user(from.id, &syscalls->id);
	rt_put_user(from.in, &syscalls->in);
	if (from.serv->suspdepth >= -from.nr) {
		from.serv->priority = rt_current->priority + BASE_SOFT_PRIORITY;
		rt_task_resume(from.serv);
	}
	if (from.mode == SYNC_LINUX_SYSCALL) {
		rt_task_suspend(rt_current);
		rt_get_user(regs->LINUX_SYSCALL_RETREG, &syscalls->syscall[in].retval);
	} else {
		regs->LINUX_SYSCALL_RETREG = id;
	}
	return;
}
#endif

/* ++++++++++++++++++++ END OF COMMON FUNCTIONALITIES +++++++++++++++++++++++ */

#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>
#include <rtai_nam2num.h>

extern struct proc_dir_entry *rtai_proc_root;

/* ----------------------< proc filesystem section >----------------------*/

static int PROC_READ_FUN(rtai_read_lxrt)
{
	struct rt_registry_entry entry;
	char *type_name[] = { "TASK", "SEM", "RWL", "SPL", "MBX", "PRX", "HPCK" };
	unsigned int i = 1;
	char name[8];
	PROC_PRINT_VARS;

	PROC_PRINT("\nRTAI LXRT Information.\n\n");
	PROC_PRINT("    MAX_SLOTS = %d\n\n", MAX_SLOTS);
	PROC_PRINT("                                         Linux_Owner         Parent PID\n");
	PROC_PRINT("Slot Name   ID         Type   RT_Handle    Pointer   Tsk_PID   MEM_Sz   USG Cnt\n");
	PROC_PRINT("-------------------------------------------------------------------------------\n");
	for (i = 1; i <= max_slots; i++) {
		if (rt_get_registry_slot(i, &entry)) {
			num2nam(entry.name, name);
			PROC_PRINT("%4d %-6.6s 0x%08lx %-6.6s 0x%p 0x%p  %7d   %8d %7d\n",
			i,    			// the slot number
			name,       		// the name in 6 char asci
			entry.name, 		// the name as unsigned long hex
			entry.type >= PAGE_SIZE ? "SHMEM" : 
			entry.type > sizeof(type_name)/sizeof(char *) ? 
			"ALIEN" : 
			type_name[entry.type],	// the Type
			entry.adr,		// The RT Handle
			entry.tsk,   		// The Owner task pointer
			entry.tsk ? entry.tsk->pid : 0,	// The Owner PID
			entry.type == IS_TASK && ((RT_TASK *)entry.adr)->lnxtsk ? (((RT_TASK *)entry.adr)->lnxtsk)->pid : entry.type >= PAGE_SIZE ? entry.type : 0, entry.count);
		 }
	}
        PROC_PRINT_DONE;
}  /* End function - rtai_read_lxrt */

PROC_READ_OPEN_OPS(rtai_lxrt_fops, rtai_read_lxrt);

int rtai_proc_lxrt_register(void)
{
	struct proc_dir_entry *proc_lxrt_ent;

	proc_lxrt_ent = CREATE_PROC_ENTRY("names", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root, &rtai_lxrt_fops);
	if (!proc_lxrt_ent) {
		printk("Unable to initialize /proc/rtai/lxrt\n");
		return(-1);
	}
	SET_PROC_READ_ENTRY(proc_lxrt_ent, rtai_read_lxrt);
	return(0);
}  /* End function - rtai_proc_lxrt_register */


void rtai_proc_lxrt_unregister(void)
{
	remove_proc_entry("names", rtai_proc_root);
}  /* End function - rtai_proc_lxrt_unregister */

/* ------------------< end of proc filesystem section >------------------*/

EXPORT_SYMBOL(rt_set_sched_policy);
EXPORT_SYMBOL(rt_get_prio);
EXPORT_SYMBOL(rt_get_inher_prio);
EXPORT_SYMBOL(rt_get_priorities);
EXPORT_SYMBOL(rt_change_prio);
EXPORT_SYMBOL(rt_whoami);
EXPORT_SYMBOL(rt_task_yield);
EXPORT_SYMBOL(rt_task_suspend);
EXPORT_SYMBOL(rt_task_suspend_if);
EXPORT_SYMBOL(rt_task_suspend_until);
EXPORT_SYMBOL(rt_task_suspend_timed);
EXPORT_SYMBOL(rt_task_resume);
EXPORT_SYMBOL(rt_get_task_state);
EXPORT_SYMBOL(rt_linux_use_fpu);
EXPORT_SYMBOL(rt_task_use_fpu);
EXPORT_SYMBOL(rt_task_signal_handler);
EXPORT_SYMBOL(rt_gettimeorig);
EXPORT_SYMBOL(rt_task_make_periodic_relative_ns);
EXPORT_SYMBOL(rt_task_make_periodic);
EXPORT_SYMBOL(rt_task_wait_period);
EXPORT_SYMBOL(rt_task_set_resume_end_times);
EXPORT_SYMBOL(rt_set_resume_time);
EXPORT_SYMBOL(rt_set_period);
EXPORT_SYMBOL(next_period);
EXPORT_SYMBOL(rt_busy_sleep);
EXPORT_SYMBOL(rt_sleep);
EXPORT_SYMBOL(rt_sleep_until);
EXPORT_SYMBOL(rt_task_masked_unblock);
EXPORT_SYMBOL(rt_nanosleep);
EXPORT_SYMBOL(rt_enq_ready_edf_task);
EXPORT_SYMBOL(rt_enq_ready_task);
EXPORT_SYMBOL(rt_renq_ready_task);
EXPORT_SYMBOL(rt_rem_ready_task);
EXPORT_SYMBOL(rt_rem_ready_current);
EXPORT_SYMBOL(rt_enq_timed_task);
EXPORT_SYMBOL(rt_wake_up_timed_tasks);
EXPORT_SYMBOL(rt_rem_timed_task);
EXPORT_SYMBOL(rt_enqueue_blocked);
EXPORT_SYMBOL(rt_dequeue_blocked);
EXPORT_SYMBOL(rt_renq_current);
EXPORT_SYMBOL(rt_named_task_init);
EXPORT_SYMBOL(rt_named_task_init_cpuid);
EXPORT_SYMBOL(rt_named_task_delete);
EXPORT_SYMBOL(is_process_registered);
EXPORT_SYMBOL(rt_register);
EXPORT_SYMBOL(rt_drg_on_name);
EXPORT_SYMBOL(rt_drg_on_adr);
EXPORT_SYMBOL(rt_get_name);
EXPORT_SYMBOL(rt_get_adr);
EXPORT_SYMBOL(rt_get_type);
EXPORT_SYMBOL(rt_drg_on_name_cnt);
EXPORT_SYMBOL(rt_drg_on_adr_cnt);
EXPORT_SYMBOL(rt_get_adr_cnt);
EXPORT_SYMBOL(rt_get_registry_slot);

EXPORT_SYMBOL(rt_task_init);
EXPORT_SYMBOL(rt_task_init_cpuid);
EXPORT_SYMBOL(rt_set_runnable_on_cpus);
EXPORT_SYMBOL(rt_set_runnable_on_cpuid);
EXPORT_SYMBOL(rt_check_current_stack);
EXPORT_SYMBOL(rt_schedule);
EXPORT_SYMBOL(rt_spv_RMS);
EXPORT_SYMBOL(rt_sched_lock);
EXPORT_SYMBOL(rt_sched_unlock);
EXPORT_SYMBOL(rt_task_delete);
EXPORT_SYMBOL(rt_is_hard_timer_running);
EXPORT_SYMBOL(rt_set_periodic_mode);
EXPORT_SYMBOL(rt_set_oneshot_mode);
EXPORT_SYMBOL(rt_get_timer_cpu);
EXPORT_SYMBOL(start_rt_timer);
EXPORT_SYMBOL(stop_rt_timer);
EXPORT_SYMBOL(start_rt_apic_timers);
EXPORT_SYMBOL(rt_hard_timer_tick_count);
EXPORT_SYMBOL(rt_hard_timer_tick_count_cpuid);
EXPORT_SYMBOL(rt_set_task_trap_handler);
EXPORT_SYMBOL(rt_get_time);
EXPORT_SYMBOL(rt_get_time_cpuid);
EXPORT_SYMBOL(rt_get_time_ns);
EXPORT_SYMBOL(rt_get_time_ns_cpuid);
EXPORT_SYMBOL(rt_get_cpu_time_ns);
EXPORT_SYMBOL(rt_get_real_time);
EXPORT_SYMBOL(rt_get_real_time_ns);
EXPORT_SYMBOL(rt_get_base_linux_task);
EXPORT_SYMBOL(rt_alloc_dynamic_task);
EXPORT_SYMBOL(count2nano);
EXPORT_SYMBOL(nano2count);
EXPORT_SYMBOL(count2nano_cpuid);
EXPORT_SYMBOL(nano2count_cpuid);

EXPORT_SYMBOL(rt_smp_linux_task);
EXPORT_SYMBOL(rt_smp_current);
EXPORT_SYMBOL(rt_smp_time_h);
EXPORT_SYMBOL(wake_up_srq);
EXPORT_SYMBOL(set_rt_fun_entries);
EXPORT_SYMBOL(reset_rt_fun_entries);
EXPORT_SYMBOL(set_rt_fun_ext_index);
EXPORT_SYMBOL(reset_rt_fun_ext_index);
EXPORT_SYMBOL(max_slots);
