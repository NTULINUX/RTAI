/**
 * @ingroup lxrt
 * @file
 *
 * LXRT main header.
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * @note Copyright &copy; 2019 Alec Ari <neotheuser@ymail.com>
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

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 *
 * ACKNOWLEDGMENTS:
 * Pierre Cloutier (pcloutier@poseidoncontrols.com) has suggested the 6 
 * characters names and fixed many inconsistencies within this file.
 */

/**
 * @defgroup lxrt LXRT module.
 *
 * LXRT services (soft-hard real time in user space)
 *
 * LXRT is a module that allows you to use all the services made available by
 * RTAI and its schedulers in user space, both for soft and hard real time. At
 * the moment it is a feature youll find nowhere but with RTAI. For an
 * explanation of how it works see
 * @ref lxrt_faq "Pierre Cloutiers LXRT-INFORMED FAQs", and the explanation of
 * @ref whatis_lxrt "the implementation of hard real time in user space"
 * (contributed by: Pierre Cloutier, Paolo Mantegazza, Steve Papacharalambous).
 *
 * LXRT-INFORMED should be the production version of LXRT, the latter being the
 * development version. So it can happen that LXRT-INFORMED could be lagging
 * slightly behind LXRT.  If you need to hurry to the services not yet ported to
 * LXRT-INFORMED do it without pain. Even if you are likely to miss some useful
 * services found only in LXRT-INFORMED, we release only when a feature is
 * relatively stable.
 *
 * From what said above there should be no need for anything specific as all the
 * functions you can use in user space have been already documented in this
 * manual.   There are however a few exceptions that need to be explained.
 *
 * Note also that, as already done for the shared memory services in user space,
 * the function calls for Linux processes are inlined in the file
 * rtai_lxrt.h. This approach has been preferred to a library since it is
 * simpler, more effective, the calls are short and simple so that, even if it
 * is likely that there can be more than just a few per process, they could
 * never be charged of making codes too bigger.   Also common to shared memory
 * is the use of unsigned int to identify LXRT objects.   If you want to use
 * string identifiers the same support functions, i.e. nam2num() and
 * num2nam(), can be used.
 *
 *@{*/

#ifndef _RTAI_LXRT_H
#define _RTAI_LXRT_H

#include <rtai_sched.h>
#include <rtai_nam2num.h>

// scheduler
#define YIELD				 0
#define SUSPEND				 1
#define RESUME				 2
#define MAKE_PERIODIC 			 3
#define WAIT_PERIOD	 		 4
#define SLEEP		 		 5
#define SLEEP_UNTIL	 		 6
#define START_TIMER	 		 7
#define STOP_TIMER	 		 8
#define GET_TIME	 		 9
#define COUNT2NANO			10
#define NANO2COUNT			11
#define BUSY_SLEEP			12
#define SET_PERIODIC_MODE		13
#define SET_ONESHOT_MODE		14
#define SIGNAL_HANDLER	 		15
#define TASK_USE_FPU			16
#define GET_TASK_INFO			17  // was LINUX_USE_FPU
#define HARD_TIMER_COUNT		18
#define GET_TIME_NS			19
#define GET_CPU_TIME_NS			20
#define SET_RUNNABLE_ON_CPUS		21 
#define SET_RUNNABLE_ON_CPUID		22	 
#define GET_TIMER_CPU			23	 
#define START_RT_APIC_TIMERS		24
#define HARD_TIMER_COUNT_CPUID		25
#define COUNT2NANO_CPUID		26
#define NANO2COUNT_CPUID		27
#define GET_TIME_CPUID			28
#define GET_TIME_NS_CPUID       	29
#define MAKE_PERIODIC_NS 		30
#define SET_SCHED_POLICY 		31
#define SET_RESUME_END 			32
#define SPV_RMS 			33
#define WAKEUP_SLEEPING			34
#define CHANGE_TASK_PRIO 		35
#define SET_RESUME_TIME 		36
#define SET_PERIOD        		37
#define HARD_TIMER_RUNNING              38

// semaphores
#define TYPED_SEM_INIT 			39
#define SEM_DELETE			40
#define NAMED_SEM_INIT 			41
#define NAMED_SEM_DELETE		42
#define SEM_SIGNAL			43
#define SEM_WAIT			44
#define SEM_WAIT_IF			45
#define SEM_WAIT_UNTIL			46
#define SEM_WAIT_TIMED			47
#define SEM_BROADCAST       		48
#define SEM_WAIT_BARRIER 		49
#define SEM_COUNT			50
#define COND_WAIT			51
#define COND_WAIT_UNTIL			52
#define COND_WAIT_TIMED			53
#define RWL_INIT 			54
#define RWL_DELETE			55
#define NAMED_RWL_INIT 			56
#define NAMED_RWL_DELETE		57
#define RWL_RDLOCK 			58
#define RWL_RDLOCK_IF 			59
#define RWL_RDLOCK_UNTIL 		60
#define RWL_RDLOCK_TIMED 		61
#define RWL_WRLOCK 			62	
#define RWL_WRLOCK_IF	 		63
#define RWL_WRLOCK_UNTIL		64
#define RWL_WRLOCK_TIMED		65
#define RWL_UNLOCK 			66
#define SPL_INIT 			67
#define SPL_DELETE			68
#define NAMED_SPL_INIT 			69
#define NAMED_SPL_DELETE		70
#define SPL_LOCK 			71	
#define SPL_LOCK_IF	 		72
#define SPL_LOCK_TIMED			73
#define SPL_UNLOCK 			74

// mail boxes
#define TYPED_MBX_INIT 			75
#define MBX_DELETE			76
#define NAMED_MBX_INIT 			77
#define NAMED_MBX_DELETE		78
#define MBX_SEND			79
#define MBX_SEND_WP 			80
#define MBX_SEND_IF 			81
#define MBX_SEND_UNTIL			82
#define MBX_SEND_TIMED			83
#define MBX_RECEIVE 			84
#define MBX_RECEIVE_WP 			85
#define MBX_RECEIVE_IF 			86
#define MBX_RECEIVE_UNTIL		87
#define MBX_RECEIVE_TIMED		88
#define MBX_EVDRP			89
#define MBX_OVRWR_SEND                  90

// short intertask messages
#define SENDMSG				91
#define SEND_IF				92
#define SEND_UNTIL			93
#define SEND_TIMED			94
#define RECEIVEMSG			95
#define RECEIVE_IF			96
#define RECEIVE_UNTIL			97
#define RECEIVE_TIMED			98
#define RPCMSG				99
#define RPC_IF			       100
#define RPC_UNTIL		       101
#define RPC_TIMED		       102
#define EVDRP			       103
#define ISRPC			       104
#define RETURNMSG		       105

// extended intertask messages
#define RPCX			       106
#define RPCX_IF			       107
#define RPCX_UNTIL		       108
#define RPCX_TIMED		       109
#define SENDX			       110
#define SENDX_IF		       111
#define SENDX_UNTIL		       112
#define SENDX_TIMED		       113
#define RETURNX			       114
#define RECEIVEX		       115
#define RECEIVEX_IF		       116
#define RECEIVEX_UNTIL		       117
#define RECEIVEX_TIMED		       118
#define EVDRPX			       119

