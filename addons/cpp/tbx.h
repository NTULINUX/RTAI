/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: tbx.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#ifndef __TBX_H__
#define __TBX_H__

#include "rtai.h"
#include "rtai_tbx.h"
#include "count.h"

namespace RTAI {

/**
 * Typed Mailbox wrapper class
 */
class TypedMailbox {
public:
	TypedMailbox();
	TypedMailbox(int size, int flags);
	TypedMailbox(const char* name);
	TypedMailbox(const char* name, int size, int flags);

	virtual ~TypedMailbox();
	
	void init(int size, int flags );
	void init(const char* name );
	void init(const char* name, int size, int flags );

	int send( void *msg, int msg_size );
	int send_if( void *msg, int msg_size );
	int send_until( void *msg, int msg_size, const Count& time);
	int send_timed( void *msg, int msg_size, const Count& delay);

	int receive( void *msg, int msg_size);
	int receive_if( void *msg, int msg_size);
	int receive_until( void *msg, int msg_size, const Count& time);
	int receive_timed( void *msg, int msg_size, const Count& delay);

	int broadcast( void *msg, int msg_size);
	int broadcast_if( void *msg, int msg_size);
	int broadcast_until( void *msg, int msg_size, const Count& time);
	int broadcast_timed( void *msg, int msg_size, const Count& delay);

	int urgent( void *msg, int msg_size);
	int urgent_if( void *msg, int msg_size);
	int urgent_until( void *msg, int msg_size, const Count& time);
	int urgent_timed( void *msg, int msg_size, const Count& delay);
protected:
	TBX* m_TypedMailbox;
	bool m_Owner;
	bool m_Named;
private:
	TypedMailbox(const TypedMailbox&);
	TypedMailbox& operator=(const TypedMailbox&);
};

/**
 * templated Typed Mailbox wrapper class
 */
template <class Type>
class TypedMailboxT
:	public TypedMailbox
{
public:
	TypedMailboxT(){}
	TypedMailboxT(int size, int flags):TypedMailbox(size,flags){}
	TypedMailboxT(const char* name ):TypedMailbox(name){}
	TypedMailboxT(const char* name, int size, int flags ):TypedMailbox(name,size,flags){}

	int send( Type *msg ){
		return TypedMailbox::send( (void*)msg, sizeof( Type ) );
	}
	
	int send_if( Type *msg ){
		return TypedMailbox::send_if( (void*)msg, sizeof( Type ) );
	}
	
	int send_until( Type *msg, const Count& time){
		return TypedMailbox::send_until( (void*)msg, sizeof( Type ), time );
	}
	
	int send_timed( Type *msg, const Count& delay){
		return TypedMailbox::send_timed( (void*)msg, sizeof( Type ), delay );
	}
	

	int receive( Type *msg ){
		return TypedMailbox::receive( (void*)msg, sizeof( Type ));
	}
	
	int receive_if( Type *msg ){
		return TypedMailbox::receive_if( (void*)msg, sizeof( Type ) );
	}
	
	int receive_until( Type *msg, const Count& time){
		return TypedMailbox::receive_until( (void*)msg, sizeof( Type ), time );
	}
	
	int receive_timed( Type *msg, const Count& delay){
		return TypedMailbox::receive_timed((void*)msg, sizeof( Type ), delay );
	}
	
	int broadcast( Type *msg ){
		return TypedMailbox::broadcast( (void*)msg, sizeof( Type ) );
	}
	
	int broadcast_if( Type *msg ){
		return TypedMailbox::broadcast_if( (void*)msg, sizeof( Type ) );
	}
	
	int broadcast_until( Type *msg, const Count& time){
		return TypedMailbox::broadcast_until( (void*)msg, sizeof( Type ), time );
	}
	
	int broadcast_timed( Type *msg, const Count& delay){
		return TypedMailbox::broadcast_timed( (void*)msg, sizeof( Type ), delay );
	}

	int urgent( Type *msg ){
		return TypedMailbox::urgent( (void*)msg, sizeof( Type ) );
	}
	
	int urgent_if( Type *msg ){
		return TypedMailbox::urgent_if( (void*)msg, sizeof( Type ) );
	}
	
	int urgent_until( Type *msg, const Count& time){
		return TypedMailbox::urgent_until( (void*)msg, sizeof( Type ), time );
	}
	
	int urgent_timed( Type *msg, const Count& delay){
		return TypedMailbox::urgent_timed( (void*)msg, sizeof( Type ), delay );
	}
};

}; // namespace RTAI

#endif
