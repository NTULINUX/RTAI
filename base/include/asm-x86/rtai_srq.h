/*
 * Copyright (C) 1999-2013 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef _RTAI_ASM_X86_SRQ_H
#define _RTAI_ASM_X86_SRQ_H

#ifndef __KERNEL__

#include <sys/syscall.h>
#include <unistd.h>

#ifdef CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL
#define USE_LINUX_SYSCALL
#else
#undef USE_LINUX_SYSCALL
#endif

#define RTAI_SRQ_SYSCALL_NR  0x70000001

static inline long long rtai_srq(long srq, unsigned long args)
{
	long long retval;
        syscall(RTAI_SRQ_SYSCALL_NR, srq, args, &retval);
	return retval;
}

static inline int rtai_open_srq(unsigned long label)
{
	return (int)rtai_srq(0, label);
}

#endif /* !__KERNEL__ */

#endif /* !_RTAI_ASM_X86_SRQ_H */
