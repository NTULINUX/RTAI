/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or 
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Part of this code acked from Linux x86 FPU support:
 * Copyright (C) 1994 Linus Torvalds.
 *
 * Pentium III FXSR, SSE support
 * General FPU state handling cleanups
 * Copyright (C)     Gareth Hughes <gareth@valinux.com>, May 2000.
 * x86-64 work by 
 * Copyright (C)	Andi Kleen, 2002.
 *
 * Original idea of an RTAI own header file for the FPU stuff:
 * Copyright (C)     Pierre Cloutier <pcloutier@PoseidonControls.com>, 2000.
 * Following RTAI rewrites:
 * Copyright (C)     Paolo Mantegazza <mantegazza@aero.polimi.it>, 2005-2017.
 * Minor cleanups and further rewrites:
 * Copyright (C)     Alec Ari <neotheuser@ymail.com>, 2019-2022.
 */


#ifndef _RTAI_ASM_X86_FPU_H
#define _RTAI_ASM_X86_FPU_H

typedef union fpregs_state FPU_ENV;
#define TASK_FPENV(tsk)  (&(tsk)->thread.fpu.state)

extern unsigned int fpu_kernel_xstate_size;
#define xstate_size  (fpu_kernel_xstate_size)

// RAW FPU MANAGEMENT FOR USAGE FROM WHAT/WHEREVER RTAI DOES IN KERNEL

#define enable_fpu()  do { \
	__asm__ __volatile__ ("clts"); \
} while(0)

#define save_fpcr_and_enable_fpu(fpcr)  do { \
	fpcr = read_cr0(); \
	enable_fpu(); \
} while (0)

#define restore_fpcr(fpcr)  do { \
	if (fpcr & X86_CR0_TS) { \
		unsigned long flags; \
		rtai_save_flags_and_cli(flags); \
		fpcr = read_cr0(); \
		write_cr0(X86_CR0_TS | fpcr); \
		rtai_restore_flags(flags); \
	} \
} while (0)

/*
 * This code is based off of Linux kernel 4.19.261:
 * arch/x86/include/asm/fpu/internal.h
 */

// initialise the given fpenv union
#define __init_fpenv(fpenv)  do { \
	memset(&(fpenv)->fxsave, 0, xstate_size); \
	(fpenv)->fxsave.cwd = 0x37f; \
	(fpenv)->fxsave.mxcsr = MXCSR_DEFAULT; \
} while (0)

#define __save_fpenv(fpenv) \
({ \
	__asm__ __volatile__ ("fxsaveq %[fx]" : [fx] "=m" ((fpenv)->fxsave)); \
})

#define __restore_fpenv(fpenv) \
({ \
	kernel_insn(fxrstorq %[fx], "=m" (*fpenv), [fx] "m" (*fpenv)); \
})

// Macros used for RTAI own kernel space tasks, where it uses the FPU env union
#define init_fpenv(fpenv)	do { __init_fpenv(&(fpenv)); } while (0)
#define save_fpenv(fpenv)	do { __save_fpenv(&(fpenv)); } while (0)
#define restore_fpenv(fpenv)	do { __restore_fpenv(&(fpenv)); } while (0)

#ifdef DEFINE_FPU_FPREGS_OWNER_CTX
DEFINE_PER_CPU(struct fpu *, fpu_fpregs_owner_ctx);
#endif

// FPU MANAGEMENT DRESSED FOR IN KTHREAD/THREAD/PROCESS FPU USAGE FROM RTAI

// Macros used for user space, where Linux might use either a pointer or the FPU_ENV union
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

#include <asm/fpu/internal.h>
#define rtai_fpregs_activate fpregs_activate

#define rtai_set_fpu_used(lnxtsk) \
	do { rtai_fpregs_activate(&lnxtsk->thread.fpu); } while(0)

#define set_lnxtsk_using_fpu(lnxtsk) \
	do { rtai_set_fpu_used(lnxtsk); } while(0)

#endif /* !_RTAI_ASM_X86_FPU_H */
