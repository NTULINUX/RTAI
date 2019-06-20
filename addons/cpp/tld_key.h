/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: tld_key.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#ifndef __TLD_KEY_H__
#define __TLD_KEY_H__

#include "rtai_sched.h"

#ifdef __cplusplus
extern "C"  {
#endif

extern int cpp_key;       

#if 0
void rt_get_global_lock(void);

void rt_release_global_lock(void);
#endif

int __rt_tld_create_key(void);

int __rt_tld_free_key(int key);

int __rt_tld_set_data(RT_TASK *task,int key,void* data);

void * __rt_tld_get_data(RT_TASK *task,int key);


#ifdef __cplusplus
}
#endif

#endif 
