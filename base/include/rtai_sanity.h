/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_SANITY_H
#define _RTAI_SANITY_H

#if !( __GNUC__ == 2 && __GNUC_MINOR__ > 8 && __GNUC_MINOR__ < 96 ) && \
	__GNUC__ > 7
#warning: You are likely using an unsupported GCC version! \
          Please read GCC-WARNINGS carefully.
#endif

#endif /* !_RTAI_SANITY_H */
