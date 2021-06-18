/*
 * Copyright (C) 1999-2008 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_NETRPC_H
#define _RTAI_NETRPC_H

#include <rtai_config.h>

#include <rtai_registry.h>
#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_mbx.h>
#include <rtai_msg.h>

#ifdef CONFIG_X86_32
#define MAX_SOCKS     16
#else
#define MAX_SOCKS     32
#endif

#define MAX_MSG_SIZE  1500

#define NET_RPC_EXT  0

#define NETRPC_BASEPORT  5000

#define PORT_SHF  1
#define PORT_INC  1
#define PORT_MSK  ((1 << PORT_SHF) - 1)

#define MACH_SIZE(port)  (((port) & PORT_MSK) ? 64 : 32)

/* "for writes" - "for reads" below must be the same as those in rtai_lxrt.h */

// for writes
#define UW1(bf, sz)  ((((bf) & 0x7) << 19) | (((sz) & 0x7) << 22))
#define UW2(bf, sz)  ((((bf) & 0x7) << 25) | (((sz) & 0x7) << 28))

// for reads
#define UR1(bf, sz)  ((((bf) & 0x7) << 3) | (((sz) & 0x7) <<  6))
#define UR2(bf, sz)  ((((bf) & 0x7) << 9) | (((sz) & 0x7) << 12))

#define SIZARG sizeof(arg)

//#define PACKPORT(port, ext, fun, timed) (((port) << 18) | ((timed) << 13) | ((ext) << 8) | (fun))
#define PACKPORT(port, ext, fun, timed) ((unsigned long)(((port) << 18) | ((timed) << 13) | ((ext) << 8) | (fun)))

#define PORT(i)   ((i) >> 18)
#define FUN(i)    ((i) & 0xFF)
#define EXT(i)    (((i) >> 8) & 0x1F)
#define TIMED(i)  (((i) >> 13) & 0x1F)

/* 
 * SYNC_NET_RPC is hard wired here, no need to have it elsewhere. It must 
 * have all the bits allowed to the "fun" field, in PACKPORT above, set.
 */
#define SYNC_NET_RPC  0xFF  // hard wired here, no need to have it elsewhere

#define PRT_REQ  1
#define PRT_SRV  2
#define PRT_RTR  3
#define PRT_RCV  4
#define RPC_REQ  5
#define RPC_SRV  6
#define RPC_RTR  7
#define RPC_RCV  8

#define ADR_CNV_MSK  (0xFFFFFFFCUL)

#define OWNER(node, task) \
	( (((unsigned long long)(node)) << 32)   | \
	  (((unsigned long)(task)) & ADR_CNV_MSK) )
	
#define TSK_FRM_WNR(i)	((i) & 0xFFFFFFFF);

#define WDW     3
#define WDWMSK  ((1 << WDW) - 1)

#define  SINT  1
#define  UINT  2
#define  VADR  3
#define  RTIM  4

#define PARTYPES1(a)  (a)
#define PARTYPES2(a, b)  (((b) << WDW) | (a))
#define PARTYPES3(a, b, c)  (((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES4(a, b, c, d)  (((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES5(a, b, c, d, e)  (((e) << 4*WDW) | ((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES6(a, b, c, d, e, f)  (((f) << 5*WDW) | ((e) << 4*WDW) | ((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES7(a, b, c, d, e, f, g)  (((g) << 6*WDW) | ((f) << 5*WDW) | ((e) << 4*WDW) | ((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES8(a, b, c, d, e, f, g, h)  (((h) << 7*WDW) | ((g) << 6*WDW) | ((f) << 5*WDW) | ((e) << 4*WDW) | ((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))
#define PARTYPES9(a, b, c, d, e, f, g, h, i)  (((i) << 8*WDW) | ((h) << 7*WDW) | ((g) << 6*WDW) | ((f) << 5*WDW) | ((e) << 4*WDW) | ((d) << 3*WDW) | ((c) << 2*WDW) | ((b) << WDW) | (a))

// must be as Linux puts them, always the same likely
#ifndef KERNEL_64_ADDRESS_OFFSET
#define KERNEL_64_ADDRESS_OFFSET
static unsigned long long kadr_ofst[] = 
	{0xFFFFC20000000000ULL, 0xFFFF810000000000ULL, 0xFFFFFFFF80000000ULL };
static inline unsigned long reset_kadr(unsigned long val)
{
	return (val & ADR_CNV_MSK) | kadr_ofst[val & 0x3];
}
#endif

static inline void *RTAI_LXRT(int port, union rtai_lxrt_t retval)
{
	if (sizeof(long) == 4 && (abs(port) & PORT_MSK)) {
		int i;
		for (i = 0; i < 3; i++) {
			if ((retval.rt & 0xFF0000000000ULL) == (kadr_ofst[i] & 0xFF0000000000ULL)) {
				retval.rt |= i;
				return retval.v[LOW];
			}
		}
		return (void *)RTE_OBJINV;
	}
	return retval.v[LOW];
}

#define RT_NET_RPC(port, retval)  RTAI_LXRT(port, (union rtai_lxrt_t)retval)

#define RT_NET_ADR(port, adr) \
	 RTAI_LXRT(port, (union rtai_lxrt_t)((long long)(adr)))

#ifdef __KERNEL__

#include <rtai_sched.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_netrpc_init(void);

void __rtai_netrpc_exit(void);

RTAI_SYSCALL_MODE long long _rt_net_rpc(long fun_ext_timed,
					long type,
					void *args,
					int argsize,
					int space, 
					unsigned long partypes);

#define rt_net_rpc(fun_ext_timed, type, args, argsize, space, partypes) \
	({ union rtai_lxrt_t retval; retval.rt = _rt_net_rpc(fun_ext_timed, type, args, argsize, space, partypes); retval; })


RTAI_SYSCALL_MODE int rt_set_netrpc_timeout( int port, RTIME timeout);

RTAI_SYSCALL_MODE int rt_send_req_rel_port(unsigned long node, 
					   int port,
					   unsigned long id,
					   MBX *mbx,
					   int hard);

RTAI_SYSCALL_MODE unsigned long ddn2nl(const char *ddn);

RTAI_SYSCALL_MODE unsigned long rt_set_this_node(const char *ddn,
						 unsigned long node,
						 int hard);

RTAI_SYSCALL_MODE RT_TASK *rt_find_asgn_stub(unsigned long long owner,
					     int asgn);

RTAI_SYSCALL_MODE int rt_rel_stub(unsigned long long owner);

