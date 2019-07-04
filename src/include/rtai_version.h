/*
 * Copyright (C) 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#include <rtai_config.h>

#define RTAI_RELEASE PACKAGE_VERSION
#define RTAI_MANGLE_VERSION(a,b,c) (((a) * 65536) + ((b) * 256) + (c))
#define RTAI_VERSION_CODE   RTAI_MANGLE_VERSION(CONFIG_RTAI_VERSION_MAJOR,CONFIG_RTAI_VERSION_MINOR,CONFIG_RTAI_REVISION_LEVEL)
