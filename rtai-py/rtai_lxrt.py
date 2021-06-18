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


# lxrt services


rtai.nam2num.argtypes = [c_void_p]
rtai.nam2num.restype  = c_ulong
nam2num = rtai.nam2num

rtai.num2nam.argtypes = [c_ulong, c_void_p]
num2nam = rtai.num2nam

rtai.rt_get_adr.argtypes = [c_ulong]
rtai.rt_get_adr.restype = c_void_p
rt_get_adr = rtai.rt_get_adr

rtai.rt_get_name.argtypes = [c_void_p]
rtai.rt_get_name.restype  = c_ulong
rt_get_name = rtai.rt_get_name.restype

rt_allow_nonroot_hrt = rtai.rt_allow_nonroot_hrt

rtai.rt_task_init_schmod.argtypes = [c_ulong, c_int, c_int, c_int, c_int, c_int]
rtai.rt_task_init_schmod.restype  = c_void_p
rt_task_init_schmod = rtai.rt_task_init_schmod

rtai.rt_task_init.argtypes = [c_ulong, c_int, c_int, c_int]
rtai.rt_task_init.restype  = c_void_p
rt_task_init = rtai.rt_task_init

rtai.rt_thread_create.argtypes = [c_void_p, c_void_p, c_int]
rtai.rt_thread_create.restype  = c_ulong
rt_thread_create = rtai.rt_thread_create

rt_make_soft_real_time = rtai.rt_make_soft_real_time
rt_make_hard_real_time = rtai.rt_make_hard_real_time

rtai.rt_thread_delete.argtypes = [c_void_p]
rt_thread_delete = rtai.rt_thread_delete
rt_task_delete = rtai.rt_thread_delete

rtai.rt_thread_join.argtypes = [c_ulong]
rt_thread_join = rtai.rt_thread_join

rtai.rt_set_sched_policy.argtypes = [c_void_p, c_int, c_int]
rt_set_sched_policy = rtai.rt_set_sched_policy

rtai.rt_change_prio.argtypes = [c_void_p, c_int]
rt_change_prio = rtai.rt_change_prio

rtai.rt_is_hard_real_time.argtypes = [c_void_p]
rt_is_hard_real_time = rtai.rt_is_hard_real_time

def rt_is_soft_real_time():
	if rtai.rt_is_hard_real_time():
		return 0
	else:
		return 1

rtai.rt_task_suspend.argtypes = [c_void_p]
rt_task_suspend = rtai.rt_task_suspend

rtai.rt_task_suspend_if.argtypes = [c_void_p]
rt_task_suspend_if = rtai.rt_task_suspend_if

rtai.rt_task_suspend_until.argtypes = [c_void_p, c_longlong]
rt_task_suspend_until = rtai.rt_task_suspend_until

rtai.rt_task_suspend_timed.argtypes = [c_void_p, c_longlong]
rt_task_suspend_timed = rtai.rt_task_suspend_timed

rtai.rt_task_resume.argtypes = [c_void_p]
rt_task_resume = rtai.rt_task_resume

rtai.rt_task_masked_unblock.argtypes = [c_void_p, c_ulong]
rt_task_masked_unblock = rtai.rt_task_masked_unblock

rt_task_yield = rtai.rt_task_yield

rtai.rt_sleep.argtypes = [c_longlong]
rt_sleep = rtai.rt_sleep

rtai.rt_sleep_until.argtypes = [c_longlong]
rt_sleep_until = rtai.rt_sleep_until

rt_sched_lock = rtai.rt_sched_lock

rt_sched_unlock = rtai.rt_sched_unlock

rtai.rt_task_make_periodic.argtypes = [c_void_p, c_longlong, c_longlong]
rt_task_make_periodic = rtai.rt_task_make_periodic

rtai.rt_task_make_periodic_relative_ns.argtypes = [c_void_p, c_longlong, c_longlong]
rt_task_make_periodic_relative_ns = rtai.rt_task_make_periodic_relative_ns

rt_task_wait_period = rtai.rt_task_wait_period

rt_is_hard_timer_running = rtai.rt_is_hard_timer_running

rt_set_periodic_mode = rtai.rt_set_periodic_mode

rt_set_oneshot_mode = rtai.rt_set_oneshot_mode

rtai.start_rt_timer.restype = c_longlong
start_rt_timer = rtai.start_rt_timer

stop_rt_timer = rtai.stop_rt_timer

rtai.rt_get_time.restype = c_longlong
rt_get_time = rtai.rt_get_time

rtai.rt_get_real_time.restype = c_longlong
rt_get_real_time = rtai.rt_get_real_time

rtai.rt_get_real_time_ns.restype = c_longlong
rt_get_real_time_ns = rtai.rt_get_real_time_ns

rtai.rt_get_time_ns.restype = c_longlong
rt_get_time_ns = rtai.rt_get_time_ns

rtai.rt_get_cpu_time_ns.restype = c_longlong
rt_get_cpu_time_ns = rtai.rt_get_cpu_time_ns

rtai.rt_get_exectime.argtypes = [c_void_p, c_void_p]
rt_get_exectime = rtai.rt_get_exectime

rtai.rt_gettimeorig.argtypes = [c_void_p]
rt_gettimeorig = rtai.rt_gettimeorig

rtai.count2nano.argtypes = [c_longlong]
rtai.count2nano.restype = c_longlong
count2nano = rtai.count2nano

rtai.nano2count.argtypes = [c_longlong]
rtai.nano2count.restype = c_longlong
nano2count = rtai.nano2count

rt_busy_sleep = rtai.rt_busy_sleep

#rtai.rt_force_task_soft.restype = c_ulong

rtai.rt_agent.restype = c_ulong

rt_buddy = rtai.rt_agent

rtai.rt_get_priorities.argtypes = [c_void_p, c_void_p, c_void_p]
rt_get_priorities = rtai.rt_get_priorities

rt_gettid = rtai.rt_gettid
