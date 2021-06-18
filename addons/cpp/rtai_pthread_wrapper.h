#ifndef _RTAI_PTHREAD_WRAPPER_H_
#define _RTAI_PTHREAD_WRAPPER_H_
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

#include "linux_wrapper.h"
#include "rtai.h"
#include "rtai_pthread_int_wrapper.h"
#include "rtai_types.h"

// ----------------------------------------------------------------------------

typedef struct rt_task_struct RT_TASK;
#define SEM_ERR (0xffff)          // MUST be the same as rtai_sched.c
#define RT_SEM_MAGIC 0xaabcdeff   // MUST be the same as rtai_sched.c

// ----------------------------------------------------------------------------

#define PTHREAD_MUTEX_INITIALIZER {0, PTHREAD_MUTEX_FAST_NP, {{0, 0, 0}, RT_SEM_MAGIC, BIN_SEM, 1, 0, FIFO_Q}}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP {0, PTHREAD_MUTEX_RECURSIVE_NP, {{0, 0, 0}, RT_SEM_MAGIC, BIN_SEM, 1, 0, FIFO_Q}}
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP {0, PTHREAD_MUTEX_ERRORCHECK_NP, {{0, 0, 0}, RT_SEM_MAGIC, BIN_SEM, 1, 0, FIFO_Q}}

#define PTHREAD_COND_INITIALIZER {{{0, 0, 0}, RT_SEM_MAGIC, BIN_SEM, 1, 0, FIFO_Q}}


// ----------------------------------------------------------------------------


enum { PTHREAD_CANCEL_ENABLE, PTHREAD_CANCEL_DISABLE };
enum { PTHREAD_CANCEL_DEFERRED, PTHREAD_CANCEL_ASYNCHRONOUS };
enum { PTHREAD_INHERIT_SCHED, PTHREAD_EXPLICIT_SCHED };
enum { PTHREAD_CREATE_JOINABLE, PTHREAD_CREATE_DETACHED };
enum { PTHREAD_SCOPE_SYSTEM, PTHREAD_SCOPE_PROCESS };

enum {
  PTHREAD_MUTEX_FAST_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP
};


enum { CLOCK_REALTIME };

// ----------------------------------------------------------------------------

typedef unsigned long pthread_t;

typedef struct _pthread_descr_struct *pthread_descr;

typedef int rt_jmp_buf[6];


/* START by Peter Soetens */

struct sched_param {
        int sched_priority;
};
#define SCHED_OTHER     0
#define SCHED_FIFO      1
#define SCHED_RR        2
struct rt_queue {
	struct rt_queue *prev;
	struct rt_queue *next;
	struct rt_task_struct *task;
};

typedef struct rt_queue QUEUE;

struct rt_semaphore {
	struct rt_queue queue; //must be first in struct
	int magic;
	int type;
	int count;
	struct rt_task_struct *owndby;
	int qtype;
};

typedef struct rt_semaphore SEM;

/* END by Peter Soetens */

typedef struct {
  int detachstate;
  int schedpolicy;
  struct sched_param schedparam;
  int inheritsched;
  int scope;
} pthread_attr_t;

typedef struct {
  int mutexkind;
} pthread_mutexattr_t;

typedef struct {
  int dummy;
} pthread_condattr_t;


typedef struct {
  RT_TASK *m_owner;
  int m_kind;
  struct rt_semaphore m_semaphore;
} pthread_mutex_t;


typedef struct {
  SEM c_waiting;
} pthread_cond_t;


// Cleanup buffer.

struct _pthread_cleanup_buffer {
  void (*routine)(void *);              // Function to call.
  void *arg;                            // Its argument.
  int canceltype;                       // Saved cancellation type.
  struct _pthread_cleanup_buffer *prev; // Chaining of cleanup functions.
};




// ----------------------------------------------------------------------------

// Create a RT task with attributes ATTR, or default attributes if ATTR is NULL
// and call start function START_ROUTINE passing arguments ARG.
extern int pthread_create(pthread_t *thread, pthread_attr_t *attr,
                           void *(*start_routine) (void *), void *arg);


// Terminate the calling thread,
extern void pthread_exit(void *retval);

// Get the identifier of the current thread.
extern pthread_t pthread_self(void);

// Initialise thread attribute object, and fill it in with default values.
extern int pthread_attr_init(pthread_attr_t *attr);

// Destroy a thread attribute object.
extern int pthread_attr_destroy(pthread_attr_t *attr);

// Set the detach state for the thread.
extern int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

// Get the detach state for the thread.
extern int pthread_attr_getdetachstate(const pthread_attr_t *attr,
                                       int *detachstate);

// Set the thread scheduling parameters.
extern int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param);

// Get the thread scheduling parameters.
extern int pthread_attr_getschedparam(const pthread_attr_t *attr,
                                      struct sched_param *param);

// Set thread scheduling policy.
extern int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);

// Get thread scheduling policy.
extern int pthread_attr_getschedpolicy(const pthread_attr_t *attr,
                                       int *policy);

// Set thread scheduling inheritance.
extern int pthread_attr_setinheritsched(pthread_attr_t *attr,
                                        int inherit);

// Get thread scheduling inheritance.
extern int pthread_attr_getinheritsched(const pthread_attr_t *attr,
                                        int *inherit);

// Set thread scheduling scope.
extern int pthread_attr_setscope(pthread_attr_t *attr, int scope);


// Get thread scheduling scope.
extern int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);


// Yield the processor.
extern int sched_yield(void);

// Get the current clock count.  (Only CLOCK_REALTIME is currently supported)
extern void clock_gettime( int clockid, struct timespec *current_time);

// Delay the execution of the calling task for the time specified in req.
extern int nanosleep(const struct timespec *req, struct timespec *rem);

// Initialise mutex object.
extern int pthread_mutex_init(pthread_mutex_t *mutex,
                              const pthread_mutexattr_t *mutex_attr);


// Destroy mutex object.
extern int pthread_mutex_destroy(pthread_mutex_t *mutex);


// Initialise mutex object attributes.
extern int pthread_mutexattr_init(pthread_mutexattr_t *attr);


// Destroy mutex object attributes.
extern int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);


// Set mutex kind attribute.
extern int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind);


// Retrieve the current value of the mutex kind attribute.
extern int pthread_mutexattr_getkind_np(const pthread_mutexattr_t *attr,
                                        int *kind);

// Set thread scheduling parameters.
extern int pthread_setschedparam(pthread_t thread, int policy,
                                 const struct sched_param *param);

// Get thread scheduling parameters.
extern int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);

// Non blocking mutex lock.
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);



// Blocking mutex lock.
extern int pthread_mutex_lock(pthread_mutex_t *mutex);


// Mutex unlock.
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);


// Initialise conditional variable.
extern int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *cond_attr);


// Destroy conditional variable.
extern int pthread_cond_destroy(pthread_cond_t *cond);


// Initialise condition attribute object.
extern int pthread_condattr_init(pthread_condattr_t *attr);


// Destroy condition attribute object.
extern int pthread_condattr_destroy(pthread_condattr_t *attr);


// Wait for condition variable to be signaled.
extern int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);


// Wait for condition variable to be signaled or timeout expires.
extern int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                                  const struct timespec *abstime);


// Restart one of the threads waiting on the conditional variable cond.
extern int pthread_cond_signal(pthread_cond_t *cond);


// Restart all of the threads waiting on the conditional variable cond.
extern int pthread_cond_broadcast(pthread_cond_t *cond);


// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // _RTAI_PTHREAD_WRAPPER_H_
