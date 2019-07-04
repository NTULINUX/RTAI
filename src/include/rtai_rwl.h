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

#ifndef _RTAI_RWL_H
#define _RTAI_RWL_H

#include <rtai_sem.h>

struct rtai_rwlock;

#ifdef __KERNEL__

#ifndef __cplusplus

typedef struct rtai_rwlock {
    SEM wrmtx,
	wrsem,
	rdsem;
} RWL;

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

RTAI_SYSCALL_MODE int rt_typed_rwl_init(RWL *rwl, int type);

#define rt_rwl_init(rwl)  rt_typed_rwl_init(rwl, RESEM_RECURS)

RTAI_SYSCALL_MODE int rt_rwl_delete(struct rtai_rwlock *rwl);

RTAI_SYSCALL_MODE RWL *_rt_named_rwl_init(unsigned long rwl_name);

RTAI_SYSCALL_MODE int rt_named_rwl_delete(RWL *rwl);

RTAI_SYSCALL_MODE int rt_rwl_rdlock(struct rtai_rwlock *rwl);

RTAI_SYSCALL_MODE int rt_rwl_rdlock_if(struct rtai_rwlock *rwl);

RTAI_SYSCALL_MODE int rt_rwl_rdlock_until(struct rtai_rwlock *rwl, RTIME time);

RTAI_SYSCALL_MODE int rt_rwl_rdlock_timed(struct rtai_rwlock *rwl, RTIME delay);

RTAI_SYSCALL_MODE int rt_rwl_wrlock(struct rtai_rwlock *rwl);

RTAI_SYSCALL_MODE int rt_rwl_wrlock_if(struct rtai_rwlock *rwl);

RTAI_SYSCALL_MODE int rt_rwl_wrlock_until(struct rtai_rwlock *rwl, RTIME time);

RTAI_SYSCALL_MODE int rt_rwl_wrlock_timed(struct rtai_rwlock *rwl, RTIME delay);

RTAI_SYSCALL_MODE int rt_rwl_unlock(struct rtai_rwlock *rwl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define rt_rwl_init(rwl)  rt_typed_rwl_init(rwl, RESEM_RECURS)

RTAI_PROTO(struct rtai_rwlock *, rt_typed_rwl_init,(unsigned long name, int type))
{
        struct { unsigned long name; long type; } arg = { name, type };
       	return (struct rtai_rwlock *)rtai_lxrt(BIDX, SIZARG, LXRT_RWL_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_rwl_delete,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, LXRT_RWL_DELETE, &arg).i[LOW];
}

RTAI_PROTO(struct rtai_rwlock *, rt_named_rwl_init,(const char *name))
{
	struct { unsigned long name; } arg = { nam2num(name) };
       	return (struct rtai_rwlock *)rtai_lxrt(BIDX, SIZARG, NAMED_RWL_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_named_rwl_delete,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, NAMED_RWL_DELETE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_rdlock,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_rdlock_if,(struct rtai_rwlock *rwl))
{
	struct { void *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_rdlock_until,(struct rtai_rwlock *rwl, RTIME time))
{
	struct { struct rtai_rwlock *rwl; RTIME time; } arg = { rwl, time };
	return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_rdlock_timed,(struct rtai_rwlock *rwl, RTIME delay))
{
	struct { struct rtai_rwlock *rwl; RTIME delay; } arg = { rwl, delay };
	return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_wrlock,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_wrlock_if,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_wrlock_until,(struct rtai_rwlock *rwl, RTIME time))
{
	struct { struct rtai_rwlock *rwl; RTIME time; } arg = { rwl, time };
	return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_wrlock_timed,(struct rtai_rwlock *rwl, RTIME delay))
{
	struct { struct rtai_rwlock *rwl; RTIME delay; } arg = { rwl, delay };
	return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_rwl_unlock,(struct rtai_rwlock *rwl))
{
	struct { struct rtai_rwlock *rwl; } arg = { rwl };
	return rtai_lxrt(BIDX, SIZARG, RWL_UNLOCK, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct rtai_rwlock {
    int opaque;
} RWL;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_RWL_H */
