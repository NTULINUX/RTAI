/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
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

#ifndef _RTAI_SCHED_H
#define _RTAI_SCHED_H

#include <rtai.h>
#ifndef __KERNEL__
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <rtai_types.h>
#endif /* __KERNEL__ */

#define RT_SCHED_UP   1
#define RT_SCHED_SMP  2
#define RT_SCHED_MUP  3

#define RT_SCHED_HIGHEST_PRIORITY  0
#define RT_SCHED_LOWEST_PRIORITY   0x3fffFfff
#define RT_SCHED_LINUX_PRIORITY    0x7fffFfff

#define RT_RESEM_SUSPDEL  (-0x7fffFfff)

#define RT_SCHED_READY        1
#define RT_SCHED_SUSPENDED    2
#define RT_SCHED_DELAYED      4
#define RT_SCHED_SEMAPHORE    8
#define RT_SCHED_SEND        16
#define RT_SCHED_RECEIVE     32
#define RT_SCHED_RPC         64
#define RT_SCHED_RETURN     128
#define RT_SCHED_MBXSUSP    256
#define RT_SCHED_SFTRDY     512
#define RT_SCHED_POLL      1024
#define RT_SCHED_SIGSUSP    (1 << 15)

#define RT_RWLINV     (11)  // keep this the highest
#define RT_CHGPORTERR (10)
#define RT_CHGPORTOK  (9)
#define RT_NETIMOUT   (8)
#define RT_DEADLOK    (7)
#define RT_PERM       (6)
#define RT_OBJINV     (5)
#define RT_OBJREM     (4)
#define RT_TIMOUT     (3)
#define RT_UNBLKD     (2)
#define RT_TMROVRN    (1)  // keep this the lowest, must be 1
#define RTP_RWLINV     ((void *)RT_RWLINV)
#define RTP_CHGPORTERR ((void *)RT_CHGPORTERR)
#define RTP_CHGPORTOK  ((void *)RT_CHGPORTOK)
#define RTP_NETIMOUT   ((void *)RT_NETIMOUT)
#define RTP_DEADLOK    ((void *)RT_DEADLOK)
#define RTP_PERM       ((void *)RT_PERM)
#define RTP_OBJINV     ((void *)RT_OBJINV)
#define RTP_OBJREM     ((void *)RT_OBJREM)
#define RTP_TIMOUT     ((void *)RT_TIMOUT)
#define RTP_UNBLKD     ((void *)RT_UNBLKD)
#define RTP_TMROVRN    ((void *)RT_TMROVRN)
#define RTP_HIGERR     (RTP_RWLINV)
#define RTP_LOWERR     (RTP_TMROVRN)
#if CONFIG_RTAI_USE_NEWERR
#define RTE_BASE       (0x3FFFFF00)
#define RTE_RWLINV     (RTE_BASE + RT_RWLINV)
#define RTE_CHGPORTERR (RTE_BASE + RT_CHGPORTERR)
#define RTE_CHGPORTOK  (RTE_BASE + RT_CHGPORTOK)
#define RTE_NETIMOUT   (RTE_BASE + RT_NETIMOUT)
#define RTE_DEADLOK    (RTE_BASE + RT_DEADLOK)
#define RTE_PERM       (RTE_BASE + RT_PERM)
#define RTE_OBJINV     (RTE_BASE + RT_OBJINV)
#define RTE_OBJREM     (RTE_BASE + RT_OBJREM)
#define RTE_TIMOUT     (RTE_BASE + RT_TIMOUT)
#define RTE_UNBLKD     (RTE_BASE + RT_UNBLKD)
#define RTE_TMROVRN    (RTE_BASE + RT_TMROVRN)
#define RTE_HIGERR     (RTE_RWLINV)
#define RTE_LOWERR     (RTE_TMROVRN)
#else
#define RTE_BASE       (0xFFFB)
#define RTE_RWLINV     (RTE_BASE + RT_RWLINV)
#define RTE_CHGPORTERR (RTE_BASE + RT_CHGPORTERR)
#define RTE_CHGPORTOK  (RTE_BASE + RT_CHGPORTOK)
#define RTE_NETIMOUT   (RTE_BASE + RT_NETIMOUT)
#define RTE_DEADLOK    (RTE_BASE + RT_DEADLOK)
#define RTE_PERM       (RTE_BASE + RT_PERM)
#define RTE_OBJINV     (RTE_BASE + RT_OBJREM)
#define RTE_OBJREM     (RTE_BASE + RT_OBJREM)
#define RTE_TIMOUT     (RTE_BASE + RT_TIMOUT)
#define RTE_UNBLKD     (RTE_BASE + RT_UNBLKD)
#define RTE_TMROVRN    (RTE_BASE + RT_TMROVRN)
#define RTE_HIGERR     (RTE_RWLINV)
#define RTE_LOWERR     (RTE_TMROVRN)
#endif

