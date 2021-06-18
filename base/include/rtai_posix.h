/*
 * Copyright (C) 1999-2006 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 *
 */


#ifndef _RTAI_POSIX_H_
#define _RTAI_POSIX_H_

#define sem_open_rt                     sem_open
#define sem_close_rt                    sem_close
#define sem_init_rt                     sem_init
#define sem_destroy_rt                  sem_destroy
#define sem_wait_rt                     sem_wait
#define sem_trywait_rt                  sem_trywait
#define sem_timedwait_rt                sem_timedwait
#define sem_post_rt                     sem_post
#define sem_getvalue_rt                 sem_getvalue

#define pthread_mutex_open_rt           pthread_mutex_open
#define pthread_mutex_close_rt          pthread_mutex_close
#define pthread_mutex_init_rt           pthread_mutex_init
#define pthread_mutex_destroy_rt        pthread_mutex_destroy
#define pthread_mutex_lock_rt           pthread_mutex_lock
#define pthread_mutex_timedlock_rt      pthread_mutex_timedlock
#define pthread_mutex_trylock_rt        pthread_mutex_trylock
#define pthread_mutex_unlock_rt         pthread_mutex_unlock

#define pthread_cond_open_rt            pthread_cond_open
#define pthread_cond_close_rt           pthread_cond_close
#define pthread_cond_init_rt            pthread_cond_init
#define pthread_cond_destroy_rt         pthread_cond_destroy
#define pthread_cond_signal_rt          pthread_cond_signal
#define pthread_cond_broadcast_rt       pthread_cond_broadcast
#define pthread_cond_wait_rt            pthread_cond_wait
#define pthread_cond_timedwait_rt       pthread_cond_timedwait

#define pthread_barrier_open_rt         pthread_barrier_open
#define pthread_barrier_close_rt        pthread_barrier_close
#define pthread_barrier_init_rt         pthread_barrier_init
#define pthread_barrier_destroy_rt      pthread_barrier_destroy
#define pthread_barrier_wait_rt         pthread_barrier_wait

#define pthread_rwlock_open_rt          pthread_rwlock_open
#define pthread_rwlock_close_rt         pthread_rwlock_close
#define pthread_rwlock_init_rt          pthread_rwlock_init
#define pthread_rwlock_destroy_rt       pthread_rwlock_destroy
#define pthread_rwlock_rdlock_rt        pthread_rwlock_rdlock
#define pthread_rwlock_tryrdlock_rt     pthread_rwlock_tryrdlock
#define pthread_rwlock_timedrdlock_rt   pthread_rwlock_timedrdlock
#define pthread_rwlock_wrlock_rt        pthread_rwlock_wrlock
#define pthread_rwlock_trywrlock_rt     pthread_rwlock_trywrlock
#define pthread_rwlock_timedwrlock_rt   pthread_rwlock_timedwrlock
#define pthread_rwlock_unlock_rt        pthread_rwlock_unlock

#define pthread_spin_init_rt            pthread_spin_init
#define pthread_spin_destroy_rt         pthread_spin_destroy
#define pthread_spin_lock_rt            pthread_spin_lock
#define pthread_spin_trylock_rt         pthread_spin_trylock
#define pthread_spin_unlock_rt          pthread_spin_unlock

#define sched_get_max_priority_rt       sched_get_max_priority
#define sched_get_min_priority_rt       sched_get_min_priority

#define pthread_create_rt               pthread_create
#define pthread_yield_rt                pthread_yield
#define pthread_exit_rt                 pthread_exit
#define pthread_join_rt                 pthread_join
#define pthread_cancel_rt               pthread_cancel
#define pthread_equal_rt                pthread_equal
#define pthread_self_rt                 pthread_self
#define pthread_attr_init_rt            pthread_attr_init
#define pthread_attr_destroy_rt         pthread_attr_destroy
#define pthread_attr_setschedparam_rt   pthread_attr_setschedparam
#define pthread_attr_getschedparam_rt   pthread_attr_getschedparam
#define pthread_attr_setschedpolicy_rt  pthread_attr_setschedpolicy
#define pthread_attr_getschedpolicy_rt  pthread_attr_getschedpolicy
#define pthread_attr_setschedrr_rt      pthread_attr_setschedrr
#define pthread_attr_getschedrr_rt      pthread_attr_getschedrr
#define pthread_attr_setstacksize_rt    pthread_attr_setstacksize
#define pthread_attr_getstacksize_rt    pthread_attr_getstacksize
#define pthread_attr_setstack_rt        pthread_attr_setstack
#define pthread_attr_getstack_rt        pthread_attr_getstack
#define pthread_testcancel_rt           pthread_testcancel

#define clock_gettime_rt                clock_gettime
#define nanosleep_rt                    nanosleep

#define pthread_cleanup_push_rt         pthread_cleanup_push
#define pthread_cleanup_pop_rt          pthread_cleanup_pop

/*
 * _RT DO NOTHING FUNCTIONS 
 */

#define pthread_attr_setdetachstate_rt(attr, detachstate)
#define pthread_detach_rt(thread)
#define pthread_setconcurrency_rt(level)

#ifdef __KERNEL__

/*
 * KERNEL DO NOTHING FUNCTIONS (FOR RTAI HARD REAL TIME)
 */

#define pthread_setcanceltype_rt(type, oldtype)
#define pthread_setcancelstate_rt(state, oldstate)
#define pthread_attr_getstackaddr_rt(attr, stackaddr) 
#define pthread_attr_setstackaddr_rt(attr, stackaddr)
#define pthread_attr_setguardsize_rt(attr, guardsize) 
#define pthread_attr_getguardsize_rt(attr, guardsize)
#define pthread_attr_setscope_rt(attr, scope)
#define pthread_attr_getscope_rt(attr, scope)
#define pthread_attr_getdetachstate_rt(attr, detachstate)
#define pthread_attr_getdetachstate(attr, detachstate)
#define pthread_attr_setinheritsched_rt(attr, inherit)
#define pthread_attr_getinheritsched_rt(attr, inherit)
#define pthread_attr_setinheritsched(attr, inherit)
#define pthread_attr_getinheritsched(attr, inherit)

#include <linux/fcntl.h>
#include <linux/delay.h>

#include <rtai_malloc.h>
#include <rtai_rwl.h>
#include <rtai_spl.h>
#include <rtai_sem.h>
#include <rtai_sched.h>
#include <rtai_schedcore.h>


#define SET_ADR(s)     (((void **)s)[0])

#define RTAI_PNAME_MAXSZ  6
#define PTHREAD_BARRIER_SERIAL_THREAD -1


#ifndef MAX_PRIO
#define MAX_PRIO  99
#endif
#ifndef MIN_PRIO
#define MIN_PRIO  1
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  0
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC  1
#endif

#define STACK_SIZE     8192
#define RR_QUANTUM_NS  1000000

typedef struct { SEM sem; } sem_t;

typedef struct { SEM mutex; } pthread_mutex_t;

typedef unsigned long pthread_mutexattr_t;

typedef struct { SEM cond; } pthread_cond_t;

typedef unsigned long pthread_condattr_t;

typedef struct { SEM barrier; } pthread_barrier_t;

typedef unsigned long pthread_barrierattr_t;

typedef struct { RWL rwlock; } pthread_rwlock_t;

typedef unsigned long pthread_rwlockattr_t;

typedef unsigned long pthread_spinlock_t;

typedef struct rt_task_struct *pthread_t;

typedef struct pthread_attr {
	int stacksize;
	int policy;
	int rr_quantum_ns;
	int priority;
} pthread_attr_t;

typedef struct pthread_cookie {
	RT_TASK task;
	SEM sem;
	void (*task_fun)(long);
	long arg;
	void *cookie;
} pthread_cookie_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * SEMAPHORES
 */

static inline sem_t *sem_open(const char *namein, int oflags, int value, int type)
{
	char nametmp[RTAI_PNAME_MAXSZ + 1];
	int i;
	if (strlen(namein) > RTAI_PNAME_MAXSZ) {
		return (sem_t *)-ENAMETOOLONG;
	}
	
	for(i = 0; i < strlen(namein); i++) {
		if ((nametmp[i] = namein [i]) >= 'a' && nametmp[i] <= 'z') nametmp[i] += 'A' - 'a';
	}
	nametmp[i]='\0';
	if (!oflags || value <= SEM_TIMOUT) {
		SEM *tsem; 
		unsigned long handle = 0UL;
		if ((tsem = _rt_typed_named_sem_init(nam2num(nametmp), value, type, &handle))) {
			if ((handle) && (oflags == (O_CREAT | O_EXCL))) 	{
				return (sem_t *)-EEXIST;
			}
			return (sem_t *)tsem;
		}
		return (sem_t *)-ENOSPC;
	}
	return (sem_t *)-EINVAL;
}

static inline int sem_close(sem_t *sem)
{
	if (rt_sem_wait_if(&sem->sem)< 0) {
		return -EBUSY;
	}
	rt_named_sem_delete(&sem->sem);
	
	rt_free(sem);
	
	return  0;
}

