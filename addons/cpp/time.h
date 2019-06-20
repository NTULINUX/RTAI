/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: time.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#ifndef __TIME_H__
#define __TIME_H__

namespace RTAI {

/**
 * time wrapper class
 *
 * Time keeps time in nanoseconds.
 */
class Time {
public:
    /**
     * The count ticks
     */
    typedef long long ticks;

    /**
     * The time in nanoseconds
     */
    typedef long long nsecs;
    
    /**
     * Initialise a time instance denoting zero nanoseconds
     */
	Time();
    
    /**
     * Make a deep copy of a Time instance
     */
	Time(const Time& time);
    /**
     * Initialise a time instance with <time> 
     * 
     * @param time The time in nanoseconds
     */
	explicit Time(nsecs time);

	Time& operator=(const Time& time);
	Time& operator=(nsecs time);

    /**
     * Returns the current system time in nanoseconds
     */
	static Time now();
    
    /**
     * Returns the largest system time (==infinity)
     */
	static Time end();

	inline operator nsecs() const { return m_Time; }

    /**
     * Converts this Time to internal system count units
     */
	ticks to_count() const;

    /**
     * Factory Method. Creates a time instance from
     * with a time converted from system count units
     * to nanoseconds
     *
     * @param The system time in count units
     */
	static Time from_count(ticks count);

	bool operator==(const Time& time) const;
	bool operator==(nsecs time) const;

	inline bool operator!=(const Time& time) const { return !this->operator==(time); }
	inline bool operator!=(nsecs time) const { return !this->operator==(time); }

	bool operator<(const Time& time)const;
	bool operator>(const Time& time)const;

	bool operator<=(const Time& time)const;
	bool operator>=(const Time& time)const;

	Time& operator+=(const Time& time);
	Time& operator+=(nsecs time);

	Time& operator-=(const Time& time);
	Time& operator-=(nsecs time);
	
	Time  operator+(const Time& time) const;
	Time  operator+(nsecs time) const;

	Time  operator-(const Time& time) const;
	Time  operator-(nsecs time) const;
protected:
	nsecs m_Time; // nanosecs
};

}; // namespace RTAI

#endif

