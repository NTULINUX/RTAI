# Copyright (C) 2008 Paolo Mantegazza <mantegazza@aero.polimi.it>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from ctypes import *

cdll.LoadLibrary("libc.so.6")
libc = CDLL("libc.so.6")

cdll.LoadLibrary("liblxrt.so")
rtai = CDLL("liblxrt.so")

NULL = None

PRIO_Q = 0
FIFO_Q = 4
RES_Q  = 3

BIN_SEM = 1
CNT_SEM = 2
RES_SEM = 3

RESEM_RECURS = 1
RESEM_BINSEM = 0
RESEM_CHEKWT = -1

#define SEM_ERR     (RTE_OBJINV)
#define SEM_TIMOUT  (RTE_TIMOUT)

GLOBAL_HEAP_ID = 0x9ac6d9e7

USE_VMALLOC = 0
USE_GFP_KERNEL = 1
USE_GFP_ATOMIC = 2
USE_GFP_DMA = 3

