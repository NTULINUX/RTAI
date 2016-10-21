/*
 * e_fmax.c
 * Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mathP.h"

	double __ieee754_fmax(double __x, double __y)
{
	return __y < __x || __builtin_isnan(__x) ? __y : __x;
}
