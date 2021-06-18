/*
 * pqueues interface for Real Time Linux.
 *
 * Copyright (©) 1999 Zentropic Computing, All rights reserved
 *  
 * Authors:         Trevor Woolven (trevw@zentropix.com)
 *
 * Original date:   Thu 15 Jul 1999
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
 *
 * 2005, cleaned and revised Paolo Mantegazza <mantegazza@aero.polimi.it>.
 *
 */

#ifndef _RTAI_MQ_H
#define _RTAI_MQ_H

#include <linux/version.h>
#include <rtai_sem.h>
#include <rtai_schedcore.h>

#define	MQ_OPEN_MAX	8	/* Maximum number of message queues per process */
#ifndef	MQ_PRIO_MAX
#define	MQ_PRIO_MAX	32	/* Maximum number of message priorities */
#endif
#define	MQ_BLOCK	0	/* Flag to set queue into blocking mode */
#define	MQ_NONBLOCK	1	/* Flag to set queue into non-blocking mode */
#define MQ_NAME_MAX	80	/* Maximum length of a queue name string */

#define MQ_MIN_MSG_PRIORITY 0			/* Lowest priority message */
#define MQ_MAX_MSG_PRIORITY MQ_PRIO_MAX		/* Highest priority message */

#define MAX_PQUEUES     4       /* Maximum number of message queues in module,
				   remember to update rtai_pmq.h too.         */
#define MAX_MSGSIZE     50      /* Maximum message size per queue (bytes) */
#define MAX_MSGS        10      /* Maximum number of messages per queue */

#define O_NOTIFY_NP 	0x1000

typedef struct mq_attr {
    long mq_maxmsg;		/* Maximum number of messages in queue */
    long mq_msgsize;		/* Maximum size of a message (in bytes) */
    long mq_flags;		/* Blocking/Non-blocking behaviour specifier */
    long mq_curmsgs;		/* Number of messages currently in queue */
} MQ_ATTR;

#define	INVALID_PQUEUE	0

#ifdef __KERNEL__

#include <linux/types.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,6)
typedef int mqd_t;
#endif

#ifndef __cplusplus

typedef int mq_bool_t;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef struct msg_hdr {
    size_t size;		/* Actual message size */
    uint priority;		/* Usage priority (message/task) */
    void *next;			/* Pointer to next message on queue */
} MSG_HDR;

#define MSG_HDR_SIZE	(sizeof(MSG_HDR))

typedef struct queue_control {
    int nodind;
    void **nodes;
    void *base;		/* Pointer to the base of the queue in memory */
    void *head;		/* Pointer to the element at the front of the queue */
    void *tail;		/* Pointer to the element at the back of the queue */
    MQ_ATTR attrs;	/* Queue attributes */
} Q_CTRL;

typedef struct msg {
    MSG_HDR hdr;
    char data;		/* Anchor point for message data */
} MQMSG;

struct notify {
    RT_TASK *task;
    struct sigevent data;
};

typedef struct _pqueue_descr_struct {
    RT_TASK *owner;		/* Task that created the queue */
    int open_count;		/* Count of the number of tasks that have */
				/*  'opened' the queue for access */
    char q_name[MQ_NAME_MAX];	/* Name supplied for queue */
    uint q_id;			/* Queue Id (index into static list of queues) */
    mq_bool_t marked_for_deletion;	/* Queue can be deleted once all tasks have  */
				/*  closed it	*/
    Q_CTRL data;		/* Data queue (real messages) */
    mode_t permissions;		/* Permissions granted by creator (ugo, rwx) */
    struct notify notify;	/* Notification data (empty -> !empty) */
    SEM emp_cond;		/* For blocking on empty queue */
    SEM full_cond;		/* For blocking on full queue */
    SEM mutex;			/* For synchronisation of queue */
} MSG_QUEUE;

struct _pqueue_access_data {
    int q_id;
    int oflags;			/* Queue access permissions & blocking spec */
    struct sigevent *usp_notifier;
};

typedef struct _pqueue_access_struct {
    RT_TASK *this_task;
    int n_open_pqueues;
    struct _pqueue_access_data q_access[MQ_OPEN_MAX];
} *QUEUE_CTRL;

typedef enum {
    FOR_READ,
    FOR_WRITE
} Q_ACCESS;

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

int __rtai_mq_init(void);

void __rtai_mq_exit(void);

