/**
 * @ingroup lxrt
 * @file
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * @note Copyright &copy; 2019 Alec Ari <neotheuser@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTAI_SEM_H
#define _RTAI_SEM_H

#include <rtai_types.h>
#include <rtai_nam2num.h>
#include <rtai_sched.h>

#define RT_SEM_MAGIC 0x3f83ebb  // nam2num("rtsem")

#define SEM_ERR     (RTE_OBJINV)
#define SEM_TIMOUT  (RTE_TIMOUT)

struct rt_poll_s { void *what; unsigned long forwhat; };

// do not use 0 for any "forwhat" below
#define RT_POLL_NOT_TO_USE    0
#define RT_POLL_MBX_RECV      1
#define RT_POLL_MBX_SEND      2
#define RT_POLL_SEM_WAIT_ALL  3
#define RT_POLL_SEM_WAIT_ONE  4

#if defined(__KERNEL__) && !defined(__cplusplus)

struct rt_poll_ql { QUEUE pollq; spinlock_t pollock; };
struct rt_poll_enc { unsigned long offset; int (*topoll)(void *); };
extern struct rt_poll_enc rt_poll_ofstfun[];

typedef struct rt_semaphore {
	struct rt_queue queue; /* <= Must be first in struct. */
	int magic;
	int type, restype;
	int count;
	struct rt_task_struct *owndby;
	int qtype;
	struct rt_queue resq;
#ifdef CONFIG_RTAI_RT_POLL
	struct rt_poll_ql poll_wait_all;
	struct rt_poll_ql poll_wait_one;
#endif
} SEM;

#ifdef CONFIG_RTAI_RT_POLL

RTAI_SYSCALL_MODE int _rt_poll(struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout, int space);
static inline int rt_poll(struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout)
{
	return _rt_poll(pdsa, nr, timeout, 1);
}

void rt_wakeup_pollers(struct rt_poll_ql *ql, int reason);

#else

static inline int rt_poll(struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout)
{
	return RTE_OBJINV;
}

#define rt_wakeup_pollers(ql, reason)

#endif

#else /* !__KERNEL__ || __cplusplus */

typedef struct rt_semaphore {
    int opaque;
} SEM;

#endif /* __KERNEL__ && !__cplusplus */

typedef SEM CND;

#ifdef __KERNEL__

#include <linux/errno.h>

typedef SEM psem_t;

typedef SEM pmutex_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_sem_init(void);

void __rtai_sem_exit(void);

RTAI_SYSCALL_MODE void rt_typed_sem_init(SEM *sem,
		       int value,
		       int type);

RTAI_SYSCALL_MODE int rt_sem_delete(SEM *sem);

RTAI_SYSCALL_MODE SEM *_rt_typed_named_sem_init(unsigned long sem_name,
			     int value,
			     int type,
			     unsigned long *handle);

static inline SEM *rt_typed_named_sem_init(const char *sem_name,
					   int value,
					   int type) {
    return _rt_typed_named_sem_init(nam2num(sem_name), value, type, NULL);
}

RTAI_SYSCALL_MODE int rt_named_sem_delete(SEM *sem);

void rt_sem_init(SEM *sem,
		 int value);

RTAI_SYSCALL_MODE int rt_sem_signal(SEM *sem);

RTAI_SYSCALL_MODE int rt_sem_broadcast(SEM *sem);

RTAI_SYSCALL_MODE int rt_sem_wait(SEM *sem);

RTAI_SYSCALL_MODE int rt_sem_wait_if(SEM *sem);

int rt_cntsem_wait_if_and_lock(SEM *sem);

RTAI_SYSCALL_MODE int rt_sem_wait_until(SEM *sem,
		      RTIME time);

RTAI_SYSCALL_MODE int rt_sem_wait_timed(SEM *sem,
		      RTIME delay);

RTAI_SYSCALL_MODE int rt_sem_wait_barrier(SEM *sem);

RTAI_SYSCALL_MODE int rt_sem_count(SEM *sem);

RTAI_SYSCALL_MODE int rt_cond_signal(CND *cnd);

RTAI_SYSCALL_MODE int rt_cond_wait(CND *cnd,
		 SEM *mtx);

RTAI_SYSCALL_MODE int rt_cond_wait_until(CND *cnd,
		       SEM *mtx,
		       RTIME time);

