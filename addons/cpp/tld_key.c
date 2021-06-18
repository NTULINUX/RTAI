/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: tld_key.c,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "tld_key.h"
#include "rtai.h"

#if 0
void __rt_get_global_lock(void){
	rt_get_global_lock();
}
 
void __rt_release_global_lock(void){
	rt_release_global_lock();
}

int __hard_cpu_id( void ){
	return hard_cpu_id();
}
#endif


int cpp_key;

// get a new key and mark it in tld_used_mask 
int __rt_tld_create_key(void){
#if 1
	return 0;
#else
	return rt_tld_create_key();
#endif
}

// free the bit in tld_used_mask
int __rt_tld_free_key(int key){
#if 1
	return 0;
#else	
	return rt_tld_free_key(key);
#endif
}

// set the data in the currents RT_TASKS tld_data array
int __rt_tld_set_data(RT_TASK *task,int key,void* data){
#if 1
	task->system_data_ptr = (void*)data;
	return 0;
#else
	return rt_tld_set_data(task,key,data);
#endif
}

// get the data from the current task
void* __rt_tld_get_data(RT_TASK *task,int key){
#if 1
	return (void*)task->system_data_ptr;
#else
	return rt_tld_get_data(task,key);
#endif
}

