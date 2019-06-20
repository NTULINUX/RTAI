/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: tbx.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "tbx.h"
#include "iostream.h"

namespace RTAI {

TypedMailbox::TypedMailbox()
{
	rt_printk("TypedMailbox::TypedMailbox() %p\n",this);
	m_TypedMailbox = 0;   
	m_Owner = true;
	m_Named = false;
}

TypedMailbox::TypedMailbox(int size, int flags)
{
	rt_printk("TypedMailbox::TypedMailbox( int size=%d, int flags=%d) %p\n",size,flags,this);
	m_TypedMailbox = 0;   
	m_Owner = true;
	m_Named = false;

	init( size , flags );
}

TypedMailbox::TypedMailbox(const char* name)
{
	m_TypedMailbox = 0;   
	m_Named = true;
	m_Owner = false;
	
	init( name );
}

TypedMailbox::TypedMailbox(const char* name, int size, int flags)
{
	m_TypedMailbox = 0;   
	m_Named = true;
	m_Owner = true;
	init(name,size,flags);
}

TypedMailbox::~TypedMailbox()
{
	rt_printk("TypedMailbox::~TypedMailbox() %p\n",this);

	if(m_TypedMailbox != 0){
		if( m_Owner ){
			if( m_Named ) {
				// nothing yet
			} else {
				rt_tbx_delete(m_TypedMailbox);
			}
		}
	}
}
	
void TypedMailbox::init(int size, int flags )
{
	rt_printk("TypedMailbox::init(int size=%d, int flags=%d) %p\n",size,flags,this);
	if(m_TypedMailbox == 0)
		rt_tbx_init(m_TypedMailbox, size,flags);
}

void TypedMailbox::init(const char* name )
{
	std::cerr << "TypedMailbox::init(name) NOT IMPLEMENTED" << std::endl;
}

void TypedMailbox::init(const char* name, int size, int flags )
{
	std::cerr << "TypedMailbox::init(name,size,flags) NOT IMPLEMENTED" << std::endl;
}

int TypedMailbox::send( void *msg, int msg_size )
{
	return rt_tbx_send( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::send_if( void *msg, int msg_size )
{
	return rt_tbx_send_if( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::send_until( void *msg, int msg_size, const Count& time)
{
	return rt_tbx_send_until( m_TypedMailbox, msg, msg_size, time );
}

int TypedMailbox::send_timed( void *msg, int msg_size, const Count& delay)
{
	return rt_tbx_send_timed( m_TypedMailbox, msg, msg_size, delay );
}

int TypedMailbox::receive( void *msg, int msg_size)
{
	return rt_tbx_receive( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::receive_if( void *msg, int msg_size)
{
	return rt_tbx_receive_if( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::receive_until( void *msg, int msg_size, const Count& time)
{
	return rt_tbx_receive_until( m_TypedMailbox, msg, msg_size, time );
}

int TypedMailbox::receive_timed( void *msg, int msg_size, const Count& delay)
{
	return rt_tbx_receive_timed( m_TypedMailbox, msg, msg_size, delay );
}

int TypedMailbox::broadcast( void *msg, int msg_size)
{
	return rt_tbx_broadcast( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::broadcast_if( void *msg, int msg_size)
{
	return rt_tbx_broadcast_if( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::broadcast_until( void *msg, int msg_size, const Count& time)
{
	return rt_tbx_broadcast_until( m_TypedMailbox, msg, msg_size, time);
}

int TypedMailbox::broadcast_timed( void *msg, int msg_size, const Count& delay)
{
	return rt_tbx_broadcast_timed( m_TypedMailbox, msg, msg_size, delay );
}

int TypedMailbox::urgent( void *msg, int msg_size)
{
	return rt_tbx_urgent( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::urgent_if( void *msg, int msg_size)
{
	return rt_tbx_urgent_if( m_TypedMailbox, msg, msg_size );
}

int TypedMailbox::urgent_until( void *msg, int msg_size, const Count& time)
{
	return rt_tbx_urgent_until( m_TypedMailbox, msg, msg_size, time );
}

int TypedMailbox::urgent_timed( void *msg, int msg_size, const Count& delay)
{
	return rt_tbx_urgent_timed( m_TypedMailbox, msg, msg_size, delay );
}

}; // namespace RTAI