RTAI_SYSCALL_MODE int rt_cond_wait_timed(CND *cnd,
		       SEM *mtx,
		       RTIME delay);

#define rt_named_sem_init(sem_name, value)  rt_typed_named_sem_init(sem_name, value, CNT_SEM)

static inline int rt_psem_init(psem_t *sem, int pshared, unsigned int value)
{
	if (value < SEM_TIMOUT) {
		rt_typed_sem_init(sem, value, pshared | PRIO_Q);
		return 0;
	}
	return -EINVAL;
}

static inline int rt_psem_destroy(psem_t *sem)
{
	if (rt_sem_wait_if(sem) >= 0) {
		rt_sem_signal(sem);
		return rt_sem_delete(sem);
	}
	return -EBUSY;
}

static inline int rt_psem_wait(psem_t *sem) {
    return rt_sem_wait(sem) < SEM_TIMOUT ? 0 : -1;
}

static inline int rt_psem_timedwait(psem_t *sem, struct timespec *abstime) {
    return rt_sem_wait_until(sem, timespec2count(abstime)) < SEM_TIMOUT ? 0 : -1;
}

static inline int rt_psem_trywait(psem_t *sem) {
    return rt_sem_wait_if(sem) > 0 ? 0 : -EAGAIN;
}

static inline int rt_psem_post(psem_t *sem) {
    return rt_sem_signal(sem);
}

static inline int rt_psem_getvalue(psem_t *sem, int *sval)
{
	if ((*sval = rt_sem_wait_if(sem)) > 0) {
		rt_sem_signal(sem);
	}
	return 0;
}

static inline int rt_pmutex_init(pmutex_t *mutex, void *mutexattr)
{
	rt_typed_sem_init(mutex, 1, RES_SEM);
	return 0;
}

static inline int rt_pmutex_destroy(pmutex_t *mutex)
{
	if (rt_sem_wait_if(mutex) > 0) {
		rt_sem_signal(mutex);
		return rt_sem_delete(mutex);
	}
	return -EBUSY;
}

static inline int rt_pmutex_lock(pmutex_t *mutex) {
    return rt_sem_wait(mutex) < SEM_TIMOUT ? 0 : -EINVAL;
}

static inline int rt_pmutex_trylock(pmutex_t *mutex) {
    return rt_sem_wait_if(mutex) > 0 ? 0 : -EBUSY;
}

static inline int rt_pmutex_timedlock(pmutex_t *sem, struct timespec *abstime) {
    return rt_sem_wait_until(sem, timespec2count(abstime)) < SEM_TIMOUT ? 0 : -1;
}

static inline int rt_pmutex_unlock(pmutex_t *mutex) {
    return rt_sem_signal(mutex);
}

#undef rt_mutex_init
#define rt_mutex_init(mtx)             rt_typed_sem_init(mtx, 1, RES_SEM)
#define rt_mutex_delete(mtx)           rt_sem_delete(mtx)
#define rt_mutex_destroy(mtx)          rt_sem_delete(mtx)
#define rt_mutex_trylock(mtx)          rt_sem_wait_if(mtx)
#define rt_mutex_lock(mtx)             rt_sem_wait(mtx)
#define rt_mutex_timedlock(mtx, time)  rt_sem_wait_until(mtx, time)
#define rt_mutex_unlock(mtx)           rt_sem_signal(mtx)

#define rt_cond_init(cnd)                  rt_typed_sem_init(cnd, 0, BIN_SEM | PRIO_Q)
#define rt_cond_delete(cnd)                rt_sem_delete(cnd)
#define rt_cond_destroy(cnd)               rt_sem_delete(cnd)
#define rt_cond_broadcast(cnd)             rt_sem_broadcast(cnd)

static inline int rt_cond_timedwait(CND *cnd, SEM *mtx, RTIME time) {
    return rt_cond_wait_until(cnd, mtx, time) < SEM_TIMOUT ? 0 : -1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <rtai_lxrt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(SEM *, rt_typed_sem_init,(unsigned long name, int value, int type))
{
	struct { unsigned long name; long value, type; } arg = { name ? name : rt_get_name(NULL), value, type };
	return (SEM *)rtai_lxrt(BIDX, SIZARG, LXRT_SEM_INIT, &arg).v[LOW];
}

/**
 * @ingroup lxrt
 * Initialize a counting semaphore.
 *
 * Allocates and initializes a semaphore to be referred by @a name.
 *
 * @param name name of the semaphore.
 *
 * @param value is the initial value of the semaphore
 *
 * It is important to remark that the returned task pointer cannot be used
 * directly, they are for kernel space data, but just passed as arguments when
 * needed.
 *
 * @return a pointer to the semaphore to be used in related calls or 0 if an
 * error has occured.
 */ 
#define rt_sem_init(name, value) rt_typed_sem_init(name, value, CNT_SEM)

#define rt_named_sem_init(sem_name, value) \
	rt_typed_named_sem_init(sem_name, value, CNT_SEM)

RTAI_PROTO(int, rt_sem_delete,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, LXRT_SEM_DELETE, &arg).i[LOW];
}

