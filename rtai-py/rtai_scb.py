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

from rtai_def import *


# non blocking shared memory circular buffers


HDRSIZ = (3*sizeof(c_int))

rtai.rt_scb_init.argtypes = [c_ulong, c_int, c_ulong]
rtai.rt_scb_init.restype = c_void_p
rt_scb_init = rtai.rt_scb_init

rtai.rt_scb_delete.argtypes = [c_ulong]
rt_scb_delete = rtai.rt_scb_delete

rtai.rt_scb_bytes.argtypes = [c_void_p]
rt_scb_bytes = rtai.rt_scb_bytes

rtai.rt_scb_get.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_get = rtai.rt_scb_get

rtai.rt_scb_evdrp.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_evdrp = rtai.rt_scb_evdrp

rtai.rt_scb_put.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_put = rtai.rt_scb_put

rtai.rt_scb_ovrwr.argtypes = [c_void_p, c_void_p, c_int]
rt_scb_ovrwr = rtai.rt_scb_ovrwr
