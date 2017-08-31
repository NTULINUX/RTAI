/*
 * Copyright (C) 1999-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * extensions for user space modules are jointly copyrighted (2000) with:
 *		Pierre Cloutier <pcloutier@poseidoncontrols.com>,
 *		Steve Papacharalambous <stevep@zentropix.com>.
 *
 *  Unification of x86 architecture:
 *  Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#ifndef _RTAI_ASM_I386_LXRT_H
#define _RTAI_ASM_I386_LXRT_H

#include <linux/version.h>

#define RTAI_SYSCALL_NR  0x70000000

#if defined(__KERNEL__)

#define RT_REG_ORIG_AX	orig_ax
#define RT_REG_SP	     sp
#define RT_REG_FLAGS	  flags
#define RT_REG_IP	     ip
#define RT_REG_CS	     cs
#define RT_REG_BP	     bp

#define RTAI_SYSCALL_CODE     bx
#define RTAI_SYSCALL_ARGS     cx
#define RTAI_SYSCALL_RETPNT   dx

#define LINUX_SYSCALL_NR      RT_REG_ORIG_AX
#define LINUX_SYSCALL_REG1    bx
#define LINUX_SYSCALL_REG2    cx
#define LINUX_SYSCALL_REG3    dx
#define LINUX_SYSCALL_REG4    si
#define LINUX_SYSCALL_REG5    di
#define LINUX_SYSCALL_REG6    RT_REG_BP
#define LINUX_SYSCALL_RETREG  ax
#define LINUX_SYSCALL_FLAGS   RT_REG_FLAGS

#else

#define RT_REG_ORIG_AX	   orig_eax
#define RT_REG_SP		esp
#define RT_REG_FLAGS	     eflags
#define RT_REG_IP		eip
#define RT_REG_CS		xcs
#define RT_REG_BP		ebp

#define RTAI_SYSCALL_CODE     ebx
#define RTAI_SYSCALL_ARGS     ecx
#define RTAI_SYSCALL_RETPNT   edx

#define LINUX_SYSCALL_NR      RT_REG_ORIG_AX
#define LINUX_SYSCALL_REG1    ebx
#define LINUX_SYSCALL_REG2    ecx
#define LINUX_SYSCALL_REG3    edx
#define LINUX_SYSCALL_REG4    esi
#define LINUX_SYSCALL_REG5    edi
#define LINUX_SYSCALL_REG6    RT_REG_BP
#define LINUX_SYSCALL_RETREG  eax
#define LINUX_SYSCALL_FLAGS   RT_REG_FLAGS

#endif

#define LOW  0
#define HIGH 1

#define TIMER_NAME	"APIC"
#define TIMER_TYPE  1
#define HRT_LINUX_TIMER_NAME  "lapic"
#define FAST_TO_READ_TSC
#define TIMER_FREQ	RTAI_FREQ_APIC
#define TIMER_LATENCY     RTAI_LATENCY_APIC
#define TIMER_SETUP_TIME  RTAI_SETUP_TIME_APIC
#define ONESHOT_SPAN      (CPU_FREQ/(CONFIG_RTAI_CAL_FREQS_FACT + 2)) //(0x7FFFFFFFLL*(CPU_FREQ/TIMER_FREQ))
#define USE_LINUX_TIMER
#define update_linux_timer(cpuid) \
	do { hal_pend_uncond(LOCAL_TIMER_IPI, cpuid); } while (0)

union rtai_lxrt_t { RTIME rt; int i[2]; void *v[2]; };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __KERNEL__

static inline void _lxrt_context_switch (struct task_struct *prev, struct task_struct *next, int cpuid)
{
	extern void context_switch(void *, void *, void *);
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,16,0)
	prev->thread.fpu_counter = 0;
#else
	prev->fpu_counter = 0;
#endif
	context_switch(0, prev, next);
}

#include <linux/slab.h>

#define rt_copy_from_user(a, b, c)  \
	( { int ret = __copy_from_user_inatomic(a, b, c); ret; } )
#define rt_copy_to_user(a, b, c)  \
	( { int ret = __copy_to_user_inatomic(a, b, c); ret; } )
#define rt_strncpy_from_user(a, b, c)  \
	( { int ret = strncpy_from_user(a, b, c); ret; } )
#define rt_put_user  __put_user
#define rt_get_user  __get_user

#else /* !__KERNEL__ */

#include <sys/syscall.h>
#include <unistd.h>

static inline union rtai_lxrt_t rtai_lxrt(short int dynx, short int lsize, int srq, void *arg)
{
	union rtai_lxrt_t ret;
	syscall(RTAI_SYSCALL_NR, ENCODE_LXRT_REQ(dynx, srq, lsize), arg, &ret);
	return ret;
}

#define rtai_iopl()  do { extern int iopl(int); iopl(3); } while (0)

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_ASM_I386_LXRT_H */
