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


# mail boxes


rtai.rt_typed_mbx_init.argtypes = [c_ulong, c_int, c_int]
rtai.rt_typed_mbx_init.restype = c_void_p
rt_typed_mbx_init = rtai.rt_typed_mbx_init

def rt_mbx_init(name, size) :
	return rt_typed_mbx_init(name, size, FIFO_Q)

rtai.rt_mbx_delete.argtypes = [c_void_p]
rt_mbx_delete = rtai.rt_mbx_delete

rtai.rt_typed_named_mbx_init.argtypes = [c_void_p, c_int, c_int]
rtai.rt_typed_named_mbx_init.restype = c_void_p
rt_typed_named_mbx_init = rtai.rt_typed_named_mbx_init

rtai.rt_named_mbx_delete.argtypes = [c_void_p]
rt_named_mbx_delete = rtai.rt_named_mbx_delete

def rt_named_mbx_init(mbx_name, size) :
	return rtai.rt_typed_named_mbx_init(mbx_name, size, FIFO_Q)

rtai.rt_mbx_send.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send = rtai.rt_mbx_send

rtai.rt_mbx_send_if.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send_if = rtai.rt_mbx_send_if

rtai.rt_mbx_send_until.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_send_until = rtai.rt_mbx_send_until

rtai.rt_mbx_send_timed.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_send_timed = rtai.rt_mbx_send_timed

rtai.rt_mbx_send_wp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_send_wp = rtai.rt_mbx_send_wp

rtai.rt_mbx_ovrwr_send.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_ovrwr_send = rtai.rt_mbx_ovrwr_send

rtai.rt_mbx_evdrp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_evdrp = rtai.rt_mbx_evdrp

rtai.rt_mbx_receive.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive = rtai.rt_mbx_receive

rtai.rt_mbx_receive_if.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive_if = rtai.rt_mbx_receive_if

rtai.rt_mbx_receive_until.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_receive_until = rtai.rt_mbx_receive_until

rtai.rt_mbx_receive_timed.argtypes = [c_void_p, c_void_p, c_int, c_longlong]
rt_mbx_receive_timed = rtai.rt_mbx_receive_timed

rtai.rt_mbx_receive_wp.argtypes = [c_void_p, c_void_p, c_int]
rt_mbx_receive_wp = rtai.rt_mbx_receive_wp
