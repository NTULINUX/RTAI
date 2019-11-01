/*
 * Copyright (C) 2002,2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_SPL_H
#define _RTAI_SPL_H

#include <rtai_sem.h>

struct rtai_spl;

#ifdef __KERNEL__

#ifndef __cplusplus

typedef struct rtai_spl {
    void *owndby;
    int count;
    unsigned long flags;
} SPL;

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

RTAI_SYSCALL_MODE int rt_spl_init(struct rtai_spl *spl);

RTAI_SYSCALL_MODE int rt_spl_delete(struct rtai_spl *spl);

RTAI_SYSCALL_MODE SPL *_rt_named_spl_init(unsigned long spl_name);

RTAI_SYSCALL_MODE int rt_named_spl_delete(SPL *spl);

RTAI_SYSCALL_MODE int rt_spl_lock(struct rtai_spl *spl);

RTAI_SYSCALL_MODE int rt_spl_lock_if(struct rtai_spl *spl);

RTAI_SYSCALL_MODE int rt_spl_lock_timed(struct rtai_spl *spl,
		      unsigned long ns);

RTAI_SYSCALL_MODE int rt_spl_unlock(struct rtai_spl *spl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(struct rtai_spl *, rt_spl_init,(unsigned long name))
{
	struct { unsigned long name; } arg = { name };
	return (struct rtai_spl *)rtai_lxrt(BIDX, SIZARG, LXRT_SPL_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_spl_delete,(struct rtai_spl *spl))
{
	struct { struct rtai_spl *spl; } arg = { spl };
	return rtai_lxrt(BIDX, SIZARG, LXRT_SPL_DELETE, &arg).i[LOW];
}

RTAI_PROTO(struct rtai_spl *, rt_named_spl_init,(const char *name))
{
        struct { unsigned long name; } arg = { nam2num(name) };
       	return (struct rtai_spl *)rtai_lxrt(BIDX, SIZARG, NAMED_SPL_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_named_spl_delete,(struct rtai_spl *spl))
{
	struct { struct rtai_spl *spl; } arg = { spl };
	return rtai_lxrt(BIDX, SIZARG, NAMED_SPL_DELETE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spl_lock,(struct rtai_spl *spl))
{
	struct { struct rtai_spl *spl; } arg = { spl };
	return rtai_lxrt(BIDX, SIZARG, SPL_LOCK, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spl_lock_if,(struct rtai_spl *spl))
{
	struct { struct rtai_spl *spl; } arg = { spl };
	return rtai_lxrt(BIDX, SIZARG, SPL_LOCK_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spl_lock_timed,(struct rtai_spl *spl, RTIME delay))
{
	struct { struct rtai_spl *spl; RTIME delay; } arg = { spl, delay };
	return rtai_lxrt(BIDX, SIZARG, SPL_LOCK_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spl_unlock,(struct rtai_spl *spl))
{
	struct { struct rtai_spl *spl; } arg = { spl };
	return rtai_lxrt(BIDX, SIZARG, SPL_UNLOCK, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct rtai_spl {
    int opaque;
} SPL;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_SPL_H */