// proxies
#define PROXY_ATTACH                   120
#define PROXY_DETACH     	       121
#define PROXY_TRIGGER                  122


// synchronous user space specific intertask messages and related proxies
#define RT_SEND                        123
#define RT_RECEIVE                     124
#define RT_CRECEIVE          	       125
#define RT_REPLY                       126
#define RT_PROXY_ATTACH                127
#define RT_PROXY_DETACH                128
#define RT_TRIGGER                     129
#define RT_NAME_ATTACH                 130
#define RT_NAME_DETACH                 131
#define RT_NAME_LOCATE                 132

// pqueue
#define MQ_OPEN         	       133
#define MQ_RECEIVE      	       134
#define MQ_SEND         	       135
#define MQ_CLOSE        	       136
#define MQ_GETATTR     		       137
#define MQ_SETATTR      	       138
#define MQ_NOTIFY       	       139
#define MQ_UNLINK       	       140
#define MQ_TIMEDRECEIVE 	       141
#define MQ_TIMEDSEND    	       142

// named tasks init/delete
#define NAMED_TASK_INIT 	       143
#define NAMED_TASK_INIT_CPUID 	       144
#define NAMED_TASK_DELETE	       145

// registry
#define GET_ADR         	       146
#define GET_NAME         	       147

// a semaphore extension
#define COND_SIGNAL		       148

// new shm
#define SHM_ALLOC                      149
#define SHM_FREE                       150
#define SHM_SIZE                       151
#define HEAP_SET                       152
#define HEAP_ALLOC                     153
#define HEAP_FREE                      154
#define HEAP_NAMED_ALLOC               155
#define HEAP_NAMED_FREE                156
#define MALLOC                         157
#define FREE                           158
#define NAMED_MALLOC                   159
#define NAMED_FREE                     160

#define SUSPEND_IF		       161
#define SUSPEND_UNTIL	 	       162
#define SUSPEND_TIMED		       163
#define IRQ_WAIT		       164
#define IRQ_WAIT_IF		       165
#define IRQ_WAIT_UNTIL		       166
#define IRQ_WAIT_TIMED		       167
#define IRQ_SIGNAL		       168
#define REQUEST_IRQ_TASK	       169
#define RELEASE_IRQ_TASK	       170
#define SCHED_LOCK		       171
#define SCHED_UNLOCK		       172
#define PEND_LINUX_IRQ		       173
#define SET_LINUX_SYSCALL_MODE	       174
#define REQUEST_RTC                    175
#define RELEASE_RTC                    176
#define RT_GETTID                      177
#define GET_REAL_TIME		       178
#define GET_REAL_TIME_NS	       179

#define MQ_REG_USP_NOTIFIER	       180

#define RT_SIGNAL_HELPER   	       181
#define RT_SIGNAL_WAITSIG  	       182
#define RT_SIGNAL_REQUEST  	       183
#define RT_SIGNAL_RELEASE  	       184
#define RT_SIGNAL_ENABLE	       185
#define RT_SIGNAL_DISABLE	       186
#define RT_SIGNAL_TRIGGER	       187

#define SEM_RT_POLL 		       188

#define MAX_LXRT_FUN		       189

// not recovered yet 
// Qblk's 
#define RT_INITTICKQUEUE		69
#define RT_RELEASETICKQUEUE     	70
#define RT_QDYNALLOC            	71
#define RT_QDYNFREE             	72
#define RT_QDYNINIT             	73
#define RT_QBLKWAIT			74
#define RT_QBLKREPEAT			75
#define RT_QBLKSOON			76
#define RT_QBLKDEQUEUE			77
#define RT_QBLKCANCEL			78
#define RT_QSYNC			79
#define RT_QRECEIVE			80
#define RT_QLOOP			81
#define RT_QSTEP			82
#define RT_QBLKBEFORE			83
#define RT_QBLKAFTER			84
#define RT_QBLKUNHOOK			85
#define RT_QBLKRELEASE			86
#define RT_QBLKCOMPLETE			87
#define RT_QHOOKFLUSH			88
#define RT_QBLKATHEAD			89
#define RT_QBLKATTAIL			90
#define RT_QHOOKINIT			91
#define RT_QHOOKRELEASE			92
#define RT_QBLKSCHEDULE			93
#define RT_GETTICKQUEUEHOOK		94
// Testing
#define RT_BOOM				95
#define RTAI_MALLOC			96
#define RT_FREE				97
#define RT_MMGR_STATS			98
#define RT_STOMP                	99
// VC
#define RT_VC_ATTACH            	100
#define RT_VC_RELEASE           	101
#define RT_VC_RESERVE          		102
// Linux Signal Support
#define RT_GET_LINUX_SIGNAL		103
#define RT_GET_ERRNO			104
#define RT_SET_LINUX_SIGNAL_HANDLER	105
// end of not recovered yet

#define LXRT_GET_ADR		1000
#define LXRT_GET_NAME   	1001
#define LXRT_TASK_INIT 		1002
#define LXRT_TASK_DELETE 	1003
#define LXRT_SEM_INIT  		1004
#define LXRT_SEM_DELETE		1005
#define LXRT_MBX_INIT 		1006
#define LXRT_MBX_DELETE		1007
#define MAKE_SOFT_RT		1008
#define MAKE_HARD_RT		1009
#define PRINT_TO_SCREEN		1010 // free, can be reused
#define NONROOT_HRT		1011
#define RT_BUDDY		1012
#define HRT_USE_FPU     	1013
#define USP_SIGHDL      	1014
#define GET_USP_FLAGS   	1015
#define SET_USP_FLAGS   	1016
#define GET_USP_FLG_MSK 	1017
#define SET_USP_FLG_MSK 	1018
#define IS_HARD         	1019
#define LINUX_SERVER		1020
#define ALLOC_REGISTER 		1021
#define DELETE_DEREGISTER	1022
#define HARD_SOFT_TOGGLER	1023
#define PRINTK			1024
#define GET_EXECTIME		1025
#define GET_TIMEORIG 		1026
#define LXRT_RWL_INIT		1027
#define LXRT_RWL_DELETE 	1028
#define LXRT_SPL_INIT		1029
#define LXRT_SPL_DELETE 	1030
#define SCHED_LATENCIES   	1031
#define GET_CPU_FREQ		1032

#define FORCE_SOFT 0x80000000

// Keep LXRT call enc/decoding together, so you are sure to act consistently.
// This is the encoding, note " | GT_NR_SYSCALLS" to ensure not a Linux syscall, ...
#define GT_NR_SYSCALLS  (1 << 11)
#define ENCODE_LXRT_REQ(dynx, srq, lsize)  (((dynx) << 24) | ((srq) << 12) | GT_NR_SYSCALLS | (lsize))
// ... and this is the decoding.
#define SRQ(x)   (((x) >> 12) & 0xFFF)
#define LXRT_NARG(x)  ((x) & (GT_NR_SYSCALLS - 1))
#define INDX(x)  (((x) >> 24) & 0xF)

