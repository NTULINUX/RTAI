/*
 * Copyright (C) 2002 Thomas Leibner (leibner@t-online.de) (first complete writeup)
 * Copyright (C) 2002 David Schleef (ds@schleef.org) (COMEDI master)
 * Copyright (C) 2002 Lorenzo Dozio (dozio@aero.polimi.it) (made it all work)
 * Copyright (C) 2002 Paolo Mantegazza (mantegazza@aero.polimi.it) (hints/support)
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

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
 * This file is the non-inline KComedilib-LXRT userspace lib.
 * It will be compiled into a libkcomedilxrt.a library with which
 * your application can link if it included the rtai_comedi.h file.
 */

#include <rtai_comedi.h>
