/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: sem.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "sem.h"
#include "rtai_registry.h"

namespace RTAI {

Semaphore::Semaphore(int value, int type){
	m_Owner = true;
	m_Named = false;
    rt_typed_sem_init(m_Sem, value,type);
}

Semaphore::Semaphore(const char* name, int value, int type){
	m_Owner = true;
	m_Named = true;
	m_Sem = rt_typed_named_sem_init(name,value,type);
}

Semaphore::Semaphore(const char* name){
	m_Named = true;
	m_Owner = false;
    m_Sem   = 0;
	unsigned long num = nam2num(name);
	if(  rt_get_type( num ) == IS_SEM ){
		m_Sem = static_cast<SEM*>( rt_get_adr( num  ) );
	}
}

Semaphore::~Semaphore(){
	if(m_Sem != 0){
		if( m_Owner ){
			if( m_Named )
				rt_named_sem_delete(m_Sem);
			else
				rt_sem_delete(m_Sem);
		
		}
	}
}

int Semaphore::signal(){
	return rt_sem_signal(m_Sem);
}

int Semaphore::broadcast(){
	return rt_sem_broadcast(m_Sem);
}

int Semaphore::wait(){
	return rt_sem_wait(m_Sem);
}

int Semaphore::wait_if(){
	return rt_sem_wait_if(m_Sem);
}

int Semaphore::wait_until(const Time& time){
	return rt_sem_wait_until(m_Sem,time.to_count());
}

int Semaphore::wait_timed(const Time& delay){
	return rt_sem_wait_timed(m_Sem,delay.to_count());
}

BinarySemaphore::BinarySemaphore()
:	Semaphore(0,BIN_SEM)
{
}

BinarySemaphore::~BinarySemaphore(){
}
  
CountingSemaphore::CountingSemaphore()
:	Semaphore(0,CNT_SEM)
{
}

CountingSemaphore::~CountingSemaphore(){
}

ResourceSemaphore::ResourceSemaphore()
:	Semaphore(0,RES_SEM)
{
}

ResourceSemaphore::~ResourceSemaphore(){
}

};
