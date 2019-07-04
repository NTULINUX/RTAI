/*
 * Copyright (C) 2002 POSEIDON CONTROLS INC <pcloutier@poseidoncontrols.com>
 * Copyright (C) 2002 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_MSG_H
#define _RTAI_MSG_H

#include <rtai_types.h>

#define MSG_ERR ((RT_TASK *)RTE_OBJINV)

struct rt_task_struct;
struct QueueBlock;
struct QueueHook;

#ifdef __KERNEL__

typedef struct t_msgcb { /* Message control block structure. */
    int  cmd;
    void *sbuf;
    size_t sbytes;
    void *rbuf;
    size_t rbytes;
} MSGCB;

#define PROXY_MIN_STACK_SIZE 2048

struct proxy_t {
    struct rt_task_struct *receiver;
    int nmsgs, nbytes;
    char *msg;
};

#define SYNCMSG          0
#define PROXY           -1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_msg_init(void);

void __rtai_msg_exit(void);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_send(struct rt_task_struct *task,
			       unsigned long msg);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_send_if(struct rt_task_struct *task,
				  unsigned long msg);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_send_until(struct rt_task_struct *task,
				     unsigned long msg,
				     RTIME time);
    
RTAI_SYSCALL_MODE struct rt_task_struct *rt_send_timed(struct rt_task_struct *task,
				     unsigned long msg,
				     RTIME delay);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_evdrp(struct rt_task_struct *task,
				void *msg);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receive(struct rt_task_struct *task,
				  void *msg);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receive_if(struct rt_task_struct *task,
				     void *msg);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receive_until(struct rt_task_struct *task,
					void *msg,
					RTIME time);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receive_timed(struct rt_task_struct *task,
					void *msg,
					RTIME delay);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpc(struct rt_task_struct *task,
			      unsigned long to_do,
			      void *result);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpc_if(struct rt_task_struct *task,
				 unsigned long to_do,
				 void *result);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpc_until(struct rt_task_struct *task,
				    unsigned long to_do,
				    void *result,
				    RTIME time);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpc_timed(struct rt_task_struct *task,
				    unsigned long to_do,
				    void *result,
				    RTIME delay);

RTAI_SYSCALL_MODE int rt_isrpc(struct rt_task_struct *task);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_return(struct rt_task_struct *task,
				 unsigned long result);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpcx(struct rt_task_struct *task,
			       void *smsg,
			       void *rmsg,
			       int ssize,
			       int rsize);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpcx_if(struct rt_task_struct *task,
				  void *smsg,
				  void *rmsg,
				  int ssize,
				  int rsize);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpcx_until(struct rt_task_struct *task,
				     void *smsg,
				     void *rmsg,
				     int ssize,
				     int rsize,
				     RTIME time);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_rpcx_timed(struct rt_task_struct *task,
				     void *smsg,
				     void *rmsg,
				     int ssize,
				     int rsize,
				     RTIME delay);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_sendx(struct rt_task_struct *task,
				void *msg,
				int size);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_sendx_if(struct rt_task_struct *task,
				   void *msg,
				   int size);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_sendx_until(struct rt_task_struct *task,
				      void *msg,
				      int size,
				      RTIME time);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_sendx_timed(struct rt_task_struct *task,
				      void *msg,
				      int size,
				      RTIME delay);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_returnx(struct rt_task_struct *task,
				  void *msg,
				  int size);

#define rt_isrpcx(task) rt_isrpc(task)

RTAI_SYSCALL_MODE struct rt_task_struct *rt_evdrpx(struct rt_task_struct *task,
				 void *msg,
				 int size,
				 long *len);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receivex(struct rt_task_struct *task,
				   void *msg,
				   int size,
				   long *len);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receivex_if(struct rt_task_struct *task,
				      void *msg,
				      int size,
				      long *len);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receivex_until(struct rt_task_struct *task,
					 void *msg,
					 int size,
					 long *len,
					 RTIME time);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_receivex_timed(struct rt_task_struct *task,
					 void *msg,
					 int size,
					 long *len,
					 RTIME delay);

struct rt_task_struct *__rt_proxy_attach(void (*func)(long),
					 struct rt_task_struct *task,
					 void *msg,
					 int nbytes,
					 int priority);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_proxy_attach(struct rt_task_struct *task,
				       void *msg,
				       int nbytes,
				       int priority);

RTAI_SYSCALL_MODE int rt_proxy_detach(struct rt_task_struct *proxy);