static inline int sem_unlink(const char *namein)
{
	char nametmp[RTAI_PNAME_MAXSZ + 1];
	int i;
	SEM *sem;
	if (strlen(namein) > RTAI_PNAME_MAXSZ) {
		return -ENAMETOOLONG;
	}
	
	for(i = 0; i < strlen(namein); i++) {
		if ((nametmp[i] = namein [i]) >= 'a' && nametmp[i] <= 'z') nametmp[i] += 'A' - 'a';
	}
	nametmp[i]='\0';
	sem = rt_get_adr_cnt(nam2num(nametmp));
	if (sem) {
		if (rt_sem_wait_if(sem) >= 0) {
			rt_sem_signal(sem);
			rt_named_sem_delete(sem);
			return  0;
		}
		return -EBUSY;
	}
	return -ENOENT;
}


static inline int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	if (value < SEM_TIMOUT) {
		rt_typed_sem_init(&sem->sem, value, CNT_SEM | PRIO_Q);
		return 0;
	}
	return -EINVAL;
}

static inline int sem_destroy(sem_t *sem)
{
	if (rt_sem_wait_if(&sem->sem) >= 0) {
		rt_sem_signal(&sem->sem);
		rt_sem_delete(&sem->sem);
		return  0;
	}
	return -EBUSY;
}

static inline int sem_wait(sem_t *sem)
{
	return rt_sem_wait(&sem->sem) < SEM_TIMOUT ? 0 : -1;
}

static inline int sem_trywait(sem_t *sem)
{	
	return rt_sem_wait_if(&sem->sem) > 0 ? 0 : -EAGAIN;
}

static inline int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{	
	return rt_sem_wait_until(&sem->sem, timespec2count(abstime)) < SEM_TIMOUT ? 0 : -ETIMEDOUT;
}

static inline int sem_post(sem_t *sem)
{
	return rt_sem_signal(&sem->sem) < SEM_TIMOUT ? 0 : -ERANGE;
}

static inline int sem_getvalue(sem_t *sem, int *sval)
{
	*sval = rt_sem_count(&sem->sem);
	return 0;
}

/*
 * MUTEXES
 */
 
enum {
  PTHREAD_PROCESS_PRIVATE,
#define PTHREAD_PROCESS_PRIVATE PTHREAD_PROCESS_PRIVATE
  PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED  PTHREAD_PROCESS_SHARED
};

enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL,
  PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP
};

#define RTAI_MUTEX_DEFAULT    (1 << 0)
#define RTAI_MUTEX_ERRCHECK   (1 << 1)
#define RTAI_MUTEX_RECURSIVE  (1 << 2)
#define RTAI_MUTEX_PSHARED    (1 << 3)
 
static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	rt_typed_sem_init(&mutex->mutex,  !mutexattr || (((long *)mutexattr)[0] & RTAI_MUTEX_DEFAULT) ? RESEM_BINSEM : (((long *)mutexattr)[0] & RTAI_MUTEX_ERRCHECK) ? RESEM_CHEKWT : RESEM_RECURS, RES_SEM);
	return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	if (rt_sem_wait_if(&mutex->mutex) >= 0) {
		rt_sem_signal(&mutex->mutex);
		rt_sem_delete(&mutex->mutex);
		return  0;
	}
	return -EBUSY;	
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return rt_sem_wait(&mutex->mutex) < SEM_TIMOUT ? 0 : -EINVAL;
}

static inline int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
	return rt_sem_wait_until(&mutex->mutex, timespec2count(abstime)) < SEM_TIMOUT ? 0 : -1;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return rt_sem_wait_if(&mutex->mutex) > 0 ? 0 : -EBUSY;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return rt_sem_signal(&mutex->mutex) >= 0 ? 0 : -EINVAL;
}

static inline int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	((long *)attr)[0] = RTAI_MUTEX_DEFAULT;
	return 0;
}

static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	return 0;
}

static inline int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{	
	*pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
	return 0;
}

static inline int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		if (pshared == PTHREAD_PROCESS_PRIVATE) {
			((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
		} else {
			((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
		}
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
	switch (kind) {
		case PTHREAD_MUTEX_DEFAULT:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_RECURSIVE | RTAI_MUTEX_ERRCHECK)) | RTAI_MUTEX_DEFAULT;
			break;
		case PTHREAD_MUTEX_ERRORCHECK:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_RECURSIVE | RTAI_MUTEX_DEFAULT)) | RTAI_MUTEX_ERRCHECK;
			break;
		case PTHREAD_MUTEX_RECURSIVE:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_DEFAULT | RTAI_MUTEX_ERRCHECK)) | RTAI_MUTEX_RECURSIVE;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static inline int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *kind)
{
	switch (((long *)attr)[0] & (RTAI_MUTEX_DEFAULT | RTAI_MUTEX_ERRCHECK | RTAI_MUTEX_RECURSIVE)) {
		case RTAI_MUTEX_DEFAULT:
			*kind = PTHREAD_MUTEX_DEFAULT;
			break;
		case RTAI_MUTEX_ERRCHECK:
			*kind = PTHREAD_MUTEX_ERRORCHECK;
			break;
		case RTAI_MUTEX_RECURSIVE:
			*kind = PTHREAD_MUTEX_RECURSIVE;
			break;
	}
	return 0;
}

/*
 * CONDVARS
 */

static inline int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
	rt_typed_sem_init(&cond->cond, 0,  BIN_SEM | PRIO_Q);
	return 0;
}

static inline int pthread_cond_destroy(pthread_cond_t *cond)
{
	if (rt_sem_wait_if(&cond->cond) < 0) {
		return -EBUSY;
	}
	rt_sem_delete(&cond->cond);
	return  0;	
}

static inline int pthread_cond_signal(pthread_cond_t *cond)
{
	return rt_sem_signal(&cond->cond);
}

static inline int pthread_cond_broadcast(pthread_cond_t *cond)
{
	return rt_sem_broadcast(&cond->cond);
}

static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	return rt_cond_wait(&cond->cond, &mutex->mutex);
}

static inline int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
	return rt_cond_wait_until(&cond->cond, &mutex->mutex, timespec2count(abstime)) < SEM_TIMOUT ? 0 : -ETIMEDOUT;
}

static inline int pthread_condattr_init(unsigned long *attr)
{
	((long *)attr)[0] = 0;
	return 0;
}

static inline int pthread_condattr_destroy(pthread_condattr_t *attr)
{
	return 0;
}

static inline int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
	*pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
        return 0;
}

static inline int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		if (pshared == PTHREAD_PROCESS_PRIVATE) {
			((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
		} else {
			((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
		}
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_condattr_setclock(pthread_condattr_t *condattr, clockid_t clockid)
{
	if (clockid == CLOCK_MONOTONIC || clockid == CLOCK_REALTIME) {
		((int *)condattr)[0] = clockid;
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_condattr_getclock(pthread_condattr_t *condattr, clockid_t *clockid)
{
        if (clockid) {
		*clockid = ((int *)condattr)[0];
                return 0;
        }
        return -EINVAL;
}

/*
 * BARRIER
 */

static inline int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
	if (count > 0) {
		rt_typed_sem_init(&barrier->barrier, count, CNT_SEM | PRIO_Q);
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	if (rt_sem_wait_if(&barrier->barrier) < 0) {
		return -EBUSY;
	}
	return rt_sem_delete(&barrier->barrier) == RT_OBJINV ? -EINVAL : 0;
}

static inline int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	return rt_sem_wait_barrier(&barrier->barrier);
}

static inline int wrap_pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
	((long *)attr)[0] = PTHREAD_PROCESS_PRIVATE;
	return 0;
}

static inline int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
	return 0;
}

static inline int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		((long *)attr)[0] = pshared;
		return 0;
	}
	return -EINVAL;
}

static inline int wrap_pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared)
{
	*pshared = ((long *)attr)[0];
	return 0;
}

/*
 * RWLOCKS
 */

static inline int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
	return rt_rwl_init(&rwlock->rwlock);
}

static inline int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	return rt_rwl_delete(&rwlock->rwlock);
}

static inline int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	if (rt_rwl_rdlock(&rwlock->rwlock)) {
			return -EDEADLK;
	}
	return 0;
}

static inline int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	if (rt_rwl_rdlock_if(&rwlock->rwlock)) {
		return -EBUSY;
	}
	return 0;
}

static inline int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, struct timespec *abstime)
{
	return rt_rwl_rdlock_until(&rwlock->rwlock, timespec2count(abstime));
}

static inline int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	return rt_rwl_wrlock(&rwlock->rwlock);
}

static inline int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	if (rt_rwl_wrlock_if(&rwlock->rwlock)) {
		return -EBUSY;
	}
	return 0;	
}

static inline int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, struct timespec *abstime)
{
	return rt_rwl_wrlock_until(&rwlock->rwlock, timespec2count(abstime));
}

static inline int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	return rt_rwl_unlock(&rwlock->rwlock);
}

static inline int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	((long *)attr)[0] = 0;
	return 0;
}

static inline int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
	return 0;
}

static inline int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
        *pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
        return 0;

	return 0;
}