RTAI_SYSCALL_MODE int rt_waiting_return(unsigned long node,
					int port);

int rt_get_net_rpc_ret(MBX *mbx,
		       unsigned long long *retval,
		       void *msg1,
		       int *msglen1,
		       void *msg2,
		       int *msglen2,
		       RTIME timeout,
		       int type);

static inline int rt_sync_net_rpc(unsigned long node, int port)
{
	if (node) {
		struct { long dummy; } arg = { 0 };
		return rt_net_rpc(PACKPORT(abs(port), NET_RPC_EXT, SYNC_NET_RPC, 0), 0, &arg, SIZARG, 1, PARTYPES1(UINT)).i[LOW];
	}
	return 1;
} 

static inline void *RT_get_adr(unsigned long node, int port, const char *sname)
{
	if (node) {
		struct { unsigned long name; } arg = { nam2num(sname) };
		return RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, GET_ADR, 0), 0, &arg, SIZARG, 1, PARTYPES1(UINT)));
	}
	return rt_get_adr(nam2num(sname));
} 

static inline RTIME RT_get_time_ns(unsigned long node, int port)
{
	if (node) {
		struct { long dummy; } arg = { 0 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, GET_TIME_NS, 0), 0, &arg, SIZARG, 1, PARTYPES1(UINT)).rt;
	}
	return rt_get_time_ns();
}

static inline RTIME RT_get_time_ns_cpuid(unsigned long node, int port, int cpuid)
{
	if (node) {
		struct { long cpuid; } arg = { cpuid };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, GET_TIME_NS_CPUID, 0), 0, &arg, SIZARG, 1, PARTYPES1(UINT)).rt;
	}
	return rt_get_time_ns_cpuid(cpuid);
}

static inline RTIME RT_get_cpu_time_ns(unsigned long node, int port)
{
	if (node) {
		struct { long dummy; } arg = { 0 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, GET_CPU_TIME_NS, 0), 0, &arg, SIZARG, 1, PARTYPES1(UINT)).rt;
	}
	return rt_get_cpu_time_ns();
}

static inline int RT_task_suspend(unsigned long node, int port, RT_TASK *task)
{
	if (node) {
		struct { RT_TASK *task; } arg = { task };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SUSPEND, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_task_suspend(task);
}

static inline int RT_task_resume(unsigned long node, int port, RT_TASK *task)
{
	if (node) {
		struct { RT_TASK *task; } arg = { task };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RESUME, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_task_resume(task);
}

static inline void RT_sleep(unsigned long node, int port, RTIME delay)
{
	if (node) {
		struct { RTIME delay; } arg = { delay };
		rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SLEEP, 1), 0, &arg, SIZARG, 1, PARTYPES1(RTIM));
		return;
	}
	rt_sleep(nano2count(delay));
} 

static inline void RT_sleep_until(unsigned long node, int port, RTIME time)
{
	if (node) {
		struct { RTIME time; } arg = { time };
		rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SLEEP_UNTIL, 1), 0, &arg, SIZARG, 1, PARTYPES1(RTIM));
		return;
	}
	rt_sleep_until(nano2count(time));
} 

#if CONFIG_RTAI_SEM

static inline SEM *RT_typed_named_sem_init(unsigned long node, int port, const char *sem_name, int value, int type)
{
	if (node) {
		struct { unsigned long sem_name; long value; long type; unsigned long *handle; } arg = { nam2num(sem_name), value, type, NULL };
		return (SEM *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, NAMED_SEM_INIT, 0), 0, &arg, SIZARG, 1, PARTYPES3(VADR, SINT, SINT)));
	}
	return rt_typed_named_sem_init(sem_name, value, type);
}

static inline int RT_named_sem_delete(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, NAMED_SEM_DELETE, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_named_sem_delete(sem);
}

static inline int RT_sem_signal(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_SIGNAL, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_sem_signal(sem);
} 

static inline int RT_sem_broadcast(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_BROADCAST, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_sem_broadcast(sem);
} 

static inline int RT_sem_wait(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_WAIT, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_sem_wait(sem);
} 

static inline int RT_sem_wait_if(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_WAIT_IF, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_sem_wait_if(sem);
} 

static inline int RT_sem_wait_until(unsigned long node, int port, SEM *sem, RTIME time)
{
	if (node) {
		struct { SEM *sem; RTIME time; } arg = { sem, time };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_WAIT_UNTIL, 2), 0, &arg, SIZARG, 1, PARTYPES2(VADR, RTIM)).i[LOW];
	}
	return rt_sem_wait_until(sem, nano2count(time));
} 

static inline int RT_sem_wait_timed(unsigned long node, int port, SEM *sem, RTIME delay)
{
	if (node) {
		struct { SEM *sem; RTIME delay; } arg = { sem, delay };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEM_WAIT_TIMED, 2), 0, &arg, SIZARG, 1, PARTYPES2(VADR, RTIM)).i[LOW];
	}
	return rt_sem_wait_timed(sem, nano2count(delay));
} 

#ifdef CONFIG_RTAI_RT_POLL

static inline int RT_poll_4to8(unsigned long node, int port, struct rt_poll_s *pdsain, unsigned long nr, RTIME timeout)
{
	unsigned int i;
	int retval;
	struct rt_poll_lls { unsigned long long what, forwhat; } pdsa[nr];
	struct { void *pdsa1; void *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_lls), timeout };
	for (i = 0; i < nr; i++) {
		pdsa[i].what    = (unsigned long)pdsain[i].what;
		pdsa[i].forwhat = pdsain[i].forwhat;
	}
	retval = rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(UINT, UINT, UINT, RTIM)).i[LOW];
	for (i = 0; i < nr; i++) {
		pdsain[i].what    = (void *)(unsigned long)pdsa[i].what;
		pdsain[i].forwhat = pdsa[i].forwhat;
	}
	return retval;
} 

static inline int RT_poll_8to4(unsigned long node, int port, struct rt_poll_s *pdsain, unsigned long nr, RTIME timeout)
{
	unsigned int i;
	int retval;
	struct rt_poll_is { unsigned int what, forwhat; } pdsa[nr];
	struct { void *pdsa1; void *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_is), timeout };
	for (i = 0; i < nr; i++) {
		pdsa[i].what    = (unsigned int)(unsigned long)pdsain[i].what;
		pdsa[i].forwhat = pdsain[i].forwhat;
	}
	retval = rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(UINT, UINT, UINT, RTIM)).i[LOW];
	for (i = 0; i < nr; i++) {
		pdsain[i].what    = (void *)(unsigned long)pdsa[i].what;
		pdsain[i].forwhat = pdsa[i].forwhat;
	}
	return retval;
} 