#define LINUX_SYSCALL_GET_MODE       0
#define SYNC_LINUX_SYSCALL           1
#define ASYNC_LINUX_SYSCALL          2
#define LINUX_SYSCALL_CANCELED       3
#define LINUX_SYSCALL_GET_CALLBACK   ((void *)4)

#define NSYSCALL_ARGS     7
#define NSYSCALL_PACARGS  6

struct linux_syscall { long args[NSYSCALL_ARGS], mode; void (*cbfun)(long, long); int id; long pacargs[NSYSCALL_PACARGS]; long retval; };
struct linux_syscalls_list { int in, out, nr, id, mode; void (*cbfun)(long, long); void *serv; struct linux_syscall *syscall; RT_TASK *task; };

#ifdef __KERNEL__

#include <asm/rtai_lxrt.h>

/*
     Encoding of system call argument
            31                                    0  
soft SRQ    .... |||| |||| |||| .... .... .... ....  0 - 4095 max
int  NARG   .... .... .... .... |||| |||| |||| ||||  
arg  INDX   |||| .... .... .... .... .... .... ....
*/

/*
These USP (unsigned long) type fields allow to read and write up to 2 arguments.  
                                               
The high part of the unsigned long encodes writes
W ARG1 BF .... .... ..|| |... .... .... .... ....
W ARG1 SZ .... ...| ||.. .... .... .... .... ....
W ARG2 BF .... |||. .... .... .... .... .... ....
W ARG2 SZ .||| .... .... .... .... .... .... ....

The low part of the unsigned long encodes writes
R ARG1 BF .... .... .... .... .... .... ..|| |...
R ARG1 SZ .... .... .... .... .... ...| ||.. ....
R ARG2 BF .... .... .... .... .... |||. .... ....
R ARG2 SZ .... .... .... .... .||| .... .... ....

The low part of the unsigned long encodes also
RT Switch .... .... .... .... .... .... .... ...|

If SZ is zero sizeof(int) is copied by default, if LL bit is set sizeof(long long) is copied.
*/

// These are for setting appropriate bits in any function entry structure, OR
// them in fun entry type to obtain the desired encoding

// for writes
#define UW1(bf, sz)  ((((bf) & 0x7) << 19) | (((sz) & 0x7) << 22))
#define UW2(bf, sz)  ((((bf) & 0x7) << 25) | (((sz) & 0x7) << 28))

// for reads
#define UR1(bf, sz)  ((((bf) & 0x7) << 3) | (((sz) & 0x7) <<  6))
#define UR2(bf, sz)  ((((bf) & 0x7) << 9) | (((sz) & 0x7) << 12))

#define	NEED_TO_RW(x)	((x) & 0xFFFFFFFE)

#define NEED_TO_W(x)	((x) & (0x3F << 19))
#define NEED_TO_W2ND(x)	((x) & (0x3F << 25))

#define NEED_TO_R(x)	((x) & (0x3F <<  3))
#define NEED_TO_R2ND(x)	((x) & (0x3F <<  9))

#define USP_WBF1(x)   	(((x) >> 19) & 0x7)
#define USP_WSZ1(x)    	(((x) >> 22) & 0x7)
#define USP_WBF2(x)    	(((x) >> 25) & 0x7)
#define USP_WSZ2(x)    	(((x) >> 28) & 0x7)

#define USP_RBF1(x)  	(((x) >>  3) & 0x7)
#define USP_RSZ1(x)    	(((x) >>  6) & 0x7)
#define USP_RBF2(x)    	(((x) >>  9) & 0x7)
#define USP_RSZ2(x)    	(((x) >> 12) & 0x7)

struct rt_fun_entry {
    unsigned long type;
    void *fun;
};

struct rt_native_fun_entry {
    struct rt_fun_entry fun;
    int index;
};

extern struct rt_fun_entry rt_fun_lxrt[];

void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

int set_rt_fun_entries(struct rt_native_fun_entry *entry);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*+++++++++++ INLINES FOR PIERRE's PROXIES AND INTERTASK MESSAGES ++++++++++++*/

#include <linux/types.h>
RT_TASK *rt_find_task_by_pid(pid_t);

static inline struct rt_task_struct *pid2rttask(pid_t pid)
{
	return rt_find_task_by_pid(pid);
}

