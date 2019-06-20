/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: sem.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "rtai_sched.h"
#include "rtai_sem.h"
#include "time.h"

#ifndef __SEM_H__
#define __SEM_H__

namespace RTAI {

/**
 * Abstract Semaphore base class.
 */
class Semaphore {
protected:
	Semaphore(int value,int type);
	Semaphore(const char* name, int value, int type);
	Semaphore(const char* name );
public:
	virtual ~Semaphore();
	int signal();
	int broadcast();
	int wait();
	int wait_if();
	int wait_until(const Time& time);
	int wait_timed(const Time& delay);
protected:
	SEM* m_Sem;
	bool m_Owner;
	bool m_Named;
private:
	Semaphore(const Semaphore&);
	Semaphore& operator=(const Semaphore&);
};

/**
 * Binary Semaphore class
 */
class BinarySemaphore
:	public Semaphore 
{
public:
	BinarySemaphore();
	virtual ~BinarySemaphore();
protected:
private:
	BinarySemaphore(const BinarySemaphore&);
	BinarySemaphore& operator=(const BinarySemaphore&);   	
};

/**
 * Counting Semaphore class
 */
class CountingSemaphore 
:	public Semaphore 
{
public:
	CountingSemaphore();
	virtual ~CountingSemaphore();
protected:	
private:
	CountingSemaphore(const CountingSemaphore&);
	CountingSemaphore& operator=(const CountingSemaphore&);
};

/**
 * Resource Semaphore class
 */
class ResourceSemaphore 
:	public Semaphore 
{
public:
	ResourceSemaphore();
	~ResourceSemaphore();
protected:
private:
	ResourceSemaphore(const ResourceSemaphore&);
	ResourceSemaphore& operator=(const ResourceSemaphore&);
};

}; // namesapce RTAI

#endif
