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

#ifndef _RTAI_ASM_X86_64_LXRT_H
#define _RTAI_ASM_X86_64_LXRT_H

#include <linux/version.h>

#ifndef NR_syscalls
#define NR_syscalls __NR_syscall_max
#endif

#define RTAI_SYSCALL_NR      0x70000000

#if defined(__KERNEL__)

#define RT_REG_ORIG_AX	   orig_ax
#define RT_REG_SP		sp
#define RT_REG_SS		ss
#define RT_REG_FLAGS	     flags
#define RT_REG_IP		ip
#define RT_REG_CS		cs
#define RT_REG_BP		bp
#define RT_REG_BX		bx
#define RT_REG_CX		cx

#define RTAI_SYSCALL_CODE     di
#define RTAI_SYSCALL_ARGS     si
#define RTAI_SYSCALL_RETPNT   dx

#define LINUX_SYSCALL_NR      RT_REG_ORIG_AX
#define LINUX_SYSCALL_REG1    di
#define LINUX_SYSCALL_REG2    si
#define LINUX_SYSCALL_REG3    dx
#define LINUX_SYSCALL_REG4    r10
#define LINUX_SYSCALL_REG5    r8
#define LINUX_SYSCALL_REG6    r9
#define LINUX_SYSCALL_RETREG  ax
#define LINUX_SYSCALL_FLAGS   RT_REG_FLAGS

#else

#define RT_REG_ORIG_AX	   orig_rax
#define RT_REG_SP		rsp
#define RT_REG_SS		ss
#define RT_REG_FLAGS	     eflags
#define RT_REG_IP		rip
#define RT_REG_CS		cs
#define RT_REG_BP		rbp
#define RT_REG_BX		rbx
#define RT_REG_CX		rcx

#define RTAI_SYSCALL_CODE     rdi
#define RTAI_SYSCALL_ARGS     rsi
#define RTAI_SYSCALL_RETPNT   rdx

#define LINUX_SYSCALL_NR      RT_REG_ORIG_AX
#define LINUX_SYSCALL_REG1    rdi
#define LINUX_SYSCALL_REG2    rsi
#define LINUX_SYSCALL_REG3    rdx
#define LINUX_SYSCALL_REG4    r10
#define LINUX_SYSCALL_REG5    r8
#define LINUX_SYSCALL_REG6    r9
#define LINUX_SYSCALL_RETREG  rax
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

union rtai_lxrt_t { RTIME rt; long i[1]; void *v[1]; };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __KERNEL__

static inline void _lxrt_context_switch (struct task_struct *prev, struct task_struct *next, int cpuid)
{
	extern void *context_switch(void *, void *, void *);
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,16,0)
	prev->thread.fpu_counter = 0;
#else
	prev->fpu_counter = 0;
#endif
	context_switch(NULL, prev, next);
}

#define rt_copy_from_user(a, b, c)  \
	( { int ret = __copy_from_user_inatomic(a, b, c); ret; } )

#define rt_copy_to_user(a, b, c)  \
	( { int ret = __copy_to_user_inatomic(a, b, c); ret; } )

#define rt_put_user  __put_user
#define rt_get_user  __get_user

/*
 * From Linux lib/usercopy.c.
 */

#define __do_strncpy_from_user(dst,src,count,res)			   \
do {									   \
	long __d0, __d1, __d2;						   \
	__asm__ __volatile__(						   \
		"	testq %1,%1\n"					   \
		"	jz 2f\n"					   \
		"0:	lodsb\n"					   \
		"	stosb\n"					   \
		"	testb %%al,%%al\n"				   \
		"	jz 1f\n"					   \
		"	decq %1\n"					   \
		"	jnz 0b\n"					   \
		"1:	subq %1,%0\n"					   \
		"2:\n"							   \
		".section .fixup,\"ax\"\n"				   \
		"3:	movq %5,%0\n"					   \
		"	jmp 2b\n"					   \
		".previous\n"						   \
		".section __ex_table,\"a\"\n"				   \
		"	.align 8\n"					   \
		"	.quad 0b,3b\n"					   \
		".previous"						   \
		: "=r"(res), "=c"(count), "=&a" (__d0), "=&S" (__d1),	   \
		  "=&D" (__d2)						   \
		: "i"(-EFAULT), "0"(count), "1"(count), "3"(src), "4"(dst) \
		: "memory");						   \
} while (0)

static inline long rt_strncpy_from_user(char *dst, const char __user *src, long count)
{
	long res;
	__do_strncpy_from_user(dst, src, count, res);
	return res;
}

#else /* !__KERNEL__ */

#include <sys/syscall.h>
#include <unistd.h>

static union rtai_lxrt_t _rtai_lxrt(long srq, void *arg)
{
	union rtai_lxrt_t retval;
	syscall(RTAI_SYSCALL_NR, srq, arg, &retval);
	return retval;
}

static inline union rtai_lxrt_t rtai_lxrt(long dynx, long lsize, long srq, void *arg)
{
	return _rtai_lxrt(ENCODE_LXRT_REQ(dynx, srq, lsize), arg);
}

#define rtai_iopl()  do { extern int iopl(int); iopl(3); } while (0)

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_ASM_X86_64_LXRT_H */
