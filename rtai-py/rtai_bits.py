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


# bits


ALL_SET = 0
ANY_SET = 1
ALL_CLR = 2
ANY_CLR = 3

ALL_SET_AND_ANY_SET = 4
ALL_SET_AND_ALL_CLR = 5
ALL_SET_AND_ANY_CLR = 6
ANY_SET_AND_ALL_CLR = 7
ANY_SET_AND_ANY_CLR = 8
ALL_CLR_AND_ANY_CLR = 9

ALL_SET_OR_ANY_SET = 10
ALL_SET_OR_ALL_CLR = 11
ALL_SET_OR_ANY_CLR = 12
ANY_SET_OR_ALL_CLR = 13
ANY_SET_OR_ANY_CLR = 14
ALL_CLR_OR_ANY_CLR = 15

SET_BITS = 0
CLR_BITS = 1
SET_CLR_BITS = 2
NOP_BITS = 3

rtai.rt_bits_init.argtypes = [c_ulong, c_ulong]
rtai.rt_bits_init.restype = c_void_p
rt_bits_init = rtai.rt_bits_init

rtai.rt_bits_delete.argtypes = [c_void_p]
rt_bits_delete = rtai.rt_bits_delete

rtai.rt_get_bits.argtypes = [c_void_p]
rtai.rt_get_bits.restype = c_ulong
rt_get_bits = rtai.rt_get_bits

rtai.rt_bits_reset.argtypes = [c_void_p, c_ulong]
rtai.rt_bits_reset.restype = c_ulong
rt_bits_reset = rtai.rt_bits_reset

rtai.rt_bits_signal.argtypes = [c_void_p, c_int, c_ulong]
rtai.rt_bits_signal.restype = c_ulong
rt_bits_signal = rtai.rt_bits_signal

rtai.rt_bits_wait.argtypes = [c_void_p, c_int, c_ulong, c_int, c_ulong, c_void_p]
rt_bits_wait = rtai.rt_bits_wait

rtai.rt_bits_wait_if.argtypes = [c_void_p, c_int, c_ulong, c_int, c_ulong, c_void_p]
rt_bits_wait_if = rtai.rt_bits_wait_if

rtai.rt_bits_wait_until.argtypes = [c_void_p, c_int, c_ulong, c_int, c_ulong, c_longlong, c_void_p]
rt_bits_wait_until = rtai.rt_bits_wait_until

rtai.rt_bits_wait_timed.argtypes = [c_void_p, c_int, c_ulong, c_int, c_ulong, c_longlong, c_void_p]
rt_bits_wait_timed = rtai.rt_bits_wait_timed
