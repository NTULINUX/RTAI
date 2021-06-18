/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: mutex.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "rtai.h"
#include "time.h"
#include "sem.h"

#ifndef __MUTEX_H__
#define __MUTEX_H__

namespace RTAI {

/**
 * mutal exclusion lock class
 */
class Mutex
:	public Semaphore 
{
friend class Condition;
public:
	Mutex();
	virtual ~Mutex();
private:
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);
};

/**
 * Scoped mutal exclusion lock class
 */ 
class ScopedMutex {
public:
	ScopedMutex( Mutex& m );
	~ScopedMutex();
protected:
	Mutex& m_Mutex;
private:
	ScopedMutex( const ScopedMutex& );
	ScopedMutex operator=( const ScopedMutex& );
};


}; // namespace RTAI

#endif