static inline int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
        if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
                if (pshared == PTHREAD_PROCESS_PRIVATE) {
                        ((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
                } else {
                        ((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
                }
                return 0;
        }
        return -EINVAL;
}

static inline int pthread_rwlockattr_getkind_np(const pthread_rwlockattr_t *attr, int *pref)
{
	return 0;
}

static inline int pthread_rwlockattr_setkind_np(pthread_rwlockattr_t *attr, int pref)
{
	return 0;
}

/*
 * SCHEDULING
 */
 
static inline int get_max_priority(int policy)
{
	return MAX_PRIO;
}

static inline int get_min_priority(int policy)
{
	return MIN_PRIO;
}

static void posix_wrapper_fun(pthread_cookie_t *cookie)
{
	cookie->task_fun(cookie->arg);
	rt_sem_broadcast(&cookie->sem);
	rt_sem_delete(&cookie->sem);
//	rt_task_suspend(&cookie->task);
} 

static inline int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	void *cookie_mem;

	cookie_mem = (void *)rt_malloc(sizeof(pthread_cookie_t) + L1_CACHE_BYTES);
	if (cookie_mem) {
		pthread_cookie_t *cookie;
		int err;
		/* align memory for RT_TASK to L1_CACHE_BYTES boundary */
		cookie = (pthread_cookie_t *)( (((unsigned long)cookie_mem) + ((unsigned long)L1_CACHE_BYTES)) & ~(((unsigned long)L1_CACHE_BYTES) - 1UL) );
		cookie->cookie = cookie_mem; /* save real memory block for pthread_join to free for us */
		(cookie->task).magic = 0;
		cookie->task_fun = (void *)start_routine;
		cookie->arg = (long)arg;
		if (!(err = rt_task_init(&cookie->task, (void *)posix_wrapper_fun, (long)cookie, (attr) ? attr->stacksize : STACK_SIZE, (attr) ? attr->priority : RT_SCHED_LOWEST_PRIORITY, 1, NULL))) {
			rt_typed_sem_init(&cookie->sem, 0, BIN_SEM | FIFO_Q);
			rt_task_resume(&cookie->task);
			*thread = &cookie->task;
			return 0;
		} else {
			rt_free(cookie->cookie);
			return err;
		}
	}
	return -ENOMEM;
}

static inline int pthread_yield(void)
{
	rt_task_yield();
	return 0;
}

static inline void pthread_exit(void *retval)
{
	RT_TASK *rt_task;
	SEM *sem;
	rt_task = rt_whoami();
	sem = &((pthread_cookie_t *)rt_task)->sem;
	rt_sem_broadcast(sem);
	rt_sem_delete(sem);
	rt_task->retval = (long)retval;
	rt_task_suspend(rt_task);
}

static inline int pthread_join(pthread_t thread, void **thread_return)
{
	int retval1, retval2;
	long retval_thread;
	SEM *sem;
	sem = &((pthread_cookie_t *)thread)->sem;
	if (rt_whoami()->priority != RT_SCHED_LINUX_PRIORITY){
		retval1 = rt_sem_wait(sem);
	} else {
		while ((retval1 = rt_sem_wait_if(sem)) <= 0) {
			msleep(10);
		}
	}
//	retval1 = 0;
	retval_thread = ((RT_TASK *)thread)->retval;
	if (thread_return) {
		*thread_return = (void *)retval_thread;
	}
	retval2 = rt_task_delete(thread);
	rt_free(((pthread_cookie_t *)thread)->cookie);
	return (retval1) ? retval1 : retval2;
}

static inline int pthread_cancel(pthread_t thread)
{
	int retval;
	if (!thread) {
		thread = rt_whoami();
	}
	retval = rt_task_delete(thread);
	rt_free(((pthread_cookie_t *)thread)->cookie);
	return retval;
}

static inline int pthread_equal(pthread_t thread1,pthread_t thread2)
{
	return thread1 == thread2;
}

static inline pthread_t pthread_self(void)
{
	return rt_whoami();
}

static inline int pthread_attr_init(pthread_attr_t *attr)
{
	attr->stacksize     = STACK_SIZE;
	attr->policy        = SCHED_FIFO;
	attr->rr_quantum_ns = RR_QUANTUM_NS;
	attr->priority      = 1;
	return 0;
}

static inline int pthread_attr_destroy(pthread_attr_t *attr)
{
	return 0;
}

static inline int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
	if(param->sched_priority < MIN_PRIO || param->sched_priority > MAX_PRIO) {
		return(-EINVAL);
	}
	attr->priority = MAX_PRIO - param->sched_priority;
	return 0;
}

static inline int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
	param->sched_priority = MAX_PRIO - attr->priority;
	return 0;
}

static inline int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	if(policy != SCHED_FIFO && policy != SCHED_RR) {
		return -EINVAL;
	}
	if ((attr->policy = policy) == SCHED_RR) {
		rt_set_sched_policy(rt_whoami(), SCHED_RR, attr->rr_quantum_ns);
	}
	return 0;
}


static inline int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
	*policy = attr->policy;
	return 0;
}

static inline int pthread_attr_setschedrr(pthread_attr_t *attr, int rr_quantum_ns)
{
	attr->rr_quantum_ns = rr_quantum_ns;
	return 0;
}


static inline int pthread_attr_getschedrr(const pthread_attr_t *attr, int *rr_quantum_ns)
{
	*rr_quantum_ns = attr->rr_quantum_ns;
	return 0;
}

static inline int pthread_attr_setstacksize(pthread_attr_t *attr, int stacksize)
{
	attr->stacksize = stacksize;
	return 0;
}

static inline int pthread_attr_getstacksize(const pthread_attr_t *attr, int *stacksize)
{
	*stacksize = attr->stacksize;
	return 0;
}

static inline int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, int stacksize)
{
	attr->stacksize = stacksize;
	return 0;
}

static inline int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, int *stacksize)
{
	*stacksize = attr->stacksize;
	return 0;
}

static inline void pthread_testcancel(void)
{
	rt_task_delete(rt_whoami());
	pthread_exit(NULL);
}

/*
 * SPINLOCKS
 */

static inline int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	if (lock) {
		*lock = 0UL;
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	if (lock) {
		if (*lock) {
			return -EBUSY;
		}
		*lock = 0UL;
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_spin_lock(pthread_spinlock_t *lock)
{
	if (lock) {
		unsigned long tid;
		if (((unsigned long *)lock)[0] == (tid = (unsigned long)(pthread_self()))) {
			return -EDEADLOCK;
		}
		while (atomic_cmpxchg((atomic_t *)lock, 0, tid));
		return 0;
	}
	return -EINVAL;
}

static inline int pthread_spin_trylock(pthread_spinlock_t *lock)
{
	if (lock) {
		unsigned long tid;
		tid = (unsigned long)(pthread_self());
		return atomic_cmpxchg((atomic_t *)lock, 0, tid) ? -EBUSY : 0;
	}
	return -EINVAL;
}

static inline int pthread_spin_unlock(pthread_spinlock_t *lock)
{
	if (lock) {
#if 0
		*lock = 0UL;
		return 0;
#else
		if (*lock != (unsigned long)pthread_self()) {
			return -EPERM;
		}
		*lock = 0UL;
		return 0;
#endif
	}
	return -EINVAL;
}

static inline int clock_getres(int clockid, struct timespec *res)
{
	res->tv_sec = 0;
	if (!(res->tv_nsec = count2nano(1))) {
		res->tv_nsec = 1;
	}
	return 0;
}

static inline int clock_gettime(int clockid, struct timespec *tp)
{
	count2timespec(rt_get_time(), tp);
	return 0;
}

static inline int clock_settime(int clockid, const struct timespec *tp)
{
	return 0;
}

static inline int clock_nanosleep(int clockid, int flags, const struct timespec *rqtp, struct timespec *rmtp)
{
	RTIME expire;
	if (rqtp->tv_nsec >= 1000000000L || rqtp->tv_nsec < 0 || rqtp->tv_sec < 0) {
		return -EINVAL;
	}
	rt_sleep_until(expire = flags ? timespec2count(rqtp) : rt_get_time() + timespec2count(rqtp));
	if ((expire -= rt_get_time()) > 0) {
		if (rmtp) {
			count2timespec(expire, rmtp);
		}
		return -EINTR;
	}
        return 0;
}

static inline int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
        RTIME expire;
        if (rqtp->tv_nsec >= 1000000000L || rqtp->tv_nsec < 0 || rqtp->tv_sec <
0) {
                return -EINVAL;
        }
        rt_sleep_until(expire = rt_get_time() + timespec2count(rqtp));
        if ((expire -= rt_get_time()) > 0) {
                if (rmtp) {
                        count2timespec(expire, rmtp);
                }
                return -EINTR;
        }
        return 0;
}

/*
 * TIMERS
 */
 
struct rt_handler_support {
	void (*_function)(sigval_t); 
	sigval_t funarg;
};

#ifndef RTAI_POSIX_HANDLER_WRPR
#define RTAI_POSIX_HANDLER_WRPR

static void handler_wrpr(unsigned long sup_data)
{
	((struct rt_handler_support *)sup_data)->_function(((struct rt_handler_support *)sup_data)->funarg);
}

#endif

static inline int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
	struct rt_tasklet_struct *timer;
	struct rt_handler_support *handler_data;			
		
	if (clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME) {
		return -EINTR; 
	}	
	if (evp == NULL) {
		return -EINTR; 
	} else {
		if (evp->sigev_notify == SIGEV_SIGNAL) {
			return -EINTR; 
		} else if (evp->sigev_notify == SIGEV_THREAD) {
			timer = rt_malloc(sizeof(struct rt_tasklet_struct));
			handler_data = rt_malloc(sizeof(struct rt_handler_support));
			handler_data->funarg = evp->sigev_value;
			handler_data->_function = evp->_sigev_un._sigev_thread._function;
			*timerid = rt_kptimer_create(timer, handler_wrpr, (unsigned long)handler_data, 1, 0);
		} else {
			return -EINTR; 
		}
	}
		
	return 0;
}

