/*
 * Project: rtai_cpp - RTAI C++ Framework 
 *
 * File: $Id: time.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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

#include "time.h"
#include "rtai_sched.h"

namespace RTAI {

Time::Time(){
	m_Time = 0;
}

Time::Time(const Time& time)
:	m_Time(time.m_Time)
{
}

Time::Time(long long time)
:	m_Time(time)
{
}

Time& Time::operator=(const Time& time)
{
	m_Time = time.m_Time;

	return *this;
}

Time& Time::operator=(long long time)
{
	m_Time = time;
	
	return *this;
}

Time Time::now(){
	return Time( count2nano( rt_get_time() ) );
}

Time Time::end(){
	return Time(0x7FFFFFFFFFFFFFFFLL);
}

long long Time::to_count() const {
	return nano2count(m_Time);
} 

Time Time::from_count(long long count) {
	Time tmp( count2nano(count) );

	return tmp;
}

bool Time::operator==(const Time& time) const 
{
	return m_Time == time.m_Time;
}

bool Time::operator==(long long time) const 
{
	return m_Time == time;
}

bool Time::operator<(const Time& time)const{
	return m_Time < time.m_Time;
}

bool Time::operator>(const Time& time)const {
	return m_Time > time.m_Time;
}

bool Time::operator<=(const Time& time)const{
	return m_Time <= time.m_Time;
}

bool Time::operator>=(const Time& time)const {
	return m_Time >= time.m_Time;
}

Time& Time::operator+=(const Time& time){
	m_Time += time.m_Time;
	return *this;
}

Time& Time::operator+=(long long time) {
	m_Time += time;
	return *this;
}

Time& Time::operator-=(const Time& time) {
	m_Time -= time.m_Time;
	return *this;
}

Time& Time::operator-=(long long time){
	m_Time -= time;
	return *this;  
}

Time  Time::operator+(const Time& time) const {
	Time tmp(m_Time + time.m_Time);
	return tmp;
}

Time  Time::operator+(long long time) const {
        Time tmp(m_Time + time);
        return tmp;
}

Time  Time::operator-(const Time& time) const {
	Time tmp(m_Time - time.m_Time);
	return tmp;
}

Time  Time::operator-(long long time) const {
	Time tmp(m_Time - time);
	return tmp;
}

}; // namespace RTAI