RTAI_SYSCALL_MODE struct rt_task_struct *rt_trigger(struct rt_task_struct *proxy);

#define exist(name)  rt_get_adr(nam2num(name))

RTAI_SYSCALL_MODE int rt_Send(pid_t pid,
	    void *smsg,
	    void *rmsg,
	    size_t ssize,
	    size_t rsize);

RTAI_SYSCALL_MODE pid_t rt_Receive(pid_t pid,
		 void *msg,
		 size_t maxsize,
		 size_t *msglen);

RTAI_SYSCALL_MODE pid_t rt_Creceive(pid_t pid,
		  void *msg,
		  size_t maxsize,
		  size_t *msglen,
		  RTIME delay);

RTAI_SYSCALL_MODE int rt_Reply(pid_t pid,
	     void *msg,
	     size_t size);

RTAI_SYSCALL_MODE pid_t rt_Proxy_attach(pid_t pid,
		      void *msg,
		      int nbytes,
		      int priority);

RTAI_SYSCALL_MODE int rt_Proxy_detach(pid_t pid);

RTAI_SYSCALL_MODE pid_t rt_Trigger(pid_t pid);

RTAI_SYSCALL_MODE pid_t rt_Name_attach(const char *name);

RTAI_SYSCALL_MODE pid_t rt_Name_locate(const char *host,
		     const char *name);