static inline int timer_getoverrun(timer_t timerid)
{
	return rt_ptimer_overrun(timerid);
}

static inline int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	RTIME timer_times[2];
	
	rt_ptimer_gettime(timerid, timer_times);
	count2timespec( timer_times[0], &(value->it_value) );
	count2timespec( timer_times[1], &(value->it_interval) );
	
	return 0;
}

static inline int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,  struct itimerspec *ovalue)
{
	if (ovalue != NULL) {
		timer_gettime(timerid, ovalue);
	}	
	rt_ptimer_settime(timerid, value, 0, flags);

	return 0;
}

static inline int timer_delete(timer_t timerid)
{
	rt_ptimer_delete(timerid, 0);
	return 0;	
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else  /* !__KERNEL__ */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <ctype.h>

#include <semaphore.h>
#include <limits.h>
#include <pthread.h>

struct task_struct;

#undef  SEM_VALUE_MAX 
#define SEM_VALUE_MAX  (SEM_TIMOUT - 1)
#define SEM_BINARY     (0x7FFFFFFF)

#define RTAI_PNAME_MAXSZ  6
#define SET_ADR(s)     (((void **)s)[0])
#define SET_VAL(s)     (((void **)s)[1])
#define INC_VAL(s)     atomic_inc((atomic_t *)&(((void **)s)[1]))
#define DEC_VAL(s)     atomic_dec_and_test((atomic_t *)&(((void **)s)[1]))
#define TST_VAL(s)     (((void **)s)[1])

#define LINUX_SIGNAL  32
#define LINUX_RT_SIGNAL  32

#include <asm/rtai_atomic.h>
#include <rtai_sem.h>
#include <rtai_signal.h>
#include <rtai_tasklets.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * SUPPORT STUFF
 */

static inline int MAKE_SOFT(void)
{
	if (rt_is_hard_real_time(rt_buddy())) {
		rt_make_soft_real_time();
		return 1;
	}
	return 0;
}

#define MAKE_HARD(hs)  do { if (hs) rt_make_hard_real_time(); } while (0)

RTAI_PROTO(void, count2timespec, (RTIME rt, struct timespec *t))
{
	t->tv_sec = (rt = count2nano(rt))/1000000000;
	t->tv_nsec = rt - t->tv_sec*1000000000LL;
}

RTAI_PROTO(void, nanos2timespec, (RTIME rt, struct timespec *t))
{
	t->tv_sec = rt/1000000000;
	t->tv_nsec = rt - t->tv_sec*1000000000LL;
}

RTAI_PROTO(RTIME, timespec2count, (const struct timespec *t))
{
	return nano2count(t->tv_sec*1000000000LL + t->tv_nsec);
}

RTAI_PROTO(RTIME, timespec2nanos,(const struct timespec *t))
{
	return t->tv_sec*1000000000LL + t->tv_nsec;
}

RTAI_PROTO(int, pthread_get_name_np, (void *adr, unsigned long *nameid))
{
	return (*nameid = rt_get_name(SET_ADR(adr))) ? 0 : EINVAL;
}

RTAI_PROTO(int, pthread_get_adr_np, (unsigned long nameid, void *adr))
{
	return (SET_ADR(adr) = rt_get_adr(nameid)) ? 0 : EINVAL;
}

/*
 * SEMAPHORES
 */

#define str2upr(si, so) \
do { int i; for (i = 0; i <= RTAI_PNAME_MAXSZ; i++) so[i] = toupper(si[i]); } while (0) 

RTAI_PROTO(sem_t *, __wrap_sem_open, (const char *namein, int oflags, int value, int type))
{
	char name[RTAI_PNAME_MAXSZ + 1];
	if (strlen(namein) > RTAI_PNAME_MAXSZ) {
		errno = ENAMETOOLONG;
		return SEM_FAILED;
	}
	str2upr(namein, name);
	if (!oflags || value <= SEM_VALUE_MAX) {
		void *tsem;
		unsigned long handle = 0UL;
		struct { unsigned long name; long value, type; unsigned long *handle; } arg = { nam2num(name), value, type, &handle };
		if ((tsem = rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW])) {
			int fd;
			void *psem;
			if (handle == (unsigned long)tsem) {
				if (oflags == (O_CREAT | O_EXCL)) {
					errno = EEXIST;
					return SEM_FAILED;
				}
				while ((fd = open(name, O_RDONLY)) <= 0 || read(fd, &psem, sizeof(psem)) != sizeof(psem));
				close(fd);
			} else {
				rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg);
				psem = malloc(sizeof(void *));
				((void **)psem)[0] = tsem;
				fd = open(name, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
				write(fd, &psem, sizeof(psem));
				close(fd);
			}
			return (sem_t *)psem;
		}
		errno = ENOSPC;
		return SEM_FAILED;
	}
	errno = EINVAL;
	return SEM_FAILED;
}

RTAI_PROTO(int, __wrap_sem_close, (sem_t *sem))
{
	struct { void *sem; } arg = { SET_ADR(sem) };
	if (arg.sem) {
		char name[RTAI_PNAME_MAXSZ + 1];
		num2nam(rt_get_name(SET_ADR(sem)), name);
		if (rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW] < 0) {
			errno = EBUSY;
			return -1;
		}
		if (!rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW]) {
			while (!unlink(name));
			free(sem);
		}
		return 0;
	}
	errno =  EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_unlink, (const char *namein))
{
	char name[RTAI_PNAME_MAXSZ + 1];
	int fd;
	void *psem;
	if (strlen(namein) > RTAI_PNAME_MAXSZ) {
		errno = ENAMETOOLONG;
		return -1;
	}
	str2upr(namein, name);
	if ((fd = open(name, O_RDONLY)) > 0 && read(fd, &psem, sizeof(psem)) == sizeof(psem)) {
		return __wrap_sem_close((sem_t *)psem);
	}
	errno = ENOENT;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_init, (sem_t *sem, int pshared, unsigned int value))
{
	if (value <= SEM_VALUE_MAX) {
		struct { unsigned long name; long value, type; unsigned long *handle; } arg = { rt_get_name(0), value, CNT_SEM | PRIO_Q, NULL };
		if (!(SET_ADR(sem) = rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW])) {
			errno = ENOSPC;
			return -1;
		}
		return 0;
	}
	errno = EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_destroy, (sem_t *sem))
{
	struct { void *sem; } arg = { SET_ADR(sem) };
	if (arg.sem) {
		if (rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW] < 0) {
			errno = EBUSY;
			return -1;
		}
		SET_ADR(sem) = NULL;
		while (rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW]);
		return 0;
	}
	errno =  EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_wait, (sem_t *sem))
{
	int oldtype, retval = -1;
	struct { void *sem; } arg = { SET_ADR(sem) };
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	pthread_testcancel();
	if (arg.sem) {
		if (abs(rtai_lxrt(BIDX, SIZARG, SEM_WAIT, &arg).i[LOW]) >= RTE_BASE) {
			errno =  EINTR;
		} else {
			retval = 0;
		}
	} else {
		errno =  EINVAL;
	}
	pthread_testcancel();
	pthread_setcanceltype(oldtype, NULL);
	return retval;
}

RTAI_PROTO(int, __wrap_sem_trywait, (sem_t *sem))
{
	struct { void *sem; } arg = { SET_ADR(sem) };
	if (arg.sem) {
		int retval;
		if (abs(retval = rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW]) >= RTE_BASE) {
			errno =  EINTR;
			return -1;
		}
		if (retval <= 0) {
			errno = EAGAIN;
			return -1;
		}
		return 0;
	}
	errno = EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_timedwait, (sem_t *sem, const struct timespec *abstime))
{
	int oldtype, retval = -1;
	struct { void *sem; RTIME time; } arg = { SET_ADR(sem), timespec2count(abstime) };
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	pthread_testcancel();
	if (arg.sem) {
		int ret;
		if (abs(ret = rtai_lxrt(BIDX, SIZARG, SEM_WAIT_UNTIL, &arg).i[LOW]) == RTE_TIMOUT) {
			errno =  ETIMEDOUT;
		} else if (ret >= RTE_BASE) {
			errno = EINTR;
		} else {
			retval = 0;
		}
	} else {
		errno =  EINVAL;
	}
	pthread_testcancel();
	pthread_setcanceltype(oldtype, NULL);
	return retval;
}

