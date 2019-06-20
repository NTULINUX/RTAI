/*
 * Development of this (watchdog) module was sponsored by Alcatel, Strasbourg
 * as part of general debugging enhancements to RTAI.
 *
 * Copyright (©) 2001 Ian Soanes <ians@lineo.com>, All rights reserved
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

#ifndef _RTAI_WD_H
#define _RTAI_WD_H

#include <rtai_types.h>

// Useful bits and pieces
#define NSECS_PER_SEC 1000000000

// For logging stuff
#define WDLOG(fmt, args...) rt_printk("RTAI[watchdog]: " fmt, ##args)

// What should happen to misbehaving tasks
typedef enum watchdog_policy { 
    WD_NOTHING, 				// Not recommended
    WD_RESYNC,				// Good for occasional overruns
    WD_DEBUG,				// Good for debugging tasks
    WD_STRETCH,				// Good for overrunning tasks
    WD_SLIP, 				// Good for infinite loops
    WD_SUSPEND, 				// Good for most things
    WD_KILL 				// Death sentence
} wd_policy;

// Information about a misbehaving task
typedef struct bad_rt_task {
    RT_TASK		*task;		// Pointer to the bad task
    int			 in_use;	// Allocated (static memory management)
    int		 	 slipping;	// In the process of being slipped
    int		 	 countdown;	// Watchdog ticks to stay suspended
    int		 	 count;		// How many times it's offended
    int			 valid;		// Still an existing RT task
    int			 forced;	// Forcibly suspended
    RTIME		 orig_period;	// Original period before adjustments
    wd_policy		 policy;	// Policy at time of last offence
    struct bad_rt_task	*next;		// Next in list
} BAD_RT_TASK;

#define WD_INDX          2

#define WD_SET_GRACE     1
#define WD_SET_GRACEDIV  2 
#define WD_SET_SAFETY    3 
#define WD_SET_POLICY    4
#define WD_SET_SLIP      5
#define WD_SET_STRETCH   6
#define WD_SET_LIMIT     7 

#ifdef __KERNEL__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_wd_init(void);

void __rtai_wd_exit(void);

// API for setting parameters
RTAI_SYSCALL_MODE int rt_wdset_grace(int new_value);

RTAI_SYSCALL_MODE int rt_wdset_gracediv(int new_value);

RTAI_SYSCALL_MODE wd_policy rt_wdset_policy(wd_policy new_value);

RTAI_SYSCALL_MODE int rt_wdset_slip(int new_value);

RTAI_SYSCALL_MODE int rt_wdset_stretch(int new_value);

RTAI_SYSCALL_MODE int rt_wdset_limit(int new_value);

RTAI_SYSCALL_MODE int rt_wdset_safety(int new_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// API for setting parameters
RTAI_PROTO(int, rt_wdset_grace, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_GRACE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_wdset_gracediv, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_GRACEDIV, &arg).i[LOW];
}

RTAI_PROTO(wd_policy, rt_wdset_policy, (wd_policy new_value))
{
	struct { long new_value; } arg = { new_value };
	return (wd_policy)rtai_lxrt(WD_INDX, SIZARG, WD_SET_POLICY, &arg).i[LOW];
}

RTAI_PROTO(int, rt_wdset_slip, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_SLIP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_wdset_stretch, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_STRETCH, &arg).i[LOW];
}

RTAI_PROTO(int, rt_wdset_limit, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_LIMIT, &arg).i[LOW];
}

RTAI_PROTO(int, rt_wdset_safety, (int new_value))
{
	struct { long new_value; } arg = { new_value };
	return rtai_lxrt(WD_INDX, SIZARG, WD_SET_SAFETY, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#endif /* !_RTAI_WD_H */
