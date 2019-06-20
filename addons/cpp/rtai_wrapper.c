/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: rtai_wrapper.c,v 1.4 2013/10/22 14:54:14 ando Exp $
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
#include "rtai_wrapper.h"

#include <rtai.h>
#include <rtai_malloc.h>
#include "tld_key.h"

void __rt_get_global_lock(void){
	rt_get_global_lock();
}
 
void __rt_release_global_lock(void){
	rt_release_global_lock();
}

int __hard_cpu_id( void ){
	return hard_cpu_id();
}

/* task functions */

RT_TASK * __rt_task_init(void (*rt_thread)(int), int data,
                         int stack_size, int priority, int uses_fpu,
                          void(*signal)(void))
{
        RT_TASK * task;

        task = rt_malloc( sizeof(RT_TASK) );

        if(task == 0)
                return 0;

        memset(task,0,sizeof(RT_TASK));

        rt_task_init(task,rt_thread,data,stack_size,priority,uses_fpu,signal);

       __rt_tld_set_data(task,cpp_key,(void*)data);

       return task;
}

int __rt_task_delete(RT_TASK *task)
{
        int result;
        rt_printk("__rt_task_delete(%p)\n",task);

        if(task == 0)
                return -1;

        rt_task_suspend(task);

        result = rt_task_delete(task);

        rt_free(task);

        return result;
}

#ifdef CONFIG_RTAI_TRACE

void __trace_destroy_event( int id ){
    trace_destroy_event( id );
}

int __trace_create_event( const char* name, void* p){
    return trace_create_event( (char *)name, NULL, CUSTOM_EVENT_FORMAT_TYPE_NONE, (char *)p);
}

int __trace_raw_event( int id, int size, void* p){
    return trace_raw_event( id, size, p);
}

#endif /* CONFIG_RTAI_TRACE */