RTAI_SYSCALL_MODE mqd_t _mq_open(char *mq_name, int oflags, mode_t permissions, struct mq_attr *mq_attr, long space);
static inline mqd_t mq_open(char *mq_name, int oflags, mode_t permissions, struct mq_attr *mq_attr)
{
	return _mq_open(mq_name, oflags, permissions, mq_attr, 1);
}

RTAI_SYSCALL_MODE size_t _mq_receive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, int space);
static inline size_t mq_receive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio)
{
	return _mq_receive(mq, msg_buffer, buflen, msgprio, 1);
}

RTAI_SYSCALL_MODE int _mq_send(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, int space);
static inline int mq_send(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio)
{
	return _mq_send(mq, msg, msglen, msgprio, 1);
}

RTAI_SYSCALL_MODE int mq_close(mqd_t mq);

RTAI_SYSCALL_MODE int mq_getattr(mqd_t mq, struct mq_attr *attrbuf);

RTAI_SYSCALL_MODE int mq_setattr(mqd_t mq, const struct mq_attr *new_attrs, struct mq_attr *old_attrs);

RTAI_SYSCALL_MODE int _mq_notify(mqd_t mq, RT_TASK *task, long space, long rem, const struct sigevent *notification);
static inline int mq_notify(mqd_t mq, const struct sigevent *notification)
{
	return _mq_notify(mq, rt_whoami(), 1, (notification ? 0 : 1), notification );
}

RTAI_SYSCALL_MODE int mq_unlink(char *mq_name);

RTAI_SYSCALL_MODE size_t _mq_timedreceive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, const struct timespec *abstime, int space);
static inline size_t mq_timedreceive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, const struct timespec *abstime)
{
	return _mq_timedreceive(mq, msg_buffer, buflen, msgprio, abstime, 1);
}

RTAI_SYSCALL_MODE int _mq_timedsend(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, const struct timespec *abstime, int space);
static inline int mq_timedsend(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, const struct timespec *abstime)
{
	return _mq_timedsend(mq, msg, msglen, msgprio, abstime, 1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <signal.h>
#include <rtai_lxrt.h>
#include <rtai_signal.h>

#define MQIDX  0

typedef int mqd_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct suprt_fun_arg { mqd_t mq; RT_TASK *task; unsigned long cpuid; pthread_t self; };

#ifndef __SIGNAL_SUPPORT_FUN_MQ__
#define __SIGNAL_SUPPORT_FUN_MQ__

static void signal_suprt_fun_mq(struct suprt_fun_arg *fun_arg)
{		
	struct sigtsk_t { RT_TASK *sigtask; RT_TASK *task; };
	struct suprt_fun_arg arg = *fun_arg;
 	struct sigreq_t { RT_TASK *sigtask; RT_TASK *task; long signal;} sigreq = {NULL, arg.task, (arg.mq + MAXSIGNALS)};	
 	struct sigevent notification;
	
	if ((sigreq.sigtask = rt_thread_init(rt_get_name(0), SIGNAL_TASK_INIPRIO, 0, SCHED_FIFO, 1 << arg.cpuid))) {
		if (!rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigreq_t), RT_SIGNAL_REQUEST, &sigreq).i[LOW]) {
			struct arg_reg { mqd_t mq; RT_TASK *task; struct sigevent *usp_notification;} arg_reg = {arg.mq, arg.task, &notification};
			rtai_lxrt(MQIDX, sizeof(struct arg_reg), MQ_REG_USP_NOTIFIER, &arg_reg);
			mlockall(MCL_CURRENT | MCL_FUTURE);
			rt_make_hard_real_time();
			while (rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(struct sigtsk_t), RT_SIGNAL_WAITSIG, &sigreq).i[LOW]) {
				if (notification.sigev_notify == SIGEV_THREAD) {
					notification._sigev_un._sigev_thread._function((sigval_t)notification.sigev_value.sival_int);
				} else if (notification.sigev_notify == SIGEV_SIGNAL) {
					pthread_kill((pthread_t)arg.self, notification.sigev_signo);
				}
			}
			rt_make_soft_real_time();
		}
		rt_task_delete(sigreq.sigtask);
	}
}	

#endif

RTAI_PROTO(int, rt_request_signal_mq, (mqd_t mq))
{
		struct suprt_fun_arg arg = { mq, NULL, 0, pthread_self() };
		arg.cpuid = rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(void *), RT_SIGNAL_HELPER, (void *)&arg.task).i[LOW];
		arg.task = rt_buddy();	
		if (rt_thread_create(signal_suprt_fun_mq, &arg, SIGNAL_TASK_STACK_SIZE)) {
			int ret;
			ret = rtai_lxrt(RTAI_SIGNALS_IDX, sizeof(RT_TASK *), RT_SIGNAL_HELPER, &arg.task).i[LOW];
			return ret;
		}
	return -1;		
}


