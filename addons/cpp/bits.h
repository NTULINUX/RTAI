/*
 * Project: rtai_cpp - RTAI C++ Framework
 *
 * File: $Id: bits.h,v 1.4 2013/10/22 14:54:14 ando Exp $
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
 

#ifndef __BITS_H__
#define __BITS_H__

#include "count.h"
#include "rtai_bits.h"

namespace RTAI {

/**
 * Bits a wrapper class around the RTAI BITS
 */
class Bits 
{
public:
	/**
	 * Function types used by Bits
	 */
	enum Function {
		all_set = ALL_SET,	///< all set
		any_set = ANY_SET,	///< any set
		all_clr = ALL_CLR,	///< all clear
		any_clr = ANY_CLR,	///< any clear

		all_set_and_any_set = ALL_SET_AND_ANY_SET,	///< all set and any clear
		all_set_and_all_clr = ALL_SET_AND_ALL_CLR,	///< all set and all clear
		all_set_and_any_clr = ALL_SET_AND_ANY_CLR,	///< all set and any clear
		any_set_and_all_clr = ANY_SET_AND_ALL_CLR,	///< any set and all clear
		any_set_and_any_clr = ANY_SET_AND_ANY_CLR,	///< any set and any clear
		all_clr_and_any_clr = ALL_CLR_AND_ANY_CLR,	///< all clear and any clear

		all_set_or_any_set = ALL_SET_OR_ANY_SET,	///< all set or any clear
		all_set_or_all_clr = ALL_SET_OR_ALL_CLR,	///< all set or all clear
		all_set_or_any_clr = ALL_SET_OR_ANY_CLR,	///< all set or any clear
		any_set_or_all_clr = ANY_SET_OR_ALL_CLR,	///< all set or all clear
		any_set_or_any_clr = ANY_SET_OR_ANY_CLR,	///< any set or any clear
		all_clr_or_any_clr = ALL_CLR_OR_ANY_CLR,	///< all clear or any clear
	};
public:
	/**
	 * Default Constructor.
	 * when using the default constructor one has to call
	 * the init methode by hand before the object can be used.
	 */
	Bits();
	/**
	 *Contructor
	 *@param mask The masked 
	 * This constructor will automaticly call init
	 */
	Bits( unsigned long mask );
	/**
	 * Destructor
	 */
	virtual ~Bits();

	/**
	 * init will initialize the under laying BITS
	 * @param mask The mask
	 */
	void init(unsigned long mask);
	/**
	 * Get the current status of the bits
	 * @return The 32 bits
	 */
	unsigned long get_bits();
	/**
	 * Reset the bits
	 * @param mask the mask 
	 * @return ?
	 */
	int reset( unsigned long mask );
	/**
	 * Signal the bits
	 * @param setfun The set function
	 * @param masks the mask
	 * @return ? 
	 */
	unsigned long signal( Function setfun, unsigned long masks );
	/**
	 * Wait for a change
	 * @param testfun ?
	 * @param testmasks ?
	 * @param exitfun ?
	 * @param exitmask ?
	 * @return ?
	 */
	int wait(Function testfun, unsigned long testmasks, 
		Function exitfun, unsigned long exitmasks,
		unsigned long *resulting_mask );
	/**
	 * Wait for a change
	 * @param testfun ?
	 * @param testmasks ?
	 * @param exitfun ?
	 * @param exitmasks
	 * @return ?
	 */
	int wait_if(Function testfun, unsigned long testmasks, 
		Function exitfun, unsigned long exitmasks,
		unsigned long *resulting_mask );
	/**
	 * Wait for a change
	 * @param testfun ?
	 * @param testmasks ?
	 * @param exitfun ?
	 * @param exitmasks
	 * @return ?
	 */
	int wait_until(Function testfun, unsigned long testmasks, 
		Function exitfun, unsigned long exitmasks, const Count& time,
		unsigned long *resulting_mask );
	/**
	 * Wait for a change
	 * @param testfun ?
	 * @param testmasks ?
	 * @param exitfun ?
	 * @param exitmasks
	 * @return ?
	 */
	int wait_timed(Function testfun, unsigned long testmasks, 
		Function exitfun, unsigned long exitmasks, const Count& delay,
		unsigned long *resulting_mask);
protected:
	BITS* m_Bits;
private:	
	Bits(const Bits&);
	Bits& operator=(const Bits&);
};

}; // namespace RTAI

#endif
