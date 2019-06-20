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


# read write locks



def rt_rwl_init(rwl) :
	return rt_typed_rwl_init(rwl, RESEM_RECURS)

rtai.rt_typed_rwl_init.argtypes = [c_ulong, c_int]
rtai.rt_typed_rwl_init.restype = c_void_p
rt_typed_rwl_init = rtai.rt_typed_rwl_init

rtai.rt_rwl_delete.argtypes = [c_void_p]
rt_rwl_delete = rtai.rt_rwl_delete

rtai.rt_named_rwl_init.argtypes = [c_void_p]
rtai.rt_named_rwl_init.restype = c_void_p
rt_named_rwl_init = rtai.rt_named_rwl_init

rtai.rt_named_rwl_delete.argtypes = [c_void_p]
rt_named_rwl_delete = rtai.rt_named_rwl_delete

rtai.rt_rwl_rdlock.argtypes = [c_void_p]
rt_rwl_rdlock = rtai.rt_rwl_rdlock

rtai.rt_rwl_rdlock_if.argtypes = [c_void_p]
rt_rwl_rdlock_if = rtai.rt_rwl_rdlock_if

rtai.rt_rwl_rdlock_until.argtypes = [c_void_p, c_longlong]
rt_rwl_rdlock_until = rtai.rt_rwl_rdlock_until

rtai.rt_rwl_rdlock_timed.argtypes = [c_void_p, c_longlong]
rt_rwl_rdlock_timed = rtai.rt_rwl_rdlock_timed

rtai.rt_rwl_wrlock.argtypes = [c_void_p]
rt_rwl_wrlock = rtai.rt_rwl_wrlock

rtai.rt_rwl_wrlock_if.argtypes = [c_void_p]
rt_rwl_wrlock_if = rtai.rt_rwl_wrlock_if

rtai.rt_rwl_wrlock_until.argtypes = [c_void_p, c_longlong]
rt_rwl_wrlock_until = rtai.rt_rwl_wrlock_until

rtai.rt_rwl_wrlock_timed.argtypes = [c_void_p, c_longlong]
rt_rwl_wrlock_timed = rtai.rt_rwl_wrlock_timed

rtai.rt_rwl_unlock.argtypes = [c_void_p]
rt_rwl_unlock = rtai.rt_rwl_unlock