#define RT_EINTR      (RTE_UNBLKD)

#define rt_is_reterr(i)  (i >= RTE_LOWERR)

#define RT_IRQ_TASK         0
#define RT_IRQ_TASKLET      1
#define RT_IRQ_TASK_ERR     0x7FFFFFFF

struct rt_task_struct;

typedef struct rt_task_info { 
	RTIME period; long base_priority, priority; 
} RT_TASK_INFO;

#ifdef __KERNEL__

#include <linux/time.h>
#include <linux/errno.h>

#if defined(CONFIG_RTAI_LONG_TIMED_LIST)
#include <linux/rbtree.h>
typedef struct rb_node rb_node_t;
typedef struct rb_root rb_root_t;
#endif

#define RT_TASK_MAGIC 0x9ad25f6f  // nam2num("rttask")

#ifndef __cplusplus

#include <linux/sched.h>

typedef struct rt_queue {
	struct rt_queue *prev;
	struct rt_queue *next;
	struct rt_task_struct *task;
} QUEUE;

struct mcb_t {
	void *sbuf;
	int sbytes;
	void *rbuf;
	int rbytes;
};

/*Exit handler functions are called like C++ destructors in rt_task_delete().*/
typedef struct rt_ExitHandler {
	struct rt_ExitHandler *nxt;
	void (*fun) (void *arg1, int arg2);
	void *arg1;
	int   arg2;
} XHDL;

struct rt_heap_t { void *heap, *kadr, *uadr; };

#define RTAI_MAX_NAME_LENGTH  32

typedef struct rt_task_struct {
	long *stack __attribute__ ((__aligned__ (L1_CACHE_BYTES)));
	int uses_fpu;
	int magic;
	volatile int state, running;
	unsigned long runnable_on_cpus;
	long *stack_bottom;
	volatile int priority;
	int base_priority;
	int policy;
	int sched_lock_priority;
	struct rt_task_struct *prio_passed_to;
	RTIME period;
	RTIME resume_time;
	RTIME periodic_resume_time;
	RTIME yield_time;
	int rr_quantum, rr_remaining;
	int suspdepth;
	struct rt_queue queue;
	int owndres;
	struct rt_queue *blocked_on;
	struct rt_queue msg_queue;
	int tid;  /* trace ID */
	unsigned long msg;
	struct rt_queue ret_queue;
	void (*signal)(void);
	FPU_ENV fpu_reg __attribute__ ((__aligned__ (L1_CACHE_BYTES)));
	struct rt_task_struct *prev, *next;
	struct rt_task_struct *tprev, *tnext;
	struct rt_task_struct *rprev, *rnext;

	/* For calls from LINUX. */
	long *fun_args;
	long *bstack;
	struct task_struct *lnxtsk;
	long long retval;
	char *msg_buf[2];
	long max_msg_size[2];
	char task_name[RTAI_MAX_NAME_LENGTH];
	void *system_data_ptr;
	struct rt_task_struct *nextp, *prevp;

	RT_TRAP_HANDLER task_trap_handler[RTAI_NR_TRAPS];

	long unblocked;
	void *rt_signals;
	volatile unsigned long pstate;
	unsigned long usp_flags;
	unsigned long usp_flags_mask;
	unsigned long force_soft;
	volatile int is_hard;
	int kerrno;

	long busy_time_align;
	void *linux_syscall_server;

	/* For use by watchdog. */
	int resync_frame;

	/* For use by exit handler functions. */
	XHDL *ExitHook;

	RTIME exectime[2];
	struct mcb_t mcb;

	/* Real time heaps. */
	struct rt_heap_t heap[2];

	long scheduler;

#ifdef CONFIG_RTAI_LONG_TIMED_LIST
	rb_root_t rbr;
	rb_node_t rbn;
#endif
	struct rt_queue resq;
	unsigned long resumsg;
	int schedlat;
} RT_TASK __attribute__ ((__aligned__ (L1_CACHE_BYTES)));

