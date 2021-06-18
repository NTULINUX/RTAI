/*
 * Support for user to kernel-space feature.
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
#ifndef _RTAI_ASM_ARM_VECTORS_H
#define _RTAI_ASM_ARM_VECTORS_H

#include <rtai_config.h>

/*
 * ARM SWI assembler instructen = 8 bit SWI op-code + 24 bit info
 *
 * SWI info = 4 bit OS-number (Linux = 9) + 20 bit syscall number
 *
 * SWI info for Linux syscall:  ..9x_xxxx (syscall number: 0_0000 - f_ffff)
 *
 * Linux syscall number range:  ..90_0000 - ..9e_ffff
 * ARM private syscalls:	..9f_0000 - ..9f_ffef
 * RTAI syscalls:		..9f_fff0 - ..9f_ffff
 *
 * Old style "vectors" could be in least signigicant half-byte of SWI info (but
 * currently only one RTAI syscall is used ..9f_fff0).
 */

#define RTAI_NUM_VECTORS	16		/* number of usable vectors */
#define RTAI_SWI_SCNO_MASK	0x00FFFFF0
#define RTAI_SWI_VEC_MASK	0x0000000F

#define RTAI_SYS_VECTOR		0x0		/* only one used (-> LXRT requests and SRQs) */
#if 0
/* shm not implemented yet (trouble with virtual addresses in cache) and when it
 * will be implemented it's unlikely that a seperate vector will be used) */
#define RTAI_SHM_VECTOR		0x2
#endif

/* The "memory" clobber constraint is essential! It forces the compiler to not
 * cache in registers across this call (e.g. data that is written to a user
 * structure by the call might not be (re)read after the call without the
 * constraint. (glibc implements syscalls pretty much the same way) */

#define _RTAI_DO_SWI(scno_magic, srq, parg) ({ 					\
    union {									\
	long long ll;								\
	struct {								\
	    unsigned long low;							\
	    unsigned long high;							\
	} l;									\
    } _retval;									\
    {										\
	register int _r0 asm ("r0") = (int)(srq);				\
	register int _r1 asm ("r1") = (int)(parg);				\
	asm volatile("swi %[nr]"						\
	    : "+r" (_r0), "+r" (_r1)						\
	    : [nr] "i" (scno_magic)						\
	    : "memory"								\
	);									\
	_retval.l.low = _r0;							\
	_retval.l.high = _r1;							\
    }										\
    _retval.ll;									\
})

#ifdef CONFIG_ARCH_PXA
#	define RTAI_SWI_SCNO_MAGIC		0x00404404
#	define RTAI_DO_SWI(vector, srq, parg)	_RTAI_DO_SWI(RTAI_SWI_SCNO_MAGIC, srq, parg)
#else /* !CONFIG_ARCH_PXA */
#	define RTAI_SWI_SCNO_MAGIC		0x009FFFF0
#	define RTAI_DO_SWI(vector, srq, parg)	_RTAI_DO_SWI((RTAI_SWI_SCNO_MAGIC | ((vector) & RTAI_SWI_VEC_MASK)), srq, parg)
#endif /* CONFIG_ARCH_PXA */

#endif /* _RTAI_ASM_ARM_VECTORS_H */
