/*
 * Copyright (C) 2007-2015 Paolo Mantegazza <mantegazza@aero.polimi.it>.
 * Copyright (C) 2003 Philippe Gerum <rpm@xenomai.org>.
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

#ifndef _RTAI_ASM_X86_64_ATOMIC_H
#define _RTAI_ASM_X86_64_ATOMIC_H

#ifndef __KERNEL__

#ifdef CONFIG_SMP
#define LOCK_PREFIX "lock ; "
#else
#define LOCK_PREFIX ""
#endif

typedef struct { volatile int counter; } atomic_t;

struct __rtai_xchg_dummy { unsigned long a[100]; };
#define __rtai_xg(x) ((struct __rtai_xchg_dummy *)(x))

static inline unsigned long atomic_xchg (volatile void *ptr, unsigned long x)
{
    __asm__ __volatile__(LOCK_PREFIX "xchgq %0,%1"
			 :"=r" (x)
			 :"m" (*__rtai_xg(ptr)), "0" (x)
			 :"memory");
    return x;
}

static inline unsigned long atomic_cmpxchg (volatile void *ptr, unsigned long o, unsigned long n)
{
    unsigned long prev;

    __asm__ __volatile__(LOCK_PREFIX "cmpxchgq %1,%2"
			 : "=a"(prev)
			 : "q"(n), "m" (*__rtai_xg(ptr)), "0" (o)
			 : "memory");

    return prev;
}

static __inline__ int atomic_dec_and_test(atomic_t *v)
{
        unsigned char c;

        __asm__ __volatile__(
                LOCK_PREFIX "decl %0; sete %1"
                :"=m" (v->counter), "=qm" (c)
                :"m" (v->counter) : "memory");
        return c != 0;
}

static __inline__ void atomic_inc(atomic_t *v)
{
        __asm__ __volatile__(
                LOCK_PREFIX "incl %0"
                :"=m" (v->counter)
                :"m" (v->counter));
}

/* Depollute the namespace a bit. */
#undef ADDR

#endif /* __KERNEL__ */

#endif /* !_RTAI_ASM_X86_64_ATOMIC_H */
