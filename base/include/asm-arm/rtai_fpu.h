/*
 * FPU support.
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
 * RTAI/ARM over Adeos rewrite for PXA255_2.6.7:
 *   Copyright (c) 2005 Stefano Gafforelli (stefano.gafforelli@tiscali.it)
 *   Copyright (c) 2005 Luca Pizzi (lucapizzi@hotmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RTAI_ASM_ARM_FPU_H
#define _RTAI_ASM_ARM_FPU_H

#ifdef CONFIG_RTAI_FPU_SUPPORT
#error "Sorry, there is no FPU support in RTAI for ARM (you don't need it for soft-float or FPU-emulation)"
#endif

/* All the work is done by the soft-float library or the kernel FPU emulator. */

#define enable_fpu()
#define save_fpcr_and_enable_fpu(fpcr)
#define restore_fpcr(fpcr)
#define init_hard_fpenv()
#define init_fpenv(fpenv)
#define save_fpenv(fpenv)
#define restore_fpenv(fpenv)
#define init_hard_fpu(lnxtsk)
#define init_fpu(lnxtsk)
#define restore_fpu(lnxtsk)

typedef struct arm_fpu_env { unsigned long fpu_reg[1]; } FPU_ENV;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define set_lnxtsk_uses_fpu(lnxtsk) \
	do { (lnxtsk)->used_math = 1; } while(0)
#define clear_lnxtsk_uses_fpu(lnxtsk) \
	do { (lnxtsk)->used_math = 0; } while(0)
#define lnxtsk_uses_fpu(lnxtsk)  ((lnxtsk)->used_math)

#define set_lnxtsk_using_fpu(lnxtsk) \
	do { (lnxtsk)->flags |= PF_USEDFPU; } while(0)

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

#define set_lnxtsk_uses_fpu(lnxtsk) \
	do { (lnxtsk)->used_math = 1; } while(0)
#define clear_lnxtsk_uses_fpu(lnxtsk) \
	do { (lnxtsk)->used_math = 0; } while(0)
#define lnxtsk_uses_fpu(lnxtsk)  ((lnxtsk)->used_math)

#define set_lnxtsk_using_fpu(lnxtsk) \
	do { (lnxtsk)->thread_info->status |= TS_USEDFPU; } while(0)

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11) */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)

#define set_lnxtsk_uses_fpu(lnxtsk) \
	do { set_stopped_child_used_math(lnxtsk); } while(0)
#define clear_lnxtsk_uses_fpu(lnxtsk) \
	do { clear_stopped_child_used_math(lnxtsk); } while(0)
#define lnxtsk_uses_fpu(lnxtsk)  (tsk_used_math(lnxtsk))

#define set_lnxtsk_using_fpu(lnxtsk) \
	do { (lnxtsk)->thread_info->status |= TS_USEDFPU; } while(0)

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11) */

#endif /* _RTAI_ASM_ARM_FPU_H */
