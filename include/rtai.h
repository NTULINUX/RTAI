/*
 * Copyright (C) 1999-2015 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef _RTAI_RTAI_H
#define _RTAI_RTAI_H

#ifdef __KERNEL__
#include <linux/version.h>
#endif /* __KERNEL__ */

#include <asm/rtai.h>

#ifdef __KERNEL__
// see: Computing Practices, ACM, vol. 31, n. 10, 1988, pgs 1192-1201.

#define TWOPWR31M1 2147483647  // 2^31 - 1

static inline long next_rand(long rand)
{
	const long a = 16807;
	const long m = TWOPWR31M1;
	const long q = 127773;
	const long r = 2836;

	long lo, hi;

	hi = rand/q;
	lo = rand - hi*q;
	rand = a*lo - r*hi;
	if (rand <= 0) {
		rand += m;
	}
	return rand;
}

static inline long irandu(unsigned long range)
{
	static long seed = 783637;
	const long m = TWOPWR31M1;

	seed = next_rand(seed);
	return rtai_imuldiv(seed, range, m);
}
#endif /* __KERNEL__ */

#endif /* !_RTAI_RTAI_H */
