/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: count.h,v 1.4 2013/10/22 14:54:14 ando Exp $
 *
 * Copyright: (C) 2001,2002 Erwin Rol <erwin@muffin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef __COUNT_H__
#define __COUNT_H__

namespace RTAI {

/**
 * Tick Count helper class
 * 
 * Ticks are sometimes needed by the system calls. 
 * For human readable time keeping, use the Time class.
 */
class Count {
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
     * Instantiates a Count object with count zero
     */
	Count();

    /**
     * Make a deep copy of a Count instance
     */
	Count(const Count& count);

    /**
     * Instantiates a Count object with counts on <count>
     *
     * @oaram count The system counts to be used
     */
	explicit Count(ticks count);

	Count& operator=(const Count& count);
	Count& operator=(ticks count);

	static Count now();
	static Count end();

	inline operator ticks() const { return m_Count; }

    /**
     * Returns the time in nanoseconds that this Count
     * instance represents.
     */
	nsecs to_time() const;

    /**
     * Factory Method. Creates a Count instance
     * denoting <time> in nanoseconds
     *
     * @param time The time in nanoseconds
     */
	static Count from_time(nsecs time);

	bool operator==(const Count& count) const;
	bool operator==(ticks count) const;

	inline bool operator!=(const Count& count) const { return !this->operator==(count); }
	inline bool operator!=(ticks count) const { return !this->operator==(count); }

	bool operator<(const Count& count)const;
	bool operator>(const Count& count)const;

	bool operator<=(const Count& count)const;
	bool operator>=(const Count& count)const;

	Count& operator+=(const Count& count);
	Count& operator+=(ticks count);

	Count& operator-=(const Count& count);
	Count& operator-=(ticks count);
	
	Count  operator+(const Count& count) const;
	Count  operator+(ticks count) const;

	Count  operator-(const Count& count) const;
	Count  operator-(ticks count) const;
protected:
	ticks m_Count; // internal count ticks
};

}; // namespace RTAI

#endif
