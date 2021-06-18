#ifndef _RTAI_PTHREAD_INT_WRAPPER_H_
#define _RTAI_PTHREAD_INT_WRAPPER_H_
//////////////////////////////////////////////////////////////////////////////
//
//      Copyright (©) 1999 Zentropic Computing, All rights reserved
//
// Authors:             Steve Papacharalambous (stevep@zentropix.com)
// Original date:       Thu 15 Jul 1999
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, see <http://www.gnu.org/licenses/>.
// 
//
// pthreads interface for Real Time Linux.
//
// Modified for wrapping the pthreads to rtai_cpp by Peter Soetens
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------------------------------------------

#define POSIX_THREADS_MAX 64
#define PTHREAD_KEYS_MAX 1024

// Thread specific data is kept in a special data structure, a two-level
// array.  The top-level array contains pointers to dynamically allocated
// arrays of a certain number of data pointers.  So a sparse array can be
// implemented.  Each dynamic second-level array has
//      PTHREAD_KEY_2NDLEVEL_SIZE
// entries,and this value shouldn't be too large.
#define PTHREAD_KEY_2NDLEVEL_SIZE       32

// Need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
// keys in each subarray.
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
  / PTHREAD_KEY_2NDLEVEL_SIZE)


#define TASK_FPU_DISABLE 0
#define TASK_FPU_ENABLE 1

#define NSECS_PER_SEC 1000000000


// ----------------------------------------------------------------------------


// Arguments passed to thread creation routine.

struct pthread_start_args;

#define PTHREAD_START_ARGS_INITIALIZER { NULL, NULL, 0, 0, { 0 } }


struct _pthread_descr_struct;

#ifdef __cplusplus
}
#endif

#endif  // _RTAI_PTHREAD_INT_WRAPPER_H_
