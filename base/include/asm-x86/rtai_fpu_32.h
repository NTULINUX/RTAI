/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Part of this code acked from Linux i387.h and i387.c:
 * Copyright (C) 1994 Linus Torvalds,
 * Copyright (C) 2000 Gareth Hughes <gareth@valinux.com>,
 * original idea of an RTAI own header file for the FPU stuff:
 * Copyright (C) 2000 Pierre Cloutier <pcloutier@PoseidonControls.com>,
 * this final rewrite and cleanup:
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>.
 * The real final cleanup:
 * Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>.
 */

#ifndef _RTAI_ASM_I386_FPU_H
#define _RTAI_ASM_I386_FPU_H

#ifndef __cplusplus
#include <asm/processor.h>
#endif /* !__cplusplus */

typedef union thread_xstate FPU_ENV;
#define TASK_FPENV(tsk)  ((tsk)->thread.fpu.state)

/* RAW FPU MANAGEMENT FOR USAGE FROM WHAT/WHEREVER RTAI DOES IN KERNEL */

#define enable_fpu()  do { \
	__asm__ __volatile__ ("clts"); \
} while(0)

#define save_fpcr_and_enable_fpu(fpcr)  do { \
	__asm__ __volatile__ ("movl %%cr0, %0; clts": "=r" (fpcr)); \
} while (0)

#define restore_fpcr(fpcr)  do { \
	if (fpcr & 8) { \
		__asm__ __volatile__ ("movl %%cr0, %0": "=r" (fpcr)); \
		__asm__ __volatile__ ("movl %0, %%cr0": :"r" (fpcr | 8)); \
	} \
} while (0)

/* initialise the hard fpu unit directly */
#define init_hard_fpenv() do { \
	__asm__ __volatile__ ("clts; fninit"); \
	if (cpu_has_xmm) { \
		unsigned long __mxcsr = (0xffbfu & 0x1f80u); \
		__asm__ __volatile__ ("ldmxcsr %0": : "m" (__mxcsr)); \
	} \
} while (0)

/* initialise the given fpenv union, without touching the related hard fpu unit */
#define __init_fpenv(fpenv)  do { \
	if (cpu_has_fxsr) { \
		memset(&(fpenv)->fxsave, 0, sizeof(struct i387_fxsave_struct));\
		(fpenv)->fxsave.cwd = 0x37f; \
		if (cpu_has_xmm) { \
			(fpenv)->fxsave.mxcsr = 0x1f80; \
		} \
	} else { \
		memset(&(fpenv)->fsave, 0, sizeof(struct i387_fsave_struct)); \
		(fpenv)->fsave.cwd = 0xffff037fu; \
		(fpenv)->fsave.swd = 0xffff0000u; \
		(fpenv)->fsave.twd = 0xffffffffu; \
		(fpenv)->fsave.fos = 0xffff0000u; \
	} \
} while (0)

#define __save_fpenv(fpenv)  do { \
	if (cpu_has_fxsr) { \
		__asm__ __volatile__ ("fxsave %0; fnclex": "=m" ((fpenv)->fxsave)); \
	} else { \
		__asm__ __volatile__ ("fnsave %0; fwait": "=m" ((fpenv)->fsave)); \
	} \
} while (0)

#define __restore_fpenv(fpenv)  do { \
	if (cpu_has_fxsr) { \
		__asm__ __volatile__ ("fxrstor %0": : "m" ((fpenv)->fxsave)); \
	} else { \
		__asm__ __volatile__ ("frstor %0": : "m" ((fpenv)->fsave)); \
	} \
} while (0)

/* FPU MANAGEMENT DRESSED FOR IN KTHREAD/THREAD/PROCESS FPU USAGE FROM RTAI */

/* Macros used for RTAI own kernel space tasks, where it uses the FPU env union */
#define init_fpenv(fpenv)  do { __init_fpenv(&(fpenv)); } while (0)
#define save_fpenv(fpenv)  do { __save_fpenv(&(fpenv)); } while (0)
#define restore_fpenv(fpenv)  do { __restore_fpenv(&(fpenv)); } while (0)

/* Macros used for user space, where Linux might use eother a pointer or the FPU env union */
#define init_hard_fpu(lnxtsk)  do { \
	init_hard_fpenv(); \
	set_lnxtsk_uses_fpu(lnxtsk); \
	set_lnxtsk_using_fpu(lnxtsk); \
} while (0)

#define init_fpu(lnxtsk)  do { \
	__init_fpenv(TASK_FPENV(lnxtsk)); \
	set_lnxtsk_uses_fpu(lnxtsk); \
} while (0)

#define restore_fpu(lnxtsk)  do { \
	enable_fpu(); \
	__restore_fpenv(TASK_FPENV(lnxtsk)); \
	set_lnxtsk_using_fpu(lnxtsk); \
} while (0)

#define set_lnxtsk_uses_fpu(lnxtsk) \
	do { set_stopped_child_used_math(lnxtsk); } while(0)
#define clear_lnxtsk_uses_fpu(lnxtsk) \
	do { clear_stopped_child_used_math(lnxtsk); } while(0)
#define lnxtsk_uses_fpu(lnxtsk)  (tsk_used_math(lnxtsk))

#undef init_fpu
#include <asm/i387.h>
#include <asm/fpu-internal.h>
#define rtai_set_fpu_used(lnxtsk) __thread_set_has_fpu(lnxtsk)

#define set_lnxtsk_using_fpu(lnxtsk) \
	do { rtai_set_fpu_used(lnxtsk); } while(0)

#endif /* !_RTAI_ASM_I386_FPU_H */
