/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: bits_init.c,v 1.4 2013/10/22 14:54:14 ando Exp $
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
 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <asm/unistd.h>
#include <rtai_sched.h>
#include <rtai_bits.h>
#include <rtai_malloc.h>

// RTAI-- MODULE INIT and CLEANUP functions

MODULE_LICENSE("GPL");
MODULE_AUTHOR("the RTAI-Team (contact person Erwin Rol)");
MODULE_DESCRIPTION("RTAI C++ BITS support");

BITS* __rt_bits_init(unsigned long mask){
	BITS * bits;
	
	bits = rt_malloc(sizeof(BITS));
	
	if(bits == 0)
		return 0;
		

	memset(bits,0,sizeof(BITS));

	rt_bits_init(bits,mask);

	return bits;
}

int __rt_bits_delete(BITS *bits){
	int result;
	
	if(bits == 0)
		return -1;

	result =  rt_bits_delete(bits);

	rt_free(bits);

	return result;
}

int __init rtai_cpp_bits_init(void){
	return 0;
}

void rtai_cpp_bits_cleanup(void)
{
}

module_init(rtai_cpp_bits_init)
module_exit(rtai_cpp_bits_cleanup)
  
