/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: rtf.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "rtf.h"

namespace RTAI {

RTAI::fifo* RTAI::fifo::m_this_table[MAX_FIFOS];

fifo::fifo(){
}

fifo::fifo(int id, int size){
	create(id,size);
}

int fifo::create(int id, int size){
	int res = rtf_create(id,size);
	m_fifo = id;
	m_this_table[id] = this;
	
	return res;
}

fifo::~fifo(){
	if(m_fifo != -1){
		m_this_table[m_fifo] = 0;
		rtf_destroy(m_fifo);
	}
}
	
int fifo::reset(){
	return rtf_reset(m_fifo);
}

int fifo::resize(int size){
	return rtf_resize(m_fifo,size);	
}

int fifo::put(const void* buffer, int count){
	return rtf_put(m_fifo,const_cast<void*>(buffer),count);
}

int fifo::get(void* buffer, int count){
	return rtf_get(m_fifo,buffer,count);
}

int fifo::activate_handler(){
	return -1;
}

int fifo::deactivate_handler(){
	return -1;
}

}; // namespace RTAI
