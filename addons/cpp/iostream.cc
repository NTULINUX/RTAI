/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: iostream.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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


#include "rtai.h"
#include "iostream.h"
#include <new.h>

namespace std {

ostream cerr;
ostream cout;

ostream& endl( ostream& s )
{
	return s.put( '\n' );
}

ostream& output_on( ostream& s )
{
	return s.set_output_on( true );
}

ostream& output_off( ostream& s )
{
	return s.set_output_on( false );
}

ostream::ostream()
:	m_OutputOn( true )
{
	rt_printk("ostream::ostream() %p\n",(void*)this);
}

ostream::~ostream()
{
	rt_printk("ostream::~ostream() %p\n",(void*)this);
}

ostream& ostream::set_output_on( bool f )
{
	m_OutputOn = f;

	return *this;
}

ostream& ostream::put(char c)
{
	if( m_OutputOn )
		rt_printk( "%c" , c );

	return *this;
}

ostream& ostream::write(const char *s)
{
	if( m_OutputOn )
		rt_printk("%s",s);

	return *this;
}

ostream& ostream::write(const char *s, int n)
{
	if ( m_OutputOn )
	{
		for (int i = 0; i < n; i++) {
			put(*s++);
		}
	}

	return *this;
}

ostream& ostream::operator<<( const char* s )
{	
	return write(s);;
}


ostream& ostream::operator<<( const void* p )
{
	if( m_OutputOn )
		rt_printk("%p",p);

	return *this;
}

ostream& ostream::operator<<( unsigned char c)
{
	if( m_OutputOn )
		rt_printk("%c",c);

	return *this;
}
	
ostream& ostream::operator<<( unsigned short n)
{
	if( m_OutputOn )
		rt_printk("%d",n);

	return *this;
}

ostream& ostream::operator<<( unsigned int n)
{
	if( m_OutputOn )
		rt_printk("%d",n);

	return *this;
}

ostream& ostream::operator<<( unsigned long n )
{
	if( m_OutputOn )
		rt_printk("%lu",n);

	return *this;
}

ostream& ostream::operator<<( unsigned long long n)
{
	if( m_OutputOn )
		rt_printk("<unsigned long long not implemented>");

	return *this;
}

ostream& ostream::operator<<( signed char c)
{
	if( m_OutputOn )
		rt_printk("%c",c);

	return *this;
}


ostream& ostream::operator<<( signed short n)
{
	if( m_OutputOn )
		rt_printk("%d",n);

	return *this;
}

ostream& ostream::operator<<( signed int n)
{
	if( m_OutputOn )
		rt_printk("%d",n);

	return *this;
}

ostream& ostream::operator<<( signed long n )
{
	if( m_OutputOn )
		rt_printk("%ld",n);

	return *this;
}

ostream& ostream::operator<<( signed long long n)
{
	if( m_OutputOn )
		rt_printk("<long long not implemented>");

	return *this;
}

ostream& ostream::operator<<( float f )
{
	if( m_OutputOn )
		rt_printk("<float not implemented>");

	return *this;
}

ostream& ostream::operator<<( double f )
{
	if( m_OutputOn )
		rt_printk("<double not implemented>");

	return *this;
}

ostream& ostream::operator<<( long double f)
{
	if( m_OutputOn )
		rt_printk("<long double not implemented>");

	return *this;
}

}; // namespace std

// doing a placement new with the address of the two global
// objects. This is a dirty hack to have just these two 
// constructors called, because at the moment the rtai_cpp
// modules only support one module doing global constructors/destructors
// and atexit handling
extern "C" {
void init_iostream()
{
	new( (void*)&std::cerr ) std::ostream;
	new( (void*)&std::cout ) std::ostream;
}

} // end extern "C" 