static inline int RT_poll(unsigned long node, int port, struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout)
{
	if (node) {
		if (sizeof(long) == 4 && (abs(port) & PORT_MSK)) {
			return RT_poll_4to8(node, port, pdsa, nr, timeout);
		} else if (sizeof(long) == 8 && !(abs(port) & PORT_MSK)) {
			return RT_poll_8to4(node, port, pdsa, nr, timeout);
		} else {
			struct { struct rt_poll_s *pdsa1; struct rt_poll_s *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_s), timeout };
			return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(UINT, UINT, UINT, RTIM)).i[LOW];
		}
	}
	return rt_poll(pdsa, nr, nano2count(timeout));
} 

#endif

#endif /* CONFIG_RTAI_SEM */

#if CONFIG_RTAI_MSG

static inline RT_TASK *RT_send(unsigned long node, int port, RT_TASK *task, unsigned long msg)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; } arg = { task, msg };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SENDMSG, 0), 0, &arg, SIZARG, 1, PARTYPES2(VADR, UINT)));
	}
	return rt_send(task, msg);
}

static inline RT_TASK *RT_send_if(unsigned long node, int port, RT_TASK *task, unsigned long msg)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; } arg = { task, msg };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEND_IF, 0), 0, &arg, SIZARG, 1, PARTYPES2(VADR, UINT)));
	}
	return rt_send_if(task, msg);
}

static inline RT_TASK *RT_send_until(unsigned long node, int port, RT_TASK *task, unsigned long msg, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; RTIME time; } arg = { task, msg, time };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEND_UNTIL, 3), 0, &arg, SIZARG, 1, PARTYPES3(VADR, UINT, RTIM)));
	}
	return rt_send_until(task, msg, nano2count(time));
}

static inline RT_TASK *RT_send_timed(unsigned long node, int port, RT_TASK *task, unsigned long msg, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; RTIME delay; } arg = { task, msg, delay };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SEND_TIMED, 3), 0, &arg, SIZARG, 1, PARTYPES3(VADR, UINT, RTIM)));
	}
	return rt_send_timed(task, msg, nano2count(delay));
}

static inline RT_TASK *RT_receive(unsigned long node, int port, RT_TASK *task, unsigned long *msg)
{
	if (!task || !node) {
		return rt_receive(task, msg);
	}
	return rt_receive(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
}

static inline RT_TASK *RT_receive_if(unsigned long node, int port, RT_TASK *task, unsigned long *msg)
{
	if (!task || !node) {
		return rt_receive_if(task, msg);
	}
	return rt_receive_if(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
}

static inline RT_TASK *RT_receive_until(unsigned long node, int port, RT_TASK *task, unsigned long *msg, RTIME time)
{
	if (!task || !node) {
		return rt_receive_until(task, msg, nano2count(time));
	}
	return rt_receive_until(rt_find_asgn_stub(OWNER(node, task), 1), msg, nano2count(time)) ? task : 0;
}

static inline RT_TASK *RT_receive_timed(unsigned long node, int port, RT_TASK *task, unsigned long *msg, RTIME delay)
{
	if (!task || !node) {
		return rt_receive_timed(task, msg, nano2count(delay));
	}
	return rt_receive_timed(rt_find_asgn_stub(OWNER(node, task), 1), msg, nano2count(delay)) ? task : 0;
}

static inline RT_TASK *RT_rpc(unsigned long node, int port, RT_TASK *task, unsigned long msg, unsigned long *ret)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; unsigned long *ret; } arg = { task, msg, ret };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPCMSG, 0), UW1(3, 0), &arg, SIZARG, 1, PARTYPES3(VADR, UINT, UINT)));
	}
	return rt_rpc(task, msg, ret);
}

static inline RT_TASK *RT_rpc_if(unsigned long node, int port, RT_TASK *task, unsigned long msg, unsigned long *ret)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; unsigned long *ret; } arg = { task, msg };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPC_IF, 0), UW1(3, 0), &arg, SIZARG, 1, PARTYPES3(VADR, UINT, UINT)));
	}
	return rt_rpc_if(task, msg, ret);
}

static inline RT_TASK *RT_rpc_until(unsigned long node, int port, RT_TASK *task, unsigned long msg, unsigned long *ret, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; unsigned long *ret; RTIME time; } arg = { task, msg, ret, time };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPC_UNTIL, 4), UW1(3, 0), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, RTIM)));
	}
	return rt_rpc_until(task, msg, ret, nano2count(time));
}

static inline RT_TASK *RT_rpc_timed(unsigned long node, int port, RT_TASK *task, unsigned long msg, unsigned long *ret, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; unsigned long *ret; RTIME delay; } arg = { task, msg, ret, delay };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPC_TIMED, 4), UW1(3, 0), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, RTIM)));
	}
	return rt_rpc_timed(task, msg, ret, nano2count(delay));
}

static inline int RT_isrpc(unsigned long node, int port, RT_TASK *task)
{
        if (node) {
                struct { RT_TASK *task; } arg = { task };
                return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, ISRPC, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
        }
        return rt_isrpc(task);
}

static inline RT_TASK *RT_return(unsigned long node, int port, RT_TASK *task, unsigned long result)
{
	if (!task || !node) {
		return rt_return(task, result);
        }
	return rt_return(rt_find_asgn_stub(OWNER(node, task), 1), result) ? task : 0;
}

static inline RT_TASK *RT_evdrp(unsigned long node, int port, RT_TASK *task, unsigned long *msg)
{
	if (!task || !node) {
		return rt_evdrp(task, msg);
	}
	return rt_evdrp(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
}

static inline RT_TASK *RT_rpcx(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize)
{
	if (node) {
		struct { RT_TASK *task; void *smsg; void *rmsg; long ssize; long rsize; } arg = { task, smsg, rmsg, ssize, rsize };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPCX, 0), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, UINT, UINT)));
	}
	return rt_rpcx(task, smsg, rmsg, ssize, rsize);
}

static inline RT_TASK *RT_rpcx_if(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize)
{
	if (node) {
		struct { RT_TASK *task; void *smsg; void *rmsg; long ssize; long rsize; } arg = { task, smsg, rmsg, ssize, rsize };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPCX_IF, 0), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, UINT, UINT)));
	}
	return rt_rpcx_if(task, smsg, rmsg, ssize, rsize);
}

