/*
 * Copyright (C) 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_BITS_H
#define _RTAI_BITS_H

#include <rtai_types.h>

#define RT_BITS_MAGIC 0x9ac24448  // nam2num("rtbits")

#define ALL_SET               0
#define ANY_SET               1
#define ALL_CLR               2
#define ANY_CLR               3

#define ALL_SET_AND_ANY_SET   4
#define ALL_SET_AND_ALL_CLR   5
#define ALL_SET_AND_ANY_CLR   6
#define ANY_SET_AND_ALL_CLR   7
#define ANY_SET_AND_ANY_CLR   8
#define ALL_CLR_AND_ANY_CLR   9

#define ALL_SET_OR_ANY_SET   10
#define ALL_SET_OR_ALL_CLR   11
#define ALL_SET_OR_ANY_CLR   12
#define ANY_SET_OR_ALL_CLR   13
#define ANY_SET_OR_ANY_CLR   14
#define ALL_CLR_OR_ANY_CLR   15

#define SET_BITS              0
#define CLR_BITS              1
#define SET_CLR_BITS          2
#define NOP_BITS              3

#define BITS_ERR     (RTE_OBJINV)  // same as semaphores
#define BITS_TIMOUT  (RTE_TIMOUT)  // same as semaphores

struct rt_bits_struct;

#ifdef __KERNEL__

#ifndef __cplusplus

typedef struct rt_bits_struct {

    struct rt_queue queue;  /* <= Must be first in struct. */
    int magic;
    int type;  // to align mask to semaphore count, for easier uspace init
    unsigned long mask;

} BITS;

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

int __rtai_bits_init(void);

void __rtai_bits_exit(void);

void rt_bits_init(struct rt_bits_struct *bits, unsigned long mask);

int rt_bits_delete(struct rt_bits_struct *bits);

RTAI_SYSCALL_MODE unsigned long rt_get_bits(struct rt_bits_struct *bits);

RTAI_SYSCALL_MODE unsigned long rt_bits_reset(struct rt_bits_struct *bits, unsigned long mask);

RTAI_SYSCALL_MODE unsigned long rt_bits_signal(struct rt_bits_struct *bits, int setfun, unsigned long masks);

RTAI_SYSCALL_MODE int _rt_bits_wait(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask, int space);
static inline int rt_bits_wait(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask)
{
	return  _rt_bits_wait(bits, testfun, testmasks, exitfun, exitmasks, resulting_mask, 1);
}

RTAI_SYSCALL_MODE int _rt_bits_wait_if(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask, int space);
static inline int rt_bits_wait_if(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask)
{
	return  _rt_bits_wait_if(bits, testfun, testmasks, exitfun, exitmasks, resulting_mask, 1);
}

RTAI_SYSCALL_MODE int _rt_bits_wait_until(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME time, unsigned long *resulting_mask, int space);
static inline int rt_bits_wait_until(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME time, unsigned long *resulting_mask)
{
	return  _rt_bits_wait_until(bits, testfun, testmasks, exitfun, exitmasks, time, resulting_mask, 1);
}

RTAI_SYSCALL_MODE int _rt_bits_wait_timed(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME delay, unsigned long *resulting_mask, int space);
static inline int rt_bits_wait_timed(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME delay, unsigned long *resulting_mask)
{
	return  _rt_bits_wait_timed(bits, testfun, testmasks, exitfun, exitmasks, delay, resulting_mask, 1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <rtai_lxrt.h>

#define BITSIDX 0

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(struct rt_bits_struct *,rt_bits_init,(unsigned long name, unsigned long mask))
{
	struct { unsigned long name, mask; } arg = { name, mask };
	return (struct rt_bits_struct *)rtai_lxrt(BITSIDX, SIZARG, BITS_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_bits_delete,(struct rt_bits_struct *bits))
{
	struct { struct rt_bits_struct *bits; } arg = { bits };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_DELETE, &arg).i[LOW];
}

RTAI_PROTO(unsigned long, rt_get_bits,(struct rt_bits_struct *bits))
{
	struct { struct rt_bits_struct *bits; } arg = { bits };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_GET, &arg).i[LOW];
}

RTAI_PROTO(unsigned long, rt_bits_reset,(struct rt_bits_struct *bits, unsigned long mask))
{
	struct { struct rt_bits_struct *bits; unsigned long mask; } arg = { bits, mask };
	return (unsigned long)rtai_lxrt(BITSIDX, SIZARG, BITS_RESET, &arg).i[LOW];
}

RTAI_PROTO(unsigned long, rt_bits_signal,(struct rt_bits_struct *bits, int setfun, unsigned long masks))
{
	struct { struct rt_bits_struct *bits; long setfun; unsigned long masks; } arg = { bits, setfun, masks };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_SIGNAL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_bits_wait,(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask))
{
	struct { struct rt_bits_struct *bits; long testfun; unsigned long testmasks; long exitfun; unsigned long exitmasks; unsigned long *resulting_mask; long space; } arg = { bits, testfun, testmasks, exitfun, exitmasks, resulting_mask, 0 };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_WAIT, &arg).i[LOW];
}

RTAI_PROTO(int, rt_bits_wait_if,(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, unsigned long *resulting_mask))
{
	struct { struct rt_bits_struct *bits; long testfun; unsigned long testmasks; long exitfun; unsigned long exitmasks; unsigned long *resulting_mask; long space; } arg = { bits, testfun, testmasks, exitfun, exitmasks, resulting_mask, 0 };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_WAIT_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_bits_wait_until,(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME time, unsigned long *resulting_mask))
{
	struct { struct rt_bits_struct *bits; long testfun; unsigned long testmasks; long exitfun; unsigned long exitmasks; RTIME time; unsigned long *resulting_mask; long space; } arg = { bits, testfun, testmasks, exitfun, exitmasks, time, resulting_mask, 0 };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_WAIT_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_bits_wait_timed,(struct rt_bits_struct *bits, int testfun, unsigned long testmasks, int exitfun, unsigned long exitmasks, RTIME delay, unsigned long *resulting_mask))
{
	struct { struct rt_bits_struct *bits; long testfun; unsigned long testmasks; long exitfun; unsigned long exitmasks; RTIME delay; unsigned long *resulting_mask; long space; } arg = { bits, testfun, testmasks, exitfun, exitmasks, delay, resulting_mask, 0 };
	return rtai_lxrt(BITSIDX, SIZARG, BITS_WAIT_TIMED, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct rt_bits_struct {
    int opaque;
} BITS;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_BITS_H */
