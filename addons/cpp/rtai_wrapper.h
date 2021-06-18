/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: rtai_wrapper.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#ifndef __RTAI_WRAPPER_H__
#define __RTAI_WRAPPER_H__

/**
 * The aim of this file is to contain all RTAI functions that can by no
 * means be included with the C++ compiler.
 */

#ifdef __cplusplus
extern "C"  {
#endif

#include <rtai_types.h>
	
void __rt_get_global_lock(void);

void __rt_release_global_lock(void);

int __hard_cpu_id(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTAI_WRAPPER_H__ */
