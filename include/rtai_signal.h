/*
 * Copyright (C) 2006 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 *
 */

#ifndef _RTAI_SIGNAL_H_
#define _RTAI_SIGNAL_H_

#define RTAI_SIGNALS_IDX  BIDX

#include <rtai_sched.h>

#define MAXSIGNALS  16

#define SIGNAL_TASK_INIPRIO     0

struct rt_signal_t { unsigned long flags; RT_TASK *sigtask; };

struct sigsuprt_t { RT_TASK *sigtask; RT_TASK *task; long signal; void (*sighdl)(long, RT_TASK *); unsigned long cpuid; };

#ifdef __KERNEL__

#define SIGNAL_ENBIT   0
#define SIGNAL_PNDBIT  1

#define SIGNAL_TASK_STACK_SIZE  8192

RTAI_SYSCALL_MODE int rt_signal_helper(RT_TASK *task);

int rt_request_signal(long signal, void (*sighdl)(long, RT_TASK *));

RTAI_SYSCALL_MODE int rt_request_signal_(RT_TASK *sigtask, RT_TASK *task, long signal);

RTAI_SYSCALL_MODE int rt_release_signal(long signal, RT_TASK *task);

RTAI_SYSCALL_MODE void rt_enable_signal(long signal, RT_TASK *task);

RTAI_SYSCALL_MODE void rt_disable_signal(long signal, RT_TASK *task);

RTAI_SYSCALL_MODE void rt_trigger_signal(long signal, RT_TASK *task);

RTAI_SYSCALL_MODE int rt_wait_signal(RT_TASK *sigtask, RT_TASK *task);

#else /* !__KERNEL__ */

#include <sys/mman.h>

#include <rtai_lxrt.h>

#define SIGNAL_TASK_STACK_SIZE  64*1024

#ifndef __SIGNAL_SUPPORT_FUN__
#define __SIGNAL_SUPPORT_FUN__

static void signal_suprt_fun(struct sigsuprt_t *funarg)
{		
	struct sigtsk_t { RT_TASK *sigtask; RT_TASK *task; };
	struct sigreq_t { RT_TASK *sigtask; RT_TASK *task; long signal; void (*sighdl)(int, RT_TASK *); };
	struct sigsuprt_t arg = *funarg;

	if ((arg.sigtask = rt_thread_init(rt_get_name(0), SIGNAL_TASK_INIPRIO, 0, SCHED_FIFO, 1 << arg.cpuid))) {
		if (!rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigreq_t), RT_SIGNAL_REQUEST, &arg).i[LOW]) {
			rt_grow_and_lock_stack(SIGNAL_TASK_STACK_SIZE/2);
			rt_make_hard_real_time();
			while (rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigtsk_t), RT_SIGNAL_WAITSIG, &arg).i[LOW]) {
				arg.sighdl(arg.signal, arg.task);
			}
			rt_make_soft_real_time();
		}
		rt_task_delete(arg.sigtask);
	}
}

#endif /* __SIGNAL_SUPPORT_FUN__ */

RTAI_PROTO(int, rt_request_signal, (long signal, void (*sighdl)(long, RT_TASK *)))
{
	if (signal >= 0 && sighdl) {
		struct sigsuprt_t arg = { NULL, rt_buddy(), signal, sighdl };
		arg.cpuid = rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(void *), RT_SIGNAL_HELPER, &arg.sigtask).i[LOW];
		if (rt_thread_create((void *)signal_suprt_fun, &arg, SIGNAL_TASK_STACK_SIZE)) {
			return rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(RT_TASK *), RT_SIGNAL_HELPER, &arg.task).i[LOW];
		}
	}
	return -EINVAL;
}

RTAI_PROTO(int, rt_release_signal, (long signal, RT_TASK *task))
{
	struct { long signal; RT_TASK *task; } arg = { signal, task };
	return rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, RT_SIGNAL_RELEASE, &arg).i[LOW];
}

RTAI_PROTO(void, rt_enable_signal, (long signal, RT_TASK *task))
{
	struct { long signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, RT_SIGNAL_ENABLE, &arg);
}

RTAI_PROTO(void, rt_disable_signal, (long signal, RT_TASK *task))
{
	struct { long signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, RT_SIGNAL_DISABLE, &arg);
}

RTAI_PROTO(void, rt_trigger_signal, (long signal, RT_TASK *task))
{
	struct { long signal; RT_TASK *task; } arg = { signal, task };
	rtai_lxrt(RTAI_SIGNALS_IDX, SIZARG, RT_SIGNAL_TRIGGER, &arg);
}

#endif /* __KERNEL__ */

#endif /* !_RTAI_SIGNAL_H_ */