static inline RT_TASK *RT_rpcx_until(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; void *smsg; void *rmsg; long ssize; long rsize; RTIME time; } arg = { task, smsg, rmsg, ssize, rsize, time };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPCX_UNTIL, 6), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 1, PARTYPES6(VADR, UINT, UINT, UINT, UINT, RTIM)));
	}
	return rt_rpcx_until(task, smsg, rmsg, ssize, rsize, nano2count(time));
}

static inline RT_TASK *RT_rpcx_timed(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; void *smsg; void *rmsg; long ssize; long rsize; RTIME delay; } arg = { task, smsg, rmsg, ssize, rsize, delay };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, RPCX_TIMED, 6), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 1, PARTYPES6(VADR, UINT, UINT, UINT, UINT, RTIM)));
	}
	return rt_rpcx_timed(task, smsg, rmsg, ssize, rsize, nano2count(delay));
}

static inline RT_TASK *RT_sendx(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; } arg = { task, msg, size };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SENDX, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES3(VADR, UINT, UINT)));
	}
	return rt_sendx(task, msg, size);
}

static inline RT_TASK *RT_sendx_if(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; } arg = { task, msg, size };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SENDX_IF, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES3(VADR, UINT, UINT)));
	}
	return rt_sendx_if(task, msg, size);
}

static inline RT_TASK *RT_sendx_until(unsigned long node, int port, RT_TASK *task, void *msg, int size, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; RTIME time; } arg = { task, msg, size, time };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SENDX_UNTIL, 4), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, RTIM)));
	}
	return rt_sendx_until(task, msg, size, nano2count(time));
}

static inline RT_TASK *RT_sendx_timed(unsigned long node, int port, RT_TASK *task, void *msg, int size, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; RTIME delay; } arg = { task, msg, size, delay };
		return (RT_TASK *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, SENDX_TIMED, 4), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, RTIM)));
	}
	return rt_sendx_timed(task, msg, size, nano2count(delay));
}

static inline RT_TASK *RT_returnx(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	if (!task || !node) {
		return rt_returnx(task, msg, size);
	}
	return rt_returnx(rt_find_asgn_stub(OWNER(node, task), 1), msg, size) ? task : 0;
}

static inline RT_TASK *RT_evdrpx(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	if (!task || !node) {
		return rt_evdrpx(task, msg, size, len);
	}
	return rt_evdrpx(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
}

static inline RT_TASK *RT_receivex(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	if (!task || !node) {
		return rt_receivex(task, msg, size, len);
	}
	return rt_receivex(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
}

static inline RT_TASK *RT_receivex_if(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	if (!task || !node) {
		return rt_receivex_if(task, msg, size, len);
	}
	return rt_receivex_if(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
}

static inline RT_TASK *RT_receivex_until(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len, RTIME time)
{
	if (!task || !node) {
		return rt_receivex_until(task, msg, size, len, nano2count(time));
	}
	return rt_receivex_until(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len, nano2count(time)) ? task : 0;
}

static inline RT_TASK *RT_receivex_timed(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len, RTIME delay)
{
	if (!task || !node) {
		return rt_receivex_timed(task, msg, size, len, nano2count(delay));
	}
	return rt_receivex_timed(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len, nano2count(delay)) ? task : 0;
}

#endif /* CONFIG_RTAI_MSG */

#if CONFIG_RTAI_MBX

static inline MBX *RT_typed_named_mbx_init(unsigned long node, int port, const char *mbx_name, int size, int qtype)
{
	if (node) {
		struct { unsigned long mbx_name; long size; long qype; } arg = { nam2num(mbx_name), size, qtype };
		return (MBX *)RT_NET_RPC(port, rt_net_rpc(PACKPORT(port, NET_RPC_EXT, NAMED_MBX_INIT, 0), 0, &arg, SIZARG, 1, PARTYPES3(UINT, UINT, UINT)));
	}
	return rt_typed_named_mbx_init(mbx_name, size, qtype);
}

static inline int RT_named_mbx_delete(unsigned long node, int port, MBX *mbx)
{
	if (node) {
		struct { MBX *mbx; } arg = { mbx };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, NAMED_MBX_DELETE, 0), 0, &arg, SIZARG, 1, PARTYPES1(VADR)).i[LOW];
	}
	return rt_named_mbx_delete(mbx);
}

static inline int RT_mbx_send(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_SEND, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_send(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_wp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_SEND_WP, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_send_wp(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_if(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_SEND_IF, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_send_if(mbx, msg, msg_size);
} 

static inline int RT_mbx_ovrwr_send(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_OVRWR_SEND, 0), UR1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_ovrwr_send(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_until(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME time)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME time; long space; } arg = { mbx, msg, msg_size, time, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_SEND_UNTIL, 4), UR1(2, 3), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, RTIM, UINT)).i[LOW];
	}
	return rt_mbx_send_until(mbx, msg, msg_size, nano2count(time));
} 

static inline int RT_mbx_send_timed(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME delay)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME delay; long space; } arg = { mbx, msg, msg_size, delay, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_SEND_TIMED, 4), UR1(2, 3), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, RTIM, UINT)).i[LOW];
	}
	return rt_mbx_send_timed(mbx, msg, msg_size, nano2count(delay));
} 

static inline int RT_mbx_evdrp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_EVDRP, 0), UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_evdrp(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE, 0), UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_receive(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_wp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_WP, 0), UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_receive_wp(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_if(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_IF, 0), UW1(2, 3), &arg, SIZARG, 1, PARTYPES4(VADR, UINT, UINT, UINT)).i[LOW];
	}
	return rt_mbx_receive_if(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_until(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME time)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME time; long space; } arg = { mbx, msg, msg_size, time, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_UNTIL, 4), UW1(2, 3), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, RTIM, UINT)).i[LOW];
	}
	return rt_mbx_receive_until(mbx, msg, msg_size, nano2count(time));
} 

static inline int RT_mbx_receive_timed(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME delay)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME delay; long space; } arg = { mbx, msg, msg_size, delay, 1 };
		return rt_net_rpc(PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_TIMED, 4), UW1(2, 3), &arg, SIZARG, 1, PARTYPES5(VADR, UINT, UINT, RTIM, UINT)).i[LOW];
	}
	return rt_mbx_receive_timed(mbx, msg, msg_size, nano2count(delay));
} 

#endif /* CONFIG_RTAI_MBX */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <stdlib.h>

