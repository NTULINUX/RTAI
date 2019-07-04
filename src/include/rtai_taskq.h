/*
 * Copyright (C) 2008-2010 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#ifndef _RTAI_TASKQ_H
#define _RTAI_TASKQ_H

#include <rtai_schedcore.h>

extern struct epoch_struct boot_epoch;

typedef struct rt_task_queue {
	struct rt_queue queue; /* <= Must be first in struct. */
	int qtype;
	unsigned long status;
} TASKQ;

#define XNTIMEO  0x00000001  // RTE_TIMOUT
#define XNRMID   0x00000002  // RTE_OBJREM
#define XNBREAK  0x00000004  // RTE_UNBLKD

#define TASKQ_PRIO    0x0
#define TASKQ_FIFO    0x1

#define RT_SCHED_TASKQ  RT_SCHED_SEMAPHORE

#ifdef CONFIG_SMP

extern volatile unsigned long tosched_mask;

#define TOSCHED_TASK(task) \
	do { tosched_mask |= (1 << (task)->runnable_on_cpus); } while (0)

#else /* !CONFIG_SMP */

#define TOSCHED_TASK(task)

#endif /* CONFIG_SMP */

void rt_schedule_readied(void);

void rt_taskq_init(TASKQ *taskq, unsigned int type);

RT_TASK *rt_taskq_ready_one(TASKQ *taskq);

int rt_taskq_ready_all(TASKQ *taskq, unsigned long why);

void rt_taskq_wait(TASKQ *taskq);

void rt_taskq_wait_until(TASKQ *taskq, RTIME time);

#define rt_taskq_delete(taskq)  rt_taskq_ready_all(taskq, RTE_OBJREM)

#define rt_taskq_wait_timed(taskq, delay) \
	do { rt_taskq_wait_until(taskq, get_time() + delay); } while (0)

static inline int rt_task_test_taskq_retval(RT_TASK *task, unsigned long mask)
{
	return ((task)->retval & mask); 
}

#endif /* !_RTAI_TASKQ_H */
