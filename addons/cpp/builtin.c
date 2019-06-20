/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: builtin.c,v 1.4 2013/10/22 14:54:14 ando Exp $
 *
 * Copyright: (C) 2001,2002 Erwin Rol <erwin@muffin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
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
 

#include <rtai_sched.h>
#include <rtai_malloc.h>

#if __GNUC__ < 3

void
__builtin_delete(void* vp){
	rt_printk("__builtin_delete %p\n",vp);
	if(vp != 0)
		rt_free(vp);
}

void*
__builtin_new(int size){
	void* vp = rt_malloc(size);
	rt_printk("__builtin_new %p %d\n",vp,size);
	return vp;
}

void
__builtin_vec_delete(void* vp){
	rt_printk("__builtin_vec_delete %p\n",vp);
	if(vp != 0)
		rt_free(vp);
}

void*
__builtin_vec_new(int size){
	void* vp = rt_malloc(size);
	rt_printk("__builtin_vec_new %p %d\n",vp,size);
	return vp;
}

#endif

extern void __default_terminate (void) __attribute__ ((__noreturn__));

void
__default_terminate(void)
{
	while(1)
		rt_task_suspend(rt_whoami());
}
  
void (*__terminate_func)(void) = __default_terminate;

void
__terminate(void)
{
	(*__terminate_func)();
}
    
void
__pure_virtual(void)
{
	rt_printk("pure virtual method called\n");
	__terminate ();
}
/*  Declare a pointer to void function type.  */

typedef void (*func_ptr) (void);

static func_ptr* atexit_function_tab;
static int atexit_function_tab_size;
static int atexit_function_tab_len;

int atexit( func_ptr function )
{
	rt_printk("atexit %p\n",function);
	// we check if the pointer is NULL, this
	// can happen when a second C++ modules is loaded
	// and than unloaded. At unload time the second module
	// will call the atexit handlers and delete
	// the function table
	if( atexit_function_tab == 0 ){
		rt_printk("atexit_function_tab == NULL\n");
		return -1;
	}

	if( atexit_function_tab_len >= atexit_function_tab_size)
		return -1;
	
	if( function == 0 )
		return -1;
	
	atexit_function_tab[ atexit_function_tab_len++ ] = function;	
	
	return 0;
}

void __do_atexit( void )
{
	func_ptr *p = atexit_function_tab;

	rt_printk("__do_atexit START\n");

	// we check if the pointer is NULL, this
	// can happen when a second C++ modules is loaded
	// and than unloaded. At unload time the second module
	// will call the atexit handlers and delete
	// the function table
	if( atexit_function_tab == 0 ){
		rt_printk("atexit_function_tab == NULL\n");
		return;
	}
	
	while(*p)
	{
		rt_printk("calling atexit function %p\n",p );
		(*(p))();
		p++;
		if( atexit_function_tab_len-- )
			break;	
	}

	vfree( atexit_function_tab );

 	atexit_function_tab_size = 0;
 	atexit_function_tab_len = 0;
 	atexit_function_tab = 0;
 			
	rt_printk("__do_atexit END\n");	
}
void __init_atexit( int max_size )
{
	// we check if the pointer is NULL, this
	// can happen when a second C++ modules is loaded
	// and than unloaded. At unload time the second module
	// will call the atexit handlers and delete
	// the function table
	if( atexit_function_tab != 0 ){
		rt_printk("__init_atexit atexit_function_tab != NULL\n");
		return;
	}
	
	atexit_function_tab_size = max_size;
	atexit_function_tab_len = 0;
	atexit_function_tab = vmalloc( sizeof(func_ptr) * atexit_function_tab_size);
    	memset( atexit_function_tab , 0 , sizeof(func_ptr) * atexit_function_tab_size );
}