#define NET_RPC_IDX  0

#define SIZARGS sizeof(args)

static inline int rt_send_req_rel_port(unsigned long node, int port, unsigned long id, MBX *mbx, int hard)
{
	struct { unsigned long node; long port; unsigned long id; MBX *mbx; long hard; } args = { node, port, id, mbx, hard };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, SEND_REQ_REL_PORT, &args).i[LOW];
} 

static inline int rt_set_netrpc_timeout(int port, RTIME timeout)
{
	struct { long port; RTIME timeout; } args = { port, timeout };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, SET_NETRPC_TIMEOUT, &args).i[LOW];
}

static inline unsigned long ddn2nl(const char *ddn)
{
	struct { const char *ddn; } args = { ddn };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, DDN2NL, &args).i[LOW];
} 

static inline unsigned long rt_set_this_node(const char *ddn, unsigned long node, int hard)
{
	struct { const char *ddn; unsigned long node; long hard; } args = { ddn, node, hard };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, SET_THIS_NODE, &args).i[LOW];
} 

static inline RT_TASK *rt_find_asgn_stub(unsigned long long owner, int asgn)
{
	struct { unsigned long long owner; long asgn; } args = { owner, asgn };
	return (RT_TASK *)rtai_lxrt(NET_RPC_IDX, SIZARGS, FIND_ASGN_STUB, &args).v[LOW];
} 

static inline int rt_rel_stub(unsigned long long owner)
{
	struct { unsigned long long owner; } args = { owner };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, REL_STUB, &args).i[LOW];
} 

static inline int rt_waiting_return(unsigned long node, int port)
{
	struct { unsigned long node; long port; } args = { node, port };
	return rtai_lxrt(NET_RPC_IDX, SIZARGS, WAITING_RETURN, &args).i[LOW];
} 

static inline int rt_sync_net_rpc(unsigned long node, int port)
{
	if (node) {
                struct { long dummy; } arg = { 0 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(abs(port), NET_RPC_EXT, SYNC_NET_RPC, 0), 0, &arg, SIZARG, 0, PARTYPES1(UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return 1;
} 

static inline void *RT_get_adr(unsigned long node, int port, const char *sname)
{
	if (node) {
		struct { unsigned long name; } arg = { nam2num(sname) };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, GET_ADR, 0), 0, &arg, SIZARG, 0, PARTYPES1(UINT) };
//		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).v[LOW];
		return RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	} 
	return rt_get_adr(nam2num(sname));
} 

static inline RTIME RT_get_time_ns(unsigned long node, int port)
{
	if (node) {
		struct { long dummy; } arg = { 0 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, GET_TIME_NS, 0), 0, &arg, SIZARG, 0, PARTYPES1(UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).rt;
	}
	return rt_get_time_ns();
} 

static inline RTIME RT_get_time_ns_cpuid(unsigned long node, int port, int cpuid)
{
	if (node) {
		struct { long cpuid; } arg = { cpuid };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, GET_TIME_NS_CPUID, 0), 0, &arg, SIZARG, 0, PARTYPES1(UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).rt;
	}
	return rt_get_time_ns_cpuid(cpuid);
} 

static inline RTIME RT_get_cpu_time_ns(unsigned long node, int port)
{
	if (node) {
		struct { long dummy; } arg = { 0 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, GET_CPU_TIME_NS, 0), 0, &arg, SIZARG, 0, PARTYPES1(UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).rt;
	}
	return rt_get_cpu_time_ns();
} 

static inline void RT_task_suspend(unsigned long node, int port, RT_TASK *task)
{
	if (node) {
		struct { RT_TASK *task; } arg = { task };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SUSPEND, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args);
		return;
	}
	rt_task_suspend(task);
} 

static inline void RT_task_resume(unsigned long node, int port, RT_TASK *task)
{
	if (node) {
		struct { RT_TASK *task; } arg = { task };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RESUME, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args);
		return;
	}
	rt_task_resume(task);
} 

static inline void RT_sleep(unsigned long node, int port, RTIME delay)
{
	if (node) {
		struct { RTIME delay; } arg = { delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SLEEP, 1), 0, &arg, SIZARG, 0, PARTYPES1(RTIM) };
		rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args);
		return;
	}
	rt_sleep(nano2count(delay));
} 

static inline void RT_sleep_until(unsigned long node, int port, RTIME time)
{
	if (node) {
		struct { RTIME time; } arg = { time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SLEEP_UNTIL, 1), 0, &arg, SIZARG, 0, PARTYPES1(RTIM) };
		rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args);
		return;
	}
	rt_sleep(nano2count(time));
} 

#if CONFIG_RTAI_SEM

static inline SEM *RT_typed_named_sem_init(unsigned long node, int port, const char *sem_name, int value, int type)
{
	if (node) {
		struct { unsigned long sem_name; long value; long type; unsigned long *handle; } arg = { nam2num(sem_name), value, type, NULL };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, NAMED_SEM_INIT, 0), 0, &arg, SIZARG, 0, PARTYPES3(VADR, SINT, SINT) };
		return (SEM *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_typed_named_sem_init(sem_name, value, type);
}

static inline int RT_named_sem_delete(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, NAMED_SEM_DELETE, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_named_sem_delete(sem);
}

static inline int RT_sem_signal(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_SIGNAL, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_sem_signal(sem);
} 

static inline int RT_sem_broadcast(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_BROADCAST, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_sem_broadcast(sem);
} 

static inline int RT_sem_wait(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_WAIT, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_sem_wait(sem);
} 

