/*
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
 */


#ifndef _RTAI_RT_MSGQ_H
#define _RTAI_RT_MSGQ_H

#include <linux/version.h>
#include <rtai_sem.h>

#define MSGQ_INIT            TBX_INIT
#define MSGQ_DELETE          TBX_DELETE
#define NAMED_MSGQ_INIT      NAMED_TBX_INIT
#define NAMED_MSGQ_DELETE    NAMED_TBX_DELETE
#define MSG_SEND             TBX_SEND
#define MSG_SEND_IF          TBX_SEND_IF
#define MSG_SEND_UNTIL       TBX_SEND_UNTIL
#define MSG_SEND_TIMED       TBX_SEND_TIMED
#define MSG_RECEIVE          TBX_RECEIVE
#define MSG_RECEIVE_IF       TBX_RECEIVE_IF
#define MSG_RECEIVE_UNTIL    TBX_RECEIVE_UNTIL
#define MSG_RECEIVE_TIMED    TBX_RECEIVE_TIMED
#define MSG_BROADCAST        TBX_BROADCAST
#define MSG_BROADCAST_IF     TBX_BROADCAST_IF
#define MSG_BROADCAST_UNTIL  TBX_BROADCAST_UNTIL
#define MSG_BROADCAST_TIMED  TBX_BROADCAST_TIMED
#define MSG_EVDRP            TBX_URGENT

#define TBX  RT_MSGQ

#ifdef __KERNEL__

typedef struct rt_msgh {
	void *malloc;
	int broadcast;
	int size;
	int priority;
	void *next;	
} RT_MSGH;

#define RT_MSGH_SIZE  (sizeof(RT_MSGH))

typedef struct rt_msg {
	RT_MSGH hdr;
	char msg[1];
} RT_MSG;

typedef struct rt_msgq {
	int nmsg;
	int fastsize;
	int slot;
	void **slots;
	void *firstmsg;
	SEM receivers, senders;
	SEM received, freslots;
	SEM broadcast;
	spinlock_t lock;
} RT_MSGQ;

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

int __rtai_msg_queue_init(void);

void __rtai_msg_queue_exit(void);

RTAI_SYSCALL_MODE int rt_msgq_init(RT_MSGQ *msgq, int nmsg, int msg_size);

RTAI_SYSCALL_MODE int rt_msgq_delete(RT_MSGQ *msgq);

RTAI_SYSCALL_MODE RT_MSGQ *_rt_named_msgq_init(unsigned long msgq_name, int nmsg, int size);
static inline RT_MSGQ *rt_named_msgq_init(const char *msgq_name, int nmsg, int size)
{
	return _rt_named_msgq_init(nam2num(msgq_name), nmsg, size);
}

RTAI_SYSCALL_MODE int rt_named_msgq_delete(RT_MSGQ *msgq);