RTAI_SYSCALL_MODE int rt_Name_detach(pid_t pid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <rtai_lxrt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(struct rt_task_struct *,rt_send,(struct rt_task_struct *task, unsigned long msg))
{
	struct { struct rt_task_struct *task; unsigned long msg; } arg = { task, msg };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SENDMSG, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_send_if,(struct rt_task_struct *task, unsigned long msg))
{
	struct { struct rt_task_struct *task; unsigned long msg; } arg = { task, msg };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SEND_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_send_until,(struct rt_task_struct *task, unsigned long msg, RTIME time))
{
	struct { struct rt_task_struct *task; unsigned long msg; RTIME time; } arg = { task, msg, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SEND_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_send_timed,(struct rt_task_struct *task, unsigned long msg, RTIME delay))
{
	struct { struct rt_task_struct *task; unsigned long msg; RTIME delay; } arg = { task, msg, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SEND_TIMED, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_evdrp,(struct rt_task_struct *task, void *msg))
{
	struct { struct rt_task_struct *task; void *msg; } arg = { task, msg };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, EVDRP, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receive,(struct rt_task_struct *task, void *msg))
{
	struct { struct rt_task_struct *task; void *msg; } arg = { task, msg };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVEMSG, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receive_if,(struct rt_task_struct *task, void *msg))
{
	struct { struct rt_task_struct *task; void *msg; } arg = { task, msg };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVE_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receive_until,(struct rt_task_struct *task, void *msg, RTIME time))
{
	struct { struct rt_task_struct *task; void *msg; RTIME time; } arg = { task, msg, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVE_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receive_timed,(struct rt_task_struct *task, void *msg, RTIME delay))
{
	struct { struct rt_task_struct *task; void *msg; RTIME delay; } arg = { task, msg, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVE_TIMED, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpc,(struct rt_task_struct *task, unsigned long to_do, void *result))
{
	struct { struct rt_task_struct *task; unsigned long to_do; void *result; } arg = { task, to_do, result };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPCMSG, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpc_if,(struct rt_task_struct *task, unsigned long to_do, void *result))
{
	struct { struct rt_task_struct *task; unsigned long to_do; void *result; } arg = { task, to_do, result };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPC_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpc_until,(struct rt_task_struct *task, unsigned long to_do, void *result, RTIME time))
{
	struct { struct rt_task_struct *task; unsigned long to_do; void *result; RTIME time; } arg = { task, to_do, result, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPC_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpc_timed,(struct rt_task_struct *task, unsigned long to_do, void *result, RTIME delay))
{
	struct { struct rt_task_struct *task; unsigned long to_do; void *result; RTIME delay; } arg = { task, to_do, result, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPC_TIMED, &arg).v[LOW];
}

RTAI_PROTO(int, rt_isrpc,(struct rt_task_struct *task))
{
	struct { struct rt_task_struct *task; } arg = { task };
	return (int)rtai_lxrt(BIDX, SIZARG, ISRPC, &arg).i[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_return,(struct rt_task_struct *task, unsigned long result))
{
	struct { struct rt_task_struct *task; unsigned long result; } arg = { task, result };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RETURNMSG, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpcx,(struct rt_task_struct *task, void *smsg, void *rmsg, int ssize, int rsize))
{
	struct { struct rt_task_struct *task; void *smsg; void *rmsg; long ssize; long rsize; } arg = { task, smsg, rmsg, ssize, rsize };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPCX, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpcx_if,(struct rt_task_struct *task, void *smsg, void *rmsg, int ssize, int rsize))
{
	struct { struct rt_task_struct *task; void *smsg; void *rmsg; long ssize; long rsize; } arg = { task, smsg, rmsg, ssize, rsize };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPCX_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpcx_until,(struct rt_task_struct *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME time))
{
	struct { struct rt_task_struct *task; void *smsg; void *rmsg; long ssize; long rsize; RTIME time; } arg = { task, smsg, rmsg, ssize, rsize, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPCX_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_rpcx_timed,(struct rt_task_struct *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME delay))
{
	struct { struct rt_task_struct *task; void *smsg; void *rmsg; long ssize; long rsize; RTIME delay; } arg = { task, smsg, rmsg, ssize, rsize, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RPCX_TIMED, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_sendx,(struct rt_task_struct *task, void *msg, int size))
{
	struct { struct rt_task_struct *task; void *msg; long size; } arg = { task, msg, size };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SENDX, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_sendx_if,(struct rt_task_struct *task, void *msg, int size))
{
	struct { struct rt_task_struct *task; void *msg; long size; } arg = { task, msg, size };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SENDX_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_sendx_until,(struct rt_task_struct *task, void *msg, int size, RTIME time))
{
	struct { struct rt_task_struct *task; void *msg; long size; RTIME time; } arg = { task, msg, size, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SENDX_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_sendx_timed,(struct rt_task_struct *task, void *msg, long size, RTIME delay))
{
	struct { struct rt_task_struct *task; void *msg; long size; RTIME delay; } arg = { task, msg, size, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, SENDX_TIMED, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_returnx,(struct rt_task_struct *task, void *msg, int size))
{
	struct { struct rt_task_struct *task; void *msg; long size; } arg = { task, msg, size };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RETURNX, &arg).v[LOW];
}

#define rt_isrpcx(task)  rt_isrpc(task)

RTAI_PROTO(struct rt_task_struct *,rt_evdrpx,(struct rt_task_struct *task, void *msg, int size, long *len))
{
	struct { struct rt_task_struct *task; void *msg; long size; long *len; } arg = { task, msg, size, len };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, EVDRPX, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receivex,(struct rt_task_struct *task, void *msg, int size, long *len))
{
	struct { struct rt_task_struct *task; void *msg; long size; long *len; } arg = { task, msg, size, len };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVEX, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receivex_if,(struct rt_task_struct *task, void *msg, int size, long *len))
{
	struct { struct rt_task_struct *task; void *msg; long size; long *len; } arg = { task, msg, size, len };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVEX_IF, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receivex_until,(struct rt_task_struct *task, void *msg, int size, long *len, RTIME time))
{
	struct { struct rt_task_struct *task; void *msg; long size; long *len; RTIME time; } arg = { task, msg, size, len, time };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVEX_UNTIL, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_receivex_timed,(struct rt_task_struct *task, void *msg, int size, long *len, RTIME delay))
{
	struct { struct rt_task_struct *task; void *msg; long size; long *len; RTIME delay; } arg = { task, msg, size, len, delay };
	return (struct rt_task_struct *)rtai_lxrt(BIDX, SIZARG, RECEIVEX_TIMED, &arg).v[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_proxy_attach,(struct rt_task_struct *proxy, void *msg, int nbytes, int priority))
{
	struct { struct rt_task_struct *proxy; void *msg; long nbytes, priority;} arg = { proxy, msg, nbytes, priority };
	return (struct rt_task_struct *) rtai_lxrt(BIDX, SIZARG, PROXY_ATTACH, &arg).v[LOW];
}

RTAI_PROTO(int, rt_proxy_detach,(struct rt_task_struct *proxy))
{
	struct { struct rt_task_struct *proxy; } arg = { proxy };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, PROXY_DETACH, &arg).i[LOW];
}

RTAI_PROTO(struct rt_task_struct *,rt_trigger,(struct rt_task_struct *proxy))
{
	struct { struct rt_task_struct *proxy; } arg = { proxy };
	return (struct rt_task_struct *) rtai_lxrt(BIDX, SIZARG, PROXY_TRIGGER, &arg).v[LOW];
}

RTAI_PROTO(int, rt_Send,(pid_t pid, void *smsg, void *rmsg, size_t ssize, size_t rsize ))
{
	struct { long pid; void *smsg; void *rmsg; unsigned long ssize, rsize;} arg = { pid, smsg, rmsg, ssize, rsize };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_SEND, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Receive,(pid_t pid, void *msg, size_t maxsize, size_t *msglen))
{
	struct { long pid; void *msg; unsigned long maxsize; size_t *msglen; } arg = { pid, msg, maxsize, msglen };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_RECEIVE, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Creceive,(pid_t pid, void *msg, size_t maxsize, size_t *msglen, RTIME delay))
{
	struct { long pid; void *msg; unsigned long maxsize; size_t *msglen; RTIME delay;} arg = { pid, msg, maxsize, msglen, delay };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_CRECEIVE, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Reply,(pid_t pid, void *msg, size_t size))
{
	struct { long pid; void *msg; unsigned long size;} arg = { pid, msg, size };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_REPLY, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Proxy_attach,(pid_t pid, void *msg, int nbytes, int priority))
{
	struct { long pid; void *msg; long nbytes, priority;} arg = { pid, msg, nbytes, priority };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_PROXY_ATTACH, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Proxy_detach,(pid_t pid))
{
	struct { long pid; } arg = { pid };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_PROXY_DETACH, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Trigger,(pid_t pid))
{
	struct { long pid; } arg = { pid };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_TRIGGER, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Alias_attach,(const char *name))
{
	struct { const char *name; } arg = { name};
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_NAME_ATTACH, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_Name_locate,(const char *host, const char *name))
{
	struct { const char *host, *name; } arg = { host, name };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_NAME_LOCATE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_Name_detach,(pid_t pid))
{
	struct { long pid; } arg = { pid };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_NAME_DETACH, &arg).i[LOW];
}

RTAI_PROTO(int, rt_InitTickQueue,(void))
{
	struct { unsigned long dummy; } arg;
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_INITTICKQUEUE, &arg).i[LOW];
}

RTAI_PROTO(void, rt_ReleaseTickQueue,(void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, RT_RELEASETICKQUEUE, &arg);
}

RTAI_PROTO(unsigned long, rt_qDynAlloc,(unsigned long n))
{
	struct { unsigned long n; } arg = { n };
	return (unsigned long) rtai_lxrt(BIDX, SIZARG, RT_QDYNALLOC, &arg).i[LOW];
} 

RTAI_PROTO(unsigned long, rt_qDynFree,(int n))
{
	struct { long n; } arg = { n };
	return (unsigned long) rtai_lxrt(BIDX, SIZARG, RT_QDYNFREE, &arg).i[LOW];
}

RTAI_PROTO(struct QueueBlock *,rt_qDynInit,(struct QueueBlock **q, void (*fun)(void *, int), void *data, int evn ))
{
	struct QueueBlock *r;

	struct { struct QueueBlock **q; void (*fun)(void *, int), *data; long evn; } arg = { 0, fun, data, evn };
	r  = (struct QueueBlock *) rtai_lxrt(BIDX, SIZARG, RT_QDYNINIT, &arg).v[LOW];
	if (q) *q = r;
	return r;
}

RTAI_PROTO(void, rt_qBlkWait,(struct QueueBlock *q, RTIME t))
{
	struct { struct QueueBlock *q; RTIME t; } arg = { q, t } ;
	rtai_lxrt(BIDX, SIZARG, RT_QBLKWAIT, &arg);
}

RTAI_PROTO(void, rt_qBlkRepeat,(struct QueueBlock *q, RTIME t))
{
	struct { struct QueueBlock *q; RTIME t; } arg = { q, t } ;
	rtai_lxrt(BIDX, SIZARG, RT_QBLKREPEAT, &arg);
}

RTAI_PROTO(void, rt_qBlkSoon,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKSOON, &arg);
}

RTAI_PROTO(void, rt_qBlkDequeue,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKDEQUEUE, &arg);
}

RTAI_PROTO(void, rt_qBlkCancel,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKCANCEL, &arg);
}

RTAI_PROTO(void, rt_qBlkBefore,(struct QueueBlock *cur, struct QueueBlock *nxt))
{
	struct { struct QueueBlock *cur, *nxt; } arg = { cur, nxt };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKBEFORE, &arg);
}

RTAI_PROTO(void, rt_qBlkAfter,(struct QueueBlock *cur, struct QueueBlock *prv))
{
	struct { struct QueueBlock *cur, *prv; } arg = { cur, prv };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKAFTER, &arg);
}

RTAI_PROTO(struct QueueBlock *,rt_qBlkUnhook,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	return (struct QueueBlock *) rtai_lxrt(BIDX, SIZARG, RT_QBLKUNHOOK, &arg).v[LOW];
}

RTAI_PROTO(void, rt_qBlkRelease,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKRELEASE, &arg);
}

RTAI_PROTO(void, rt_qBlkComplete,(struct QueueBlock *q))
{
	struct { struct QueueBlock *q; } arg = { q };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKCOMPLETE, &arg);
}

RTAI_PROTO(int, rt_qSync,(void))
{
	struct { unsigned long long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, RT_QSYNC, &arg).i[LOW];
}

RTAI_PROTO(pid_t, rt_qReceive,(pid_t target, void *buf, size_t maxlen, size_t *msglen))
{
	struct { long target; void *buf; unsigned long maxlen; size_t *msglen; } arg = { target, buf, maxlen, msglen };
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_QRECEIVE, &arg).i[LOW];
}