RTAI_PROTO(int, __wrap_sem_post, (sem_t *sem))
{
	struct { void *sem; } arg = { SET_ADR(sem) };
	if (arg.sem) {
		rtai_lxrt(BIDX, SIZARG, SEM_SIGNAL, &arg);
		return 0;
	}
	errno =  EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_sem_getvalue, (sem_t *sem, int *sval))
{
	struct { void *sem; } arg = { SET_ADR(sem) };
	if (arg.sem) {
		*sval = rtai_lxrt(BIDX, SIZARG, SEM_COUNT, &arg).i[LOW];
		return 0;
	}
	errno =  EINVAL;
	return -1;
}

/*
 * MUTEXES
 */

#define RTAI_MUTEX_DEFAULT    (1 << 0)
#define RTAI_MUTEX_ERRCHECK   (1 << 1)
#define RTAI_MUTEX_RECURSIVE  (1 << 2)
#define RTAI_MUTEX_PSHARED    (1 << 3)

RTAI_PROTO(int, __wrap_pthread_mutex_init, (pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr))
{
	struct { unsigned long name; long value, type; unsigned long *handle; } arg = { rt_get_name(0), !mutexattr || (((long *)mutexattr)[0] & RTAI_MUTEX_DEFAULT) ? RESEM_BINSEM : (((long *)mutexattr)[0] & RTAI_MUTEX_ERRCHECK) ? RESEM_CHEKWT : RESEM_RECURS, RES_SEM, NULL };
	SET_VAL(mutex) = 0;
	if (!(SET_ADR(mutex) = rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW])) {
		return ENOMEM;
	}
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_mutex_destroy, (pthread_mutex_t *mutex))
{
	struct { void *mutex; } arg = { SET_ADR(mutex) };
	if (arg.mutex) {
		int count;
		if (TST_VAL(mutex)) {
			return EBUSY;
		}
		if ((count = rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW]) <= 0 || count > 1) {
			if (count > 1 && count != RTE_DEADLOK) {
				rtai_lxrt(BIDX, SIZARG, SEM_SIGNAL, &arg);
			}
			return EBUSY;
		}
		SET_ADR(mutex) = NULL;
		while (rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW]);
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_mutex_lock, (pthread_mutex_t *mutex))
{
	struct { void *mutex; } arg = { SET_ADR(mutex) };
	if (arg.mutex) {
		int retval;
		while ((retval = rtai_lxrt(BIDX, SIZARG, SEM_WAIT, &arg).i[LOW]) == RTE_UNBLKD);
		return abs(retval) < RTE_BASE ? 0 : EDEADLOCK;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_mutex_trylock, (pthread_mutex_t *mutex))
{
	struct { void *mutex; } arg = { SET_ADR(mutex) };
	if (arg.mutex) {
		if (rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW] <= 0) {
			return EBUSY;
		}
		return 0;
	}
	return EINVAL;
}

#ifdef __USE_XOPEN2K
RTAI_PROTO(int, __wrap_pthread_mutex_timedlock, (pthread_mutex_t *mutex, const struct timespec *abstime))
{
	struct { void *mutex; RTIME time; } arg = { SET_ADR(mutex), timespec2count(abstime) };
	if (arg.mutex && abstime->tv_nsec >= 0 && abstime->tv_nsec < 1000000000) {
		int retval;
		while ((retval = rtai_lxrt(BIDX, SIZARG, SEM_WAIT_UNTIL, &arg).i[LOW]) == RTE_UNBLKD);
		if (abs(retval) < RTE_BASE) {
			return 0;
		}
		if (retval == RTE_TIMOUT) {
			return ETIMEDOUT;
		}
	}
	return EINVAL;
}
#endif

RTAI_PROTO(int, __wrap_pthread_mutex_unlock, (pthread_mutex_t *mutex))
{
	struct { void *mutex; } arg = { SET_ADR(mutex) };
	if (arg.mutex) {
		return rtai_lxrt(BIDX, SIZARG, SEM_SIGNAL, &arg).i[LOW] == RTE_PERM ? EPERM : 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_init, (pthread_mutexattr_t *attr))
{
	((long *)attr)[0] = RTAI_MUTEX_DEFAULT;
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_destroy, (pthread_mutexattr_t *attr))
{
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_getpshared, (const pthread_mutexattr_t *attr, int *pshared))
{	
	*pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_setpshared, (pthread_mutexattr_t *attr, int pshared))
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		if (pshared == PTHREAD_PROCESS_PRIVATE) {
			((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
		} else {
			((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
		}
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_settype, (pthread_mutexattr_t *attr, int kind))
{
	switch (kind) {
		case PTHREAD_MUTEX_DEFAULT:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_RECURSIVE | RTAI_MUTEX_ERRCHECK)) | RTAI_MUTEX_DEFAULT;
			break;
		case PTHREAD_MUTEX_ERRORCHECK:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_RECURSIVE | RTAI_MUTEX_DEFAULT)) | RTAI_MUTEX_ERRCHECK;
			break;
		case PTHREAD_MUTEX_RECURSIVE:
			((long *)attr)[0] = (((long *)attr)[0] & ~(RTAI_MUTEX_DEFAULT | RTAI_MUTEX_ERRCHECK)) | RTAI_MUTEX_RECURSIVE;
			break;
		default:
			return EINVAL;
	}
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_mutexattr_gettype, (const pthread_mutexattr_t *attr, int *kind))
{
	switch (((long *)attr)[0] & (RTAI_MUTEX_DEFAULT | RTAI_MUTEX_ERRCHECK | RTAI_MUTEX_RECURSIVE)) {
		case RTAI_MUTEX_DEFAULT:
			*kind = PTHREAD_MUTEX_DEFAULT;
			break;
		case RTAI_MUTEX_ERRCHECK:
			*kind = PTHREAD_MUTEX_ERRORCHECK;
			break;
		case RTAI_MUTEX_RECURSIVE:
			*kind = PTHREAD_MUTEX_RECURSIVE;
			break;
	}
	return 0;
}

RTAI_PROTO(int, pthread_make_periodic_np, (pthread_t thread, struct timespec *start_delay, struct timespec *period))
{
        struct { RT_TASK *task; RTIME start_time, period; } arg = { NULL, start_delay->tv_sec*1000000000LL + start_delay->tv_nsec, period->tv_sec*1000000000LL + period->tv_nsec };
	int retval;
        return !(retval = rtai_lxrt(BIDX, SIZARG, MAKE_PERIODIC_NS, &arg).i[LOW]) ? 0 : retval == RTE_UNBLKD ? EINTR : ETIMEDOUT;
}

RTAI_PROTO(int, pthread_wait_period_np, (void))
{
        struct { unsigned long dummy; } arg;
        return rtai_lxrt(BIDX, SIZARG, WAIT_PERIOD, &arg).i[LOW];
}

/*
 * CONDVARS
 */

