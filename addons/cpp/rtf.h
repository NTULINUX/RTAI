/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: rtf.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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


#ifndef __RTF_H__
#define __RTF_H__

#include "rtai.h"
#include "rtai_fifos.h"

namespace RTAI {

/**
 * RTF wrapper class
 */
class fifo {
public:
	fifo();
	fifo(int id, int size);
	virtual ~fifo();
	
	int create(int id, int size);
	
	int reset();
	int resize(int size);
	int put(const void* buffer, int count);
	int get(void* buffer, int count);

	int activate_handler();
	int deactivate_handler();
	
	virtual void handler(){};
protected:	
	int m_fifo;	
	static fifo* m_this_table[MAX_FIFOS];
};

template <class Type>
class fifoT
:	public fifo 
{
public:
	fifoT(){}
	fifoT(int id,int size) : fifo(id,size){}

	int put(const Type* type) {
		return fifo::put((const void*)type,sizeof(Type));
	}
	
	int get(Type* type) {
		return fifo::get((void*)type,sizeof(Type));
	}
};

};

#endif
