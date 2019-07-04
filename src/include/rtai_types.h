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

#ifndef _RTAI_TYPES_H
#define _RTAI_TYPES_H

#include <rtai_config.h>
#include <rtai_wrappers.h>

#define PRIO_Q    0 
#define FIFO_Q    4 
#define RES_Q     3

#define BIN_SEM   1 
#define CNT_SEM   2 
#define RES_SEM   3

#define RESEM_RECURS   1
#define RESEM_BINSEM   0
#define RESEM_CHEKWT  -1

#define RT_SCHED_FIFO  0 
#define RT_SCHED_RR    1 

struct pt_regs;

struct rt_task_struct;

typedef long long RTIME;

typedef int (*RT_TRAP_HANDLER)(int, int, struct pt_regs *,void *);

struct rt_times {
	int linux_tick;
	int periodic_tick;
	RTIME tick_time;
	RTIME linux_time;
	RTIME intr_time;
};

#endif /* !_RTAI_TYPES_H */