static inline long rttask2pid(struct rt_task_struct *task)
{
        return task->tid;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int set_rtai_callback(void (*fun)(void));

void remove_rtai_callback(void (*fun)(void));

RT_TASK *rt_lxrt_whoami(void);

void exec_func(void (*func)(void *data, int evn),
	       void *data,
	       int evn);

int  set_rt_fun_ext_index(struct rt_fun_entry *fun,
			  int idx);

void reset_rt_fun_ext_index(struct rt_fun_entry *fun,
			    int idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else /* !__KERNEL__ */

#include <sys/types.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asm/rtai_lxrt.h>

struct apic_timer_setup_data;

#define rt_grow_and_lock_stack(incr) \
	do { \
		char buf[incr]; \
		memset(buf, 0, incr); \
		mlockall(MCL_CURRENT | MCL_FUTURE); \
	} while (0)

#define BIDX   0 // rt_fun_ext[0]
#define SIZARG sizeof(arg)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Get an object address by its name.
 *
 * rt_get_adr returns the address associated to @a name.
 *
 * @return the address associated to @a name on success, 0 on failure
 */
RTAI_PROTO(void *, rt_get_adr, (unsigned long name))
{
	struct { unsigned long name; } arg = { name };
	return rtai_lxrt(BIDX, SIZARG, LXRT_GET_ADR, &arg).v[LOW];
} 

/**
 * Get an object name by its address.
 *
 * rt_get_name returns the name pointed by the address @a adr.
 *
 * @return the identifier pointed by the address @a adr on success, 0 on
 * failure.
 */
RTAI_PROTO(unsigned long, rt_get_name, (void *adr))
{
	struct { void *adr; } arg = { adr };
	return rtai_lxrt(BIDX, SIZARG, LXRT_GET_NAME, &arg).i[LOW];
}

#include <signal.h>

#ifdef CONFIG_RTAI_HARD_SOFT_TOGGLER
#ifndef __SUPPORT_HARD_SOFT_TOGGLER__
#define __SUPPORT_HARD_SOFT_TOGGLER__

static void hard_soft_toggler(int sig) 
{ 
	if (sig == SIGUSR1) {
		struct { RT_TASK *task; } arg = { NULL };
		rtai_lxrt(BIDX, SIZARG, HARD_SOFT_TOGGLER, &arg);
	}
}

#endif

#define SET_SIGNAL_TOGGLER() do { signal(SIGUSR1, hard_soft_toggler); } while(0)

#else

#define SET_SIGNAL_TOGGLER() do { } while(0)

#endif

RTAI_PROTO(RT_TASK *, rt_task_init_schmod, (unsigned long name, int priority, int stack_size, int max_msg_size, int policy, int cpus_allowed))
{
        struct sched_param mysched;
        struct { unsigned long name; long priority, stack_size, max_msg_size, cpus_allowed; } arg = { name ? name : rt_get_name(NULL), priority, stack_size, max_msg_size, cpus_allowed };

	SET_SIGNAL_TOGGLER();
        if (policy == SCHED_OTHER) {
        	mysched.sched_priority = 0;
	} else if ((mysched.sched_priority = sched_get_priority_max(policy) - priority) < 1) {
		mysched.sched_priority = 1;
	}
        if (sched_setscheduler(0, policy, &mysched) < 0) {
                return 0;
        }
	rtai_iopl();
	mlockall(MCL_CURRENT | MCL_FUTURE);

	return (RT_TASK *)rtai_lxrt(BIDX, SIZARG, LXRT_TASK_INIT, &arg).v[LOW];
}

#define RT_THREAD_STACK_MIN  16*1024

#include <pthread.h>

RTAI_PROTO(long, rt_thread_create, (void *fun, void *args, int stack_size))
{
	long thread;
	pthread_attr_t attr;

        pthread_attr_init(&attr);
	if (!pthread_attr_setstacksize(&attr, stack_size > RT_THREAD_STACK_MIN ? stack_size : RT_THREAD_STACK_MIN)) {
		struct { unsigned long hs; } arg = { 0 };
		if ((arg.hs = rtai_lxrt(BIDX, SIZARG, IS_HARD, &arg).i[LOW])) {
			rtai_lxrt(BIDX, SIZARG, MAKE_SOFT_RT, &arg);
		}
		if (pthread_create((pthread_t *)&thread, &attr, (void *(*)(void *))fun, args)) {
			thread = 0;
		}
		if (arg.hs) {
			rtai_lxrt(BIDX, SIZARG, MAKE_HARD_RT, &arg);
		}
	} else {
		thread = 0;
	}
	return thread;
}

RTAI_PROTO(int, rt_thread_join, (long thread))
{
	return pthread_join((pthread_t)thread, NULL);
}

#ifndef __SUPPORT_LINUX_SERVER__
#define __SUPPORT_LINUX_SERVER__

#include <unistd.h>
#include <sys/mman.h>

static void linux_syscall_server_fun(struct linux_syscalls_list *list)
{
	struct linux_syscalls_list syscalls;

	syscalls = *list;
	syscalls.serv = &syscalls;
	if ((syscalls.serv = rtai_lxrt(BIDX, sizeof(struct linux_syscalls_list), LINUX_SERVER, &syscalls).v[LOW])) {
		long *args;
		struct linux_syscall *todo;
		struct linux_syscall calldata[syscalls.nr];
		syscalls.syscall = calldata;
		memset(calldata, 0, sizeof(calldata));
                mlockall(MCL_CURRENT | MCL_FUTURE);
		list->serv = &syscalls;
		rtai_lxrt(BIDX, sizeof(RT_TASK *), RESUME, &syscalls.task);
		while (abs(rtai_lxrt(BIDX, sizeof(RT_TASK *), SUSPEND, &syscalls.serv).i[LOW]) < RTE_LOWERR) {
			if (syscalls.syscall[syscalls.out].mode != LINUX_SYSCALL_CANCELED) {
				todo = &syscalls.syscall[syscalls.out];
				args = todo->args;
				todo->retval = syscall(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
				todo->id = -todo->id;
				if (todo->mode == SYNC_LINUX_SYSCALL) {
					rtai_lxrt(BIDX, sizeof(RT_TASK *), RESUME, &syscalls.task);
				} else if (syscalls.cbfun) {
					todo->cbfun(args[0], todo->retval);
				}
			}
			if (++syscalls.out >= syscalls.nr) {
				syscalls.out = 0;
			}
		}
        }
	rtai_lxrt(BIDX, sizeof(RT_TASK *), LXRT_TASK_DELETE, &syscalls.serv);
}

#endif /* __SUPPORT_LINUX_SERVER__ */

RTAI_PROTO(int, rt_set_linux_syscall_mode, (int mode, void (*cbfun)(long, long)))
{
	struct { long mode; void (*cbfun)(long, long); } arg = { mode, cbfun };
	return rtai_lxrt(BIDX, SIZARG, SET_LINUX_SYSCALL_MODE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_linux_syscall_mode, (struct linux_syscalls_list *syscalls, int mode))
{
	int retval; 
	if (syscalls == NULL) {
		return EINVAL;
	}
	retval = syscalls->mode; 
	if (mode == SYNC_LINUX_SYSCALL || mode == ASYNC_LINUX_SYSCALL) {
		syscalls->mode = mode; 
	}
	return retval;
}

RTAI_PROTO(void *, rt_linux_syscall_cbfun, (struct linux_syscalls_list *syscalls, void (*cbfun)(long, long)))
{
	void *retval; 
	if (syscalls == NULL) {
		return (void *)EINVAL;
	}
	retval = (void *)((unsigned long)syscalls->cbfun);
	if ((unsigned long)cbfun > (unsigned long)LINUX_SYSCALL_GET_CALLBACK) {
		syscalls->cbfun = cbfun;
	}
	return retval;
}

RTAI_PROTO(int, rt_linux_syscall_status, (struct linux_syscalls_list *syscalls, int id, int *retval))
{
	int slot, slotid;
	if (syscalls == NULL || id < 0) {
		return EINVAL;
	}
	if (id != abs(slotid = syscalls->syscall[slot = id%syscalls->nr].id)) {
		return ENOENT;
	}
	if (syscalls->syscall[slot].mode == LINUX_SYSCALL_CANCELED) {
		return ECANCELED;
	}
	if (slotid > 0) {
		return EINPROGRESS;
	}
	if (retval) {
		*retval =  syscalls->syscall[slot].retval;
	}
	return 0;
}

RTAI_PROTO(int, rt_linux_syscall_cancel, (struct linux_syscalls_list *syscalls, int id))
{
	int slot, slotid;
	if (syscalls == NULL || id < 0) {
		return EINVAL;
	}
	if (id != abs(slotid = syscalls->syscall[slot = id%syscalls->nr].id)) {
		return ENOENT;
	}
	if (slotid < 0) {
		return slotid;
	}
	syscalls->syscall[slot].mode = LINUX_SYSCALL_CANCELED;
	return 0;
}

RTAI_PROTO(void *, rt_create_linux_syscall_server, (RT_TASK *task, int mode, void (*cbfun)(long, long), int nr_bufd_async_calls))
{
	if ((task || (task = (RT_TASK *)rtai_lxrt(BIDX, sizeof(RT_TASK *), RT_BUDDY, &task).v[LOW])) && nr_bufd_async_calls > 0) {
		struct linux_syscalls_list syscalls;
		memset(&syscalls, 0, sizeof(syscalls));
		syscalls.task  = task;
		syscalls.cbfun = cbfun;
		syscalls.nr    = nr_bufd_async_calls + 1;
		syscalls.mode  = mode;
		syscalls.serv  = NULL;
		if (rt_thread_create((void *)linux_syscall_server_fun, &syscalls, RT_THREAD_STACK_MIN + syscalls.nr*sizeof(struct linux_syscall))) {
			rtai_lxrt(BIDX, sizeof(RT_TASK *), SUSPEND, &task);
			return syscalls.serv;
		}
	}
	return NULL;
}

#define rt_sync_async_linux_syscall_server_create(task, mode, cbfun, nr_calls)  rt_create_linux_syscall_server(task, mode, cbfun, nr_calls)

#define rt_linux_syscall_server_create(task)  rt_sync_async_linux_syscall_server_create(task, SYNC_LINUX_SYSCALL, NULL, 1);

RTAI_PROTO(void, rt_destroy_linux_syscall_server, (RT_TASK *task))
{
	struct linux_syscalls_list s;
	s.nr = 0;
	s.task = task;
	rtai_lxrt(BIDX, sizeof(struct linux_syscalls_list), LINUX_SERVER, &s);
}

RTAI_PROTO(RT_TASK *, rt_thread_init, (unsigned long name, int priority, int max_msg_size, int policy, int cpus_allowed))
{
	return rt_task_init_schmod(name, priority, 0, max_msg_size, policy, cpus_allowed);
}

/**
 * Create an RTAI task extension for a Linux process/task in user space.
 * 
 * rt_task_init extends the Linux task structure, making it possible to use
 * RTAI APIs that wants to access RTAI scheduler services.   It needs no task
 * function as none is used, but it does need to setup an RTAI task structure
 * and initialize it appropriately as the provided services are carried out as
 * if the Linux process has become an RTAI task also.   Because of that it 
 * requires less arguments and returns the pointer to the RTAI task extension
 * that is to be used in related calls.
 *
 * @param name is a unique identifier that is possibly used to ease
 * referencing the RTAI task extension of a peer Linux process.
 *
 * @param priority is the priority of the RTAI task extension.
 *
 * @param stack_size, a legacy parameter used nomore; kept for portability 
 * reasons only. (It was just what is implied by such a name and referred to 
 * the stack size used by the buddy in the very first implementation of LXRT).
 *
 * @param max_msg_size is a hint for the size of the most lengthy intertask
 * message that is likely to be exchanged.
 *
 * @a max_msg_size can be zero, in which case a default internal value is
 * used.  Keep an eye on such a default message (256) size. It could be 
 * possible that a larger size is required to suite your needs best. In such 
 * a case either recompile sys.c with the macro MSG_SIZE set appropriately, 
 * or assign a larger size here esplicitly.  Note that the message size is 
 * not critical though. In fact the module reassigns it, dynamically and 
 * appropriately sized, whenever it is needed.  The cost is a real time 
 * allocation of the new buffer.
 * Note also that @a max_msg_size is for a buffer to be used to copy whatever
 * intertask message from user to kernel space, as intertask messages are not 
 * necessarily used immediately.
 *
 * It is important to remark that the returned task pointers cannot be used
 * directly, they are for kernel space data, but just passed as arguments when
 * needed.
 *
 * @return On success a pointer to the task structure initialized in kernel
 * space.
 * @return On failure a NULL value is returned if it was not possible to setup 
 * the RTAI task extension or something using the same name was found.
 */
RTAI_PROTO(RT_TASK *,rt_task_init,(unsigned long name, int priority, int stack_size, int max_msg_size))
{
	(void)stack_size;
	return rt_task_init_schmod(name, priority, 0, max_msg_size, SCHED_FIFO, 0xFF);
}

RTAI_PROTO(void,rt_set_sched_policy,(RT_TASK *task, int policy, int rr_quantum_ns))
{
	struct { RT_TASK *task; long policy; long rr_quantum_ns; } arg = { task, policy, rr_quantum_ns };
	rtai_lxrt(BIDX, SIZARG, SET_SCHED_POLICY, &arg);
}

RTAI_PROTO(int,rt_change_prio,(RT_TASK *task, int priority))
{
	struct { RT_TASK *task; long priority; } arg = { task, priority };
	return rtai_lxrt(BIDX, SIZARG, CHANGE_TASK_PRIO, &arg).i[LOW];
}

/**
 * Return a hard real time Linux process, or pthread to the standard Linux
 * behavior.
 *
 * rt_make_soft_real_time returns to soft Linux POSIX real time a process, from
 * which it is called, that was made hard real time by a call to
 * rt_make_hard_real_time.
 *
 * Only the process itself can use this functions, it is not possible to impose
 * the related transition from another process.
 *
 */
RTAI_PROTO(void, rt_make_soft_real_time, (void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, MAKE_SOFT_RT, &arg);
}

RTAI_PROTO(int, rt_thread_delete,(RT_TASK *task))
{
	struct { RT_TASK *task; } arg = { task };
	rt_make_soft_real_time();
	return rtai_lxrt(BIDX, SIZARG, LXRT_TASK_DELETE, &arg).i[LOW];
}

#define rt_task_delete(task)  rt_thread_delete(task)

RTAI_PROTO(int,rt_task_yield,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, YIELD, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_suspend,(RT_TASK *task))
{
	struct { RT_TASK *task; } arg = { task };
	return rtai_lxrt(BIDX, SIZARG, SUSPEND, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_suspend_if,(RT_TASK *task))
{
	struct { RT_TASK *task; } arg = { task };
	return rtai_lxrt(BIDX, SIZARG, SUSPEND_IF, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_suspend_until,(RT_TASK *task, RTIME time))
{
	struct { RT_TASK *task; RTIME time; } arg = { task, time };
	return rtai_lxrt(BIDX, SIZARG, SUSPEND_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_suspend_timed,(RT_TASK *task, RTIME delay))
{
	struct { RT_TASK *task; RTIME delay; } arg = { task, delay };
	return rtai_lxrt(BIDX, SIZARG, SUSPEND_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_resume,(RT_TASK *task))
{
	struct { RT_TASK *task; } arg = { task };
	return rtai_lxrt(BIDX, SIZARG, RESUME, &arg).i[LOW];
}

RTAI_PROTO(void, rt_sched_lock, (void))
{
	struct { long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, SCHED_LOCK, &arg);
}

RTAI_PROTO(void, rt_sched_unlock, (void))
{
	struct { long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, SCHED_UNLOCK, &arg);
}

RTAI_PROTO(void, rt_pend_linux_irq, (unsigned irq))
{
	struct { unsigned long irq; } arg = { irq };
	rtai_lxrt(BIDX, SIZARG, PEND_LINUX_IRQ, &arg);
}

RTAI_PROTO(int, rt_irq_wait, (unsigned irq))
{
	struct { unsigned long irq; } arg = { irq };
	return rtai_lxrt(BIDX, SIZARG, IRQ_WAIT, &arg).i[LOW];
}

RTAI_PROTO(int, rt_irq_wait_if, (unsigned irq))
{
	struct { unsigned long irq; } arg = { irq };
	return rtai_lxrt(BIDX, SIZARG, IRQ_WAIT_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rt_irq_wait_until, (unsigned irq, RTIME time))
{
	struct { unsigned long irq; RTIME time; } arg = { irq, time };
	return rtai_lxrt(BIDX, SIZARG, IRQ_WAIT_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_irq_wait_timed, (unsigned irq, RTIME delay))
{
	struct { unsigned long irq; RTIME delay; } arg = { irq, delay };
	return rtai_lxrt(BIDX, SIZARG, IRQ_WAIT_TIMED, &arg).i[LOW];
}

RTAI_PROTO(int, rt_irq_signal, (unsigned irq))
{
	struct { unsigned long irq; } arg = { irq };
	return rtai_lxrt(BIDX, SIZARG, IRQ_SIGNAL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_request_irq_task, (unsigned irq, void *handler, int type, int affine2task))
{
	struct { unsigned long irq; void *handler; long type, affine2task; } arg = { irq, handler, type, affine2task };
	return rtai_lxrt(BIDX, SIZARG, REQUEST_IRQ_TASK, &arg).i[LOW];
}


RTAI_PROTO(int, rt_release_irq_task, (unsigned irq))
{
	struct { unsigned long irq; } arg = { irq };
	return rtai_lxrt(BIDX, SIZARG, RELEASE_IRQ_TASK, &arg).i[LOW];
}

RTAI_PROTO(int, rt_task_make_periodic,(RT_TASK *task, RTIME start_time, RTIME period))
{
	struct { RT_TASK *task; RTIME start_time, period; } arg = { task, start_time, period };
	return rtai_lxrt(BIDX, SIZARG, MAKE_PERIODIC, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_make_periodic_relative_ns,(RT_TASK *task, RTIME start_delay, RTIME period))
{
	struct { RT_TASK *task; RTIME start_time, period; } arg = { task, start_delay, period };
	return rtai_lxrt(BIDX, SIZARG, MAKE_PERIODIC_NS, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_wait_period,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, WAIT_PERIOD, &arg).i[LOW];
}

RTAI_PROTO(int,rt_sleep,(RTIME delay))
{
	struct { RTIME delay; } arg = { delay };
	return rtai_lxrt(BIDX, SIZARG, SLEEP, &arg).i[LOW];
}

RTAI_PROTO(int,rt_sleep_until,(RTIME time))
{
	struct { RTIME time; } arg = { time };
	return rtai_lxrt(BIDX, SIZARG, SLEEP_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(int,rt_is_hard_timer_running,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, HARD_TIMER_RUNNING, &arg).i[LOW];
}

RTAI_PROTO(RTIME, start_rt_timer, (int period))
{
	int hs;
	RTIME retval;
	struct { long period; } arg = { 0 };
	if ((hs = rtai_lxrt(BIDX, SIZARG, IS_HARD, &arg).i[LOW])) {
		rtai_lxrt(BIDX, SIZARG, MAKE_SOFT_RT, &arg);
	}
	arg.period = period;
	retval = rtai_lxrt(BIDX, SIZARG, START_TIMER, &arg).rt;
	if (hs) {
		rtai_lxrt(BIDX, SIZARG, MAKE_HARD_RT, &arg);
	}
	return retval;
}

RTAI_PROTO(void, stop_rt_timer, (void))
{
	struct { long hs; } arg = { 0 };
	if ((arg.hs = rtai_lxrt(BIDX, SIZARG, IS_HARD, &arg).i[LOW])) {
		rtai_lxrt(BIDX, SIZARG, MAKE_SOFT_RT, &arg);
	}
	rtai_lxrt(BIDX, SIZARG, STOP_TIMER, &arg);
	if (arg.hs) {
		rtai_lxrt(BIDX, SIZARG, MAKE_HARD_RT, &arg);
	}
}

RTAI_PROTO(void, rt_request_rtc, (int rtc_freq, void *handler))
{
	struct { long rtc_freq; void *handler; } arg = { rtc_freq, handler };
	rtai_lxrt(BIDX, SIZARG, REQUEST_RTC, &arg);
}

RTAI_PROTO(void, rt_release_rtc, (void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, RELEASE_RTC, &arg);
}

RTAI_PROTO(RTIME, rt_get_time, (void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_TIME, &arg).rt;
}

RTAI_PROTO(RTIME, rt_get_real_time, (void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_REAL_TIME, &arg).rt;
}

RTAI_PROTO(RTIME, rt_get_real_time_ns, (void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_REAL_TIME_NS, &arg).rt;
}

RTAI_PROTO(RTIME, count2nano, (RTIME count))
{
	struct { RTIME count; } arg = { count };
	return rtai_lxrt(BIDX, SIZARG, COUNT2NANO, &arg).rt;
}

RTAI_PROTO(RTIME, nano2count, (RTIME nanos))
{
	struct { RTIME nanos; } arg = { nanos };
	return rtai_lxrt(BIDX, SIZARG, NANO2COUNT, &arg).rt;
}

RTAI_PROTO(void, rt_busy_sleep, (int ns))
{
	struct { long ns; } arg = { ns };
	rtai_lxrt(BIDX, SIZARG, BUSY_SLEEP, &arg);
}

RTAI_PROTO(void, rt_set_periodic_mode, (void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, SET_PERIODIC_MODE, &arg);
}

RTAI_PROTO(void, rt_set_oneshot_mode, (void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, SET_ONESHOT_MODE, &arg);
}

RTAI_PROTO(int, rt_task_signal_handler, (RT_TASK *task, void (*handler)(void)))
{
	struct { RT_TASK *task; void (*handler)(void); } arg = { task, handler };
	return rtai_lxrt(BIDX, SIZARG, SIGNAL_HANDLER, &arg).i[LOW];
}

RTAI_PROTO(int,rt_task_use_fpu,(RT_TASK *task, int use_fpu_flag))
{
        struct { RT_TASK *task; long use_fpu_flag; } arg = { task, use_fpu_flag };
        if (rtai_lxrt(BIDX, SIZARG, RT_BUDDY, &arg).v[LOW] != task) {
                return rtai_lxrt(BIDX, SIZARG, TASK_USE_FPU, &arg).i[LOW];
        } else {
// note that it would be enough to do whatever FP op here to have it OK. But
// that is scary if it is done when already in hard real time, and we do not
// want to force users to call this before making it hard.
                rtai_lxrt(BIDX, SIZARG, HRT_USE_FPU, &arg);
                return 0;
        }
}

RTAI_PROTO(int,rt_buddy_task_use_fpu,(RT_TASK *task, int use_fpu_flag))
{
	struct { RT_TASK *task; long use_fpu_flag; } arg = { task, use_fpu_flag };
	return rtai_lxrt(BIDX, SIZARG, TASK_USE_FPU, &arg).i[LOW];
}

/*
RTAI_PROTO(int,rt_linux_use_fpu,(int use_fpu_flag))
{
	struct { long use_fpu_flag; } arg = { use_fpu_flag };
	return rtai_lxrt(BIDX, SIZARG, LINUX_USE_FPU, &arg).i[LOW];
}
*/

RTAI_PROTO(int, rt_task_get_info, (RT_TASK *task, RT_TASK_INFO *task_info))
{
	struct { RT_TASK *task; RT_TASK_INFO *taskinfo; } arg = { task, task_info };
	if (task_info && !rtai_lxrt(BIDX, SIZARG, GET_TASK_INFO, &arg).i[LOW]) {
		return 0;
	}
	return -EINVAL;
}

RTAI_PROTO(int, rt_get_priorities, (RT_TASK *task, int *priority, int *base_priority))
{
	RT_TASK_INFO task_info;
	if (priority && base_priority && !rt_task_get_info(task, &task_info)) {
		*priority      = task_info.priority;
		*base_priority = task_info.base_priority;
		return 0;
	}
	return -EINVAL;
}

RTAI_PROTO(int, rt_hard_timer_tick, (void))
{
	struct { long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, HARD_TIMER_COUNT, &arg).i[LOW];
}

RTAI_PROTO(RTIME,rt_get_time_ns,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_TIME_NS, &arg).rt;
}

RTAI_PROTO(RTIME,rt_get_cpu_time_ns,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_CPU_TIME_NS, &arg).rt;
}

#define rt_named_task_init(task_name, thread, data, stack_size, prio, uses_fpu, signal) \
	rt_task_init(nam2num(task_name), thread, data, stack_size, prio, uses_fpu, signal)

#define rt_named_task_init_cpuid(task_name, thread, data, stack_size, prio, uses_fpu, signal, run_on_cpu) \
	rt_task_init_cpuid(nam2num(task_name), thread, data, stack_size, prio, uses_fpu, signal, run_on_cpu)

RTAI_PROTO(void,rt_set_runnable_on_cpus,(RT_TASK *task, unsigned long cpu_mask))
{
	struct { RT_TASK *task; unsigned long cpu_mask; } arg = { task, cpu_mask };
	rtai_lxrt(BIDX, SIZARG, SET_RUNNABLE_ON_CPUS, &arg);
}

RTAI_PROTO(void,rt_set_runnable_on_cpuid,(RT_TASK *task, unsigned int cpuid))
{
	struct { RT_TASK *task; unsigned long cpuid; } arg = { task, cpuid };
	rtai_lxrt(BIDX, SIZARG, SET_RUNNABLE_ON_CPUID, &arg);
}

RTAI_PROTO(int,rt_get_timer_cpu,(void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_TIMER_CPU, &arg).i[LOW];
}

RTAI_PROTO(void,start_rt_apic_timers,(struct apic_timer_setup_data *setup_mode, unsigned int rcvr_jiffies_cpuid))
{
	struct { struct apic_timer_setup_data *setup_mode; unsigned long rcvr_jiffies_cpuid; } arg = { setup_mode, rcvr_jiffies_cpuid };
	rtai_lxrt(BIDX, SIZARG, START_RT_APIC_TIMERS, &arg);
}

RTAI_PROTO(int, rt_hard_timer_tick_cpuid, (int cpuid))
{
	struct { long cpuid; } arg = { cpuid };
	return rtai_lxrt(BIDX, SIZARG, HARD_TIMER_COUNT_CPUID, &arg).i[LOW];
}

RTAI_PROTO(RTIME,count2nano_cpuid,(RTIME count, unsigned int cpuid))
{
	struct { RTIME count; unsigned long cpuid; } arg = { count, cpuid };
	return rtai_lxrt(BIDX, SIZARG, COUNT2NANO_CPUID, &arg).rt;
}

RTAI_PROTO(RTIME,nano2count_cpuid,(RTIME nanos, unsigned int cpuid))
{
	struct { RTIME nanos; unsigned long cpuid; } arg = { nanos, cpuid };
	return rtai_lxrt(BIDX, SIZARG, NANO2COUNT_CPUID, &arg).rt;
}

RTAI_PROTO(RTIME,rt_get_time_cpuid,(unsigned int cpuid))
{
	struct { unsigned long cpuid; } arg = { cpuid };
	return rtai_lxrt(BIDX, SIZARG, GET_TIME_CPUID, &arg).rt;
}

RTAI_PROTO(RTIME,rt_get_time_ns_cpuid,(unsigned int cpuid))
{
	struct { unsigned long cpuid; } arg = { cpuid };
	return rtai_lxrt(BIDX, SIZARG, GET_TIME_NS_CPUID, &arg).rt;
}

RTAI_PROTO(void,rt_boom,(void))
{
	struct { long dummy; } arg = { 0 };
	rtai_lxrt(BIDX, SIZARG, RT_BOOM, &arg);
}

RTAI_PROTO(void,rt_mmgr_stats,(void))
{
	struct { long dummy; } arg = { 0 };
	rtai_lxrt(BIDX, SIZARG, RT_MMGR_STATS, &arg);
}

RTAI_PROTO(void,rt_stomp,(void) )
{
	struct { long dummy; } arg = { 0 };
	rtai_lxrt(BIDX, SIZARG, RT_STOMP, &arg);
}

RTAI_PROTO(int,rt_get_linux_signal,(RT_TASK *task))
{
    struct { RT_TASK *task; } arg = { task };
    return rtai_lxrt(BIDX, SIZARG, RT_GET_LINUX_SIGNAL, &arg).i[LOW];
}

RTAI_PROTO(int,rt_get_errno,(RT_TASK *task))
{
    struct { RT_TASK *task; } arg = { task };
    return rtai_lxrt(BIDX, SIZARG, RT_GET_ERRNO, &arg).i[LOW];
}

RTAI_PROTO(int,rt_set_linux_signal_handler,(RT_TASK *task, void (*handler)(int sig)))
{
    struct { RT_TASK *task; void (*handler)(int sig); } arg = { task, handler };
    return rtai_lxrt(BIDX, SIZARG, RT_SET_LINUX_SIGNAL_HANDLER, &arg).i[LOW];
}

#define VSNPRINTF_BUF_SIZE 256
RTAI_PROTO(int,rt_printk,(const char *format, ...))
{
	char display[VSNPRINTF_BUF_SIZE];
	struct { const char *display; long nch; } arg = { display, 0 };
	va_list args;

	va_start(args, format);
	arg.nch = vsnprintf(display, VSNPRINTF_BUF_SIZE, format, args);
	va_end(args);
	rtai_lxrt(BIDX, SIZARG, PRINTK, &arg);
	return arg.nch;
}

RTAI_PROTO(int,rtai_print_to_screen,(const char *format, ...))
{
        char display[VSNPRINTF_BUF_SIZE];
        struct { const char *display; long nch; } arg = { display, 0 };
        va_list args;

        va_start(args, format);
        arg.nch = vsnprintf(display, VSNPRINTF_BUF_SIZE, format, args);
        va_end(args);
        rtai_lxrt(BIDX, SIZARG, PRINTK, &arg);
        return arg.nch;
}

RTAI_PROTO(int,rt_usp_signal_handler,(void (*handler)(void)))
{
	struct { void (*handler)(void); } arg = { handler };
	return rtai_lxrt(BIDX, SIZARG, USP_SIGHDL, &arg).i[0];
}

RTAI_PROTO(unsigned long,rt_get_usp_flags,(RT_TASK *rt_task))
{
	struct { RT_TASK *task; } arg = { rt_task };
	return rtai_lxrt(BIDX, SIZARG, GET_USP_FLAGS, &arg).i[LOW];
}

RTAI_PROTO(unsigned long,rt_get_usp_flags_mask,(RT_TASK *rt_task))
{
	struct { RT_TASK *task; } arg = { rt_task };
	return rtai_lxrt(BIDX, SIZARG, GET_USP_FLG_MSK, &arg).i[LOW];
}

RTAI_PROTO(void,rt_set_usp_flags,(RT_TASK *rt_task, unsigned long flags))
{
	struct { RT_TASK *task; unsigned long flags; } arg = { rt_task, flags };
	rtai_lxrt(BIDX, SIZARG, SET_USP_FLAGS, &arg);
}

RTAI_PROTO(void,rt_set_usp_flags_mask,(unsigned long flags_mask))
{
	struct { unsigned long flags_mask; } arg = { flags_mask };
	rtai_lxrt(BIDX, SIZARG, SET_USP_FLG_MSK, &arg);
}

RTAI_PROTO(pid_t, rt_get_linux_tid, (RT_TASK *task))
{
	struct { RT_TASK *task; } arg = { task };
	return rtai_lxrt(BIDX, SIZARG, HARD_SOFT_TOGGLER, &arg).i[LOW];
}

RTAI_PROTO(RT_TASK *,rt_agent,(void))
{
	struct { unsigned long dummy; } arg;
	return (RT_TASK *)rtai_lxrt(BIDX, SIZARG, RT_BUDDY, &arg).v[LOW];
}

#define rt_buddy() rt_agent()

RTAI_PROTO(int, rt_gettid, (void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, RT_GETTID, &arg).i[LOW];
}

/**
 * Give a Linux process, or pthread, hard real time execution capabilities 
 * allowing full kernel preemption.
 *
 * rt_make_hard_real_time makes the soft Linux POSIX real time process, from
 * which it is called, a hard real time LXRT process.   It is important to
 * remark that this function must be used only with soft Linux POSIX processes
 * having their memory locked in memory.   See Linux man pages.
 *
 * Only the process itself can use this functions, it is not possible to impose
 * the related transition from another process.
 *
 * Note that processes made hard real time should avoid making any Linux System
 * call that can lead to a task switch as Linux cannot run anymore processes
 * that are made hard real time.   To interact with Linux you should couple the
 * process that was made hard real time with a Linux buddy server, either
 * standard or POSIX soft real time.   To communicate and synchronize with the
 * buddy you can use the wealth of available RTAI, and its schedulers, services.
 * 
 * After all it is pure nonsense to use a non hard real time Operating System,
 * i.e. Linux, from within hard real time processes.
 */
RTAI_PROTO(void,rt_make_hard_real_time,(void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, MAKE_HARD_RT, &arg);
}

/**
 * Allows a non root user to use the Linux POSIX soft real time process 
 * management and memory lock functions, and allows it to do any input-output
 * operation from user space.
 *
 * Only the process itself can use this functions, it is not possible to impose
 * the related transition from another process.
 */
RTAI_PROTO(void,rt_allow_nonroot_hrt,(void))
{
	struct { unsigned long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, NONROOT_HRT, &arg);
}

RTAI_PROTO(int,rt_is_hard_real_time,(RT_TASK *rt_task))
{
	struct { RT_TASK *task; } arg = { rt_task };
	return rtai_lxrt(BIDX, SIZARG, IS_HARD, &arg).i[LOW];
}

#define rt_is_soft_real_time(rt_task) (!rt_is_hard_real_time((rt_task)))

RTAI_PROTO(void,rt_task_set_resume_end_times,(RTIME resume, RTIME end))
{
	struct { RTIME resume, end; } arg = { resume, end };
	rtai_lxrt(BIDX, SIZARG, SET_RESUME_END, &arg);
}

RTAI_PROTO(int,rt_set_resume_time,(RT_TASK *rt_task, RTIME new_resume_time))
{
	struct { RT_TASK *rt_task; RTIME new_resume_time; } arg = { rt_task, new_resume_time };
	return rtai_lxrt(BIDX, SIZARG, SET_RESUME_TIME, &arg).i[LOW];
}

RTAI_PROTO(int, rt_set_period, (RT_TASK *rt_task, RTIME new_period))
{
	struct { RT_TASK *rt_task; RTIME new_period; } arg = { rt_task, new_period };
	return rtai_lxrt(BIDX, SIZARG, SET_PERIOD, &arg).i[LOW];
}

RTAI_PROTO(void, rt_spv_RMS, (int cpuid))
{
	struct { long cpuid; } arg = { cpuid };
	rtai_lxrt(BIDX, SIZARG, SPV_RMS, &arg);
}

RTAI_PROTO(int, rt_task_masked_unblock, (RT_TASK *task, unsigned long mask))
{
	struct { RT_TASK *task; unsigned long mask; } arg = { task, mask };
	return rtai_lxrt(BIDX, SIZARG, WAKEUP_SLEEPING, &arg).i[LOW];
}

#define rt_task_wakeup_sleeping(task)  rt_task_masked_unblock(task, RT_SCHED_DELAYED)

RTAI_PROTO(void, rt_get_exectime, (RT_TASK *task, RTIME *exectime))
{
	struct { RT_TASK *task; RTIME *exectime; } arg = { task, exectime };
	rtai_lxrt(BIDX, SIZARG, GET_EXECTIME, &arg);
}

RTAI_PROTO(void, rt_gettimeorig, (RTIME time_orig[]))
{
	struct { RTIME *time_orig; } arg = { time_orig };
	rtai_lxrt(BIDX, SIZARG, GET_TIMEORIG, &arg);
}

RTAI_PROTO(RT_TASK *,ftask_init,(unsigned long name, int priority))
{
	struct { unsigned long name; long priority, stack_size, max_msg_size, cpus_allowed; } arg = { name, priority, 0, 0, 0 };
	return (RT_TASK *)rtai_lxrt(BIDX, SIZARG, LXRT_TASK_INIT, &arg).v[LOW];
}

RTAI_PROTO(RTIME, start_ftimer,(long period, long ftick_freq))
{
	struct { long ftick_freq; void *handler; } arg = { ftick_freq, NULL };
	if (!period) {
		rtai_lxrt(BIDX, sizeof(long), SET_ONESHOT_MODE, &period);
	} else {
		rtai_lxrt(BIDX, sizeof(long), SET_PERIODIC_MODE, &period);
	}
	rtai_lxrt(BIDX, SIZARG, REQUEST_RTC, &arg);
	return rtai_lxrt(BIDX, sizeof(long), START_TIMER, &period).rt;
}

RTAI_PROTO(RTIME, stop_ftimer,(void))
{
	struct { long dummy; } arg;
	rtai_lxrt(BIDX, SIZARG, RELEASE_RTC, &arg);
	return rtai_lxrt(BIDX, SIZARG, STOP_TIMER, &arg).rt;
}

RTAI_PROTO(int, rt_sched_latencies, (int klat, int ulat, int period))
{
	struct { long period, loops, Latency; } arg = { klat, ulat, period };
	return rtai_lxrt(BIDX, SIZARG, SCHED_LATENCIES, &arg).i[0];
}

RTAI_PROTO(unsigned int, rt_get_cpu_freq, (void))
{
	struct { unsigned long dummy; } arg;
	return rtai_lxrt(BIDX, SIZARG, GET_CPU_FREQ, &arg).i[0];
}

static inline RTIME nanos2tscnts(RTIME nanos, unsigned int cpu_freq)
{
	return (RTIME)((long double)nanos*((long double)cpu_freq/(long double)1000000000));
}

static inline RTIME tscnts2nanos(RTIME tscnts, unsigned int cpu_freq)
{
	return (RTIME)((long double)tscnts*((long double)1000000000/(long double)cpu_freq));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

/*@}*/

#endif /* !_RTAI_LXRT_H */