static inline int RT_sem_wait_if(unsigned long node, int port, SEM *sem)
{
	if (node) {
		struct { SEM *sem; } arg = { sem };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_WAIT_IF, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_sem_wait_if(sem);
} 

static inline int RT_sem_wait_until(unsigned long node, int port, SEM *sem, RTIME time)
{
	if (node) {
		struct { SEM *sem; RTIME time; } arg = { sem, time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_WAIT_UNTIL, 2), 0, &arg, SIZARG, 0, PARTYPES2(VADR, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_sem_wait_until(sem, nano2count(time));
} 

static inline int RT_sem_wait_timed(unsigned long node, int port, SEM *sem, RTIME delay)
{
	if (node) {
		struct { SEM *sem; RTIME delay; } arg = { sem, delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEM_WAIT_TIMED, 2), 0, &arg, SIZARG, 0, PARTYPES2(VADR, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_sem_wait_timed(sem, nano2count(delay));
} 

#ifdef CONFIG_RTAI_RT_POLL

static inline int RT_poll_4to8(unsigned long node, int port, struct rt_poll_s *pdsain, unsigned long nr, RTIME timeout)
{
	unsigned int i;
	int retval;
	struct rt_poll_lls { unsigned long long what, forwhat; } pdsa[nr];
	struct { void *pdsa1; void *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_lls), timeout };
	struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(UINT, UINT, UINT, RTIM) };
	(void)node;
	for (i = 0; i < nr; i++) {
		pdsa[i].what    = (unsigned long)pdsain[i].what;
		pdsa[i].forwhat = pdsain[i].forwhat;
	}
	retval = rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	for (i = 0; i < nr; i++) {
		pdsain[i].what    = (void *)(unsigned long)pdsa[i].what;
		pdsain[i].forwhat = pdsa[i].forwhat;
	}
	return retval;
} 

static inline int RT_poll_8to4(unsigned long node, int port, struct rt_poll_s *pdsain, unsigned long nr, RTIME timeout)
{
	unsigned int i;
	int retval;
	struct rt_poll_is { unsigned int what, forwhat; } pdsa[nr];
	struct { void *pdsa1; void *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_is), timeout };
	struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, UINT, RTIM) };
	(void)node;
	for (i = 0; i < nr; i++) {
		pdsa[i].what    = (unsigned int)(unsigned long)pdsain[i].what;
		pdsa[i].forwhat = pdsain[i].forwhat;
	}
	retval = rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	for (i = 0; i < nr; i++) {
		pdsain[i].what    = (void *)(unsigned long)pdsa[i].what;
		pdsain[i].forwhat = pdsa[i].forwhat;
	}
	return retval;
} 

static inline int RT_poll(unsigned long node, int port, struct rt_poll_s *pdsa, unsigned long nr, RTIME timeout)
{
	if (node) {
		if (sizeof(long) == 4 && (abs(port) & PORT_MSK)) {
			return RT_poll_4to8(node, port, pdsa, nr, timeout);
		} else if (sizeof(long) == 8 && !(abs(port) & PORT_MSK)) {
			return RT_poll_8to4(node, port, pdsa, nr, timeout);
		} else {
			struct { struct rt_poll_s *pdsa1; struct rt_poll_s *pdsa2; unsigned long pdsa_size; RTIME timeout; } arg = { pdsa, pdsa, nr*sizeof(struct rt_poll_s), timeout };
			struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RT_POLL_NETRPC, 4), UR1(1, 3) | UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, VADR, UINT, RTIM) };
			return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
		}
	}
	return rt_poll(pdsa, nr, nano2count(timeout));
} 

#endif

#endif /* CONFIG_RTAI_SEM */

#if CONFIG_RTAI_MSG

static inline RT_TASK *RT_send(unsigned long node, int port, RT_TASK *task, unsigned long msg)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; } arg = { task, msg };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SENDMSG, 0), 0, &arg, SIZARG, 0, PARTYPES2(VADR, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	} 
	return rt_send(task, msg);
} 

static inline RT_TASK *RT_send_if(unsigned long node, int port, RT_TASK *task, unsigned long msg)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; } arg = { task, msg };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEND_IF, 0), 0, &arg, SIZARG, 0, PARTYPES2(VADR, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	} 
	return rt_send_if(task, msg);
} 

static inline RT_TASK *RT_send_until(unsigned long node, int port, RT_TASK *task, unsigned long msg, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; RTIME time; } arg = { task, msg, time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEND_UNTIL, 3), 0, &arg, SIZARG, 0, PARTYPES3(VADR, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	} 
	return rt_send_until(task, msg, nano2count(time));
} 

static inline RT_TASK *RT_send_timed(unsigned long node, int port, RT_TASK *task, unsigned long msg, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; RTIME delay; } arg = { task, msg, delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SEND_TIMED, 3), 0, &arg, SIZARG, 0, PARTYPES3(VADR, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	} 
	return rt_send_timed(task, msg, nano2count(delay));
} 

static inline RT_TASK *RT_evdrp(unsigned long node, int port, RT_TASK *task, void *msg)
{
	(void)port;
        if (!task || !node) {
		return rt_evdrp(task, msg);
	} 
	return rt_evdrp(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
} 

static inline RT_TASK *RT_receive(unsigned long node, int port, RT_TASK *task, void *msg)
{
	(void)port;
        if (!task || !node) {
		return rt_receive(task, msg);
	} 
	return rt_receive(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
} 

static inline RT_TASK *RT_receive_if(unsigned long node, int port, RT_TASK *task, void *msg)
{
	(void)port;
        if (!task || !node) {
		return rt_receive_if(task, msg);
	} 
	return rt_receive_if(rt_find_asgn_stub(OWNER(node, task), 1), msg) ? task : 0;
} 

static inline RT_TASK *RT_receive_until(unsigned long node, int port, RT_TASK *task, void *msg, RTIME time)
{
	(void)port;
        if (!task || !node) {
		return rt_receive_until(task, msg, nano2count(time));
	} 
	return rt_receive_until(rt_find_asgn_stub(OWNER(node, task), 1), msg, nano2count(time)) ? task : 0;
} 

static inline RT_TASK *RT_receive_timed(unsigned long node, int port, RT_TASK *task, void *msg, RTIME delay)
{
	(void)port;
        if (!task || !node) {
		return rt_receive_timed(task, msg, nano2count(delay));
	} 
	return rt_receive_timed(rt_find_asgn_stub(OWNER(node, task), 1), msg, nano2count(delay)) ? task : 0;
} 

static inline RT_TASK *RT_rpc(unsigned long node, int port, RT_TASK *task, unsigned long msg, void *ret)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; void *ret; } arg = { task, msg, ret };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPCMSG, 0), UW1(3, 0), &arg, SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpc(task, msg, ret);
} 

static inline RT_TASK *RT_rpc_if(unsigned long node, int port, RT_TASK *task, unsigned long msg, void *ret)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; void *ret; } arg = { task, msg, ret };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPC_IF, 0), UW1(3, 0), &arg, SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpc_if(task, msg, ret);
} 

static inline RT_TASK *RT_rpc_until(unsigned long node, int port, RT_TASK *task, unsigned long msg, void *ret, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; void *ret; RTIME time; } arg = { task, msg, ret, time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPC_UNTIL, 4), UW1(3, 0), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpc_until(task, msg, ret, nano2count(time));
} 

