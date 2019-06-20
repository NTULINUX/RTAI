/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: bits.cc,v 1.4 2013/10/22 14:54:14 ando Exp $
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
 
#include "bits.h"
#include "rtai.h"

namespace RTAI {

Bits::Bits(){
	rt_printk("Bits::Bits( ) %p\n",this);
	m_Bits = 0;
}

Bits::Bits( unsigned long mask )
{
	rt_printk("Bits::Bits( unsigned long mask=%lu) %p\n",mask,this);
	m_Bits = 0;   
	init( mask  );
}

Bits::~Bits()
{
	if( m_Bits != 0 )
		rt_bits_delete( m_Bits );
}

void Bits::init(unsigned long mask)
{
	rt_bits_init(m_Bits,  mask );
}
	
unsigned long Bits::get_bits(){
	return rt_get_bits( m_Bits );
}

int Bits::reset( unsigned long mask )
{
	return rt_bits_reset( m_Bits, mask );
}

unsigned long Bits::signal( Function setfun, unsigned long masks )
{
	return rt_bits_signal( m_Bits, setfun, masks );
}

int Bits::wait(Function testfun, unsigned long testmasks, 
		Function exitfun, unsigned long exitmasks,
		unsigned long *resulting_mask )
{
	return rt_bits_wait( m_Bits, testfun, testmasks, exitfun, exitmasks, resulting_mask );
}

int Bits::wait_if(Function testfun, unsigned long testmasks, 
		  Function exitfun, unsigned long exitmasks,
		  unsigned long *resulting_mask )
{
	return rt_bits_wait_if( m_Bits, testfun, testmasks, exitfun, exitmasks, resulting_mask );
}

int Bits::wait_until(Function testfun, unsigned long testmasks, 
		     Function exitfun, unsigned long exitmasks, const Count& time, 
		     unsigned long *resulting_mask )
{
	return rt_bits_wait_until( m_Bits, testfun, testmasks, exitfun, exitmasks, time, resulting_mask );
}

int Bits::wait_timed(Function testfun, unsigned long testmasks, 
		     Function exitfun, unsigned long exitmasks, const Count& delay, 
		     unsigned long *resulting_mask )
{
	return rt_bits_wait_timed( m_Bits, testfun, testmasks, exitfun, exitmasks, delay , resulting_mask );
}

};