RTAI_PROTO(SEM *, rt_typed_named_sem_init,(const char *name, int value, int type))
{
	struct { unsigned long name; long value, type; unsigned long *handle; } arg = { nam2num(name), value, type, NULL };
	return (SEM *)rtai_lxrt(BIDX, SIZARG, NAMED_SEM_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_named_sem_delete,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, NAMED_SEM_DELETE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_signal,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_SIGNAL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_broadcast,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_BROADCAST, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_wait,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_WAIT, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_wait_if,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_WAIT_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_wait_until,(SEM *sem, RTIME time))
{
	struct { SEM *sem; RTIME time; } arg = { sem, time };
	return rtai_lxrt(BIDX, SIZARG, SEM_WAIT_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_wait_timed,(SEM *sem, RTIME delay))
{
	struct { SEM *sem; RTIME delay; } arg = { sem, delay };
	return rtai_lxrt(BIDX, SIZARG, SEM_WAIT_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_wait_barrier,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_WAIT_BARRIER, &arg).i[LOW];
}

RTAI_PROTO(int, rt_sem_count,(SEM *sem))
{
	struct { SEM *sem; } arg = { sem };
	return rtai_lxrt(BIDX, SIZARG, SEM_COUNT, &arg).i[LOW];
}

/**
 * @ingroup lxrt
 * Initialize a condition variable.
 *
 * Allocates and initializes a condition variable to be referred by @a name.
 *
 * @param name name of the condition variable.
 *
 * It is important to remark that the returned pointer cannot be used
 * directly, it is for kernel space data, but just passed as arguments when
 * needed.
 *
 * @return a pointer to the condition variable to be used in related calls or 0
 * if an error has occured.
 */ 
#define rt_cond_init(name)                 rt_typed_sem_init(name, 0, BIN_SEM)
#define rt_cond_delete(cnd)                rt_sem_delete(cnd)
#define rt_cond_destroy(cnd)               rt_sem_delete(cnd)
#define rt_cond_broadcast(cnd)             rt_sem_broadcast(cnd)
#define rt_cond_timedwait(cnd, mtx, time)  rt_cond_wait_until(cnd, mtx, time)

RTAI_PROTO(int, rt_cond_signal,(CND *cnd))
{
	struct { CND *cnd; } arg = { cnd };
	return rtai_lxrt(BIDX, SIZARG, COND_SIGNAL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_cond_wait,(CND *cnd, SEM  *mutex))
{
	struct { CND *cnd; SEM *mutex; } arg = { cnd, mutex };
	return rtai_lxrt(BIDX, SIZARG, COND_WAIT, &arg).i[LOW];
}

RTAI_PROTO(int, rt_cond_wait_until,(CND *cnd, SEM *mutex, RTIME time))
{
	struct { CND *cnd; SEM *mutex; RTIME time; } arg = { cnd, mutex, time };
	return rtai_lxrt(BIDX, SIZARG, COND_WAIT_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_cond_wait_timed,(CND *cnd, SEM *mutex, RTIME delay))
{
	struct { CND *cnd; SEM *mutex; RTIME delay; } arg = { cnd, mutex, delay };
	return rtai_lxrt(BIDX, SIZARG, COND_WAIT_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_poll, (struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout))
{
#ifdef CONFIG_RTAI_RT_POLL
	struct { struct rt_poll_s *pdsa; unsigned long nr; RTIME timeout; long space; } arg = { pdsa, nr, timeout, 0 };
	return rtai_lxrt(BIDX, SIZARG, SEM_RT_POLL, &arg).i[LOW];
#else
	(void)pdsa; (void)nr; (void)timeout;
	return RTE_OBJINV;
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#endif /* !_RTAI_SEM_H */