static inline RT_TASK *RT_rpc_timed(unsigned long node, int port, RT_TASK *task, unsigned long msg, void *ret, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; unsigned long msg; void *ret; RTIME delay; } arg = { task, msg, ret, delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPC_TIMED, 4), UW1(3, 0), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpc_timed(task, msg, ret, nano2count(delay));
} 

static inline int RT_isrpc(unsigned long node, int port, RT_TASK *task)
{
	if (node) {
		struct { RT_TASK *task; } arg = { task };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, ISRPC, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_isrpc(task);
} 

static inline RT_TASK *RT_return(unsigned long node, int port, RT_TASK *task, unsigned long result)
{
	(void)port;
        if (!task || !node) {
		return rt_return(task, result);
	} 
	return rt_return(rt_find_asgn_stub(OWNER(node, task), 1), result) ? task : 0;
} 

static inline RT_TASK *RT_rpcx(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize)
{
	if (node) {
		struct { RT_TASK *task; void *smsg, *rmsg; long ssize, rsize; } arg = { task, smsg, rmsg, ssize, rsize };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPCX, 0), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, UINT, UINT, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpcx(task, smsg, rmsg, ssize, rsize);
} 

static inline RT_TASK *RT_rpcx_if(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize)
{
	if (node) {
		struct { RT_TASK *task; void *smsg, *rmsg; long ssize, rsize; } arg = { task, smsg, rmsg, ssize, rsize };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPCX_IF, 0), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, UINT, UINT, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpcx_if(task, smsg, rmsg, ssize, rsize);
} 

static inline RT_TASK *RT_rpcx_until(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; void *smsg, *rmsg; long ssize, rsize; RTIME time; } arg = { task, smsg, rmsg, ssize, rsize, time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPCX_UNTIL, 6), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 0, PARTYPES6(VADR, UINT, UINT, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpcx_until(task, smsg, rmsg, ssize, rsize, nano2count(time));
} 

static inline RT_TASK *RT_rpcx_timed(unsigned long node, int port, RT_TASK *task, void *smsg, void *rmsg, int ssize, int rsize, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; void *smsg, *rmsg; long ssize, rsize; RTIME delay; } arg = { task, smsg, rmsg, ssize, rsize, delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, RPCX_TIMED, 6), UR1(2, 4) | UW1(3, 5), &arg, SIZARG, 0, PARTYPES6(VADR, UINT, UINT, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_rpcx_timed(task, smsg, rmsg, ssize, rsize, nano2count(delay));
} 

static inline RT_TASK *RT_sendx(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; } arg = { task, msg, size };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SENDX, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_sendx(task, msg, size);
} 

static inline RT_TASK *RT_sendx_if(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; } arg = { task, msg, size };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SENDX_IF, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
//		return (RT_TASK *)rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).v[LOW];
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_sendx_if(task, msg, size);
} 

static inline RT_TASK *RT_sendx_until(unsigned long node, int port, RT_TASK *task, void *msg, int size, RTIME time)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; RTIME time; } arg = { task, msg, size, time };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SENDX_UNTIL, 4), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_sendx_until(task, msg, size, nano2count(time));
} 

static inline RT_TASK *RT_sendx_timed(unsigned long node, int port, RT_TASK *task, void *msg, int size, RTIME delay)
{
	if (node) {
		struct { RT_TASK *task; void *msg; long size; RTIME delay; } arg = { task, msg, size, delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, SENDX_TIMED, 4), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, UINT, RTIM) };
		return (RT_TASK *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return rt_sendx_timed(task, msg, size, nano2count(delay));
} 

static inline RT_TASK *RT_returnx(unsigned long node, int port, RT_TASK *task, void *msg, int size)
{
	(void)port;
        if (!task || !node) {
		return rt_returnx(task, msg, size);
	} 
	return rt_returnx(rt_find_asgn_stub(OWNER(node, task), 1), msg, size) ? task : 0;
} 

static inline RT_TASK *RT_evdrpx(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	(void)port;
        if (!task || !node) {
		return rt_evdrpx(task, msg, size, len);
	} 
	return rt_evdrpx(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
} 

static inline RT_TASK *RT_receivex(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	(void)port;
        if (!task || !node) {
		return rt_receivex(task, msg, size, len);
	} 
	return rt_receivex(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
} 

static inline RT_TASK *RT_receivex_if(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len)
{
	(void)port;
        if (!task || !node) {
		return rt_receivex_if(task, msg, size, len);
	} 
	return rt_receivex_if(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len) ? task : 0;
} 

static inline RT_TASK *RT_receivex_until(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len, RTIME time)
{
	(void)port;
        if (!task || !node) {
		return rt_receivex_until(task, msg, size, len, nano2count(time));
	} 
	return rt_receivex_until(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len, nano2count(time)) ? task : 0;
} 

static inline RT_TASK *RT_receivex_timed(unsigned long node, int port, RT_TASK *task, void *msg, int size, long *len, RTIME delay)
{
	(void)port;
        if (!task || !node) {
		return rt_receivex_timed(task, msg, size, len, nano2count(delay));
	} 
	return rt_receivex_timed(rt_find_asgn_stub(OWNER(node, task), 1), msg, size, len, nano2count(delay)) ? task : 0;
} 

#endif /* CONFIG_RTAI_MSG */

#if CONFIG_RTAI_MBX

static inline MBX *RT_typed_named_mbx_init(unsigned long node, int port, const char *mbx_name, int size, int qtype)
{
	if (node) {
		struct { unsigned long mbx_name; long size; long qype; } arg = { nam2num(mbx_name), size, qtype };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, NAMED_MBX_INIT, 0), 0, &arg, SIZARG, 0, PARTYPES3(UINT, UINT, UINT) };
		return (MBX *)RTAI_LXRT(port, rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args));
	}
	return (MBX *)rt_typed_named_mbx_init(mbx_name, size, qtype);
}

static inline int RT_named_mbx_delete(unsigned long node, int port, MBX *mbx)
{
	if (node) {
		struct { MBX *mbx; } arg = { mbx };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, NAMED_MBX_DELETE, 0), 0, &arg, SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_named_mbx_delete(mbx);
}

static inline int RT_mbx_send(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_SEND, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_send(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_wp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_SEND_WP, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_send_wp(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_if(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_SEND_IF, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_send_if(mbx, msg, msg_size);
} 

static inline int RT_mbx_ovrwr_send(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_OVRWR_SEND, 0), UR1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_ovrwr_send(mbx, msg, msg_size);
} 

static inline int RT_mbx_send_until(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME time)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME time; long space; } arg = { mbx, msg, msg_size, time, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_SEND_UNTIL, 4), UR1(2, 3), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, SINT, RTIM, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_send_until(mbx, msg, msg_size, nano2count(time));
} 

