/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: mbx.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "mbx.h"
#include "rtai_registry.h"

namespace RTAI {

Mailbox::Mailbox(){
	rt_printk("Mailbox::Mailbox() %p\n",this);
	m_Mailbox = 0;
	m_Owner = true;
	m_Named = false;
}

Mailbox::Mailbox(int size){
	rt_printk("Mailbox::Mailbox(int size=%d) %p\n",size,this);
	m_Mailbox = 0;
	m_Owner = true;
	m_Named = false;
	init(size);
}

Mailbox::Mailbox(const char* name){
	rt_printk("Mailbox::Mailbox(const char* name=%s) %p\n",name,this);
	m_Mailbox = 0;
	m_Owner = false;
	m_Named = true;
	init(name);
}

Mailbox::Mailbox(const char* name, int size){
	rt_printk("Mailbox::Mailbox(const char* name=%s, int size=%d)%p\n",name ,size,this);
	m_Mailbox = 0;
	m_Owner = true;
	m_Named = true;
	init(name,size);
}

Mailbox::~Mailbox(){
	rt_printk("Mailbox::~Mailbox() %p\n",this);

	if(m_Mailbox != 0){
                if( m_Owner ){
                        if( m_Named){ 
                                rt_named_mbx_delete(m_Mailbox);
                        } else {
                                rt_mbx_delete(m_Mailbox);
                        }
                }
	}
}

bool Mailbox::init(int size){
	rt_printk("Mailbox::init(int size=%d) %p\n",size,this);
	if(m_Mailbox == 0){
		m_Named = false;
		m_Owner = true;
		rt_mbx_init(m_Mailbox, size);
		if( m_Mailbox == 0)
			return false;
		else
			return true;
	} else {
		return false;
	}
}

bool Mailbox::init(const char* name, int size){
	rt_printk("Mailbox::init(int size=%d) %p\n",size,this);
	if(m_Mailbox == 0){
		m_Owner = true;
		m_Named = true;
		m_Mailbox = rt_named_mbx_init(name, size);
		if( m_Mailbox == 0)
			return false;
		else
			return true;
	} else {
		return false;
	}
}

bool Mailbox::init(const char* name)
{
	rt_printk("Mailbox::init() %p\n",this);
	if(m_Mailbox == 0){
		m_Owner = false;
		m_Named = true;
		
        unsigned long num = nam2num(name);
        if(  rt_get_type( num ) == IS_MBX ){
            m_Mailbox = static_cast<MBX*>(rt_get_adr( num  ));
            return true;
        } else {
            return false;
        }
	} else {
		return false;
	}
}

int Mailbox::send(const void* msg, int msg_size){
	return rt_mbx_send(m_Mailbox,const_cast<void*>(msg),msg_size);
}

int Mailbox::send_wp(const void* msg, int msg_size){
	return rt_mbx_send_wp(m_Mailbox,const_cast<void*>(msg),msg_size);
}

int Mailbox::send_if(const void* msg, int msg_size){
	return rt_mbx_send_if(m_Mailbox,const_cast<void*>(msg),msg_size);
}

int Mailbox::send_until(const void* msg, int msg_size, const Count& time){
	return rt_mbx_send_until(m_Mailbox,const_cast<void*>(msg),msg_size,time);
}

int Mailbox::send_timed(const void* msg, int msg_size, const Count& delay){
	return rt_mbx_send_timed(m_Mailbox,const_cast<void*>(msg),msg_size,delay);
}

int Mailbox::receive(void* msg, int msg_size){
	return rt_mbx_receive(m_Mailbox,msg,msg_size);
}

int Mailbox::receive_wp(void* msg, int msg_size){
	return rt_mbx_receive_wp(m_Mailbox,msg,msg_size);
}

int Mailbox::receive_if(void* msg, int msg_size){
	return rt_mbx_receive_if(m_Mailbox,msg,msg_size);
}

int Mailbox::receive_until(void* msg, int msg_size, const Count& time){
	return rt_mbx_receive_until(m_Mailbox,msg,msg_size,time);
}

int Mailbox::receive_timed(void* msg, int msg_size, const Count& delay){
	return rt_mbx_receive_timed(m_Mailbox,msg,msg_size,delay);
}

}; // namespace RTAI
