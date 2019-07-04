/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
 * extensions for user space modules are jointly copyrighted (2000) with:
 * Copyright (C) 2000	Pierre Cloutier <pcloutier@poseidoncontrols.com>,
 * Copyright (C) 2000	Steve Papacharalambous <stevep@zentropix.com>.
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
 */


#ifndef _RTAI_ASM_X86_LXRT_H
#define _RTAI_ASM_X86_LXRT_H

#ifdef __i386__
#include "rtai_lxrt_32.h"
#else
#include "rtai_lxrt_64.h"
#endif

#define RTAI_SYSCALL_NR  0x70000000

#define LOW  0
union rtai_lxrt_t { RTIME rt; long i[2]; void *v[2]; };

#ifdef __KERNEL__

#define TIMER_NAME     RTAI_TIMER_NAME
#define TIMER_FREQ     RTAI_TIMER_FREQ
//#define SCHED_LATENCY  RTAI_SCHED_LATENCY

#define USE_LINUX_TIMER
#define update_linux_timer(cpuid) \
        do { hal_pend_uncond(RTAI_LINUX_TIMER_IRQ, cpuid); } while (0)

#define MAX_TIMER_COUNT  0x7FFFFFFFLL 
#define ONESHOT_SPAN \
	(((MAX_TIMER_COUNT*(RTAI_CLOCK_FREQ/TIMER_FREQ)) <= INT_MAX) ? \
	(MAX_TIMER_COUNT*(RTAI_CLOCK_FREQ/TIMER_FREQ)) : (INT_MAX))

static inline void _lxrt_context_switch (struct task_struct *prev, struct task_struct *next, int cpuid)
{
	extern void *context_switch(void *, void *, void *); // from LINUX
        context_switch(NULL, prev, next);
}

#define rt_copy_from_user(a, b, c)  \
        ( { int ret = __copy_from_user_inatomic(a, b, c); ret; } )
#define rt_copy_to_user(a, b, c)  \
        ( { int ret = __copy_to_user_inatomic(a, b, c); ret; } )

#ifndef CONFIG_RTAI_USE_STACK_ARGS

static inline long rt__do_strncpy_from_user(char *dst, const char *src, long count)
{
	char *c = dst - 1;
	do {
		if (__get_user(*++c, src++)) return -1;
	} while (*c && --count);
        return (c - dst);
}

#define __do_strncpy_from_user(dst, src, count, res) \
	do { res = rt__do_strncpy_from_user(dst, src, count); } while (0)

#endif

static inline long rt_strncpy_from_user(char *dst, const char __user *src, long
count)
{
        long ret;
        __do_strncpy_from_user(dst, src, count, ret);
        return ret;
}
#define rt_put_user  __put_user
#define rt_get_user  __get_user

#else /* !__KERNEL__ */

#include <unistd.h>

static inline union rtai_lxrt_t rtai_lxrt(short int dynx, short int lsize, int srq, void *arg)
{
	union rtai_lxrt_t ret;
	long rep = 0;
	syscall(RTAI_SYSCALL_NR, ENCODE_LXRT_REQ(dynx, srq, lsize), arg, &ret, &rep);
	if (rep) {
	        syscall(RTAI_SYSCALL_NR, ENCODE_LXRT_REQ(dynx, srq, lsize), arg, &ret, &rep);
	}
        return ret;
}

#define rtai_iopl()  do { extern int iopl(int); iopl(3); } while (0)

#endif /* __KERNEL__ */

#endif /* !_RTAI_ASM_X86_LXRT_H */
