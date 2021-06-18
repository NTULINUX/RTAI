/*
 * Stuff related to hard-realtime in user-space.
 *
 * Original RTAI/x86 layer implementation:
 *   Copyright (c) 2000 Paolo Mantegazza (mantegazza@aero.polimi.it)
 *   Copyright (c) 2000 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2000 Stuart Hughes
 *   and others.
 *
 * RTAI/x86 rewrite over Adeos:
 *   Copyright (c) 2002 Philippe Gerum (rpm@xenomai.org)
 *
 * Original RTAI/ARM RTHAL implementation:
 *   Copyright (c) 2000 Pierre Cloutier (pcloutier@poseidoncontrols.com)
 *   Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
 *   Copyright (c) 2002 Guennadi Liakhovetski DSA GmbH (gl@dsa-ac.de)
 *   Copyright (c) 2002 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
 *   Copyright (c) 2003 Bernard Haible, Marconi Communications
 *   Copyright (c) 2003 Thomas Gleixner (tglx@linutronix.de)
 *   Copyright (c) 2003 Philippe Gerum (rpm@xenomai.org)
 *
 * RTAI/ARM over Adeos rewrite:
 *   Copyright (c) 2004-2005 Michael Neuhauser, Firmix Software GmbH (mike@firmix.at)
 *
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details. You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */
#ifndef _RTAI_ASM_ARM_LXRT_H
#define _RTAI_ASM_ARM_LXRT_H

/* define registers (pt_regs) that hold syscall related information for
 * lxrt_intercept_syscall_prologue() (see entry-common.S:vector_swi &
 * adeos.c:__adeos_enter_syscall() in linux/arch/arm/kernel) */

#define RTAI_SYSCALL_NR      0x70000000		/* syscall number */

#define RTAI_SYSCALL_ARGS    ARM_r0		/* syscall argument */
#define SET_LXRT_RETVAL_IN_SYSCALL(retval) 	/* set long long syscall return value */ \
	(*(long long)&r->r0 = (retval))

#define LINUX_SYSCALL_NR      ARM_ip
#define LINUX_SYSCALL_REG1    ARM_r0
#define LINUX_SYSCALL_REG2    ARM_r1
#define LINUX_SYSCALL_REG3    ARM_r2
#define LINUX_SYSCALL_REG4    ARM_r3
#define LINUX_SYSCALL_REG5    ARM_r4
#define LINUX_SYSCALL_REG6    ARM_r5
#define LINUX_SYSCALL_RETREG  ARM_r0

#define NR_syscalls 322

#define LXRT_DO_IMMEDIATE_LINUX_SYSCALL(regs) \
        do { /* NOP */ } while (0)

/* endianess */
#define LOW  0
#define HIGH 1

/* for scheduler */
#define USE_LINUX_TIMER
#define TIMER_NAME			RTAI_TIMER_NAME
#define TIMER_FREQ			RTAI_TIMER_FREQ
#define TIMER_LATENCY			RTAI_TIMER_LATENCY
#define TIMER_SETUP_TIME		RTAI_TIMER_SETUP_TIME
#define ONESHOT_SPAN \
    (((long long)RTAI_TIMER_MAXVAL * RTAI_TSC_FREQ) / RTAI_TIMER_FREQ)

#define update_linux_timer(cpuid) \
do { \
	if (!IS_FUSION_TIMER_RUNNING()) { \
		hal_pend_uncond(__ipipe_mach_timerint, cpuid); \
	} \
} while (0)		

/* Adeos/ARM calls all event handlers with hw-interrupts enabled (both in threaded
 * and unthreaded mode), so there is no need for RTAI to do it again. */
#define IN_INTERCEPT_IRQ_ENABLE()	do { /* nop */ } while (0)
#define IN_INTERCEPT_IRQ_DISABLE()	do { /* nop */ } while (0)

union rtai_lxrt_t {
    RTIME rt;
    int i[2];
    void *v[2];
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __KERNEL__

#include <asm/system.h>
#include <asm/mmu_context.h>

/* use "extern inline" to force inlining (gcc 3.3.2 does not inline it into
 * lxrt_context_switch() if "static inline" is used) */
extern inline void
_lxrt_context_switch(struct task_struct *prev, struct task_struct *next, int cpuid)
{
	extern void context_switch(void *, void *, void *);
	context_switch(0, prev, next);
}

static inline void kthread_fun_set_jump(struct task_struct *lnxtsk)  { }
static inline void kthread_fun_long_jump(struct task_struct *lnxtsk) { }

#define rt_copy_from_user(a, b, c)  \
	( { int ret = __copy_from_user_inatomic(a, b, c); ret; } )

#define rt_copy_to_user(a, b, c)  \
	( { int ret = __copy_to_user_inatomic(a, b, c); ret; } )

#define rt_put_user  __put_user
#define rt_get_user  __get_user

#define rt_strncpy_from_user(a, b, c)  \
	( { int ret = strncpy_from_user(a, b, c); ret; } )

#else /* !__KERNEL__ */

#ifdef CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL
#define USE_LINUX_SYSCALL
#include <unistd.h>
#else
#undef USE_LINUX_SYSCALL
#include <asm/rtai_vectors.h>
#endif

#define RTAI_SRQ_SYSCALL_NR 0x70000000

static inline long long _rtai_lxrt(long srq, void *args)
{
	long long retval;
#ifdef USE_LINUX_SYSCALL
        syscall(RTAI_SRQ_SYSCALL_NR, srq, args, &retval);
#else
#warning "RTAI_DO_SWI is not working yet. Please configure RTAI with --enable-lxrt-use-linux-syscall."
	retval = RTAI_DO_SWI(RTAI_SYS_VECTOR, (srq), (args));
#endif
	return retval;
}

static inline union rtai_lxrt_t
rtai_lxrt(short int dynx, short int lsize, int srq, void *arg)
{
    union rtai_lxrt_t retval;
    retval.rt = _rtai_lxrt(ENCODE_LXRT_REQ(dynx, srq, lsize), arg);
    return retval;
}

#define rtai_iopl()  do { /* nop */ } while (0)

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_ASM_ARM_LXRT_H */
