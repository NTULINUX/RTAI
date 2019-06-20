/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: rtai_cpp.h,v 1.4 2013/10/22 14:54:14 ando Exp $
 *
 * Copyright: (C) 2001,2002 Erwin Rol <erwin@muffin.org>
 *
 * Licence:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef __RTAI_CPP_H__
#define __RTAI_CPP_H__

#ifdef __cplusplus
// for C++ only

#include <rtai_sched.h>

extern RT_TASK*  __rt_task_init(void (*rt_thread)(int), int data,
                                int stack_size, int priority, int uses_fpu,
                                void(*signal)(void));

extern int __rt_task_delete(RT_TASK *task);

#endif

/* for both C and C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern void init_cpp( void  );
extern void cleanup_cpp( void );

#ifdef __cplusplus
}
#endif

#endif 