RTAI_PROTO(void, rt_qLoop,(void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, RT_QLOOP, &arg);
}

RTAI_PROTO(RTIME, rt_qStep,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, RT_QSTEP, &arg).rt;
}

RTAI_PROTO(void, rt_qHookFlush,(struct QueueHook *h))
{
	struct { struct QueueHook *h; } arg = { h };
	rtai_lxrt(BIDX, SIZARG, RT_QHOOKFLUSH, &arg);
}

RTAI_PROTO(void, rt_qBlkAtHead,(struct QueueBlock *q, struct QueueHook *h))
{
	struct { struct QueueBlock *q; struct QueueHook *h; } arg = { q, h };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKATHEAD, &arg);
}

RTAI_PROTO(void, rt_qBlkAtTail,(struct QueueBlock *q, struct QueueHook *h))
{
	struct { struct QueueBlock *q; struct QueueHook *h; } arg = { q, h };
	rtai_lxrt(BIDX, SIZARG, RT_QBLKATTAIL, &arg);
}

RTAI_PROTO(struct QueueHook *,rt_qHookInit,(struct QueueHook **h, void (*c)(void *, struct QueueBlock *), void *a))
{
	struct QueueHook *r;
	struct { struct QueueHook **h; void (*c)(void *, struct QueueBlock *), *a;} arg = { 0, c, a };
	r = (struct QueueHook *) rtai_lxrt(BIDX, SIZARG, RT_QHOOKINIT, &arg).v[LOW];
	if (h) *h = r;
	return r;
}

