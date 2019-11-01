/**
 * @ingroup lxrt
 * @file
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_MBX_H
#define _RTAI_MBX_H

#include <rtai_sem.h>

#define RT_MBX_MAGIC 0x3f81aab  // nam2num("rtmbx")

struct rt_task_struct;
struct rt_mailbox;

#ifdef __KERNEL__

#ifndef __cplusplus

typedef struct rt_mailbox {
	int magic;
	SEM sndsem, rcvsem;
	struct rt_task_struct *waiting_task, *owndby;
	char *bufadr;
	int size, fbyte, lbyte, avbs, frbs;
	spinlock_t lock;
#ifdef CONFIG_RTAI_RT_POLL
	struct rt_poll_ql poll_recv;
	struct rt_poll_ql poll_send;
#endif
} MBX;

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

int __rtai_mbx_init(void);

void __rtai_mbx_exit(void);

RTAI_SYSCALL_MODE int rt_typed_mbx_init(struct rt_mailbox *mbx, int size, int qtype);

int rt_mbx_init(struct rt_mailbox *mbx, int size);

RTAI_SYSCALL_MODE int rt_mbx_delete(struct rt_mailbox *mbx);

RTAI_SYSCALL_MODE int _rt_mbx_send(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_send(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_send(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_send_wp(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_send_wp(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_send_wp(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_send_if(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_send_if(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_send_if(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_send_until(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME timei, int space);
static inline int rt_mbx_send_until(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME time)
{
	return _rt_mbx_send_until(mbx, msg, msg_size, time, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_send_timed(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay, int space);
static inline int rt_mbx_send_timed(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay)
{
	return _rt_mbx_send_timed(mbx, msg, msg_size, delay, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_ovrwr_send(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_ovrwr_send(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_ovrwr_send(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_evdrp(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_evdrp(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_evdrp(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_receive(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_receive(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_receive(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_receive_wp(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_receive_wp(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_receive_wp(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_receive_if(struct rt_mailbox *mbx, void *msg, int msg_size, int space);
static inline int rt_mbx_receive_if(struct rt_mailbox *mbx, void *msg, int msg_size)
{
	return _rt_mbx_receive_if(mbx, msg, msg_size, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_receive_until(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME time, int space);
static inline int rt_mbx_receive_until(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME time)
{
	return _rt_mbx_receive_until(mbx, msg, msg_size, time, 1);
}

RTAI_SYSCALL_MODE int _rt_mbx_receive_timed(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay, int space);
static inline int rt_mbx_receive_timed(struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay)
{
	return _rt_mbx_receive_timed(mbx, msg, msg_size, delay, 1);
}

RTAI_SYSCALL_MODE struct rt_mailbox *_rt_typed_named_mbx_init(unsigned long mbx_name, int size, int qtype);
static inline struct rt_mailbox *rt_typed_named_mbx_init(const char *mbx_name, int size, int qtype)
{
	return _rt_typed_named_mbx_init(nam2num(mbx_name), size, qtype);
}

RTAI_SYSCALL_MODE int rt_named_mbx_delete(struct rt_mailbox *mbx);

#define rt_named_mbx_init(mbx_name, size)  rt_typed_named_mbx_init(mbx_name, size, FIFO_Q)
     
#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <rtai_lxrt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(struct rt_mailbox *, rt_typed_mbx_init, (unsigned long name, int size, int qtype))
{
	struct { unsigned long name; long size; long qtype; } arg = { name, size, qtype };
	return (struct rt_mailbox *)rtai_lxrt(BIDX, SIZARG, LXRT_MBX_INIT, &arg).v[LOW];
}

/**
 * @ingroup lxrt
 * Initialize mailbox.
 * 
 * Initializes a mailbox referred to by @a name of size @a size.
 * 
 * It is important to remark that the returned task pointer cannot be used
 * directly, they are for kernel space data, but just passed as arguments when
 * needed.
 *
 * @return On success a pointer to the mail box to be used in related calls.
 * @return A 0 value is returned if it was not possible to setup the semaphore
 * or something using the same name was found.
 */
#define rt_mbx_init(name, size) rt_typed_mbx_init(name, size, FIFO_Q)

RTAI_PROTO(int, rt_mbx_delete, (struct rt_mailbox *mbx))
{
	void *arg = mbx;
	return rtai_lxrt(BIDX, SIZARG, LXRT_MBX_DELETE, &arg).i[LOW];
}

RTAI_PROTO(struct rt_mailbox *, rt_typed_named_mbx_init, (const char *name, int size, int type))
{
	struct { unsigned long name; long size, type; } arg = { nam2num(name), size, type };
	return (struct rt_mailbox *)rtai_lxrt(BIDX, SIZARG, NAMED_MBX_INIT, &arg).v[LOW];
}

RTAI_PROTO(int, rt_named_mbx_delete, (struct rt_mailbox *mbx))
{
	struct { struct rt_mailbox *mbx; } arg = { mbx };
	return rtai_lxrt(BIDX, SIZARG, NAMED_MBX_DELETE, &arg).i[LOW];
}

#define rt_named_mbx_init(mbx_name, size) \
	rt_typed_named_mbx_init(mbx_name, size, FIFO_Q)

RTAI_PROTO(int, rt_mbx_send, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_SEND, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_send_wp, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_SEND_WP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_send_if, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_SEND_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_send_until, (struct rt_mailbox *mbx, void *msg, int msg_size, RTIME time))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; RTIME time; long space; } arg = { mbx, (char *)msg, msg_size, time, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_SEND_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_send_timed, (struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; RTIME delay; long space; } arg = { mbx, (char *)msg, msg_size, delay, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_SEND_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_ovrwr_send, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_OVRWR_SEND, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_evdrp, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_EVDRP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_receive, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_RECEIVE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_receive_wp, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_RECEIVE_WP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_receive_if, (struct rt_mailbox *mbx, void *msg, int msg_size))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; long space; } arg = { mbx, (char *)msg, msg_size, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_RECEIVE_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_receive_until, (struct rt_mailbox *mbx, void *msg, int msg_size, RTIME time))
{
	struct { struct rt_mailbox *mbx; void *msg; long msg_size; RTIME time; long space; } arg = { mbx, (char *)msg, msg_size, time, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_RECEIVE_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_mbx_receive_timed, (struct rt_mailbox *mbx, void *msg, int msg_size, RTIME delay))
{
	struct { struct rt_mailbox *mbx; char *msg; long msg_size; RTIME delay; long space; } arg = { mbx, (char *)msg, msg_size, delay, 0 };
	return (int)rtai_lxrt(BIDX, SIZARG, MBX_RECEIVE_TIMED, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct rt_mailbox {
    int opaque;
} MBX;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_MBX_H */