static inline int RT_mbx_send_timed(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME delay)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME delay; long space; } arg = { mbx, msg, msg_size, delay, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_SEND_TIMED, 4), UR1(2, 3), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, SINT, RTIM, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_send_until(mbx, msg, msg_size, nano2count(delay));
} 

static inline int RT_mbx_evdrp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_EVDRP, 0), UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_evdrp(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE, 0), UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_receive(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_wp(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_WP, 0), UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_receive_wp(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_if(unsigned long node, int port, MBX *mbx, void *msg, int msg_size)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; long space; } arg = { mbx, msg, msg_size, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_IF, 0), UW1(2, 3), &arg, SIZARG, 0, PARTYPES4(VADR, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	} 
	return rt_mbx_receive_if(mbx, msg, msg_size);
} 

static inline int RT_mbx_receive_until(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME time)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME time; long space; } arg = { mbx, msg, msg_size, time, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_UNTIL, 4), UW1(2, 3), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, SINT, RTIM, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_mbx_receive_until(mbx, msg, msg_size, nano2count(time));
} 

static inline int RT_mbx_receive_timed(unsigned long node, int port, MBX *mbx, void *msg, int msg_size, RTIME delay)
{
	if (node) {
		struct { MBX *mbx; void *msg; long msg_size; RTIME delay; long space; } arg = { mbx, msg, msg_size, delay, 1 };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, NET_RPC_EXT, MBX_RECEIVE_TIMED, 4), UW1(2, 3), &arg, SIZARG, 0, PARTYPES5(VADR, UINT, SINT, RTIM, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_mbx_receive_timed(mbx, msg, msg_size, nano2count(delay));
} 

#include <stddef.h>

static inline int rt_get_net_rpc_ret(MBX *mbx, unsigned long long *retval, void *msg1, int *msglen1, void *msg2, int *msglen2, RTIME timeout, int type)
{
	struct reply_t { long long wsize, w2size, myport; unsigned long long retval; char msg[1], msg1[1]; };
	struct reply_t reply;
	int ret;

	switch (type) {
		case MBX_RECEIVE:
			ret = rt_mbx_receive(mbx, &reply, offsetof(struct reply_t, msg));
			break;
		case MBX_RECEIVE_WP:
			ret = rt_mbx_receive_wp(mbx, &reply, offsetof(struct reply_t, msg));
			break;
		case MBX_RECEIVE_IF:
			ret = rt_mbx_receive_if(mbx, &reply, offsetof(struct reply_t, msg));
			break;
		case MBX_RECEIVE_UNTIL:
			ret = rt_mbx_receive_until(mbx, &reply, offsetof(struct reply_t, msg), timeout);
			break;
		case MBX_RECEIVE_TIMED:
			ret = rt_mbx_receive_timed(mbx, &reply, offsetof(struct reply_t, msg), timeout);
		default:
			ret = -1;
	}
	if (!ret) {
		*retval = (unsigned long)reply.retval;
		if (reply.wsize) {
			char msg[reply.wsize];
			rt_mbx_receive(mbx, msg, reply.wsize);
			if (*msglen1 > reply.wsize) {
				*msglen1 = reply.wsize;
			}
			memcpy(msg1, msg, *msglen1);
		} else {
			*msglen1 = 0;
		}
		if (reply.w2size) {
			char msg[reply.w2size];
			rt_mbx_receive(mbx, msg, reply.w2size);
			if (*msglen2 > reply.w2size) {
				*msglen2 = reply.w2size;
			}
			memcpy(msg2, msg, *msglen2);
		} else {
			*msglen2 = 0;
		}
		return 0;
	}
	return ret;
}

#endif /* CONFIG_RTAI_MBX */

#endif /* __KERNEL__ */

/*
 * A set of compatibility defines for APIs that can be interpreted in various
 * ways but do the same the same things always.
 */

#define RT_isrpcx(task)  RT_isrpc(task)

#define RT_waiting_return            rt_waiting_return

#define RT_sync_net_rpc              rt_sync_net_rpc

#define RT_request_port              rt_request_port

#define RT_request_port_id           rt_request_port_id

#define RT_request_port_mbx          rt_request_port_mbx

#define RT_request_port_id_mbx       rt_request_port_id_mbx

#define RT_request_soft_port         rt_request_soft_port

#define RT_request_soft_port_id      rt_request_soft_port_id

#define RT_request_soft_port_mbx     rt_request_soft_port_mbx

#define RT_request_soft_port_id_mbx  rt_request_soft_port_id_mbx

#define RT_request_hard_port         rt_request_hard_port

#define RT_request_hard_port_id      rt_request_hard_port_id

#define RT_request_hard_port_mbx     rt_request_hard_port_mbx

#define RT_request_hard_port_id_mbx  rt_request_hard_port_id_mbx

#define RT_release_port              rt_release_port

#define rt_request_port              rt_request_soft_port 

#define rt_request_port_id           rt_request_soft_port_id

#define rt_request_port_mbx          rt_request_soft_port_mbx

#define rt_request_port_id_mbx       rt_request_soft_port_id_mbx

/*
 * End of compatibility defines.
 */

#define rt_request_soft_port(node) \
	rt_send_req_rel_port(node, 0, 0, 0, 0)

#define rt_request_soft_port_id(node, id) \
	rt_send_req_rel_port(node, 0, id, 0, 0)

#define rt_request_soft_port_mbx(node, mbx) \
	rt_send_req_rel_port(node, 0, 0, mbx, 0)

#define rt_request_soft_port_id_mbx(node, id, mbx) \
	rt_send_req_rel_port(node, 0, id, mbx, 0)

#define rt_request_hard_port(node) \
	rt_send_req_rel_port(node, 0, 0, 0, 1)

#define rt_request_hard_port_id(node, id) \
	rt_send_req_rel_port(node, 0, id, 0, 1)

#define rt_request_hard_port_mbx(node, mbx) \
	rt_send_req_rel_port(node, 0, 0, mbx, 1)

#define rt_request_hard_port_id_mbx(node, id, mbx) \
	rt_send_req_rel_port(node, 0, id, mbx, 1)

#define rt_release_port(node, port) \
	rt_send_req_rel_port(node, port, 0, 0, 0) 

#endif /*_RTAI_NETRPC_H */
