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


# spinlocks


rtai.rt_spl_init.argtypes = [c_ulong]
rtai.rt_spl_init.restype = c_void_p
rt_spl_init = rtai.rt_spl_init

rtai.rt_spl_delete.argtypes = [c_void_p]
rt_spl_delete = rtai.rt_spl_delete

rtai.rt_named_spl_init.argtypes = [c_void_p]
rtai.rt_named_spl_init.restype = c_void_p
rt_named_spl_init = rtai.rt_named_spl_init

rtai.rt_named_spl_delete.argtypes = [c_void_p]
rt_named_spl_delete = rtai.rt_named_spl_delete

rtai.rt_spl_lock.argtypes = [c_void_p]
rt_spl_lock = rtai.rt_spl_lock

rtai.rt_spl_lock_if.argtypes = [c_void_p]
rt_spl_lock_if = rtai.rt_spl_lock_if

rtai.rt_spl_lock_timed.argtypes = [c_void_p, c_longlong]
rt_spl_lock_timed = rtai.rt_spl_lock_timed

rtai.rt_spl_unlock.argtypes = [c_void_p]
rt_spl_unlock = rtai.rt_spl_unlock