RTAI_PROTO(void, rt_qHookRelease,(struct QueueHook *h))
{
	struct { struct QueueHook *h; } arg = { h };
	rtai_lxrt(BIDX, SIZARG, RT_QHOOKRELEASE, &arg);
}

RTAI_PROTO(void, rt_qBlkSchedule,(struct QueueBlock *q, RTIME t))
{
	struct { struct QueueBlock *q; RTIME t; } arg = { q, t } ;
	rtai_lxrt(BIDX, SIZARG, RT_QBLKSCHEDULE, &arg);
}

RTAI_PROTO(struct QueueHook *,rt_GetTickQueueHook,(void))
{
	struct { unsigned long dummy; } arg;
	return (struct QueueHook *) rtai_lxrt(BIDX, SIZARG, RT_GETTICKQUEUEHOOK, &arg).v[LOW];
}

RTAI_PROTO(pid_t, rt_vc_reserve,( void ))
{
	struct { unsigned long dummy; } arg;
	return (pid_t) rtai_lxrt(BIDX, SIZARG, RT_VC_RESERVE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_vc_attach,(pid_t pid))
{
	struct { long pid; } arg = { pid };
	return rtai_lxrt(BIDX, SIZARG, RT_VC_ATTACH, &arg).i[LOW];
}

RTAI_PROTO(int, rt_vc_release,(pid_t pid))
{
	struct { long pid; } arg = { pid };
	return rtai_lxrt(BIDX, SIZARG, RT_VC_RELEASE, &arg).i[LOW];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct t_msgcb {
    int opaque;
} MSGCB;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_MSG_H */
