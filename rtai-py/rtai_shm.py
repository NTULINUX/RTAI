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


# shared memory


def rtai_kmalloc(name, size) :
	return rt_shm_alloc(name, size, USE_VMALLOC)

def rtai_kfree(name) :
	return rt_shm_free(name)

rtai._rt_shm_alloc.argtypes = [c_void_p, c_ulong, c_int, c_int, c_int]
rtai._rt_shm_alloc.restype = c_void_p
_rt_shm_alloc = rtai._rt_shm_alloc

def rt_shm_alloc(name, size, suprt) :
	return _rt_shm_alloc(0, name, size, suprt, 0)

def rt_heap_open(name, size, suprt) :
	return _rt_shm_alloc(0, name, size, suprt, 1)

def rtai_malloc(name, size) :
	return _rt_shm_alloc(0, name, size, USE_VMALLOC, 0)

def rt_shm_alloc_adr(start_address, name, size, suprt) :
	return _rt_shm_alloc(start_address, name, size, suprt, 0)

def rt_heap_open_adr(start, name, size, suprt) :
	return _rt_shm_alloc(start, name, size, suprt, 1)

def rtai_malloc_adr(start_address, name, size) :
	return _rt_shm_alloc(start_address, name, size, USE_VMALLOC, 0)

rtai.rt_shm_free.argtypes = [c_ulong]
rt_shm_free = rtai.rt_shm_free

def rtai_free(name, adr) :
	return rt_shm_free(name)

rtai.rt_halloc.argtypes = [c_int]
rtai.rt_halloc.restype = c_void_p
rt_halloc = rtai.rt_halloc

rtai.rt_hfree.argtypes = [c_void_p]
rt_hfree = rtai.rt_hfree

rtai.rt_named_halloc.argtypes = [c_ulong, c_int]
rtai.rt_named_halloc.restype = c_void_p
rt_named_halloc = rtai.rt_named_halloc

rtai.rt_named_halloc.argtypes = [c_void_p]
rt_named_hfree = rtai.rt_named_hfree

rtai.rt_malloc.argtypes = [c_int]
rtai.rt_malloc.restype = c_void_p
rt_malloc = rtai.rt_malloc

rtai.rt_free.argtypes = [c_void_p]
rt_free = rtai.rt_free

rtai.rt_named_malloc.argtypes = [c_ulong, c_int]
rtai.rt_named_malloc.restype = c_void_p
rt_named_malloc = rtai.rt_named_malloc

rtai.rt_named_free.argtypes = [c_void_p]
rt_named_free = rtai.rt_named_free

def rt_heap_close(name, adr) :
	return rt_shm_free(name)

rt_heap_init = rt_heap_open
rt_heap_create = rt_heap_open
rt_heap_acquire = rt_heap_open
rt_heap_init_adr = rt_heap_open_adr
rt_heap_create_adr = rt_heap_open_adr
rt_heap_acquire_adr = rt_heap_open_adr

rt_heap_delete = rt_heap_close
rt_heap_destroy = rt_heap_close
rt_heap_release = rt_heap_close

def rt_global_heap_open() :
	return rt_heap_open(GLOBAL_HEAP_ID, 0, 0)
