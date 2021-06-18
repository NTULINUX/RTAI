/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: count.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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


#include "count.h"
#include "rtai_sched.h"

namespace RTAI {

Count::Count(){
	m_Count = 0;
}

Count::Count(const Count& count)
:	m_Count(count.m_Count)
{
}

Count::Count(long long count)
:	m_Count(count)
{
}

Count& Count::operator=(const Count& count)
{
	m_Count = count.m_Count;

	return *this;
}

Count& Count::operator=(long long count)
{
	m_Count = count;
	
	return *this;
}

Count Count::now(){
	return Count( rt_get_time() );
}

Count Count::end(){
	return Count(0x7FFFFFFFFFFFFFFFLL);
}

long long Count::to_time() const {
	return count2nano( m_Count );
} 

Count Count::from_time(long long time) {
	Count tmp( nano2count(time) );

	return tmp;
}

bool Count::operator==(const Count& count) const 
{
	return m_Count == count.m_Count;
}

bool Count::operator==(long long count) const 
{
	return m_Count == count;
}

bool Count::operator<(const Count& count)const{
	return m_Count < count.m_Count;
}

bool Count::operator>(const Count& count)const {
	return m_Count > count.m_Count;
}

bool Count::operator<=(const Count& count)const{
	return m_Count <= count.m_Count;
}

bool Count::operator>=(const Count& count)const {
	return m_Count >= count.m_Count;
}

Count& Count::operator+=(const Count& count){
	m_Count += count.m_Count;
	return *this;
}

Count& Count::operator+=(long long count) {
	m_Count += count;
	return *this;
}

Count& Count::operator-=(const Count& count) {
	m_Count -= count.m_Count;
	return *this;
}

Count& Count::operator-=(long long count){
	m_Count -= count;
	return *this;  
}

Count  Count::operator+(const Count& count) const {
	Count tmp(m_Count + count.m_Count);
	return tmp;
}

Count  Count::operator+(long long count) const {
        Count tmp(m_Count + count);
        return tmp;
}

Count  Count::operator-(const Count& count) const {
	Count tmp(m_Count - count.m_Count);
	return tmp;
}

Count  Count::operator-(long long count) const {
	Count tmp(m_Count - count);
	return tmp;
}

}; // namespace RTAI