RTAI_PROTO(int, __wrap_pthread_cond_init, (pthread_cond_t *cond, pthread_condattr_t *cond_attr))
{
	struct { unsigned long name; long value, type; unsigned long *handle; } arg = { rt_get_name(0), 0, BIN_SEM | PRIO_Q, NULL };
	if (!(SET_ADR(cond) = rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW])) {
		return ENOMEM;
	}
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_cond_destroy, (pthread_cond_t *cond))
{
	struct { void *cond; } arg = { SET_ADR(cond) };
	if (arg.cond) {
		if (rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW] < 0) {
			return EBUSY;
		}
		SET_ADR(cond) = NULL;
		while (rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW]);
	}
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_cond_signal, (pthread_cond_t *cond))
{
	struct { void *cond; } arg = { SET_ADR(cond) };
	if (arg.cond) {
		rtai_lxrt(BIDX, SIZARG, COND_SIGNAL, &arg);
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_cond_broadcast, (pthread_cond_t *cond))
{
	struct { void *cond; } arg = { SET_ADR(cond) };
	if (arg.cond) {
		rtai_lxrt(BIDX, SIZARG, SEM_BROADCAST, &arg);
		return 0;
	}
	return EINVAL;
}

static void internal_cond_cleanup(void *mutex) { DEC_VAL(mutex); }

RTAI_PROTO(int, __wrap_pthread_cond_wait, (pthread_cond_t *cond, pthread_mutex_t *mutex))
{
	int oldtype, retval;
	struct { void *cond; void *mutex; } arg = { SET_ADR(cond), SET_ADR(mutex) };
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	pthread_testcancel();
	if (arg.cond && arg.mutex) {
		pthread_cleanup_push(internal_cond_cleanup, mutex);
		INC_VAL(mutex);
		retval = rtai_lxrt(BIDX, SIZARG, COND_WAIT, &arg).i[LOW];
		retval = !retval || retval == RTE_UNBLKD ? 0 : EPERM;
		DEC_VAL(mutex);
		pthread_cleanup_pop(0);
	} else {
		retval = EINVAL;
	}
	pthread_testcancel();
	pthread_setcanceltype(oldtype, NULL);
	return retval;
}

RTAI_PROTO(int, __wrap_pthread_cond_timedwait, (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime))
{
	int oldtype, retval;
	struct { void *cond; void *mutex; RTIME time; } arg = { SET_ADR(cond), SET_ADR(mutex), timespec2count(abstime) };
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	pthread_testcancel();
	if (arg.cond && arg.mutex && abstime->tv_nsec >= 0 && abstime->tv_nsec < 1000000000) {
		pthread_cleanup_push(internal_cond_cleanup, mutex);
		INC_VAL(mutex);
		if (abs(retval = rtai_lxrt(BIDX, SIZARG, COND_WAIT_UNTIL, &arg).i[LOW]) == RTE_TIMOUT) {
			retval = ETIMEDOUT;
		} else {
			retval = !retval ? 0 : EPERM;
		}
		DEC_VAL(mutex);
		pthread_cleanup_pop(0);
	} else {
		retval = EINVAL;
	}
	pthread_testcancel();
	pthread_setcanceltype(oldtype, NULL);
	return retval;
}

RTAI_PROTO(int, __wrap_pthread_condattr_init, (pthread_condattr_t *attr))
{
	((long *)attr)[0] = 0;
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_condattr_destroy, (pthread_condattr_t *attr))
{
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_condattr_getpshared, (const pthread_condattr_t *attr, int *pshared))
{
	*pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
        return 0;
}

RTAI_PROTO(int, __wrap_pthread_condattr_setpshared, (pthread_condattr_t *attr, int pshared))
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		if (pshared == PTHREAD_PROCESS_PRIVATE) {
			((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
		} else {
			((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
		}
		return 0;
	}
	return EINVAL;
}

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC  1
#endif

RTAI_PROTO(int, __wrap_pthread_condattr_setclock, (pthread_condattr_t *condattr, clockid_t clockid))
{
	if (clockid == CLOCK_MONOTONIC || clockid == CLOCK_REALTIME) {
		((int *)condattr)[0] = clockid;
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_condattr_getclock, (pthread_condattr_t *condattr, clockid_t *clockid))
{
        if (clockid) {
		*clockid = ((int *)condattr)[0];
                return 0;
        }
        return EINVAL;
}

/*
 * RWLOCKS
 */

RTAI_PROTO(int, __wrap_pthread_rwlock_init, (pthread_rwlock_t *rwlock, pthread_rwlockattr_t *attr))
{
	struct { unsigned long name; long type; } arg = { rt_get_name(0), RESEM_CHEKWT };
	((pthread_rwlock_t **)rwlock)[0] = (pthread_rwlock_t *)rtai_lxrt(BIDX, SIZARG, LXRT_RWL_INIT, &arg).v[LOW];
        return 0;
}

RTAI_PROTO(int, __wrap_pthread_rwlock_destroy, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return rtai_lxrt(BIDX, SIZARG, LXRT_RWL_DELETE, &arg).i[LOW] > 0 ? 0 : EINVAL;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_rwlock_rdlock, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK, &arg).i[LOW] ? EDEADLOCK : 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_rwlock_tryrdlock, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK_IF, &arg).i[LOW] ? EBUSY : 0;
	}
	return EINVAL;
}

#ifdef __USE_XOPEN2K
RTAI_PROTO(int, __wrap_pthread_rwlock_timedrdlock, (pthread_rwlock_t *rwlock, struct timespec *abstime))
{
	struct { void *rwlock; RTIME time; } arg = { SET_ADR(rwlock), timespec2count(abstime) };
	if (arg.rwlock && abstime->tv_nsec >= 0 && abstime->tv_nsec < 1000000000) {
		return rtai_lxrt(BIDX, SIZARG, RWL_RDLOCK_UNTIL, &arg).i[LOW] ? ETIMEDOUT : 0;
	}
	return EINVAL;
}
#endif

RTAI_PROTO(int, __wrap_pthread_rwlock_wrlock, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK, &arg).i[LOW] ? EDEADLOCK : 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_rwlock_trywrlock, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK_IF, &arg).i[LOW] ? EBUSY : 0;
	}
	return EINVAL;
}

#ifdef __USE_XOPEN2K
RTAI_PROTO(int, __wrap_pthread_rwlock_timedwrlock, (pthread_rwlock_t *rwlock, struct timespec *abstime))
{
	struct { void *rwlock; RTIME time; } arg = { SET_ADR(rwlock), timespec2count(abstime) };
	if (arg.rwlock && abstime->tv_nsec >= 0 && abstime->tv_nsec < 1000000000) {
		return rtai_lxrt(BIDX, SIZARG, RWL_WRLOCK_UNTIL, &arg).i[LOW] ? ETIMEDOUT : 0;
	}
	return EINVAL;
}
#endif

RTAI_PROTO(int, __wrap_pthread_rwlock_unlock, (pthread_rwlock_t *rwlock))
{
	struct { void *rwlock; } arg = { SET_ADR(rwlock) };
	if (arg.rwlock) {
		return !rtai_lxrt(BIDX, SIZARG, RWL_UNLOCK, &arg).i[LOW] ? 0 : EPERM;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_init, (pthread_rwlockattr_t *attr))
{
	((long *)attr)[0] = 0;
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_destroy, (pthread_rwlockattr_t *attr))
{
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_getpshared, (const pthread_rwlockattr_t *attr, int *pshared))
{
        *pshared = (((long *)attr)[0] & RTAI_MUTEX_PSHARED) != 0 ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
        return 0;

	return 0;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_setpshared, (pthread_rwlockattr_t *attr, int pshared))
{
        if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
                if (pshared == PTHREAD_PROCESS_PRIVATE) {
                        ((long *)attr)[0] &= ~RTAI_MUTEX_PSHARED;
                } else {
                        ((long *)attr)[0] |= RTAI_MUTEX_PSHARED;
                }
                return 0;
        }
        return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_getkind_np, (const pthread_rwlockattr_t *attr, int *pref))
{
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_rwlockattr_setkind_np, (pthread_rwlockattr_t *attr, int pref))
{
	return 0;
}

/*
 * BARRIERS
 */

#ifdef __USE_XOPEN2K

RTAI_PROTO(int, __wrap_pthread_barrier_init,(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count))
{
	if (count > 0) {
		struct { unsigned long name; long count, type; unsigned long *handle; } arg = { rt_get_name(0), count, CNT_SEM | PRIO_Q, NULL };
		return (((pthread_barrier_t **)barrier)[0] = (pthread_barrier_t *)rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW]) ? 0 : ENOMEM;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_barrier_destroy,(pthread_barrier_t *barrier))
{
	struct { void *sem; } arg = { SET_ADR(barrier) };
	SET_ADR(barrier) = NULL;
	if (rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW] < 0) {
		return EBUSY;
	}
	return rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW] == RT_OBJINV ? EINVAL : 0;
}