RTAI_PROTO(mqd_t, mq_open,(char *mq_name, int oflags, mode_t permissions, struct mq_attr *mq_attr))
{
	mqd_t ret;
	struct {char *mq_name; long oflags; long permissions; struct mq_attr *mq_attr; long namesize, attrsize; long space; } arg = { mq_name, oflags, permissions, mq_attr, strlen(mq_name) + 1, sizeof(struct mq_attr), 0 };
	if ((ret = (mqd_t)rtai_lxrt(MQIDX, SIZARG, MQ_OPEN, &arg).i[LOW]) >= 0) {
		// Prepare notify task 
		if (oflags & O_NOTIFY_NP)	{
			rt_request_signal_mq (ret);
		}
	}	
	return ret;
}

RTAI_PROTO(size_t, mq_receive,(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio))
{
	struct { long mq; char *msg_buffer; long buflen; unsigned int *msgprio; long space; } arg = { mq, msg_buffer, buflen, msgprio, 0 };
	return (size_t)rtai_lxrt(MQIDX, SIZARG, MQ_RECEIVE, &arg).i[LOW];
}

RTAI_PROTO(int, mq_send,(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio))
{
	struct { long mq; const char *msg; long msglen; unsigned long msgprio; long space; } arg = { mq, msg, msglen, msgprio, 0 };
	return rtai_lxrt(MQIDX, SIZARG, MQ_SEND, &arg).i[LOW];
}

RTAI_PROTO(int, mq_close,(mqd_t mq))
{
	struct { long mq; } arg = { mq };
	return rtai_lxrt(MQIDX, SIZARG, MQ_CLOSE, &arg).i[LOW];
}

RTAI_PROTO(int, mq_getattr,(mqd_t mq, struct mq_attr *attrbuf))
{
	struct { long mq; struct mq_attr *attrbuf; long attrsize; } arg = { mq, attrbuf, sizeof(struct mq_attr) };
	return rtai_lxrt(MQIDX, SIZARG, MQ_GETATTR, &arg).i[LOW];
}

RTAI_PROTO(int, mq_setattr,(mqd_t mq, const struct mq_attr *new_attrs, struct mq_attr *old_attrs))
{
	struct { long mq; const struct mq_attr *new_attrs; struct mq_attr *old_attrs; long attrsize; } arg = { mq, new_attrs, old_attrs, sizeof(struct mq_attr) };
	return rtai_lxrt(MQIDX, SIZARG, MQ_SETATTR, &arg).i[LOW];
}

RTAI_PROTO(int, mq_notify,(mqd_t mq, const struct sigevent *notification))
{
	int ret;
	struct { long mq; RT_TASK* task; long space; long rem; const struct sigevent *notification; long size;} arg = { mq, rt_buddy(), 0, (notification ? 0 : 1), notification, sizeof(struct sigevent) };
	if ((ret = rtai_lxrt(MQIDX, SIZARG, MQ_NOTIFY, &arg).i[LOW]) >= 0) {
		if (ret == O_NOTIFY_NP) {
			rt_request_signal_mq (mq);
			ret = 0;
		}
	}
	return ret;
}

RTAI_PROTO(int, mq_unlink,(char *mq_name))
{
	struct { char *mq_name; long size; } arg = { mq_name, strlen(mq_name) + 1};
	return rtai_lxrt(MQIDX, SIZARG, MQ_UNLINK, &arg).i[LOW];
}

RTAI_PROTO(size_t, mq_timedreceive,(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, const struct timespec *abstime))
{
	struct { long mq; char *msg_buffer; long buflen; unsigned int *msgprio; const struct timespec *abstime; long space; } arg = { mq, msg_buffer, buflen, msgprio, abstime, 0 };
	return (size_t)rtai_lxrt(MQIDX, SIZARG, MQ_TIMEDRECEIVE, &arg).i[LOW];
}

RTAI_PROTO(int, mq_timedsend,(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, const struct timespec *abstime))
{
	struct { long mq; const char *msg; long msglen; unsigned long msgprio; const struct timespec *abstime; long space; } arg = { mq, msg, msglen, msgprio, abstime, 0 };
	return rtai_lxrt(MQIDX, SIZARG, MQ_TIMEDSEND, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#endif  /* !_RTAI_MQ_H */