RTAI_SYSCALL_MODE int _rt_msg_send(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, int space);
static inline int rt_msg_send(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri)
{
	return _rt_msg_send(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_send_if(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, int space);
static inline int rt_msg_send_if(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri)
{
	return _rt_msg_send_if(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_send_until(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME until, int space);
static inline int rt_msg_send_until(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME until)
{
	return _rt_msg_send_until(msgq, msg, msg_size, msgpri, until, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_send_timed(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME delay, int space);
static inline int rt_msg_send_timed(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME delay)
{
	return _rt_msg_send_timed(msgq, msg, msg_size, msgpri, delay, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_receive(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, int space);
static inline int rt_msg_receive(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri)
{
	return _rt_msg_receive(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_if(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, int space);
static inline int rt_msg_receive_if(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri)
{
	return _rt_msg_receive_if(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_until(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, RTIME until, int space);
static inline int rt_msg_receive_until(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, RTIME until)
{
	return _rt_msg_receive_until(msgq, msg, msg_size, msgpri, until, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_timed(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, RTIME delay, int space);
static inline int rt_msg_receive_timed(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, RTIME delay)
{
	return _rt_msg_receive_timed(msgq, msg, msg_size, msgpri, delay, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_evdrp(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri, int space);
static inline int rt_msg_evdrp(RT_MSGQ *msgq, void *msg, int msg_size, int *msgpri)
{
	return _rt_msg_evdrp(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, int space);
static inline int rt_msg_broadcast(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri)
{
	return _rt_msg_broadcast(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_if(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, int space);
static inline int rt_msg_broadcast_if(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri)
{
	return _rt_msg_broadcast_if(msgq, msg, msg_size, msgpri, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_until(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME until, int space);
static inline int rt_msg_broadcast_until(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME until)
{
	return _rt_msg_broadcast_until(msgq, msg, msg_size, msgpri, until, 1);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_timed(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME delay, int space);
static inline int rt_msg_broadcast_delay(RT_MSGQ *msgq, void *msg, int msg_size, int msgpri, RTIME delay)
{
	return _rt_msg_broadcast_until(msgq, msg, msg_size, msgpri, delay, 1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <signal.h>
#include <rtai_lxrt.h>

struct rt_msgh;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct rt_msgq {
	int dummy;
} RT_MSGQ;

RTAI_PROTO(RT_MSGQ *, rt_msgq_init, (unsigned long msgq, int nmsg, int msg_size))
{
	struct { unsigned long msgq; long nmsg; long msg_size; } arg = { msgq, nmsg, msg_size };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MSGQ_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_msgq_delete, (RT_MSGQ *msgq))
{
	struct { RT_MSGQ *msgq; } arg = { msgq };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MSGQ_DELETE, &arg).i[LOW];
}

RTAI_PROTO(RT_MSGQ *, rt_named_msgq_init,(const char *name, int nmsg, int size))
{
	struct { unsigned long name; long nmsg; long size; } arg = { nam2num(name), nmsg, size };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MSGQ_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_named_msgq_delete, (RT_MSGQ *msgq))
{
	struct { RT_MSGQ *msgq; } arg = { msgq };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MSGQ_DELETE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_send, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_SEND, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_send_if, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_SEND_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_send_until, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio, RTIME until))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; RTIME until; long space; } arg = { msgq, msg, msg_size, msgprio, until, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_SEND_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_send_timed, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio, RTIME delay))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; RTIME delay; long space; } arg = { msgq, msg, msg_size, msgprio, delay, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_SEND_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_receive, (RT_MSGQ *msgq, void *msg, int msg_size, int *msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; int *msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_RECEIVE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_receive_if, (RT_MSGQ *msgq, void *msg, int msg_size, int *msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; int *msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_RECEIVE_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_receive_until, (RT_MSGQ *msgq, void *msg, int msg_size, int *msgprio, RTIME until))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; int *msgprio; RTIME until; long space; } arg = { msgq, msg, msg_size, msgprio, until, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_RECEIVE_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_receive_timed, (RT_MSGQ *msgq, void *msg, int msg_size, int *msgprio, RTIME delay))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; int *msgprio; RTIME delay; long space; } arg = { msgq, msg, msg_size, msgprio, delay, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_RECEIVE_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_evdrp, (RT_MSGQ *msgq, void *msg, int msg_size, int *msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; int *msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_EVDRP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_broadcast, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_BROADCAST, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_broadcast_if, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; long space; } arg = { msgq, msg, msg_size, msgprio, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_BROADCAST_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_broadcast_until, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio, RTIME until))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; RTIME until; long space; } arg = { msgq, msg, msg_size, msgprio, until, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_BROADCAST_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_msg_broadcast_timed, (RT_MSGQ *msgq, void *msg, int msg_size, int msgprio, RTIME delay))
{
	struct { RT_MSGQ *msgq; void *msg; long msg_size; long msgprio; RTIME delay; long space; } arg = { msgq, msg, msg_size, msgprio, delay, 0 };
	return rtai_lxrt(BIDX, SIZARG, MSG_BROADCAST_TIMED, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#define rt_tbx_init(tbx, size, flags)  rt_msgq_init(tbx, size, 0)
#define rt_tbx_delete(tbx)             rt_msgq_delete(tbx)

#define rt_tbx_send(tbx, msg, msg_size)               rt_msg_send(tbx, msg, msg_size, 1)
#define rt_tbx_send_if(tbx, msg, msg_size)            rt_msg_send_if(tbx, msg, msg_size, 1)
#define rt_tbx_send_until(tbx, msg, msg_size, until)  rt_msg_send_until(tbx, msg, msg_size, 1, until)
#define rt_tbx_send_timed(tbx, msg, msg_size, delay)  rt_msg_send_timed(tbx, msg, msg_size, 1, delay)

#define rt_tbx_receive(tbx, msg, msg_size)               rt_msg_receive(tbx, msg, msg_size, 0)
#define rt_tbx_receive_if(tbx, msg, msg_size)            rt_msg_receive_if(tbx, msg, msg_size, 0)
#define rt_tbx_receive_until(tbx, msg, msg_size, until)  rt_msg_receive_until(tbx, msg, msg_size, 0, until)
#define rt_tbx_receive_timed(tbx, msg, msg_size, delay)  rt_msg_receive_timed(tbx, msg, msg_size, 0, delay)

#define rt_tbx_broadcast(tbx, msg, msg_size)               rt_msg_broadcast(tbx, msg, msg_size, 0)
#define rt_tbx_broadcast_if(tbx, msg, msg_size)            rt_msg_broadcast_if(tbx, msg, msg_size, 0)
#define rt_tbx_broadcast_until(tbx, msg, msg_size, until)  rt_msg_broadcast_until(tbx, msg, msg_size, 0, until)
#define rt_tbx_broadcast_timed(tbx, msg, msg_size, delay)  rt_msg_broadcast_timed(tbx, msg, msg_size, 0, delay)

#define rt_tbx_urgent(tbx, msg, msg_size)               rt_msg_send(tbx, msg, msg_size, 0)
#define rt_tbx_urgent_if(tbx, msg, msg_size)            rt_msg_send_if(tbx, msg, msg_size, 0)
#define rt_tbx_urgent_until(tbx, msg, msg_size, until)  rt_msg_send_until(tbx, msg, msg_size, 0, until)
#define rt_tbx_urgent_timed(tbx, msg, msg_size, delay)  rt_msg_send_timed(tbx, msg, msg_size, 0, delay)

#endif  /* !_RTAI_RT_MSGQ_H */
