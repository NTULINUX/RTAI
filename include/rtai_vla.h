/*
 * Copyright (C) 2022 Marco Morandini  <marco.morandini@polimi.it>
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


#ifndef _RTAI_VLA_H
#define _RTAI_VLA_H

#define WARN_Wvla 0

#if WARN_Wvla
#define Wvla_disable _Pragma ("GCC diagnostic push")  _Pragma("GCC diagnostic ignored \"-Wdeclaration-after-statement\"")
#else
#define Wvla_disable  _Pragma ("GCC diagnostic push")  _Pragma("GCC diagnostic ignored \"-Wdeclaration-after-statement\"") _Pragma("GCC diagnostic ignored \"-Wvla\"")
#endif

#define Wvla_enable  _Pragma ("GCC diagnostic pop")

#define ALLOCATE_VLA(X)  Wvla_disable X; Wvla_enable

#endif /* _RTAI_VLA_H */

