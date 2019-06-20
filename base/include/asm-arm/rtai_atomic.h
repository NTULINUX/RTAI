/*
 * Atomic operations.
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
 *  RTAI/ARM over Adeos rewrite for PXA255_2.6.7:
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
#ifndef _RTAI_ASM_ARM_ATOMIC_H
#define _RTAI_ASM_ARM_ATOMIC_H

#include <asm/atomic.h>

#ifdef __KERNEL__

#include <linux/bitops.h>
#include <asm/system.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

#define atomic_xchg(ptr,v)      xchg(ptr,v)

/* Poor man's cmpxchg(). */
#define atomic_cmpxchg(p, o, n) ({	\
	typeof(*(p)) __o = (o);		\
	typeof(*(p)) __n = (n);		\
	typeof(*(p)) __prev;		\
	unsigned long flags;		\
	rtai_hw_lock(flags);		\
	__prev = *(p);			\
	if (__prev == __o)		\
		*(p) = __n;		\
	rtai_hw_unlock(flags);		\
	__prev; })

#endif /* version < 2.6.11 */

#else /* !__KERNEL__ */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <asm/proc/system.h>
#else
#include <asm/system.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
typedef struct { volatile int counter; } atomic_t;
#endif

static inline unsigned long
atomic_xchg(volatile void *ptr, unsigned long x)
{
    asm volatile(
	"swp %0, %1, [%2]"
	: "=&r" (x)
	: "r" (x), "r" (ptr)
	: "memory"
    );
    return x;
}

static inline unsigned long atomic_cmpxchg(volatile void *ptr, unsigned long old, unsigned long new)
{
	unsigned long oldval, res;

	do {
		__asm__ __volatile__("@ atomic_cmpxchg\n"
		"ldrex	%1, [%2]\n"
		"teq	%1, %3\n"
		"strexeq %0, %4, [%2]\n"
		    : "=&r" (res), "=&r" (oldval)
		    : "r" (*(unsigned long*)ptr), "r" (old), "r" (new)
		    : "cc");
	} while (res);

	return oldval;
}

/*
static inline unsigned long
atomic_cmpxchg(volatile void *ptr, unsigned long o, unsigned long n)
{
    unsigned long prev;
    unsigned long flags;
    hal_hw_local_irq_save(flags);
    prev = *(unsigned long*)ptr;
    if (prev == o)
	*(unsigned long*)ptr = n;
    hal_hw_local_irq_restore(flags);
    return prev;
}
*/

static inline int atomic_add_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__("@ atomic_add_return\n"
"1:	ldrex	%0, [%2]\n"
"	add	%0, %0, %3\n"
"	strex	%1, %0, [%2]\n"
"	teq	%1, #0\n"
"	bne	1b"
	: "=&r" (result), "=&r" (tmp)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}

static inline int atomic_sub_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	__asm__ __volatile__("@ atomic_sub_return\n"
"1:	ldrex	%0, [%2]\n"
"	sub	%0, %0, %3\n"
"	strex	%1, %0, [%2]\n"
"	teq	%1, #0\n"
"	bne	1b"
	: "=&r" (result), "=&r" (tmp)
	: "r" (&v->counter), "Ir" (i)
	: "cc");

	return result;
}

#define atomic_inc(v)		(void) atomic_add_return(1, v)
#define atomic_dec_and_test(v)	(atomic_sub_return(1, v) == 0)

#endif /* __KERNEL__ */
#endif /* !_RTAI_ASM_ARM_ATOMIC_H */
