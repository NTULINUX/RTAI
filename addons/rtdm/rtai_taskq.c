/*
 * Copyright (C) 2008-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#include "rtai_taskq.h"

#ifdef CONFIG_SMP
volatile unsigned long tosched_mask;
#endif

void rt_schedule_readied(void)
{
	unsigned long flags;
#ifdef CONFIG_SMP
	unsigned long cpumask, rmask, lmask;
	flags = rt_global_save_flags_and_cli();
	lmask = tosched_mask;
	tosched_mask = 0;
	rt_global_restore_flags(flags);
	rmask = lmask & ~(cpumask = (1 << rtai_cpuid()));
	if (rmask) {
		rtai_save_flags_and_cli(flags);
		send_sched_ipi(rmask);
		rtai_restore_flags(flags);
	}
	if (lmask | cpumask)
#endif
	{
		flags = rt_global_save_flags_and_cli();
		rt_schedule();
		rt_global_restore_flags(flags);
	}
}
EXPORT_SYMBOL(rt_schedule_readied);
 
void rt_taskq_init(TASKQ *taskq, unsigned int type)
{
	taskq->qtype = (type & TASKQ_FIFO) ? 1 : 0;
	taskq->queue = (QUEUE) { &taskq->queue, &taskq->queue, NULL };
	taskq->status = 0;
}
EXPORT_SYMBOL(rt_taskq_init);

RT_TASK *rt_taskq_ready_one(TASKQ *taskq)
{
	unsigned long flags;
	RT_TASK *task = NULL;

	flags = rt_global_save_flags_and_cli();
	if ((task = (taskq->queue.next)->task)) {
		dequeue_blocked(task);
		rem_timed_task(task);
		if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_TASKQ | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
			enq_ready_task(task);
			TOSCHED_TASK(task);
		}
	}
	rt_global_restore_flags(flags);
	return task;
}
EXPORT_SYMBOL(rt_taskq_ready_one);

int rt_taskq_ready_all(TASKQ *taskq, unsigned long why)
{
	unsigned long flags, tosched;
	RT_TASK *task;
	QUEUE *q;

	tosched = 0;
	q = &(taskq->queue);
	flags = rt_global_save_flags_and_cli();
	while ((q = q->next) != &(taskq->queue) && (task = q->task)) {
		dequeue_blocked(task = q->task);
		rem_timed_task(task);
		task->retval = why;
		if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_TASKQ | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
			enq_ready_task(task);
			TOSCHED_TASK(task);
			tosched = 1;
		}
		rt_global_restore_flags(flags);
		flags = rt_global_save_flags_and_cli();
	}
	rt_global_restore_flags(flags);
	return tosched;
}
EXPORT_SYMBOL(rt_taskq_ready_all);

void rt_taskq_wait(TASKQ *taskq)
{
	RT_TASK *rt_current;
	unsigned long flags;
	void *retp;

	flags = rt_global_save_flags_and_cli();
	rt_current = RT_CURRENT;
	rt_current->retval = 0;
	rt_current->state |= RT_SCHED_TASKQ;
	rem_ready_current(rt_current);
	enqueue_blocked(rt_current, &taskq->queue, taskq->qtype);
	rt_schedule();
	if (unlikely((retp = rt_current->blocked_on) != NULL)) { 
		if (likely(retp != RTP_OBJREM)) { 
			dequeue_blocked(rt_current);
			rt_current->retval  = XNBREAK;
		} else {
			rt_current->prio_passed_to = NULL;
			rt_current->retval = XNRMID;
		}
	}
	rt_global_restore_flags(flags);
}
EXPORT_SYMBOL(rt_taskq_wait);


void rt_taskq_wait_until(TASKQ *taskq, RTIME time)
{
	DECLARE_RT_CURRENT;
	unsigned long flags;
	void *retp;

	flags = rt_global_save_flags_and_cli();
	ASSIGN_RT_CURRENT;
	rt_current->retval = 0;
	rt_current->blocked_on = &taskq->queue;
	if ((rt_current->resume_time = time) > rt_time_h) {
		rt_current->state |= (RT_SCHED_TASKQ | RT_SCHED_DELAYED);
		rem_ready_current(rt_current);
		enqueue_blocked(rt_current, &taskq->queue, taskq->qtype);
		enq_timed_task(rt_current);
		rt_schedule();
	}
	if (unlikely((retp = rt_current->blocked_on) != NULL)) {
		if (likely(retp != RTP_OBJREM)) {
			dequeue_blocked(rt_current);
			rt_current->retval = retp > RTP_HIGERR ? XNTIMEO : XNBREAK;
		} else {
			rt_current->prio_passed_to = NULL;
			rt_current->retval = XNRMID;
		}
	}
	rt_global_restore_flags(flags);
}
EXPORT_SYMBOL(rt_taskq_wait_until);
