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
#include "rtai_wrapper.h"

extern "C"
{

#include "rtai.h"
#include "rtai_malloc.h"

}

#if __GNUC__ < 3
 /* use __builtin_delete() */
#else

void operator delete(void* vp)
{
	rt_printk("my __builtin_delete %p\n",vp);

	if(vp != 0)
        	rt_free(vp);
}

/* for gcc-3.3 */

void operator delete[](void* vp){
        if(vp != 0)
                rt_free(vp);
}

void *operator new(size_t size){
        void* vp = rt_malloc(size);
        return vp;
}

void *operator new[](size_t size){
        void* vp = rt_malloc(size);
        return vp;
}

/* __cxa_pure_virtual(void) support */

extern "C" void __cxa_pure_virtual(void)
{
	rt_printk("attempt to use a virtual function before object has been constructed\n");
        for ( ; ; );
}


#endif
