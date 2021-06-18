/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: iostream.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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


#ifndef __IOSTREAM_H__
#define __IOSTREAM_H__

namespace std {

class ostream;
typedef ostream& (*__omanip)(ostream&);

/**
 * printk Helper Class
 */
class ostream {
public:
	ostream();
	virtual ~ostream();
	
	ostream& set_output_on( bool f );

	ostream& put(char c);
	ostream& put(unsigned char c) { return put((char)c); }
	ostream& put(signed char c) { return put((char)c); }

	ostream& write(const char *s);
	ostream& write(const unsigned char *s) { 
		return write((const char*)s);
	}
	ostream& write(const signed char *s) { 
		return write((const char*)s);
	}

	ostream& write(const char *s, int n);
	ostream& write(const unsigned char *s, int n) { 
		return write((const char*)s,n);
	}
	ostream& write(const signed char *s, int n) { 
		return write((const char*)s,n);
	}

	ostream& operator<<( const char* s );
	ostream& operator<<( const void* p );
	
	ostream& operator<<( unsigned char n);
	ostream& operator<<( unsigned short n);
	ostream& operator<<( unsigned int n);
	ostream& operator<<( unsigned long n );
	ostream& operator<<( unsigned long long n);

	ostream& operator<<( signed char n);
	ostream& operator<<( signed short n);
	ostream& operator<<( signed int n );
	ostream& operator<<( signed long n );
	ostream& operator<<( signed long long n);

	ostream& operator<<( float f );
	ostream& operator<<( double f );
	ostream& operator<<( long double f);

	ostream& operator<<(__omanip func) { return (*func)(*this); }
protected:
	bool m_OutputOn;
};

extern ostream& endl( ostream& s );
extern ostream& output_off( ostream& s);
extern ostream& output_on( ostream& s);

extern ostream cerr;
extern ostream cout;

}; // namespace std

#endif
