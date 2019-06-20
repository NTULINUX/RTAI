/*
 * Support for System Requests (SRQs).
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
#ifndef _RTAI_ASM_ARM_SRQ_H
#define _RTAI_ASM_ARM_SRQ_H

#ifdef CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL
#define USE_LINUX_SYSCALL
#include <unistd.h>
#else
#undef USE_LINUX_SYSCALL
#include <asm/rtai_vectors.h>
#endif

#define RTAI_SRQ_SYSCALL_NR 0x70000000

static inline long long rtai_srq(long srq, unsigned long args)
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

static inline int rtai_open_srq(unsigned int label)
{
    return (int)rtai_srq(0, label);
}

#endif /* _RTAI_ASM_ARM_SRQ_H */