RTAI_PROTO(int, __wrap_pthread_barrier_wait,(pthread_barrier_t *barrier))
{
	struct { void *sem; } arg = { SET_ADR(barrier) };
	if (arg.sem) {
		return !rtai_lxrt(BIDX, SIZARG, SEM_WAIT_BARRIER, &arg).i[LOW] ? PTHREAD_BARRIER_SERIAL_THREAD : 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_barrierattr_init, (pthread_barrierattr_t *attr))
{
	((long *)attr)[0] = PTHREAD_PROCESS_PRIVATE;
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_barrierattr_destroy, (pthread_barrierattr_t *attr))
{
	return 0;
}

RTAI_PROTO(int, __wrap_pthread_barrierattr_setpshared, (pthread_barrierattr_t *attr, int pshared))
{
	if (pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED) {
		((long *)attr)[0] = pshared;
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_barrierattr_getpshared, (const pthread_barrierattr_t *attr, int *pshared))
{
	*pshared = ((long *)attr)[0];
	return 0;
}

#endif

/*
 * SCHEDULING
 */

#define PTHREAD_SOFT_REAL_TIME_NP  1
#define PTHREAD_HARD_REAL_TIME_NP  2

RTAI_PROTO(int, pthread_setschedparam_np, (int priority, int policy, int rr_quantum_ns, unsigned long cpus_allowed, int mode))
{ 
	RT_TASK *task;
	if ((task = rt_buddy())) {
		int hs;
		if (cpus_allowed) {
			hs = MAKE_SOFT();
			rt_task_init_schmod(0, 0, 0, 0, 0, cpus_allowed);
			if (!mode) {
				MAKE_HARD(hs);
			}
		}
		if (priority >= 0) {
			rt_change_prio(task, priority);
		}
	} else if (policy == SCHED_FIFO || policy == SCHED_RR || priority >= 0 || cpus_allowed) {
		rt_task_init_schmod(rt_get_name(NULL), priority, 0, 0, policy, cpus_allowed);
		rt_grow_and_lock_stack(PTHREAD_STACK_MIN);
	} else {
		return EINVAL;
	}
	if (policy == SCHED_FIFO || policy == SCHED_RR) {
		rt_set_sched_policy(task, policy = SCHED_FIFO ? 0 : 1, rr_quantum_ns);
	}
	if (mode) {
		if (mode == PTHREAD_HARD_REAL_TIME_NP) {
			rt_make_hard_real_time();
		} else {
			rt_make_soft_real_time();
		}
	}
	return 0;
}

RTAI_PROTO(void, pthread_hard_real_time_np, (void))
{
	rt_make_hard_real_time();
}

RTAI_PROTO(void, pthread_soft_real_time_np, (void))
{
	rt_make_soft_real_time();
}

RTAI_PROTO(int, pthread_gettid_np, (void))
{
        struct { unsigned long dummy; } arg;
        return rtai_lxrt(BIDX, SIZARG, RT_GETTID, &arg).i[LOW];
}

#define PTHREAD_SOFT_REAL_TIME  PTHREAD_SOFT_REAL_TIME_NP
#define PTHREAD_HARD_REAL_TIME  PTHREAD_HARD_REAL_TIME_NP
#define pthread_init_real_time_np(a, b, c, d, e) \
	pthread_setschedparam_np (b, c, 0, d, e)
#define pthread_make_hard_real_time_np() \
	pthread_hard_real_time_np()
#define pthread_make_soft_real_time_np() \
	pthread_soft_real_time_np()

#if 0
#if 1
int __real_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
RTAI_PROTO(int, __wrap_pthread_create,(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg))
{
#include <sys/poll.h>

	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_pthread_create(thread, attr, start_routine, arg);
	MAKE_HARD(hs);
	return ret;
}
#else
#include <sys/mman.h>

struct local_pthread_args_struct { void *(*start_routine)(void *); void *arg; int pipe[3]; };

#ifndef __SUPPORT_THREAD_FUN_
#define __SUPPORT_THREAD_FUN_

static void *support_thread_fun(struct local_pthread_args_struct *args)
{
        RT_TASK *task;
	void *(*start_routine)(void *) = args->start_routine;
	void *arg = args->arg;
	pthread_t thread;
	int policy;
	struct sched_param param;
	
	pthread_getschedparam(thread = pthread_self(), &policy, &param);
	if (policy == SCHED_OTHER) {
		policy = SCHED_RR;
		param.sched_priority = sched_get_priority_min(SCHED_RR);
	}
	pthread_setschedparam(pthread_self(), policy, &param);
	task = rt_task_init_schmod(rt_get_name(0), sched_get_priority_max(policy) - param.sched_priority, 0, 0, policy, 0xF);
	close(args->pipe[1]);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	start_routine(arg);
	rt_make_soft_real_time();
	return NULL;
}

#endif /* __SUPPORT_THREAD_FUN_ */

RTAI_PROTO(int, __wrap_pthread_create,(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg))
{
	int hs, ret;
	struct local_pthread_args_struct args = { start_routine, arg };
	hs = MAKE_SOFT();
	pipe(args.pipe);
	ret = pthread_create(thread, attr, (void *)support_thread_fun, (void *)&args);
	read(args.pipe[0], &args.pipe[2], 1);
	close(args.pipe[0]);
	MAKE_HARD(hs);
	return ret;
}
#endif

int __real_pthread_cancel(pthread_t thread);
RTAI_PROTO(int, __wrap_pthread_cancel,(pthread_t thread))
{
	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_pthread_cancel(thread);
	MAKE_HARD(hs);
	return ret;
}

int __real_pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask);
RTAI_PROTO(int, __wrap_pthread_sigmask,(int how, const sigset_t *newmask, sigset_t *oldmask))
{
	return __real_pthread_sigmask(how, newmask, oldmask);
	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_pthread_sigmask(how, newmask, oldmask);
	MAKE_HARD(hs);
	return ret;
}

int __real_pthread_kill(pthread_t thread, int signo);
RTAI_PROTO(int, __wrap_pthread_kill,(pthread_t thread, int signo))
{
	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_pthread_kill(thread, signo);
	MAKE_HARD(hs);
	return ret;
}


int __real_sigwait(const sigset_t *set, int *sig);
RTAI_PROTO(int, __wrap_sigwait,(const sigset_t *set, int *sig))
{
	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_sigwait(set, sig);
	MAKE_HARD(hs);
	return ret;
}

void __real_pthread_testcancel(void);
RTAI_PROTO(void, __wrap_pthread_testcancel,(void))
{
	__real_pthread_testcancel();
	return;
	int oldtype, oldstate;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
	if (oldstate != PTHREAD_CANCEL_DISABLE && oldtype != PTHREAD_CANCEL_DEFERRED) {
		MAKE_SOFT();
		rt_task_delete(rt_buddy());
		pthread_exit(NULL);
	}
	pthread_setcanceltype(oldtype, &oldtype);
	pthread_setcancelstate(oldstate, &oldstate);
}

int __real_pthread_yield(void);
RTAI_PROTO(int, __wrap_pthread_yield,(void))
{
	if (rt_is_hard_real_time(rt_buddy())) {
		struct { unsigned long dummy; } arg;
		rtai_lxrt(BIDX, SIZARG, YIELD, &arg);
		return 0;
	}
	return __real_pthread_yield();
}

void __real_pthread_exit(void *retval);
RTAI_PROTO(void, __wrap_pthread_exit,(void *retval))
{
	MAKE_SOFT();
	rt_task_delete(NULL);
	__real_pthread_exit(retval);
}

int __real_pthread_join(pthread_t thread, void **thread_return);
RTAI_PROTO(int, __wrap_pthread_join,(pthread_t thread, void **thread_return))
{
	int hs, ret;
	hs = MAKE_SOFT();
	ret = __real_pthread_join(thread, thread_return);
	MAKE_HARD(hs);
	return ret;
}
#endif

/*
 * SPINLOCKS
 */

#ifdef __USE_XOPEN2K

#if 0
#define ORIGINAL_TEST
RTAI_PROTO(int, __wrap_pthread_spin_init, (pthread_spinlock_t *lock, int pshared))
{
	return lock ? (((pid_t *)lock)[0] = 0) : EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_destroy, (pthread_spinlock_t *lock))
{
	if (lock) {
		return ((pid_t *)lock)[0] ? EBUSY : (((pid_t *)lock)[0] = 0);
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_lock,(pthread_spinlock_t *lock))
{
	if (lock) {
		while (atomic_cmpxchg(lock, 0, 1));
		return 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_trylock,(pthread_spinlock_t *lock))
{
	if (lock) {
		return atomic_cmpxchg(lock, 0, 1) ? EBUSY : 0;
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_unlock,(pthread_spinlock_t *lock))
{
	if (lock) {
		return ((pid_t *)lock)[0] = 0;
	}
	return EINVAL;
}
#else
static inline int _pthread_gettid_np(void)
{
        struct { unsigned long dummy; } arg;
        return rtai_lxrt(BIDX, SIZARG, RT_GETTID, &arg).i[LOW];
}

RTAI_PROTO(int, __wrap_pthread_spin_init, (pthread_spinlock_t *lock, int pshared))
{
	return lock ? (((pid_t *)lock)[0] = 0) : EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_destroy, (pthread_spinlock_t *lock))
{
	if (lock) {
		return ((pid_t *)lock)[0] ? EBUSY : (((pid_t *)lock)[0] = 0);
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_lock,(pthread_spinlock_t *lock))
{
	if (lock) {
		pid_t tid = _pthread_gettid_np();
		if (((pid_t *)lock)[0] == tid) {
			return EDEADLOCK;
		} else {
			unsigned long l = lock[0];
			while (atomic_cmpxchg((void *)&l, 0, tid));
			lock[0] = l;
			return 0;
		}
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_trylock,(pthread_spinlock_t *lock))
{
	if (lock) {
		unsigned long l = lock[0];
		if (atomic_cmpxchg((void *)&l, 0, _pthread_gettid_np())) {
			return EBUSY;
		} else {
			lock[0] = l;
			return 0;
		}
	}
	return EINVAL;
}

RTAI_PROTO(int, __wrap_pthread_spin_unlock,(pthread_spinlock_t *lock))
{
	if (lock) {
#if 0
		return ((pid_t *)lock)[0] = 0;
#else
		return ((pid_t *)lock)[0] != _pthread_gettid_np() ? EPERM : (((pid_t *)lock)[0] = 0);
#endif
	}
	return EINVAL;
}
#endif

#endif

/*
 * TIMINGS
 */

RTAI_PROTO(int, __wrap_clock_getres, (clockid_t clockid, struct timespec *res))
{
	if (clockid == CLOCK_MONOTONIC || clockid == CLOCK_REALTIME) {
		res->tv_sec = 0;
		if (!(res->tv_nsec = count2nano(1))) {
			res->tv_nsec = 1;
		}
		return 0;
	}
	errno = -EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_clock_gettime, (clockid_t clockid, struct timespec *tp))
{
	if (clockid == CLOCK_MONOTONIC) {
		count2timespec(rt_get_tscnt(), tp);
		return 0;
	} else if (clockid == CLOCK_REALTIME) {
		count2timespec(rt_get_real_time(), tp);
		return 0;
	}
	errno = -EINVAL;
	return -1;
}

RTAI_PROTO(int, __wrap_clock_settime, (clockid_t clockid, const struct timespec *tp))
{
	if (clockid == CLOCK_REALTIME) {
		int hs;
		hs = MAKE_SOFT();
		rt_gettimeorig(NULL);
		MAKE_HARD(hs);
		return 0;
	}
	errno = -ENOTSUP;
	return -1;
}

RTAI_PROTO(int, __wrap_clock_nanosleep,(clockid_t clockid, int flags, const struct timespec *rqtp, struct timespec *rmtp))
{
	int canc_type, ret;
	RTIME expire;

	if (clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME) {
		return ENOTSUP;
        }

	if (rqtp->tv_nsec >= 1000000000L || rqtp->tv_nsec < 0 || rqtp->tv_sec < 0) {
		return EINVAL;
	}

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &canc_type);

	expire = timespec2count(rqtp);
	if (clockid == CLOCK_MONOTONIC) {
		if (flags != TIMER_ABSTIME) {
			expire += rt_get_tscnt();
		}
		ret = rt_sleep_until(expire);
        	expire -= rt_get_tscnt();
	} else {
		if (flags != TIMER_ABSTIME) {
			expire += rt_get_real_time();
		}
		ret = rt_sleep_until(expire);
		expire -= rt_get_real_time();
	}
	if (expire > 0) {
		if (rmtp) {
			count2timespec(expire, rmtp);
		}
		return ret == RTE_UNBLKD ? EINTR : 0;
	}
	
	pthread_setcanceltype(canc_type, NULL);

	return 0;
}

RTAI_PROTO(int, __wrap_nanosleep,(const struct timespec *rqtp, struct timespec *rmtp))
{
        int canc_type, ret;
	RTIME expire;
	if (rqtp->tv_nsec >= 1000000000L || rqtp->tv_nsec < 0 || rqtp->tv_sec < 0) {
		return -EINVAL;
	}

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &canc_type);

	ret = rt_sleep_until(expire = rt_get_tscnt() + timespec2count(rqtp));
	if ((expire -= rt_get_tscnt()) > 0) {
		if (rmtp) {
			count2timespec(expire, rmtp);
		}
		errno = -EINTR;
		return ret == RTE_UNBLKD ? -1 : 0;
	}

	pthread_setcanceltype(canc_type, NULL);

        return 0;
}

/*
 * POSIX TIMERS
 */
 
struct supfun_data { struct rt_tasklet_struct *tasklet; long signum; void (*handler)(unsigned long); unsigned long data; int done;};

static int support_posix_timer(void *data)
{
	RT_TASK *task;
	struct rt_tasklet_struct usptasklet;
	struct supfun_data supfun_data;
	
	supfun_data = *(struct supfun_data *)data;

	if (!(task = rt_thread_init((unsigned long)supfun_data.tasklet, 98, 0, SCHED_FIFO, 0xF))) {
		printf("CANNOT INIT POSIX TIMER SUPPORT TASKLET\n");
		return -1;
	} else {
		struct { struct rt_tasklet_struct *tasklet, *usptasklet; RT_TASK *task; } reg = { supfun_data.tasklet, &usptasklet, task };
		rtai_lxrt(TASKLETS_IDX, sizeof(reg), REG_TASK, &reg);
	}

	mlockall(MCL_CURRENT | MCL_FUTURE);
	rt_make_hard_real_time();
	supfun_data.done = 1;
	
	if (supfun_data.signum)	{
		while (1) {
			rt_task_suspend(task);
			if (usptasklet.handler) {
				pthread_kill((pthread_t)supfun_data.data, supfun_data.signum);
			} else {
				break;
			}
		}
	} else {	
		while (1) {	
			rt_task_suspend(task);
			if (usptasklet.handler) {
				supfun_data.handler(supfun_data.data);
			} else {
				break;
			}
		}
	}
	
	rt_make_soft_real_time();
	rt_task_delete(task);

	return 0;
}

RTAI_PROTO (int, __wrap_timer_create, (clockid_t clockid, struct sigevent *evp, timer_t *timerid))
{
	void (*handler)(unsigned long) = ((void (*)(unsigned long))1);
	int i;
	unsigned long data = 0;
	struct supfun_data supfun_data;
	
	if (clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME) {
		errno = ENOTSUP;
		return -1;
	}

		
	if (evp == NULL) {
			supfun_data.signum = SIGALRM;
	} else {
		if (evp->sigev_notify == SIGEV_SIGNAL) {
			supfun_data.signum = evp->sigev_signo;
			data = (unsigned long)evp->sigev_value.sival_ptr;
		} else if (evp->sigev_notify == SIGEV_THREAD) {
			supfun_data.signum = 0;
			data = (unsigned long)evp->sigev_value.sival_int;
			if (!(handler = (void (*)(unsigned long))evp->_sigev_un._sigev_thread._function)) {
				errno = EINVAL;
				return -1;
			}
		}
	}

	struct { struct rt_tasklet_struct **timer; } arg = { &supfun_data.tasklet };
	*timerid = (timer_t)rtai_lxrt(TASKLETS_IDX, SIZARG, PTIMER_CREATE, &arg).i[LOW];
	supfun_data.done    = 0;
	supfun_data.handler = handler; 
	supfun_data.data    = data; 
	rt_thread_create((void *)support_posix_timer, &supfun_data, TASKLET_STACK_SIZE);
#define POLLS_PER_SEC 500
	for (i = 0; i < POLLS_PER_SEC/5 && !supfun_data.done; i++) {
		struct timespec delay = { 0, 1000000000/POLLS_PER_SEC };
		nanosleep(&delay, NULL);
	}
#undef POLLS_PER_SEC
	return 0;
}

RTAI_PROTO (int, __wrap_timer_gettime, (timer_t timerid, struct itimerspec *value))
{
	RTIME timer_times[2];
	
	struct { timer_t timer; RTIME *timer_times; } arg = { timerid, timer_times };
	rtai_lxrt(TASKLETS_IDX, SIZARG, PTIMER_GETTIME, &arg);

	count2timespec( timer_times[0], &(value->it_value) );
	count2timespec( timer_times[1], &(value->it_interval) );
	
	return 0;
}

RTAI_PROTO (int, __wrap_timer_settime, (timer_t timerid, int flags, const struct itimerspec *value,  struct itimerspec *ovalue))
{
	if (ovalue != NULL) {
		__wrap_timer_gettime(timerid, ovalue);
	}
	struct { timer_t timer; const struct itimerspec *value; unsigned long data; long flags; } arg = { timerid, value, pthread_self(), flags};
	rtai_lxrt(TASKLETS_IDX, SIZARG, PTIMER_SETTIME, &arg);
	
	return 0;
}

RTAI_PROTO (int, __wrap_timer_getoverrun, (timer_t timerid))
{
	struct { timer_t timer; } arg = { timerid };
	return rtai_lxrt(TASKLETS_IDX, SIZARG, PTIMER_OVERRUN, &arg).rt;
}

RTAI_PROTO (int, __wrap_timer_delete, (timer_t timerid))
{
	int thread;
	
	struct { timer_t timer; long space;} arg_del = { timerid, 1 };
	if ((thread = rtai_lxrt(TASKLETS_IDX, sizeof(arg_del), PTIMER_DELETE, &arg_del).i[LOW])) {
		rt_thread_join(thread);
	}
	
	return 0;	
}


/*
 * FUNCTIONS (LIKELY) SAFELY USABLE IN HARD REAL TIME "AS THEY ARE", 
 * BECAUSE MAKE SENSE IN THE INITIALIZATION PHASE ONLY, I.E. BEFORE 
 * GOING HARD REAL TIME
 */

#define pthread_self_rt                  pthread_self
#define pthread_equal_rt                 pthread_equal
#define pthread_attr_init_rt             pthread_attr_init      
#define pthread_attr_destroy_rt          pthread_attr_destroy
#define pthread_attr_getdetachstate_rt   pthread_attr_getdetachstate
#define pthread_attr_setschedpolicy_rt   pthread_attr_setschedpolicy
#define pthread_attr_getschedpolicy_rt   pthread_attr_getschedpolicy 
#define pthread_attr_setschedparam_rt    pthread_attr_setschedparam
#define pthread_attr_getschedparam_rt    pthread_attr_getschedparam
#define pthread_attr_setinheritsched_rt  pthread_attr_setinheritsched
#define pthread_attr_getinheritsched_rt  pthread_attr_getinheritsched
#define pthread_attr_setscope_rt         pthread_attr_setscope
#define pthread_attr_getscope_rt         pthread_attr_getscope
#ifdef __USE_UNIX98
#define pthread_attr_setguardsize_rt     pthread_attr_setguardsize
#define pthread_attr_getguardsize_rt     pthread_attr_getguardsize
#endif
#define pthread_attr_setstackaddr_rt     pthread_attr_setstackaddr
#define pthread_attr_getstackaddr_rt     pthread_attr_getstackaddr
#ifdef __USE_XOPEN2K
#define pthread_attr_setstack_rt         pthread_attr_setstack
#define pthread_attr_getstack_rt         pthread_attr_getstack
#endif
#define pthread_attr_setstacksize_rt     pthread_attr_setstacksize
#define pthread_attr_getstacksize_rt     pthread_attr_getstacksize

/*
 * WORKING FUNCTIONS USABLE IN HARD REAL TIME, THIS IS THE REAL STUFF
 */

#define pthread_setcancelstate_rt  pthread_setcancelstate
#define pthread_setcanceltype_rt   pthread_setcanceltype

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__KERNEL__ */

#endif /* !_RTAI_POSIX_H_ */
