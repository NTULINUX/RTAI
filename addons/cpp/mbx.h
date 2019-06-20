/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: mbx.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#ifndef __MBX_H__
#define __MBX_H__

#include "rtai.h"
#include "rtai_sched.h"
#include "rtai_mbx.h"
#include "count.h"

namespace RTAI {

/**
 * MBX wrapper class
 */
class Mailbox {
public:
	Mailbox();
	Mailbox(int size);
	Mailbox(const char* name);
	Mailbox(const char* name, int size);

	virtual ~Mailbox();
	
	bool init(int size);
	bool init(const char* name);
	bool init(const char* name, int size);

	int send(const void* msg, int msg_size);
	int send_wp(const void* msg, int msg_size);
	int send_if(const void* msg, int msg_size);
	int send_until(const void* msg, int msg_size, const Count& time);
	int send_timed(const void* msg, int msg_size, const Count& delay);
	int receive(void* msg, int msg_size);
	int receive_wp(void* msg, int msg_size);
	int receive_if(void* msg, int msg_size);
	int receive_until(void* msg, int msg_size, const Count& time);
	int receive_timed(void* msg, int msg_size, const Count& delay);
protected:
	MBX* m_Mailbox;
	bool m_Named;
	bool m_Owner;
private:
	Mailbox(const Mailbox&);
	Mailbox& operator=(const Mailbox&);
};

/** 
 * templated Mailbox class
 */
template <class Type>
class MailboxT
:	public Mailbox
{
public:
	MailboxT(){}
	MailboxT(int size):Mailbox(size){}
	MailboxT(const char* name ):Mailbox(name){}
	MailboxT(const char* name, int size):Mailbox(name,size){}

	int send(const Type* msg){
		return Mailbox::send(msg,sizeof(Type));
	}
	int send_wp(const Type* msg){
		return Mailbox::send_wp(msg,sizeof(Type));
	}
	int send_if(const Type* msg){
		return Mailbox::send_if(msg,sizeof(Type));
	}
	int send_until(const Type* msg, const Count& time){
		return Mailbox::send_until(msg,sizeof(Type),time);
	}
	int send_timed(const Type* msg, const Count& delay){
		return Mailbox::send_timed(msg,sizeof(Type),delay);
	}
	int receive(Type* msg){
		return Mailbox::receive(msg,sizeof(Type));
	}
	int receive_wp(Type* msg){
		return Mailbox::receive_wp(msg,sizeof(Type));
	}
	int receive_if(Type* msg){
		return Mailbox::receive_if(msg,sizeof(Type));
	}
	int receive_until(Type* msg, const Count& time){
		return Mailbox::receive_until(msg,sizeof(Type),time);
	}
	int receive_timed(Type* msg, const Count& delay){
		return Mailbox::receive_timed(msg,sizeof(Type),delay);
	}
};

}; // namespace RTAI

#endif