#else /* __cplusplus */
extern "C" {
#endif /* !__cplusplus */

int set_rtext(RT_TASK *task, int priority, int uses_fpu, void(*signal)(void), unsigned int cpuid);

int rt_task_init(struct rt_task_struct *task,
		 void (*rt_thread)(long),
		 long data,
		 int stack_size,
		 int priority,
		 int uses_fpu,
		 void(*signal)(void));

int rt_task_init_cpuid(struct rt_task_struct *task,
		       void (*rt_thread)(long),
		       long data,
		       int stack_size,
		       int priority,
		       int uses_fpu,
		       void(*signal)(void),
		       unsigned run_on_cpu);

void rt_thread_create(void *fun, void *args, struct task_struct **thread);

RT_TASK *rt_thread_init(unsigned long name, int priority, int make_hard, int policy, int cpus_allowed);

int rt_thread_delete(RT_TASK *rt_task);


int rt_kthread_init(struct rt_task_struct *task,
		    void (*rt_thread)(long),
		    long data,
		    int stack_size,
		    int priority,
		    int uses_fpu,
		    void(*signal)(void));

int rt_kthread_init_cpuid(struct rt_task_struct *task,
		          void (*rt_thread)(long),
		          long data,
		          int stack_size,
		          int priority,
		          int uses_fpu,
		          void(*signal)(void),
		          unsigned run_on_cpu);

RTAI_SYSCALL_MODE void rt_set_runnable_on_cpus(struct rt_task_struct *task,
			     unsigned long cpu_mask);

RTAI_SYSCALL_MODE void rt_set_runnable_on_cpuid(struct rt_task_struct *task,
			      unsigned cpuid);

RTAI_SYSCALL_MODE void rt_set_sched_policy(struct rt_task_struct *task,
			 int policy,
			 int rr_quantum_ns);

int rt_task_delete(struct rt_task_struct *task);

int rt_get_task_state(struct rt_task_struct *task);

void rt_gettimeorig(RTIME time_orig[]);

int rt_get_timer_cpu(void);

int rt_is_hard_timer_running(void);

void rt_set_periodic_mode(void);

void rt_set_oneshot_mode(void);

RTAI_SYSCALL_MODE RTIME start_rt_timer(int period);

#define start_rt_timer_ns(period) start_rt_timer(nano2count((period)))

RTAI_SYSCALL_MODE void start_rt_apic_timers(struct apic_timer_setup_data *setup_mode,
			  unsigned rcvr_jiffies_cpuid);

void stop_rt_timer(void);

struct rt_task_struct *rt_whoami(void);

int rt_sched_type(void);

RTAI_SYSCALL_MODE int rt_task_signal_handler(struct rt_task_struct *task,
			   void (*handler)(void));

RTAI_SYSCALL_MODE int rt_task_use_fpu(struct rt_task_struct *task,
		    int use_fpu_flag);
  
void rt_linux_use_fpu(int use_fpu_flag);

RTAI_SYSCALL_MODE int rt_hard_timer_tick_count(void);

RTAI_SYSCALL_MODE int rt_hard_timer_tick_count_cpuid(int cpuid);

RTAI_SYSCALL_MODE RTIME count2nano(RTIME timercounts);

RTAI_SYSCALL_MODE RTIME nano2count(RTIME nanosecs);
  
RTAI_SYSCALL_MODE RTIME count2nano_cpuid(RTIME timercounts, unsigned cpuid);

RTAI_SYSCALL_MODE RTIME nano2count_cpuid(RTIME nanosecs, unsigned cpuid);
  
RTIME rt_get_time(void);

RTAI_SYSCALL_MODE RTIME rt_get_time_cpuid(unsigned cpuid);

RTIME rt_get_time_ns(void);

RTAI_SYSCALL_MODE RTIME rt_get_time_ns_cpuid(unsigned cpuid);

RTIME rt_get_cpu_time_ns(void);

RTIME rt_get_real_time(void);

RTIME rt_get_real_time_ns(void);

int rt_get_prio(struct rt_task_struct *task);

int rt_get_inher_prio(struct rt_task_struct *task);

RTAI_SYSCALL_MODE int rt_task_get_info(RT_TASK *task, RT_TASK_INFO *task_info);

RTAI_SYSCALL_MODE int rt_get_priorities(struct rt_task_struct *task, int *priority, int *base_priority);

RTAI_SYSCALL_MODE void rt_spv_RMS(int cpuid);

RTAI_SYSCALL_MODE int rt_change_prio(struct rt_task_struct *task,
		   int priority);

void rt_sched_lock(void);

void rt_sched_unlock(void);

void rt_task_yield(void);

RTAI_SYSCALL_MODE int rt_task_suspend(struct rt_task_struct *task);

RTAI_SYSCALL_MODE int rt_task_suspend_if(struct rt_task_struct *task);

RTAI_SYSCALL_MODE int rt_task_suspend_until(struct rt_task_struct *task, RTIME until);

RTAI_SYSCALL_MODE int rt_task_suspend_timed(struct rt_task_struct *task, RTIME delay);

RTAI_SYSCALL_MODE int rt_task_resume(struct rt_task_struct *task);

RTAI_SYSCALL_MODE int rt_set_linux_syscall_mode(long sync_async, void (*callback_fun)(long, long));

struct linux_syscalls_list;
void rt_exec_linux_syscall(RT_TASK *rt_current, struct linux_syscalls_list *syscalls, struct pt_regs *regs);

RTAI_SYSCALL_MODE void rt_return_linux_syscall(RT_TASK *task, unsigned long retval);

RTAI_SYSCALL_MODE int rt_irq_wait(unsigned irq);

RTAI_SYSCALL_MODE int rt_irq_wait_if(unsigned irq);

RTAI_SYSCALL_MODE int rt_irq_wait_until(unsigned irq, RTIME until);

RTAI_SYSCALL_MODE int rt_irq_wait_timed(unsigned irq, RTIME delay);

RTAI_SYSCALL_MODE void rt_irq_signal(unsigned irq);

RTAI_SYSCALL_MODE int rt_request_irq_task (unsigned irq, void *handler, int type, int affine2task);

RTAI_SYSCALL_MODE int rt_release_irq_task (unsigned irq);

RTAI_SYSCALL_MODE int rt_task_make_periodic_relative_ns(struct rt_task_struct *task,
				      RTIME start_delay,
				      RTIME period);

RTAI_SYSCALL_MODE int rt_task_make_periodic(struct rt_task_struct *task,
			  RTIME start_time,
			  RTIME period);

RTAI_SYSCALL_MODE void rt_task_set_resume_end_times(RTIME resume,
				  RTIME end);

RTAI_SYSCALL_MODE int rt_set_resume_time(struct rt_task_struct *task,
		       RTIME new_resume_time);

RTAI_SYSCALL_MODE int rt_set_period(struct rt_task_struct *task,
		  RTIME new_period);

int rt_task_wait_period(void);

void rt_schedule(void);

RTIME next_period(void);

RTAI_SYSCALL_MODE void rt_busy_sleep(int nanosecs);

RTAI_SYSCALL_MODE int rt_sleep(RTIME delay);

RTAI_SYSCALL_MODE int rt_sleep_until(RTIME time);

RTAI_SYSCALL_MODE int rt_task_masked_unblock(struct rt_task_struct *task, unsigned long mask);

#define rt_task_wakeup_sleeping(t)  rt_task_masked_unblock(t, RT_SCHED_DELAYED)

RTAI_SYSCALL_MODE struct rt_task_struct *rt_named_task_init(const char *task_name,
					  void (*thread)(long),
					  long data,
					  int stack_size,
					  int prio,
					  int uses_fpu,
					  void(*signal)(void));

RTAI_SYSCALL_MODE struct rt_task_struct *rt_named_task_init_cpuid(const char *task_name,
						void (*thread)(long),
						long data,
						int stack_size,
						int prio,
						int uses_fpu,
						void(*signal)(void),
						unsigned run_on_cpu);

RTAI_SYSCALL_MODE int rt_named_task_delete(struct rt_task_struct *task);

RT_TRAP_HANDLER rt_set_task_trap_handler(struct rt_task_struct *task,
					 unsigned vec,
					 RT_TRAP_HANDLER handler);

static inline RTIME timeval2count(struct timeval *t)
{
        return nano2count(t->tv_sec*1000000000LL + t->tv_usec*1000);
}

static inline void count2timeval(RTIME rt, struct timeval *t)
{
        t->tv_sec = rtai_ulldiv(count2nano(rt), 1000000000, (unsigned long *)&t->tv_usec);
        t->tv_usec /= 1000;
}

static inline RTIME timespec2count(const struct timespec *t)
{
        return nano2count(t->tv_sec*1000000000LL + t->tv_nsec);
}

static inline void count2timespec(RTIME rt, struct timespec *t)
{
        t->tv_sec = rtai_ulldiv(count2nano(rt), 1000000000, (unsigned long *)&t->tv_nsec);
}

static inline RTIME timespec2nanos(const struct timespec *t)
{
        return t->tv_sec*1000000000LL + t->tv_nsec;
}

static inline void nanos2timespec(RTIME rt, struct timespec *t)
{
        t->tv_sec = rtai_ulldiv(rt, 1000000000, (unsigned long *)&t->tv_nsec);
}

void rt_make_hard_real_time(RT_TASK *task);

void rt_make_soft_real_time(RT_TASK *task);

#ifdef __cplusplus
}
#else /* !__cplusplus */

#endif /* __cplusplus */

#endif /* __KERNEL__ */

#if !defined(__KERNEL__) || defined(__cplusplus)

typedef struct rt_task_struct {
    int opaque;
} RT_TASK;

typedef struct QueueBlock {
    int opaque;
} QBLK;

typedef struct QueueHook {
    int opaque;
} QHOOK;

#endif /* !__KERNEL__ || __cplusplus */

#endif /* !_RTAI_SCHED_H */
