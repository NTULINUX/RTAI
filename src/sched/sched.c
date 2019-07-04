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

/*
ACKNOWLEDGMENTS:
- Steve Papacharalambous (stevep@zentropix.com) has contributed a very 
  informative proc filesystem procedure.
- Stefano Picerno (stefanopp@libero.it) for suggesting a simple fix to
  distinguish a timeout from an abnormal retrun in timed sem waits.
- Geoffrey Martin (gmartin@altersys.com) for a fix to functions with timeouts.
*/


#undef CONFIG_TRACEPOINTS

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/timex.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/reboot.h>
#include <linux/sys.h>
#include <linux/sched/mm.h>  // for mmdrop
#include <asm/param.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mmu_context.h>
#define __KERNEL_SYSCALLS__
#include <linux/unistd.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>

static int rtai_proc_sched_register(void);
static void rtai_proc_sched_unregister(void);
int rtai_proc_lxrt_register(void);
void rtai_proc_lxrt_unregister(void);

#include <rtai.h>
#include <rtai_defs.h>
#include <asm/rtai_sched.h>
#include <rtai_lxrt.h>
#include <rtai_registry.h>
#include <rtai_nam2num.h>
#include <rtai_schedcore.h>
#include <rtai_prinher.h>
#include <rtai_signal.h>

MODULE_LICENSE("GPL");

int KernelLatency, UserLatency;
static int kernel_latency = 0;
module_param(kernel_latency, int, S_IRUGO);
static int user_latency = 0;
module_param(user_latency, int, S_IRUGO);

/* +++++++++++++++++ WHAT MUST BE AVAILABLE EVERYWHERE ++++++++++++++++++++++ */

RT_TASK rt_smp_linux_task[RTAI_NR_CPUS];

RT_TASK *rt_smp_current[RTAI_NR_CPUS];

RTIME rt_smp_time_h[RTAI_NR_CPUS];

volatile int rt_sched_timed;

struct klist_t wake_up_hts[RTAI_NR_CPUS];
struct klist_t wake_up_srq[RTAI_NR_CPUS];

/* +++++++++++++++ END OF WHAT MUST BE AVAILABLE EVERYWHERE +++++++++++++++++ */

static struct { volatile int locked, rqsted; } rt_scheduling[RTAI_NR_CPUS];
EXPORT_SYMBOL(rt_scheduling);  // to allow RTDM its preferred deferred sched in intr

static unsigned long rt_smp_linux_cr0[RTAI_NR_CPUS];

static RT_TASK *rt_smp_fpu_task[RTAI_NR_CPUS];

int rt_smp_half_tick[RTAI_NR_CPUS];

static volatile int rt_smp_timer_shot_fired[RTAI_NR_CPUS];

RT_TASK *lxrt_prev_task[RTAI_NR_CPUS];

static int lxrt_notify_reboot(struct notifier_block *nb,
			      unsigned long event,
			      void *ptr);

static struct notifier_block lxrt_reboot_notifier = {
	.notifier_call	= &lxrt_notify_reboot,
	.next		= NULL,
	.priority	= 0
};

#define tuned rtai_tunables

#define fpu_task (rt_smp_fpu_task[cpuid])

#define rt_half_tick (rt_smp_half_tick[cpuid])

#define timer_shot_fired (rt_smp_timer_shot_fired[cpuid])

#define rt_times (rt_smp_times[cpuid])

#define linux_cr0 (rt_smp_linux_cr0[cpuid])

#define MAX_FRESTK_SRQ  (2 << 6)
static struct { int srq; volatile unsigned long in, out; void *mp[MAX_FRESTK_SRQ]; } frstk_srq;

#define KTHREAD_M_PRIO MAX_LINUX_RTPRIO
#define KTHREAD_F_PRIO MAX_LINUX_RTPRIO

#ifdef CONFIG_SMP

static void rt_schedule_on_schedule_ipi(void);

static inline int rt_request_sched_ipi(void)
{
	return rt_request_irq(RTAI_RESCHED_IRQ, (void *)rt_schedule_on_schedule_ipi, NULL, 0);
}

static inline void rt_free_sched_ipi(void)
{
	rt_release_irq(RTAI_RESCHED_IRQ);
}

static inline void sched_get_global_lock(int cpuid)
{
	__rt_get_global_lock();
}

static inline void sched_release_global_lock(int cpuid)
{
	__rt_release_global_lock();
}

#else /* !CONFIG_SMP */

#define rt_request_sched_ipi()  0

#define rt_free_sched_ipi()

#define sched_get_global_lock(cpuid)

#define sched_release_global_lock(cpuid)

#endif /* CONFIG_SMP */

/* ++++++++++++++++++++++++++++++++ TASKS ++++++++++++++++++++++++++++++++++ */

#ifdef CONFIG_RTAI_MALLOC
int rtai_kstack_heap_size = (CONFIG_RTAI_KSTACK_HEAPSZ*1024);
RTAI_MODULE_PARM(rtai_kstack_heap_size, int);

static rtheap_t rtai_kstack_heap;

#define rt_kstack_alloc(sz)  rtheap_alloc(&rtai_kstack_heap, sz, 0)
#define rt_kstack_free(p)    rtheap_free(&rtai_kstack_heap, p)
#else
#define rt_kstack_alloc(sz)  rt_malloc(sz)
#define rt_kstack_free(p)    rt_free(p)
#endif

static int tasks_per_cpu[RTAI_NR_CPUS] = { 0, };

#define CPUS_ALLOWED_ALL 0xFF

int get_min_tasks_cpuid(unsigned long cpus_allowed)
{
	int i, cpuid, min;
	min =  tasks_per_cpu[cpuid = 0];
	for (i = 1; i < num_online_cpus(); i++) {
		if (test_bit(i, &cpus_allowed) && tasks_per_cpu[i] < min) {
			min = tasks_per_cpu[cpuid = i];
		}
	}
	return cpuid;
}

void put_current_on_cpu(int cpuid)
{
#ifdef CONFIG_SMP
	struct task_struct *task = current;
	if (set_cpus_allowed_ptr(task, cpumask_of(cpuid))) {
		set_cpus_allowed_ptr(task, cpumask_of(rtai_tskext_t(task, TSKEXT0)->runnable_on_cpus = rtai_cpuid()));
	}
#endif /* CONFIG_SMP */
}

int set_rtext(RT_TASK *task, int priority, int uses_fpu, void(*signal)(void), unsigned int cpuid)
{
	unsigned long flags;

	if (num_online_cpus() <= 1) {
		 cpuid = 0;
	}
	if (task->magic == RT_TASK_MAGIC || cpuid >= RTAI_NR_CPUS || priority < 0) {
		return -EINVAL;
	}
	task->uses_fpu = uses_fpu ? 1 : 0;
	task->runnable_on_cpus = cpuid;
	(task->stack_bottom = (long *)&task->fpu_reg)[0] = 0;
	task->magic = RT_TASK_MAGIC; 
	task->policy = 0;
	task->owndres = 0;
	task->running = 0;
	task->prio_passed_to = 0;
	task->period = 0;
	task->resume_time = RTAI_TIME_LIMIT;
	task->periodic_resume_time = RTAI_TIME_LIMIT;
	task->queue.prev = task->queue.next = &(task->queue);      
	task->queue.task = task;
	task->msg_queue.prev = task->msg_queue.next = &(task->msg_queue);      
	task->msg_queue.task = task;    
	task->msg = 0;  
	task->ret_queue.prev = task->ret_queue.next = &(task->ret_queue);
	task->ret_queue.task = NULL;
	task->tprev = task->tnext = task->rprev = task->rnext = task;
	task->blocked_on = NULL;        
	task->signal = signal;
	task->unblocked = 0;
	task->rt_signals = NULL;
	memset(task->task_trap_handler, 0, RTAI_NR_TRAPS*sizeof(void *));
	task->linux_syscall_server = NULL;
	task->busy_time_align = 0;
	task->resync_frame = 0;
	task->ExitHook = 0;
	task->usp_flags = task->usp_flags_mask = task->force_soft = 0;
	task->msg_buf[0] = 0;
	task->exectime[0] = task->exectime[1] = 0;
	task->system_data_ptr = 0;
	atomic_inc((atomic_t *)(tasks_per_cpu + cpuid));

	task->priority = task->base_priority = BASE_SOFT_PRIORITY + priority;
	task->suspdepth = task->is_hard = 0;
	task->state = RT_SCHED_READY;
	rtai_tskext(current, TSKEXT0) = task;
	rtai_tskext(current, TSKEXT1) = task->lnxtsk = current;
	put_current_on_cpu(cpuid);

	task->schedlat = task->lnxtsk->mm ? UserLatency : KernelLatency;
	flags = rt_global_save_flags_and_cli();
	task->next = 0;
	rt_linux_task.prev->next = task;
	task->prev = rt_linux_task.prev;
	rt_linux_task.prev = task;
	rt_global_restore_flags(flags);

	task->resq.prev = task->resq.next = &task->resq;
	task->resq.task = NULL;

	task->scheduler = 0;

	return 0;
}


asmlinkage static void rt_startup(void(*rt_thread)(long), long data)
{
	extern int rt_task_delete(RT_TASK *);
	RT_TASK *rt_current = rt_smp_current[rtai_cpuid()];
	rt_global_sti();
#if CONFIG_RTAI_MONITOR_EXECTIME
	rt_current->exectime[1] = rtai_rdtsc();
#endif
	((void (*)(long))rt_current->max_msg_size[0])(rt_current->max_msg_size[1]);
	rt_drg_on_adr(rt_current);
	rt_task_delete(rt_smp_current[rtai_cpuid()]);
	rt_printk("LXRT: task %p returned but could not be delated.\n", rt_current); 
}


static int rt_pid = (INT_MAX & ~(0xF));
static DEFINE_SPINLOCK(rt_pid_lock);

void rt_set_task_pid(RT_TASK *task)
{
	unsigned long flags;
	flags = rt_spin_lock_irqsave(&rt_pid_lock);
	task->tid = rt_pid = rt_pid - 0x10;
	rt_spin_unlock_irqrestore(flags, &rt_pid_lock);
	task->tid += task->runnable_on_cpus;
}
EXPORT_SYMBOL(rt_set_task_pid);

RT_TASK *rt_find_task_by_pid(pid_t pid)
{
	RT_TASK *task = &rt_smp_linux_task[pid & 0xF];
	while ((task = task->next)) {
		if (task->tid == pid) {
			return task;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(rt_find_task_by_pid);


int rt_task_init_cpuid(RT_TASK *task, void (*rt_thread)(long), long data, int stack_size, int priority, int uses_fpu, void(*signal)(void), unsigned int cpuid)
{
	long *st, i;
	unsigned long flags;

	if (num_online_cpus() <= 1) {
		 cpuid = 0;
	}
	if (task->magic == RT_TASK_MAGIC || cpuid >= RTAI_NR_CPUS || priority < 0) {
		return -EINVAL;
	} 
	if (!(st = (long *)rt_kstack_alloc(stack_size))) {
		return -ENOMEM;
	}

	task->bstack = task->stack = (long *)(((unsigned long)st + stack_size - 0x10) & ~0xF);
	task->stack[0] = 0;
	task->uses_fpu = uses_fpu ? 1 : 0;
	task->runnable_on_cpus = cpuid;
	atomic_inc((atomic_t *)(tasks_per_cpu + cpuid));
	*(task->stack_bottom = st) = 0;
	task->magic = RT_TASK_MAGIC; 
	task->policy = 0;
	task->suspdepth = 1;
	task->state = (RT_SCHED_SUSPENDED | RT_SCHED_READY);
	task->owndres = 0;
	task->running = 0;
	task->is_hard = 1;
	task->lnxtsk = 0;
	task->priority = task->base_priority = priority;
	task->prio_passed_to = 0;
	task->period = 0;
	task->resume_time = RTAI_TIME_LIMIT;
	task->periodic_resume_time = RTAI_TIME_LIMIT;
	task->queue.prev = &(task->queue);      
	task->queue.next = &(task->queue);      
	task->queue.task = task;
	task->msg_queue.prev = &(task->msg_queue);      
	task->msg_queue.next = &(task->msg_queue);      
	task->msg_queue.task = task;    
	task->msg = 0;  
	task->ret_queue.prev = &(task->ret_queue);
	task->ret_queue.next = &(task->ret_queue);
	task->ret_queue.task = NULL;
	task->tprev = task->tnext =
	task->rprev = task->rnext = task;
	task->blocked_on = NULL;        
	task->signal = signal;
	task->unblocked = 0;
	task->rt_signals = NULL;
	for (i = 0; i < RTAI_NR_TRAPS; i++) {
		task->task_trap_handler[i] = NULL;
	}
	task->linux_syscall_server = NULL;
	task->busy_time_align = 0;
	task->resync_frame = 0;
	task->ExitHook = 0;
	task->exectime[0] = task->exectime[1] = 0;
	task->system_data_ptr = 0;

	task->max_msg_size[0] = (long)rt_thread;
	task->max_msg_size[1] = data;
	init_arch_stack();
	task->schedlat = KernelLatency;

	flags = rt_global_save_flags_and_cli();
	task->next = 0;
	rt_linux_task.prev->next = task;
	task->prev = rt_linux_task.prev;
	rt_linux_task.prev = task;
	init_task_fpenv(task);
	rt_global_restore_flags(flags);

	task->resq.prev = task->resq.next = &task->resq;
	task->resq.task = NULL;
	rt_set_task_pid(task);

	return 0;
}

int rt_task_init(RT_TASK *task, void (*rt_thread)(long), long data,
			int stack_size, int priority, int uses_fpu,
			void(*signal)(void))
{
	return rt_task_init_cpuid(task, rt_thread, data, stack_size, priority, 
				 uses_fpu, signal, get_min_tasks_cpuid(CPUS_ALLOWED_ALL));
}


RTAI_SYSCALL_MODE void rt_set_runnable_on_cpuid(RT_TASK *task, unsigned int cpuid)
{
	unsigned long flags;
	RT_TASK *linux_task;

	if (task->lnxtsk) {
		return;
	}

	if (cpuid >= RTAI_NR_CPUS) {
		cpuid = get_min_tasks_cpuid(CPUS_ALLOWED_ALL);
	} 
	flags = rt_global_save_flags_and_cli();
	do {
		task->period = rtai_llimd(task->period, TIMER_FREQ, tuned.clock_freq);
		task->resume_time = rtai_llimd(task->resume_time, TIMER_FREQ, tuned.clock_freq);
		task->periodic_resume_time = rtai_llimd(task->periodic_resume_time, TIMER_FREQ, tuned.clock_freq);
	} while (0);
	if (!((task->prev)->next = task->next)) {
		rt_smp_linux_task[task->runnable_on_cpus].prev = task->prev;
	} else {
		(task->next)->prev = task->prev;
	}
	if ((task->state & RT_SCHED_DELAYED)) {
		rem_timed_task(task);
		task->runnable_on_cpus = cpuid;
		enq_timed_task(task);
	} else {
		task->runnable_on_cpus = cpuid;
	}
	task->next = 0;
	(linux_task = rt_smp_linux_task + cpuid)->prev->next = task;
	task->prev = linux_task->prev;
	linux_task->prev = task;
	rt_global_restore_flags(flags);
}


RTAI_SYSCALL_MODE void rt_set_runnable_on_cpus(RT_TASK *task, unsigned long run_on_cpus)
{
	int cpuid;

	if (task->lnxtsk) {
		return;
	}

#ifdef CONFIG_SMP
	run_on_cpus &= CPUMASK(cpu_online_map);
#else
	run_on_cpus = 1;
#endif
	cpuid = get_min_tasks_cpuid(CPUS_ALLOWED_ALL);
	if (!test_bit(cpuid, &run_on_cpus)) {
		cpuid = ffnz(run_on_cpus);
	}
	rt_set_runnable_on_cpuid(task, cpuid);
}


int rt_check_current_stack(void)
{
	DECLARE_RT_CURRENT;
	char *sp;

	ASSIGN_RT_CURRENT;
	if (rt_current != &rt_linux_task) {
		sp = get_stack_pointer();
		return (sp - (char *)(rt_current->stack_bottom));
	} else {
		return RT_RESEM_SUSPDEL;
	}
}


#define RR_YIELD() \
if (CONFIG_RTAI_ALLOW_RR && rt_current->policy > 0) { \
	if (rt_current->yield_time <= rt_times.tick_time) { \
		rt_current->rr_remaining = rt_current->rr_quantum; \
		if (rt_current->state == RT_SCHED_READY) { \
			RT_TASK *task; \
			task = rt_current->rnext; \
			while (rt_current->priority == task->priority) { \
				task = task->rnext; \
			} \
			if (task != rt_current->rnext) { \
				(rt_current->rprev)->rnext = rt_current->rnext; \
				(rt_current->rnext)->rprev = rt_current->rprev; \
				task->rprev = (rt_current->rprev = task->rprev)->rnext = rt_current; \
				rt_current->rnext = task; \
			} \
		} \
	} else { \
		rt_current->rr_remaining = rt_current->yield_time - rt_times.tick_time; \
	} \
} 

#define TASK_TO_SCHEDULE() \
do { \
	new_task = rt_linux_task.rnext; \
	if (CONFIG_RTAI_ALLOW_RR && new_task->policy > 0) { \
		new_task->yield_time = rt_times.tick_time + new_task->rr_remaining; \
	} \
	new_task->running = 1; \
} while (0)

#define RR_INTR_TIME(fire_shot) \
do { \
	fire_shot = 0; \
	prio = new_task->priority; \
	if (CONFIG_RTAI_ALLOW_RR && new_task->policy > 0) { \
		if (new_task->yield_time < rt_times.intr_time) { \
			rt_times.intr_time = new_task->yield_time; \
			fire_shot = 1; \
		} \
        } \
} while (0)

#define LOCK_LINUX(cpuid) \
	do { rt_switch_to_real_time(cpuid); } while (0)
#define UNLOCK_LINUX(cpuid) \
	do { rt_switch_to_linux(cpuid);     } while (0)

#define SAVE_LOCK_LINUX(cpuid) \
	do { sflags = rt_save_switch_to_real_time(cpuid); } while (0)
#define RESTORE_UNLOCK_LINUX(cpuid) \
	do { rt_restore_switch_to_linux(sflags, cpuid);   } while (0)

#ifdef LOCKED_LINUX_IN_IRQ_HANDLER
#define SAVE_LOCK_LINUX_IN_IRQ(cpuid)
#define RESTORE_UNLOCK_LINUX_IN_IRQ(cpuid)
#else
#define SAVE_LOCK_LINUX_IN_IRQ(cpuid)       LOCK_LINUX(cpuid)    
#define RESTORE_UNLOCK_LINUX_IN_IRQ(cpuid)  UNLOCK_LINUX(cpuid)
#endif

#if defined(CONFIG_RTAI_TASK_SWITCH_SIGNAL) && CONFIG_RTAI_TASK_SWITCH_SIGNAL

#define RTAI_TASK_SWITCH_SIGNAL() \
	do { \
		void (*signal)(void) = rt_current->signal; \
		if ((unsigned long)signal > MAXSIGNALS) { \
			(*signal)(); \
		} else if (signal) { \
			rt_current->signal = NULL; /* to avoid recursing */\
			rt_trigger_signal((long)signal, rt_current); \
			rt_current->signal = signal; \
		} \
	} while (0)
#else

#define RTAI_TASK_SWITCH_SIGNAL()

#endif
	
#if CONFIG_RTAI_MONITOR_EXECTIME

RTIME switch_time[RTAI_NR_CPUS];

#define SET_EXEC_TIME() \
	do { \
		RTIME now; \
		now = rtai_rdtsc(); \
		rt_current->exectime[0] += (now - switch_time[cpuid]); \
		switch_time[cpuid] = now; \
	} while (0)

#define RST_EXEC_TIME()  do { switch_time[cpuid] = rtai_rdtsc(); } while (0)

#else

#define SET_EXEC_TIME()
#define RST_EXEC_TIME()

#endif

#ifdef CONFIG_RTAI_WD
#define SAVE_PREV_TASK()  \
	do { lxrt_prev_task[cpuid] = rt_current; } while (0)
#else
#define SAVE_PREV_TASK()  do { } while (0)
#endif

void rt_do_force_soft(RT_TASK *rt_task)
{
	rt_global_cli();
	if (rt_task->state != RT_SCHED_READY) {
		rt_task->state &= ~RT_SCHED_READY;
            	enq_ready_task(rt_task);
		RT_SCHEDULE(rt_task, rtai_cpuid());
	}
	rt_global_sti();
}

#define enq_soft_ready_task(ready_task) \
do { \
	RT_TASK *task = rt_smp_linux_task[cpuid].rnext; \
	if (ready_task == task) break; \
	task->rprev = (ready_task->rprev = task->rprev)->rnext = ready_task; \
	ready_task->rnext = task; \
} while (0)


#define pend_wake_up_hts(lnxtsk, cpuid) \
do { \
	wake_up_hts[cpuid].task[wake_up_hts[cpuid].in++ & (MAX_WAKEUP_SRQ - 1)] = lnxtsk; \
	hal_pend_uncond(wake_up_srq[0].srq, cpuid); \
} while (0)


static inline void force_current_soft(RT_TASK *rt_current, int cpuid)
{
	struct task_struct *lnxtsk;
        void rt_schedule(void);
        rt_current->force_soft = 0;
	rt_current->state &= ~RT_SCHED_READY;;
	pend_wake_up_hts(lnxtsk = rt_current->lnxtsk, cpuid);
        (rt_current->rprev)->rnext = rt_current->rnext;
        (rt_current->rnext)->rprev = rt_current->rprev;
        rt_schedule();
        rt_current->is_hard = 0;
	if (rt_current->priority < BASE_SOFT_PRIORITY) {
		if (rt_current->priority == rt_current->base_priority) {
			rt_current->priority += BASE_SOFT_PRIORITY;
		}
	}
	if (rt_current->base_priority < BASE_SOFT_PRIORITY) {
		rt_current->base_priority += BASE_SOFT_PRIORITY;
	}
	rt_global_sti();
        hal_reenter_root();
// now make it as if it was scheduled soft, the tail is cared in sys_lxrt.c
	rt_global_cli();
	LOCK_LINUX(cpuid);
	rt_current->state |= RT_SCHED_READY;
	rt_smp_current[cpuid] = rt_current;
        if (rt_current->state != RT_SCHED_READY) {
        	lnxtsk->state = TASK_SOFTREALTIME;
		rt_schedule();
	} else {
		enq_soft_ready_task(rt_current);
	}
}

static RT_TASK *switch_rtai_tasks(RT_TASK *rt_current, RT_TASK *new_task, int cpuid)
{
	if (rt_current->lnxtsk) {
		unsigned long sflags;
#ifdef IPIPE_NOSTACK_FLAG
		ipipe_set_foreign_stack(&rtai_domain);
#endif
		SAVE_LOCK_LINUX(cpuid);
		rt_linux_task.prevp = rt_current;
		save_fpcr_and_enable_fpu(linux_cr0);
		if (new_task->uses_fpu) {
			save_fpenv(rt_linux_task.fpu_reg);
			fpu_task = new_task;
			restore_fpenv(fpu_task->fpu_reg);
		}
		RST_EXEC_TIME();
		SAVE_PREV_TASK();
		rt_exchange_tasks(rt_smp_current[cpuid], new_task);
		restore_fpcr(linux_cr0);
		RESTORE_UNLOCK_LINUX(cpuid);
#ifdef IPIPE_NOSTACK_FLAG
		ipipe_clear_foreign_stack(&rtai_domain);
#endif
		if (rt_linux_task.nextp != rt_current) {
			return rt_linux_task.nextp;
		}
	} else {
		if (new_task->lnxtsk) {
			rt_linux_task.nextp = new_task;
			new_task = rt_linux_task.prevp;
			if (fpu_task != &rt_linux_task) {
				save_fpenv(fpu_task->fpu_reg);
				fpu_task = &rt_linux_task;
				restore_fpenv(fpu_task->fpu_reg);
			}
		} else if (new_task->uses_fpu && fpu_task != new_task) {
			save_fpenv(fpu_task->fpu_reg);
			fpu_task = new_task;
			restore_fpenv(fpu_task->fpu_reg);
		}
		SET_EXEC_TIME();
		SAVE_PREV_TASK();
		rt_exchange_tasks(rt_smp_current[cpuid], new_task);
	}
	RTAI_TASK_SWITCH_SIGNAL();
	return NULL;
}

#define lxrt_context_switch(prev, next, cpuid) \
	do { \
		SAVE_PREV_TASK(); \
		_lxrt_context_switch(prev, next, cpuid); barrier(); \
		RTAI_TASK_SWITCH_SIGNAL(); \
	} while (0)


#ifdef USE_LINUX_TIMER

#define CHECK_LINUX_TIME() \
	if (rt_times.linux_time < rt_times.intr_time) { \
		rt_times.intr_time = rt_times.linux_time; \
		fire_shot = 1; \
		break; \
	}

#define SET_PEND_LINUX_TIMER_SHOT() \
do { \
	if (rt_times.tick_time >= rt_times.linux_time) { \
		if (rt_times.linux_tick > 0) { \
			rt_times.linux_time += rt_times.linux_tick; \
		} else { \
			rt_times.linux_time = RTAI_TIME_LIMIT; \
		} \
		update_linux_timer(cpuid); \
	} \
} while (0)

#else

#define CHECK_LINUX_TIME()

#define SET_PEND_LINUX_TIMER_SHOT()

#endif


#define SET_NEXT_TIMER_SHOT(fire_shot) \
do { \
	fire_shot = 0; \
	prio = new_task->priority; \
	if (CONFIG_RTAI_ALLOW_RR && new_task->policy > 0) { \
		if (new_task->yield_time < rt_times.intr_time) { \
			rt_times.intr_time = new_task->yield_time; \
			fire_shot = 1; \
		} \
        } \
	task = &rt_linux_task; \
	while ((task = task->tnext) != &rt_linux_task && task->resume_time < rt_times.intr_time) { \
		if (task->priority <= prio) { \
			rt_times.intr_time = task->resume_time - task->schedlat; \
			fire_shot = 1; \
			break; \
		} \
	} \
} while (0) 

#define IF_GOING_TO_LINUX_CHECK_TIMER_SHOT(fire_shot) \
do { \
	if (prio == RT_SCHED_LINUX_PRIORITY) { \
		CHECK_LINUX_TIME(); \
		if (!timer_shot_fired) {\
			fire_shot = 1; \
		} \
	} \
} while (0)

static int oneshot_span;
static int satdelay;

#define ONESHOT_DELAY(SHOT_FIRED) \
do { \
	if (!(SHOT_FIRED)) { \
		RTIME span; \
		if (unlikely((span = rt_times.intr_time - rt_time_h) > oneshot_span)) { \
			rt_times.intr_time = rt_time_h + oneshot_span; \
			delay = satdelay; \
		} else { \
			delay = (int)span - tuned.sched_latency; \
		} \
	} else { \
		delay = (int)(rt_times.intr_time - rt_time_h) - tuned.sched_latency; \
	} \
} while (0)

static void rt_timer_handler(void);

#define FIRE_NEXT_TIMER_SHOT(SHOT_FIRED) \
do { \
if (fire_shot) { \
	int delay; \
	ONESHOT_DELAY(SHOT_FIRED); \
	if (delay > tuned.setup_time_TIMER_CPUNIT) { \
		timer_shot_fired = 1; \
		rt_set_timer_delay(delay);\
	} else { \
		timer_shot_fired = -1;\
		rt_times.intr_time = rt_time_h + tuned.setup_time_TIMER_CPUNIT;\
	} \
} \
} while (0)

#define CALL_TIMER_HANDLER() \
	do { \
		if (timer_shot_fired < 0) { \
			timer_shot_fired = 1; \
			rt_timer_handler(); \
		} \
	} while (0)

#define REDO_TIMER_HANDLER() \
	do { \
		if (timer_shot_fired < 0) { \
			timer_shot_fired = 1; \
			goto redo_timer_handler; \
		} \
	} while (0)

#define FIRE_IMMEDIATE_LINUX_TIMER_SHOT() \
do { \
	LOCK_LINUX(cpuid); \
	rt_timer_handler(); \
	UNLOCK_LINUX(cpuid); \
} while (0)

#ifdef CONFIG_SMP
static void rt_schedule_on_schedule_ipi(void)
{
	RT_TASK *rt_current, *task, *new_task;
	int cpuid;

	rt_current = rt_smp_current[cpuid = rtai_cpuid()];

	sched_get_global_lock(cpuid);
	RR_YIELD();
	do {
		int prio, fire_shot;

		rt_time_h = rtai_rdtsc() + rt_half_tick;
		wake_up_timed_tasks(cpuid);
		TASK_TO_SCHEDULE();

		SET_NEXT_TIMER_SHOT(fire_shot);
		sched_release_global_lock(cpuid);
		IF_GOING_TO_LINUX_CHECK_TIMER_SHOT(fire_shot);
		FIRE_NEXT_TIMER_SHOT(timer_shot_fired);
	} while (0);

	if (new_task != rt_current) {
		if (rt_scheduling[cpuid].locked) {
			rt_scheduling[cpuid].rqsted = 1;
			goto sched_exit;
		}
		if (/*USE_RTAI_TASKS && */ (!new_task->lnxtsk || !rt_current->lnxtsk)) {
			if (!(new_task = switch_rtai_tasks(rt_current, new_task, cpuid))) {
				goto sched_exit;
			}
		}
		if (new_task->is_hard > 0 || rt_current->is_hard > 0) {
			struct task_struct *prev;
			unsigned long sflags;
			if (rt_current->is_hard <= 0) {
				SAVE_LOCK_LINUX_IN_IRQ(cpuid);
				rt_linux_task.lnxtsk = prev = current;
				RST_EXEC_TIME();
			} else {
				sflags = rtai_linux_context[cpuid].sflags;
				prev = rt_current->lnxtsk;
				SET_EXEC_TIME();
			}
			rt_smp_current[cpuid] = new_task;
			lxrt_context_switch(prev, new_task->lnxtsk, cpuid);
			if (rt_current->is_hard <= 0) {
				RESTORE_UNLOCK_LINUX_IN_IRQ(cpuid);
			} else if (lnxtsk_uses_fpu(prev)) {
				restore_fpu(prev);
			}
		}
	}
sched_exit:
	CALL_TIMER_HANDLER();
}
#endif

void rt_schedule(void)
{
	RT_TASK *rt_current, *task, *new_task;
	int cpuid;

	rt_current = rt_smp_current[cpuid = rtai_cpuid()];

	RR_YIELD();
	do {
		int prio, fire_shot;

		rt_time_h = rtai_rdtsc() + rt_half_tick;
		wake_up_timed_tasks(cpuid);
		TASK_TO_SCHEDULE();

		SET_NEXT_TIMER_SHOT(fire_shot);
		sched_release_global_lock(cpuid);
		IF_GOING_TO_LINUX_CHECK_TIMER_SHOT(fire_shot);
		FIRE_NEXT_TIMER_SHOT(timer_shot_fired);
	} while (0);

	if (new_task != rt_current) {
		if (rt_scheduling[cpuid].locked) {
			rt_scheduling[cpuid].rqsted = 1;
			goto sched_exit;
		}
		if (/*USE_RTAI_TASKS && */(!new_task->lnxtsk || !rt_current->lnxtsk)) {
			if (!(new_task = switch_rtai_tasks(rt_current, new_task, cpuid))) {
#if /*CONFIG_RTAI_SCHED_LATENCY &&*/ (RTAI_KERN_BUSY_ALIGN_RET_DELAY > 0)
			if (rt_current->busy_time_align) {
				RTIME resume_time = rt_current->resume_time - tuned.kern_latency_busy_align_ret_delay;
				rt_current->busy_time_align = 0;
				while(rtai_rdtsc() < resume_time);
			}
#endif
				goto ksched_exit;
			}
		}
		rt_smp_current[cpuid] = new_task;
		if (new_task->is_hard > 0 || rt_current->is_hard > 0) {
			struct task_struct *prev;
			unsigned long sflags;
			if (rt_current->is_hard <= 0) {
				SAVE_LOCK_LINUX(cpuid);
				rt_linux_task.lnxtsk = prev = current;
				RST_EXEC_TIME();
			} else {
				sflags = rtai_linux_context[cpuid].sflags;
				prev = rt_current->lnxtsk;
				SET_EXEC_TIME();
			}
			lxrt_context_switch(prev, new_task->lnxtsk, cpuid);
			if (rt_current->is_hard <= 0) {
				RESTORE_UNLOCK_LINUX(cpuid);
				if (rt_current->state != RT_SCHED_READY) {
					goto sched_soft;
				}
			} else {
				if (lnxtsk_uses_fpu(prev)) {
					restore_fpu(prev);
				}
				if (rt_current->force_soft) {
					force_current_soft(rt_current, cpuid);
				}
			}
		} else {
sched_soft:
			CALL_TIMER_HANDLER();
			UNLOCK_LINUX(cpuid);
			rtai_sti();

#ifdef CONFIG_RTAI_ALIGN_LINUX_PRIORITY
			if (rtai_tskext(current, TSKEXT0) && (current->policy == SCHED_FIFO || current->policy == SCHED_RR)) {
				int rt_priority;
				if ((rt_priority = rtai_tskext_t(current, TSKEXT0)->priority) >= BASE_SOFT_PRIORITY) {
					rt_priority -= BASE_SOFT_PRIORITY;
				}
				if ((rt_priority = (MAX_LINUX_RTPRIO - rt_priority)) < 1) {
					rt_priority = 1;
				}
				if (rt_priority != current->rt_priority) {
					rtai_set_linux_task_priority(current, current->policy, rt_priority);
				}
			}
#endif

			hal_test_and_fast_flush_pipeline();
			schedule();
			rt_global_cli();
			rt_current->state = (rt_current->state & ~RT_SCHED_SFTRDY) | RT_SCHED_READY;
			LOCK_LINUX(cpuid);
			enq_soft_ready_task(rt_current);
			rt_smp_current[cpuid] = rt_current;
			return;
		}
	}
sched_exit:
#if /*CONFIG_RTAI_SCHED_LATENCY &&*/ (RTAI_USER_BUSY_ALIGN_RET_DELAY > 0)
	if (rt_current->busy_time_align) {
		RTIME resume_time = rt_current->resume_time - tuned.user_latency_busy_align_ret_delay;
		rt_current->busy_time_align = 0;
		while(rtai_rdtsc() < resume_time);
	}
#endif
ksched_exit:
	CALL_TIMER_HANDLER();
	sched_get_global_lock(cpuid);
}

RTAI_SYSCALL_MODE void rt_spv_RMS(int cpuid)
{
	RT_TASK *task;
	int prio;
	if (cpuid < 0 || cpuid >= num_online_cpus()) {
		cpuid = rtai_cpuid();
	}
	prio = 0;
	task = &rt_linux_task;
	while ((task = task->next)) {
		RT_TASK *task, *htask;
		RTIME period;
		htask = 0;
		task = &rt_linux_task;
		period = RTAI_TIME_LIMIT;
		while ((task = task->next)) {
			if (task->priority >= 0 && task->policy >= 0 && task->period && task->period < period) {
				period = (htask = task)->period;
			}
		}
		if (htask) {
			htask->priority = -1;
			htask->base_priority = prio++;
		} else {
			goto ret;
		}
	}
ret:	task = &rt_linux_task;
	while ((task = task->next)) {
		if (task->priority < 0) {
			task->priority = task->base_priority;
		}
	}
	return;
}


void rt_sched_lock(void)
{
	unsigned long flags;
	int cpuid;

	rtai_save_flags_and_cli(flags);
	if (!rt_scheduling[cpuid = rtai_cpuid()].locked++) {
		rt_scheduling[cpuid].rqsted = 0;
	}
	rtai_restore_flags(flags);
}

#define SCHED_UNLOCK_SCHEDULE(cpuid) \
	do { \
		rt_scheduling[cpuid].rqsted = 0; \
		sched_get_global_lock(cpuid); \
		rt_schedule(); \
		sched_release_global_lock(cpuid); \
	} while (0)


void rt_sched_unlock(void)
{
	unsigned long flags;
	int cpuid;

	rtai_save_flags_and_cli(flags);
	if (rt_scheduling[cpuid = rtai_cpuid()].locked && !(--rt_scheduling[cpuid].locked)) {
		if (rt_scheduling[cpuid].rqsted > 0) {
			SCHED_UNLOCK_SCHEDULE(cpuid);
		}
	} else {
//		rt_printk("*** TOO MANY SCHED_UNLOCK ***\n");
	}
	rtai_restore_flags(flags);
}




void *rt_get_lxrt_fun_entry(int index);
static inline void sched_sem_signal(SEM *sem)
{
	((RTAI_SYSCALL_MODE void (*)(SEM *, ...))rt_get_lxrt_fun_entry(SEM_SIGNAL))(sem);
}

int clr_rtext(RT_TASK *task)
{
	DECLARE_RT_CURRENT;
	unsigned long flags;
	QUEUE *q;

	if (task->magic != RT_TASK_MAGIC || task->priority == RT_SCHED_LINUX_PRIORITY) {
		return -EINVAL;
	}

	flags = rt_global_save_flags_and_cli();
	ASSIGN_RT_CURRENT;
	if (!task_owns_sems(task) || task == rt_current || rt_current->priority == RT_SCHED_LINUX_PRIORITY) {
		call_exit_handlers(task);
		rem_timed_task(task);
		if (task->blocked_on) {
			if (task->state & (RT_SCHED_SEMAPHORE | RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_RETURN)) {
				(task->queue.prev)->next = task->queue.next;
				(task->queue.next)->prev = task->queue.prev;
				if (task->state & RT_SCHED_SEMAPHORE) {
					SEM *sem = (SEM *)(task->blocked_on);
					if (++sem->count > 1 && sem->type) {
						sem->count = 1;
					}
				}
			} else if (task->state & RT_SCHED_MBXSUSP) {
				MBX *mbx = (MBX *)task->blocked_on;
				mbx->waiting_task = NULL;
				sched_sem_signal(!mbx->frbs ? &mbx->sndsem : &mbx->rcvsem);
			}
		}
		q = &(task->msg_queue);
		while ((q = q->next) != &(task->msg_queue)) {
			rem_timed_task(q->task);
			if ((q->task)->state != RT_SCHED_READY && ((q->task)->state &= ~(RT_SCHED_SEND | RT_SCHED_RPC | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
				enq_ready_task(q->task);
			}       
			(q->task)->blocked_on = RTP_OBJREM;
		}       
                q = &(task->ret_queue);
                while ((q = q->next) != &(task->ret_queue)) {
			rem_timed_task(q->task);
                       	if ((q->task)->state != RT_SCHED_READY && ((q->task)->state &= ~(RT_SCHED_RETURN | RT_SCHED_DELAYED)) == RT_SCHED_READY) {
				enq_ready_task(q->task);
			}       
			(q->task)->blocked_on = RTP_OBJREM;
               	}
		if (!((task->prev)->next = task->next)) {
			rt_smp_linux_task[task->runnable_on_cpus].prev = task->prev;
		} else {
			(task->next)->prev = task->prev;
		}
		if (rt_smp_fpu_task[task->runnable_on_cpus] == task) {
			rt_smp_fpu_task[task->runnable_on_cpus] = rt_smp_linux_task + task->runnable_on_cpus;;
		}
		if (!task->lnxtsk) {
			frstk_srq.mp[frstk_srq.in++ & (MAX_FRESTK_SRQ - 1)] = task->stack_bottom;
			rt_pend_linux_srq(frstk_srq.srq);
		}
		task->magic = 0;
		rem_ready_task(task);
		task->state = 0;
		atomic_dec((void *)(tasks_per_cpu + task->runnable_on_cpus));
		if (task == rt_current) {
			rt_schedule();
		}
	} else {
		task->suspdepth = -0x7FFFFFFF;
	}
	rt_global_restore_flags(flags);
	return 0;
}


int rt_task_delete(RT_TASK *task)
{
	clr_rtext(task);
	return 0;
}


int rt_get_timer_cpu(void)
{
	return 1;
}


static void rt_timer_handler(void)
{
	RT_TASK *rt_current, *task, *new_task;
	int cpuid;

	DO_TIMER_PROPER_OP();
	rt_current = rt_smp_current[cpuid = rtai_cpuid()];

redo_timer_handler:

	rt_times.tick_time = rtai_rdtsc();
	rt_time_h = rt_times.tick_time + rt_half_tick;
	SET_PEND_LINUX_TIMER_SHOT();

	sched_get_global_lock(cpuid);
	RR_YIELD();
	wake_up_timed_tasks(cpuid);
	TASK_TO_SCHEDULE();

	do {
		int prio, fire_shot;

		timer_shot_fired = 0;
		rt_times.intr_time = RTAI_TIME_LIMIT;

		SET_NEXT_TIMER_SHOT(fire_shot);
		sched_release_global_lock(cpuid);
		IF_GOING_TO_LINUX_CHECK_TIMER_SHOT(fire_shot);
		FIRE_NEXT_TIMER_SHOT(0);
	} while (0);

	if (new_task != rt_current) {
		if (rt_scheduling[cpuid].locked) {
			rt_scheduling[cpuid].rqsted = 1;
			goto sched_exit;
		}
		if (/*USE_RTAI_TASKS && */ (!new_task->lnxtsk || !rt_current->lnxtsk)) {
			if (!(new_task = switch_rtai_tasks(rt_current, new_task, cpuid))) {
				goto sched_exit;
			}
		}
		if (new_task->is_hard > 0 || rt_current->is_hard > 0) {
			struct task_struct *prev;
			unsigned long sflags;
			if (rt_current->is_hard <= 0) {
				SAVE_LOCK_LINUX_IN_IRQ(cpuid);
				rt_linux_task.lnxtsk = prev = current;
				RST_EXEC_TIME();
			} else {
				sflags = rtai_linux_context[cpuid].sflags;
				prev = rt_current->lnxtsk;
				SET_EXEC_TIME();
			}
			rt_smp_current[cpuid] = new_task;
			lxrt_context_switch(prev, new_task->lnxtsk, cpuid);
			if (rt_current->is_hard <= 0) {
				RESTORE_UNLOCK_LINUX_IN_IRQ(cpuid);
			} else if (lnxtsk_uses_fpu(prev)) {
				restore_fpu(prev);
			}
		}
        }
sched_exit:
	REDO_TIMER_HANDLER();
	return;
	goto redo_timer_handler;
}


int rt_is_hard_timer_running(void) 
{ 
	return rt_sched_timed;
}


void rt_set_oneshot_mode(void)
{ 
	return;
}


void rt_set_periodic_mode(void) 
{ 
	return;
}

#include <linux/clockchips.h>
#include <linux/ipipe_tickdev.h>

extern void *rt_linux_hrt_next_shot;

static int _rt_linux_hrt_next_shot(unsigned long deltat, void *hrt_dev) // ??? struct ipipe_tick_device *hrt_dev)
{
	int cpuid = rtai_cpuid();
	unsigned long deltas;
	RTIME linux_time;

	deltat = nano2count(deltat);
	deltas = deltat > (tuned.setup_time_TIMER_CPUNIT + tuned.sched_latency) ? (deltat - tuned.sched_latency) : 0;

	rtai_cli();
	rt_times.linux_time = linux_time = rtai_rdtsc() + deltat;
	do {
		if (linux_time < rt_times.intr_time) {
			if (deltas > 0) {
				rt_times.intr_time = linux_time;
				rt_set_timer_delay(deltas);
				timer_shot_fired = 1;
			} else {
				rt_times.linux_time = RTAI_TIME_LIMIT;
                		update_linux_timer(cpuid);
			}
		}
	} while (0);
	rtai_sti();
	return 0;
}

static void rt_start_timers(void)
{
	unsigned long flags, cpuid;

	rt_request_timers(rt_timer_handler);
	flags = rt_global_save_flags_and_cli();
	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		tuned.timers_tol[cpuid] = rt_half_tick = tuned.sched_latency/2;
		rt_time_h = rt_times.tick_time + rt_half_tick;
		timer_shot_fired = 1;
	}
	rt_sched_timed = 1;
	rt_gettimeorig(NULL);
	rt_global_restore_flags(flags);
}


static void rt_stop_timers(void)
{
	int cpuid; 
	if (rt_sched_timed) {
		rt_sched_timed = 0;
		rt_free_timers();
		for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
			rt_time_h = RTAI_TIME_LIMIT;
		}
	}
}

RTAI_SYSCALL_MODE void start_rt_apic_timers(struct apic_timer_setup_data *setup_data, unsigned int rcvr_jiffies_cpuid) { return; }


RTAI_SYSCALL_MODE RTIME start_rt_timer(int period) { return period; }


void stop_rt_timer(void) { return; }


RTAI_SYSCALL_MODE int rt_hard_timer_tick_count(void) { return 0; }


RTAI_SYSCALL_MODE int rt_hard_timer_tick_count_cpuid(int cpuid) { return 0; }


RT_TRAP_HANDLER rt_set_task_trap_handler( RT_TASK *task, unsigned int vec, RT_TRAP_HANDLER handler)
{
	RT_TRAP_HANDLER old_handler;

	if (!task || (vec >= RTAI_NR_TRAPS)) {
		return (RT_TRAP_HANDLER) -EINVAL;
	}
	old_handler = task->task_trap_handler[vec];
	task->task_trap_handler[vec] = handler;
	return old_handler;
}

static int Latency = 0;
RTAI_MODULE_PARM(Latency, int);

static int SetupTimeTIMER;

extern void krtai_objects_release(void);

static void frstk_srq_handler(void)
{
        while (frstk_srq.out != frstk_srq.in) {
		rt_kstack_free(frstk_srq.mp[frstk_srq.out++ & (MAX_FRESTK_SRQ - 1)]);
	}
}

static void nihil(void) { };
struct rt_fun_entry rt_fun_lxrt[MAX_LXRT_FUN];

void reset_rt_fun_entries(struct rt_native_fun_entry *entry)
{
	while (entry->fun.fun) {
		if (entry->index >= MAX_LXRT_FUN) {
			rt_printk("*** RESET ENTRY %d FOR USER SPACE CALLS EXCEEDS ALLOWD TABLE SIZE %d, NOT USED ***\n", entry->index, MAX_LXRT_FUN);
		} else {
			rt_fun_lxrt[entry->index] = (struct rt_fun_entry){ 1, nihil };
		}
		entry++;
        }
}

int set_rt_fun_entries(struct rt_native_fun_entry *entry)
{
	int error;
	error = 0;
	while (entry->fun.fun) {
		if (rt_fun_lxrt[entry->index].fun != nihil) {
			rt_printk("*** SUSPICIOUS ENTRY ASSIGNEMENT FOR USER SPACE CALL AT %d, DUPLICATED INDEX OR REPEATED INITIALIZATION ***\n", entry->index);
			error = -1;
		} else if (entry->index >= MAX_LXRT_FUN) {
			rt_printk("*** ASSIGNEMENT ENTRY %d FOR USER SPACE CALLS EXCEEDS ALLOWED TABLE SIZE %d, NOT USED ***\n", entry->index, MAX_LXRT_FUN);
			error = -1;
		} else {
			rt_fun_lxrt[entry->index] = entry->fun;
		}
		entry++;
        }
	if (error) {
		reset_rt_fun_entries(entry);
	}
	return 0;
}

void *rt_get_lxrt_fun_entry(int index) {
	return rt_fun_lxrt[index].fun;
}

static void lxrt_killall (void)
{
	int cpuid;
	
	rt_stop_timers();
	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		while (rt_linux_task.next) {
			rt_task_delete(rt_linux_task.next);
		}
	}
}

static int lxrt_notify_reboot (struct notifier_block *nb, unsigned long event, void *p)
{
	switch (event) {
		case SYS_DOWN:
		case SYS_HALT:
		case SYS_POWER_OFF:
		/* FIXME: this is far too late. */
		printk("LXRT: REBOOT NOTIFIED -- KILLING TASKS\n");
		lxrt_killall();
	}
	return NOTIFY_DONE;
}

/* ++++++++++++++++++++++++++ TIME CONVERSIONS +++++++++++++++++++++++++++++ */

RTAI_SYSCALL_MODE RTIME count2nano(RTIME counts)
{
	return (counts >= 0 ? rtai_llimd(counts, 1000000000, tuned.clock_freq) : -rtai_llimd(-counts, 1000000000, tuned.clock_freq));
}


RTAI_SYSCALL_MODE RTIME nano2count(RTIME ns)
{
	return (ns >= 0 ? rtai_llimd(ns, tuned.clock_freq, 1000000000) : -rtai_llimd(-ns, tuned.clock_freq, 1000000000));
}

RTAI_SYSCALL_MODE RTIME count2nano_cpuid(RTIME counts, unsigned int cpuid)
{
	return (counts >= 0 ? rtai_llimd(counts, 1000000000, tuned.clock_freq) : -rtai_llimd(-counts, 1000000000, tuned.clock_freq));
}


RTAI_SYSCALL_MODE RTIME nano2count_cpuid(RTIME ns, unsigned int cpuid)
{
	return (ns >= 0 ? rtai_llimd(ns, tuned.clock_freq, 1000000000) : -rtai_llimd(-ns, tuned.clock_freq, 1000000000));
}

/* +++++++++++++++++++++++++++++++ TIMINGS ++++++++++++++++++++++++++++++++++ */

RTIME rt_get_time(void)
{
	return rtai_rdtsc();
}

RTAI_SYSCALL_MODE RTIME rt_get_time_cpuid(unsigned int cpuid)
{
	return rtai_rdtsc();
}

RTIME rt_get_time_ns(void)
{
	return rtai_llimd(rtai_rdtsc(), 1000000000, tuned.clock_freq);
}

RTAI_SYSCALL_MODE RTIME rt_get_time_ns_cpuid(unsigned int cpuid)
{
	return rtai_llimd(rtai_rdtsc(), 1000000000, tuned.clock_freq);
}

RTIME rt_get_cpu_time_ns(void)
{
	return rtai_llimd(rtai_rdtsc(), 1000000000, tuned.clock_freq);
}

extern struct epoch_struct boot_epoch;

RTIME rt_get_real_time(void)
{
	return boot_epoch.time[boot_epoch.touse][0] + rtai_rdtsc();
}

RTIME rt_get_real_time_ns(void)
{
	return boot_epoch.time[boot_epoch.touse][1] + rtai_llimd(rtai_rdtsc(), 1000000000, tuned.clock_freq);
}

/* +++++++++++++++++++++++++++ SECRET BACK DOORS ++++++++++++++++++++++++++++ */

RT_TASK *rt_get_base_linux_task(RT_TASK **base_linux_tasks)
{
        int cpuid;
        for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
                base_linux_tasks[cpuid] = rt_smp_linux_task + cpuid;
        }
        return rt_smp_linux_task;
}

RT_TASK *rt_alloc_dynamic_task(void)
{
#ifdef CONFIG_RTAI_MALLOC
        return rt_malloc(sizeof(RT_TASK)); // For VC's, proxies and C++ support.
#else
	return NULL;
#endif
}

/* +++++++++++++++ SUPPORT FOR LINUX TASKS AND KERNEL THREADS +++++++++++++++ */

//#define ECHO_SYSW
#ifdef ECHO_SYSW
#define SYSW_DIAG_MSG(x) x
#else
#define SYSW_DIAG_MSG(x)
#endif

static RT_TRAP_HANDLER lxrt_old_trap_handler;

static inline void _rt_schedule_soft_tail(RT_TASK *rt_task, int cpuid)
{
	rt_global_cli();
	rt_task->state &= ~(RT_SCHED_READY | RT_SCHED_SFTRDY);
	(rt_task->rprev)->rnext = rt_task->rnext;
	(rt_task->rnext)->rprev = rt_task->rprev;
	rt_smp_current[cpuid] = &rt_linux_task;
	rt_schedule();
	UNLOCK_LINUX(cpuid);
	rt_global_sti();

#ifdef CONFIG_RTAI_ALIGN_LINUX_PRIORITY
do {
	int rt_priority;
	struct task_struct *lnxtsk;

	if ((lnxtsk = rt_task->lnxtsk)->policy == SCHED_FIFO || lnxtsk->policy == SCHED_RR) {
		if ((rt_priority = rt_task->priority) >= BASE_SOFT_PRIORITY) {
			rt_priority -= BASE_SOFT_PRIORITY;
		}
		if ((rt_priority = (MAX_LINUX_RTPRIO - rt_priority)) < 1) {
			rt_priority = 1;
		}
		if (rt_priority != lnxtsk->rt_priority) {
			rtai_set_linux_task_priority(lnxtsk, lnxtsk->policy, rt_priority);
		}
	}
} while (0);
#endif
}

void rt_schedule_soft(RT_TASK *rt_task)
{
	struct fun_args *funarg;
	int cpuid;

	rt_global_cli();
	rt_task->state |= RT_SCHED_READY;
	while (rt_task->state != RT_SCHED_READY) {
		current->state = TASK_SOFTREALTIME;
		rt_global_sti();
		schedule();
		rt_global_cli();
	}
	cpuid = rt_task->runnable_on_cpus;
	LOCK_LINUX(cpuid);
	enq_soft_ready_task(rt_task);
	rt_smp_current[cpuid] = rt_task;
	rt_global_sti();
	funarg = (void *)rt_task->fun_args;
	rt_task->retval = funarg->fun(RTAI_FUNARGS);
	_rt_schedule_soft_tail(rt_task, cpuid);
}

void rt_schedule_soft_tail(RT_TASK *rt_task, int cpuid)
{
	_rt_schedule_soft_tail(rt_task, cpuid);
}

#include <linux/kthread.h>

//#define PERCPU_ACTIVE_MM
#ifdef PERCPU_ACTIVE_MM
struct mm_struct *rtai_active_mm[RTAI_NR_CPUS];

static void rtai_fun_set_active_mm(int cpuid)
{
        put_current_on_cpu(cpuid);
        atomic_inc(&current->active_mm->mm_count);
        rtai_active_mm[cpuid] = current->active_mm;
}

#define rtai_set_active_mm() \
	do { \
		for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) { \
			kthread_run((void *)rtai_fun_set_active_mm, (void *)cpuid, "RTAI_ACTIVE_MM"); \
		} \
	} while (0)	

#define rtai_drop_active_mm() \
	do { \
		int cpuid; \
		for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) { \
			mmdrop(rtai_active_mm[cpuid]); \
		} \
	} while (0)

#define rt_set_active_mm() \
	do { \
		if (!task->mm) task->active_mm = rtai_active_mm[cpuid]; \
	} while (0)

#define rt_drop_active_mm() \
	do { } while (0)
#endif

//#define SINGLE_ACTIVE_MM
#ifdef SINGLE_ACTIVE_MM
struct mm_struct *rtai_active_mm;

#define rtai_set_active_mm() \
	do { \
	        atomic_inc(&current->active_mm->mm_count); \
        	rtai_active_mm = current->active_mm; \
	} while (0)

#define rtai_drop_active_mm() \
	do { mmdrop(rtai_active_mm); } while (0)

#define rt_set_active_mm() \
	do { \
		if (!task->mm) task->active_mm = rtai_active_mm; \
	} while (0)

#define rt_drop_active_mm() \
	do { } while (0)
#endif

#define ONTHEFLY_ACTIVE_MM
#ifdef ONTHEFLY_ACTIVE_MM
#define rtai_set_active_mm() \
	do { } while (0)

#define rtai_drop_active_mm() \
	do { } while (0)

#define rt_set_active_mm() \
	do { \
		if (!task->mm) { \
			atomic_inc(&lnxtsk->active_mm->mm_count); \
			task->active_mm = lnxtsk->active_mm; \
		} \
	} while (0)

#define rt_drop_active_mm() \
	do { if (!lnxtsk->mm) mmdrop(active_mm); } while (0)
#endif

static inline void fast_schedule(struct task_struct *task)
{
	RT_TASK *new_task = rtai_tskext_t(task, TSKEXT0);
	struct task_struct *lnxtsk = current;
	int cpuid = new_task->runnable_on_cpus;
	RT_TASK *rt_current;

	rt_global_cli();
	new_task->state |= RT_SCHED_READY;
	enq_soft_ready_task(new_task);
	new_task->running = 1;
	sched_release_global_lock(cpuid);
	LOCK_LINUX(cpuid);
	(rt_current = &rt_linux_task)->lnxtsk = lnxtsk;
	SET_EXEC_TIME();
	rt_smp_current[cpuid] = new_task;
	rt_set_active_mm();
	lxrt_context_switch(lnxtsk, task, cpuid);
	CALL_TIMER_HANDLER();
	UNLOCK_LINUX(cpuid);
	rtai_sti();
}

#define SERIALIZE_STEAL_FROM_LINUX

#ifdef SERIALIZE_STEAL_FROM_LINUX

static struct sthsem { struct rt_queue queue; int count; } sthsems[RTAI_NR_CPUS];

static void rtai_init_sthsems(void)
{
	int i;
	for (i = 0; i < RTAI_NR_CPUS; i++) {
		sthsems[i].count = 1;
        	sthsems[i].queue.task = NULL;
	        sthsems[i].queue.prev = sthsems[i].queue.next = &sthsems[i].queue;
	}
}

static RTAI_SYSCALL_MODE void __sthsem_wait(RT_TASK *task, struct sthsem *sem)
{
	rt_global_cli();
	if (--sem->count < 0) {
		task->state |= RT_SCHED_SEMAPHORE;
		NON_RTAI_TASK_SUSPEND(task);
		(task->rprev)->rnext = task->rnext;
		(task->rnext)->rprev = task->rprev;
		enqueue_blocked(task, &sem->queue, 0);
		rt_schedule();
	}
	rt_global_sti();
	return;
}

static inline void sthsem_wait(RT_TASK *task, struct sthsem *sem)
{
	task->fun_args[0] = (unsigned long)task;
	task->fun_args[1] = (unsigned long)sem;
	((struct fun_args *)task->fun_args)->fun = (void *)__sthsem_wait;
	rt_schedule_soft(task);
	return;
}

static inline void sthsem_signal(struct sthsem *sem)
{
	RT_TASK *task;

	rt_global_cli();
	if ((task = (sem->queue.next)->task)) {
		sem->count++;
		dequeue_blocked(task);
		if (task->state != RT_SCHED_READY && (task->state &= ~RT_SCHED_SEMAPHORE) == RT_SCHED_READY) {
			task->state |= RT_SCHED_SFTRDY;
			NON_RTAI_TASK_RESUME(task);
			rt_schedule();
		}
	} else {
		sem->count = 1;
	}
	rt_global_sti();
}

#else

static void rtai_init_sthsems(void) { }

#endif

void steal_from_linux(RT_TASK *rt_task)
{
	struct task_struct *lnxtsk;
#ifdef SERIALIZE_STEAL_FROM_LINUX
	struct sthsem *sem = &sthsems[rt_task->runnable_on_cpus];

	sthsem_wait(rt_task, sem);
#endif
	if (signal_pending(rt_task->lnxtsk)) {
		rt_task->is_hard = -1;
		return;
	}
	if (rt_task->base_priority >= BASE_SOFT_PRIORITY) {
		rt_task->base_priority -= BASE_SOFT_PRIORITY;
	}
	if (rt_task->priority >= BASE_SOFT_PRIORITY) {
		rt_task->priority -= BASE_SOFT_PRIORITY;
	}
	rt_task->is_hard = 1;
	__ipipe_migrate_head();
#if CONFIG_RTAI_MONITOR_EXECTIME
	if (!rt_task->exectime[1]) {
		rt_task->exectime[1] = rtai_rdtsc();
	}
#endif
	(lnxtsk = rt_task->lnxtsk)->state = TASK_HARDREALTIME;
	if (lnxtsk_uses_fpu(lnxtsk)) {
		rtai_cli();
		restore_fpu(lnxtsk);
	}
	rtai_sti();
#ifdef SERIALIZE_STEAL_FROM_LINUX
	sthsem_signal(sem);
#endif
}

#ifndef set_task_state
#define set_task_state(tsk, state_value) \
	do { smp_store_mb((tsk)->state, (state_value));	} while (0)
#endif

void give_back_to_linux(RT_TASK *rt_task, int keeprio)
{
	struct task_struct *lnxtsk;
	struct mm_struct *active_mm;
	int rt_priority;

	rt_global_cli();
	(rt_task->rprev)->rnext = rt_task->rnext;
	(rt_task->rnext)->rprev = rt_task->rprev;
	rt_task->state = 0;
	pend_wake_up_hts(lnxtsk = rt_task->lnxtsk, rt_task->runnable_on_cpus);
	active_mm = lnxtsk->active_mm;
#ifdef TASK_NOWAKEUP
	set_task_state(lnxtsk, lnxtsk->state & ~TASK_NOWAKEUP);
#endif 
	rt_schedule();
	if (!(rt_task->is_hard = keeprio)) {
		if (rt_task->priority < BASE_SOFT_PRIORITY) {
			rt_priority = rt_task->priority;
			if (rt_task->priority == rt_task->base_priority) {
				rt_task->priority += BASE_SOFT_PRIORITY;
			}
		} else {
			rt_priority = rt_task->priority - BASE_SOFT_PRIORITY;
		}
		if (rt_task->base_priority < BASE_SOFT_PRIORITY) {
			rt_task->base_priority += BASE_SOFT_PRIORITY;
		}
	} else {
		if (rt_task->priority < BASE_SOFT_PRIORITY) {
			rt_priority = rt_task->priority;
		} else {
			rt_priority = rt_task->priority - BASE_SOFT_PRIORITY;
		}
	}
	rt_global_sti();
	/* Perform Linux's scheduling tail now since we woke up
	   outside the regular schedule() point. */
	hal_reenter_root();
	rt_drop_active_mm();

#ifdef CONFIG_RTAI_ALIGN_LINUX_PRIORITY
	if (lnxtsk->policy == SCHED_FIFO || lnxtsk->policy == SCHED_RR) {
		if ((rt_priority = (MAX_LINUX_RTPRIO - rt_priority)) < 1) {
			rt_priority = 1;
		}
		if (rt_priority != lnxtsk->rt_priority) {
			rtai_set_linux_task_priority(lnxtsk, lnxtsk->policy, rt_priority);
		}
	}
#endif
	return;
}

#define WAKE_UP_TASKs(klist) \
do { \
	struct klist_t *p = &klist[cpuid]; \
	while (p->out != p->in) { \
		wake_up_process(p->task[p->out++ & (MAX_WAKEUP_SRQ - 1)]); \
	} \
} while (0)

static void wake_up_srq_handler(unsigned srq)
{
	int cpuid = rtai_cpuid();
	WAKE_UP_TASKs(wake_up_hts);
	WAKE_UP_TASKs(wake_up_srq);
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,13,0)
	set_tsk_need_resched(current);
#else
	set_need_resched();
#endif
}

static unsigned long traptrans, systrans, linux_srv_calls;

static int lxrt_handle_trap(int vec, int signo, struct pt_regs *regs, void *dummy_data)
{
	RT_TASK *rt_task;

	rt_task = rt_smp_current[rtai_cpuid()];
	if ((/*USE_RTAI_TASKS && */!rt_task->lnxtsk) /*|| (rt_task->lnxtsk)->comm[0] == HARD_KTHREAD_IN_USE*/) {
		if (rt_task->task_trap_handler[vec]) {
			return rt_task->task_trap_handler[vec](vec, signo, regs, rt_task);
		}
		rt_printk("Default Trap Handler: vector %d: Suspend RT task %p\n", vec, rt_task);
		rt_task_suspend(rt_task);
		return 1;
	}

	if (rt_task->is_hard > 0) {
		if (!traptrans++) {
			rt_printk("\nLXRT CHANGED MODE (TRAP), PID = %d, NAME = %s, VEC = %d, SIGNO = %d.\n", (rt_task->lnxtsk)->pid, (rt_task->lnxtsk)->comm, vec, signo);
		}
		SYSW_DIAG_MSG(rt_printk("\n%d: FORCING IT SOFT (TRAP), PID = %d, NAME = %s, VEC = %d, SIGNO = %d.\n", traptrans, (rt_task->lnxtsk)->pid, (rt_task->lnxtsk)->comm, vec, signo););
		give_back_to_linux(rt_task, -1);
		SYSW_DIAG_MSG(rt_printk("%d:  FORCED IT SOFT (TRAP), PID = %d, NAME = %s, VEC = %d, SIGNO = %d.\n", traptrans, (rt_task->lnxtsk)->pid, (rt_task->lnxtsk)->comm, vec, signo););
	}

	return 0;
}


int _rt_task_masked_unblock(RT_TASK *, unsigned long);
struct sig_wakeup_t { struct task_struct *task; };
static int lxrt_intercept_sig_wakeup(struct task_struct *lnxtsk)
{
	RT_TASK *task;
	if ((task = rtai_tskext_t(lnxtsk, TSKEXT0))) {
		rt_global_cli();
		task->unblocked = 1;
		if (task->state && task->state != RT_SCHED_READY) {
			_rt_task_masked_unblock(task, ~RT_SCHED_READY);
		}
		rt_global_sti();
	}
	return 0;
}

static int lxrt_intercept_exit(struct task_struct *lnxtsk)
{
	extern void linux_process_termination(void);
	RT_TASK *task;
	if ((task = rtai_tskext_t(lnxtsk, TSKEXT0))) {
		if (task->is_hard > 0) {
			give_back_to_linux(task, 0);
		}
		linux_process_termination();
	}
	return 0;
}

static int lxrt_intercept_kevents(int kevent, void *data)
{
	switch (kevent) {
		case IPIPE_KEVT_SCHEDULE:
			return 0;
		case IPIPE_KEVT_SIGWAKE:
                	return lxrt_intercept_sig_wakeup(data);
		case IPIPE_KEVT_SETSCHED:
		case IPIPE_KEVT_SETAFFINITY:
			return 0;
		case IPIPE_KEVT_EXIT:
			return lxrt_intercept_exit(data);
		case IPIPE_KEVT_CLEANUP:
		case IPIPE_KEVT_HOSTRT:
		default:
			return 0;
        }
	return 0;
}

extern long long rtai_lxrt_invoke (unsigned long, void *, RT_TASK *);

static int lxrt_intercept_linux_syscall(struct pt_regs *regs, RT_TASK *task)
{
	if (task) {
		if (task->is_hard > 0) {
			if (task->linux_syscall_server) {
				if (!linux_srv_calls++) {
					SYSW_DIAG_MSG(rt_printk("\nFIRST SYSCALL TO LINUX SERVER, PID = %d, NAME = %s, SYSCALL = %d.\n", (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR););
				}
				SYSW_DIAG_MSG(rt_printk("\n%d: SYSCALL TO LINUX SERVER, PID = %d, NAME = %s, SYSCALL = %d.\n", linux_srv_calls, (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR););
				rt_exec_linux_syscall(task, ((RT_TASK *)task->linux_syscall_server)->linux_syscall_server, regs);
				SYSW_DIAG_MSG(rt_printk("%d:     LINUX SERVER DID IT, PID = %d, NAME = %s, SYSCALL = %d.\n", linux_srv_calls, (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR););
				return 1;
			}
			if (!systrans++) {
				rt_printk("\nLXRT CHANGED MODE (SYSCALL), PID = %d, NAME = %s, SYSCALL = %lu.\n", (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR);
			}
			SYSW_DIAG_MSG(rt_printk("\n%d: FORCING IT SOFT (SYSCALL), PID = %d, NAME = %s, SYSCALL = %d.\n", systrans, (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR););
			give_back_to_linux(task, -1);
			SYSW_DIAG_MSG(rt_printk("%d:  FORCED IT SOFT (SYSCALL), PID = %d, NAME = %s, SYSCALL = %d.\n", systrans, (task->lnxtsk)->pid, (task->lnxtsk)->comm, regs->LINUX_SYSCALL_NR););
		}
	}
	hal_test_and_fast_flush_pipeline();
	return 0;
}

extern long long rtai_usrq_dispatcher (unsigned long, unsigned long);

static int lxrt_intercept_syscall(struct pt_regs *regs)
{
	RT_TASK *task;
	if (likely(regs->LINUX_SYSCALL_NR >= RTAI_SYSCALL_NR)) {
		unsigned long srq  = regs->LINUX_SYSCALL_REG1;
		if ((task = rtai_tskext_t(current, TSKEXT0)) && unlikely(task->unblocked)) {
			if (task->is_hard > 0) {
				give_back_to_linux(task, -1);
			}
			task->unblocked = 0;
#ifdef CONFIG_RTAI_USE_STACK_ARGS
			*((long *)regs->LINUX_SYSCALL_REG4) = 1L;
#else
			rt_put_user(1L, (long *)regs->LINUX_SYSCALL_REG4);
#endif
		} else {
			long long retval;
			if (srq > RTAI_NR_SRQS) {
				retval = rtai_lxrt_invoke(srq, (void *)regs->LINUX_SYSCALL_REG2, task);
			} else {
				retval = rtai_usrq_dispatcher(srq, regs->LINUX_SYSCALL_REG2);
			}
#ifdef CONFIG_RTAI_USE_STACK_ARGS
			*((long long *)regs->LINUX_SYSCALL_REG3) = retval;
#else
			rt_put_user(retval, (long long *)regs->LINUX_SYSCALL_REG3);
#endif
		}
		if (!in_hrt_mode(rtai_cpuid())) {
			hal_test_and_fast_flush_pipeline();
			return 0;
		}
		return 1;
	}
	return lxrt_intercept_linux_syscall(regs, rtai_tskext_t(current, TSKEXT0));
}

static int lxrt_intercept_fastcall(struct pt_regs *regs)
{
	long long retval;
	unsigned long srq  = regs->LINUX_SYSCALL_REG1;
	if (srq > RTAI_NR_SRQS) {
		retval = rtai_lxrt_invoke(srq, (void *)regs->LINUX_SYSCALL_REG2, rtai_tskext_t(current, TSKEXT0));
	} else {
		retval = rtai_usrq_dispatcher(srq, regs->LINUX_SYSCALL_REG2);
	}
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	*((long long *)regs->LINUX_SYSCALL_REG3) = retval;
#else
	rt_put_user(retval, (long long *)regs->LINUX_SYSCALL_REG3);
#endif
	if (!in_hrt_mode(rtai_cpuid())) {
		hal_test_and_fast_flush_pipeline();
		return 0;
	}
	return 1;
}


/* ++++++++++++++++++++++++++ SCHEDULER PROC FILE +++++++++++++++++++++++++++ */
/* -----------------------< proc filesystem section >-------------------------*/

extern int rtai_global_heap_size;

#ifdef CONFIG_RTAI_USE_TLSF
#define RTAI_USES_TLSF  1
extern unsigned long tlsf_get_used_size(rtheap_t *);
#define rt_get_heap_mem_used(heap)  tlsf_get_used_size(heap)
#else
#define RTAI_USES_TLSF  0
#define rt_get_heap_mem_used(heap)  rtheap_used_mem(heap)
#endif

static int PROC_READ_FUN(rtai_read_sched)
{
        int cpuid, i = 1;
	unsigned long t;
	RT_TASK *task;
	PROC_PRINT_VARS;

	PROC_PRINT("\nRTAI LXRT Real Time Task Scheduler.\n\n");
	PROC_PRINT("    Calibrated Time Base Frequency: %lu Hz\n", tuned.clock_freq);
	PROC_PRINT("    Calibrated user space latency: %d ns\n", (int)rtai_imuldiv(UserLatency, 1000000000, tuned.clock_freq));
	PROC_PRINT("    Calibrated kernel space latency: %d ns\n", (int)rtai_imuldiv(KernelLatency, 1000000000, tuned.clock_freq));
	PROC_PRINT("    Calibrated oneshot timer setup_to_firing time: %d ns\n\n",
                  (int)rtai_imuldiv(tuned.setup_time_TIMER_CPUNIT, 1000000000, tuned.clock_freq));
	PROC_PRINT("Number of RT CPUs in system: %d (sized for %d)\n\n", num_online_cpus(), RTAI_NR_CPUS);
 
	PROC_PRINT("\n\n");

	PROC_PRINT("Global heap: size = %10d, used = %10lu; <%s>.\n", rtai_global_heap_size, rt_get_heap_mem_used(&rtai_global_heap), RTAI_USES_TLSF ? "TLSF" : "BSD");

	PROC_PRINT("Kstack heap: size = %10d, used = %10lu; <%s>.\n\n", rtai_kstack_heap_size, rt_get_heap_mem_used(&rtai_kstack_heap), RTAI_USES_TLSF ? "TLSF" : "BSD");

	PROC_PRINT("Number of forced hard/soft/hard transitions: traps %lu, syscalls %lu\n\n", traptrans, systrans);

	PROC_PRINT("Priority  Period(ns)  FPU  Sig  State  CPU  Task  HD/SF  PID  RT_TASK *  TIME\n" );
	PROC_PRINT("------------------------------------------------------------------------------\n" );
        for (cpuid = 0; cpuid < num_online_cpus(); cpuid++) {
                task = &rt_linux_task;
/*
* Display all the active RT tasks and their state.
*
* Note: As a temporary hack the tasks are given an id which is
*       the order they appear in the task list, needs fixing!
*/
		while ((task = task->next)) {
/*
* The display for the task period is set to an integer (%d) as 64 bit
* numbers are not currently handled correctly by the kernel routines.
* Hence the period display will be wrong for time periods > ~4 secs.
*/
			t = 0;
			if ((!task->lnxtsk || task->is_hard) && task->exectime[1]) {
				unsigned long den = (unsigned long)rtai_llimd(rtai_rdtsc() - task->exectime[1], 10, tuned.clock_freq);
				if (den) {
					t = 1000UL*(unsigned long)rtai_llimd(task->exectime[0], 10, tuned.clock_freq)/den;
				}				
			}
			PROC_PRINT("%-10d %-11lu %-4s %-3s 0x%-3x  %1lu:%1lu   %-4d   %-4d %-4d  %p   %-lu\n",
                               task->priority,
                               (unsigned long)count2nano_cpuid(task->period, task->runnable_on_cpus),
                               task->uses_fpu || task->lnxtsk ? "Yes" : "No",
                               task->signal ? "Yes" : "No",
                               task->state,
			       task->runnable_on_cpus, // cpuid,
			       task->lnxtsk ? CPUMASK((task->lnxtsk)->cpus_allowed) : (1 << task->runnable_on_cpus),
                               i,
			       task->is_hard,
			       task->lnxtsk ? task->lnxtsk->pid : 0,
			       task, t);
			i++;
                } /* End while loop - display all RT tasks on a CPU. */

		PROC_PRINT("TIMED\n");
		task = &rt_linux_task;
		while ((task = task->tnext) != &rt_linux_task) {
			PROC_PRINT("> %p ", task);
		}
		PROC_PRINT("\nREADY\n");
		task = &rt_linux_task;
		while ((task = task->rnext) != &rt_linux_task) {
			PROC_PRINT("> %p ", task);
		}

        }  /* End for loop - display RT tasks on all CPUs. */

	PROC_PRINT_DONE;

}  /* End function - rtai_read_sched */

PROC_READ_OPEN_OPS(rtai_sched_proc_fops, rtai_read_sched);

static int rtai_proc_sched_register(void) 
{
        struct proc_dir_entry *proc_sched_ent;


        proc_sched_ent = CREATE_PROC_ENTRY("scheduler", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root, &rtai_sched_proc_fops);
        if (!proc_sched_ent) {
                printk("Unable to initialize /proc/rtai/scheduler\n");
                return(-1);
        }
	SET_PROC_READ_ENTRY(proc_sched_ent, rtai_read_sched);
        return(0);
}  /* End function - rtai_proc_sched_register */


static void rtai_proc_sched_unregister(void) 
{
        remove_proc_entry("scheduler", rtai_proc_root);
}  /* End function - rtai_proc_sched_unregister */

/* --------------------< end of proc filesystem section >---------------------*/

/* ++++++++++++++ SCHEDULER ENTRIES AND RELATED INITIALISATION ++++++++++++++ */

static int rt_gettid(void)
{
	return current->pid;
}

RTAI_SYSCALL_MODE int rt_task_get_info_user(RT_TASK *, RT_TASK_INFO *);

static struct rt_native_fun_entry rt_sched_entries[] = {
	{ { 0, rt_set_runnable_on_cpus },	    SET_RUNNABLE_ON_CPUS },
	{ { 0, rt_set_runnable_on_cpuid },	    SET_RUNNABLE_ON_CPUID },
	{ { 0, rt_set_sched_policy },		    SET_SCHED_POLICY },
	{ { 0, rt_get_timer_cpu },		    GET_TIMER_CPU },
	{ { 0, rt_is_hard_timer_running },	    HARD_TIMER_RUNNING },
	{ { 0, rt_set_periodic_mode },		    SET_PERIODIC_MODE },
	{ { 0, rt_set_oneshot_mode },		    SET_ONESHOT_MODE },
	{ { 0, start_rt_timer },		    START_TIMER },
	{ { 0, start_rt_apic_timers },		    START_RT_APIC_TIMERS },
	{ { 0, stop_rt_timer },			    STOP_TIMER },
	{ { 0, rt_task_signal_handler },	    SIGNAL_HANDLER  },
	{ { 0, rt_task_use_fpu },		    TASK_USE_FPU },
	{ { 0, rt_hard_timer_tick_count },	    HARD_TIMER_COUNT },
	{ { 0, rt_hard_timer_tick_count_cpuid },    HARD_TIMER_COUNT_CPUID },
	{ { 0, count2nano },			    COUNT2NANO },
	{ { 0, nano2count },			    NANO2COUNT },
	{ { 0, count2nano_cpuid },		    COUNT2NANO_CPUID },
	{ { 0, nano2count_cpuid },		    NANO2COUNT_CPUID },
	{ { 0, rt_get_time },			    GET_TIME },
	{ { 0, rt_get_time_cpuid },		    GET_TIME_CPUID },
	{ { 0, rt_get_time_ns },		    GET_TIME_NS },
	{ { 0, rt_get_time_ns_cpuid },		    GET_TIME_NS_CPUID },
	{ { 0, rt_get_cpu_time_ns },		    GET_CPU_TIME_NS },
	{ { 0, rt_task_get_info_user },		    GET_TASK_INFO },
	{ { 0, rt_spv_RMS },			    SPV_RMS },
	{ { 1, rt_change_prio },		    CHANGE_TASK_PRIO },
	{ { 0, rt_sched_lock },			    SCHED_LOCK },
	{ { 0, rt_sched_unlock },		    SCHED_UNLOCK },
	{ { 1, rt_task_yield },			    YIELD },  
	{ { 1, rt_task_suspend },		    SUSPEND },
	{ { 1, rt_task_suspend_if },		    SUSPEND_IF },
	{ { 1, rt_task_suspend_until },		    SUSPEND_UNTIL },
	{ { 1, rt_task_suspend_timed },		    SUSPEND_TIMED },
	{ { 1, rt_task_resume },		    RESUME },
	{ { 1, rt_set_linux_syscall_mode },	    SET_LINUX_SYSCALL_MODE },
	{ { 1, rt_task_make_periodic_relative_ns }, MAKE_PERIODIC_NS },
	{ { 1, rt_task_make_periodic },		    MAKE_PERIODIC },
	{ { 1, rt_task_set_resume_end_times },	    SET_RESUME_END },
	{ { 0, rt_set_resume_time },  		    SET_RESUME_TIME },
	{ { 0, rt_set_period },			    SET_PERIOD },
	{ { 1, rt_task_wait_period },		    WAIT_PERIOD },
	{ { 0, rt_busy_sleep },			    BUSY_SLEEP },
	{ { 1, rt_sleep },			    SLEEP },
	{ { 1, rt_sleep_until },		    SLEEP_UNTIL },
	{ { 0, rt_task_masked_unblock },	    WAKEUP_SLEEPING },
	{ { 0, rt_named_task_init },		    NAMED_TASK_INIT },  
	{ { 0, rt_named_task_init_cpuid },	    NAMED_TASK_INIT_CPUID },  
	{ { 0, rt_named_task_delete },	 	    NAMED_TASK_DELETE },  
	{ { 0, rt_get_name },			    GET_NAME },
	{ { 0, rt_get_adr },			    GET_ADR },
	{ { 0, usr_rt_pend_linux_irq },		    PEND_LINUX_IRQ },
	{ { 0, rt_gettid },                         RT_GETTID },
	{ { 0, rt_get_real_time },		    GET_REAL_TIME },
	{ { 0, rt_get_real_time_ns },		    GET_REAL_TIME_NS },
	{ { 1, rt_signal_helper }, 		    RT_SIGNAL_HELPER },
	{ { 1, rt_wait_signal }, 		    RT_SIGNAL_WAITSIG },
	{ { 1, rt_request_signal_ },		    RT_SIGNAL_REQUEST },
	{ { 1, rt_release_signal },		    RT_SIGNAL_RELEASE },
	{ { 1, rt_enable_signal },		    RT_SIGNAL_ENABLE },
	{ { 1, rt_disable_signal },		    RT_SIGNAL_DISABLE },
	{ { 1, rt_trigger_signal }, 		    RT_SIGNAL_TRIGGER },
	{ { 0, 0 },			            000 }
};

static void rtai_isr_sched_handle(int cpuid) /* Called with interrupts off */
{
	SCHED_UNLOCK_SCHEDULE(cpuid);
}
EXPORT_SYMBOL(rtai_isr_sched_handle);

extern struct rtai_realtime_irq_s rtai_realtime_irq[];
static void rtai_hirq_dispatcher(unsigned int irq)
{
	unsigned long cpuid;
	if (rtai_domain.irqs[irq].handler) {
		unsigned long sflags;
		sflags = rt_save_switch_to_real_time(cpuid = rtai_cpuid());
#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
                if (!rt_scheduling[cpuid].locked++) { 
                        rt_scheduling[cpuid].rqsted = 0; 
                } 
#endif
		rtai_domain.irqs[irq].handler(irq, rtai_domain.irqs[irq].cookie);
#ifdef CONFIG_RTAI_SCHED_ISR_LOCK
		if (rt_scheduling[cpuid].locked && !(--rt_scheduling[cpuid].locked)) {
			if (rt_scheduling[cpuid].rqsted > 0) {
				rtai_isr_sched_handle(cpuid);
			}
		} 
#endif
		rtai_cli();
		rt_restore_switch_to_linux(sflags, cpuid);
		if (test_bit(IPIPE_STALL_FLAG, ROOT_STATUS_ADR(cpuid))) {
			return;
		}
	}
	rtai_sti();
	hal_fast_flush_pipeline();
	return;
}

int (*saved_rtai_syscall_hook)(struct pt_regs *);
extern int (*rtai_fastcall_hook)(struct pt_regs *);
extern int (*rtai_syscall_hook)(struct pt_regs *);
extern void (*rtai_migration_hook)(struct task_struct *);
extern int (*rtai_kevent_hook)(int kevent, void *);

void (*saved_dispatch_irq_head)(unsigned int);
extern void (*dispatch_irq_head)(unsigned int);

static int lxrt_init(void)
{
	void init_fun_ext(void);
	int cpuid;

	init_fun_ext();

	REQUEST_RESUME_SRQs_STUFF();

	for (cpuid = 0; cpuid < MAX_LXRT_FUN; cpuid++) {
		rt_fun_lxrt[cpuid].type = 1;
		rt_fun_lxrt[cpuid].fun  = nihil;
	}
	
	set_rt_fun_entries(rt_sched_entries);

	lxrt_old_trap_handler = rt_set_trap_handler(lxrt_handle_trap);
	rtai_proc_lxrt_register();
	rtai_init_sthsems();
	saved_rtai_syscall_hook = rtai_syscall_hook;
	rtai_fastcall_hook = lxrt_intercept_fastcall;
	rtai_syscall_hook = lxrt_intercept_syscall;
	rtai_kevent_hook = lxrt_intercept_kevents;
	rtai_migration_hook = fast_schedule; //lxrt_intercept_schedule_tail;

	saved_dispatch_irq_head = dispatch_irq_head;
	dispatch_irq_head = rtai_hirq_dispatcher;

	rtai_set_active_mm();

	return 0;
}

static void lxrt_exit(void)
{
	rtai_proc_lxrt_unregister();
	rt_set_trap_handler(lxrt_old_trap_handler);

	RELEASE_RESUME_SRQs_STUFF();

	rtai_fastcall_hook = NULL;
	rtai_syscall_hook = saved_rtai_syscall_hook;
	rtai_migration_hook = NULL;
	rtai_kevent_hook = NULL;

	dispatch_irq_head = saved_dispatch_irq_head;
    
	reset_rt_fun_entries(rt_sched_entries);
	rtai_drop_active_mm();
}

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kmod.h>

#define CAL_WITH_KTHREAD 0
#define MAX_LOOPS CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES

static int end_kernel_lat_cal;

#if 1
#define DIAG_KF_LAT_EVAL 0
#define SV 3163
#define SG 1000000000
#define R  (2*SV)
#define Q  0
#define P0 R
#define ALPHA 1000100000
#define rtai_simuldiv(s, m, d) \
	((s) > 0 ? rtai_imuldiv((s), (m), (d)) : -rtai_imuldiv(-(s), (m), (d)))
static void kf_lat_eval(long period)
{
	int loop, calok, xm;
	int xp, xe, y, pe, pp, ppe, g, q, r;
	RTIME start_time, resume_time;

	q  = Q*Q;
	r  = R*R;
	xe = 0;
	pe = P0*P0;
	xm = 1000000000;

#if DIAG_KF_LAT_EVAL
	rt_printk("INITIAL VALUES: xe %d, pe %d, q %d, r %d.\n", xe, pe, q, r);
#endif

#if CAL_WITH_KTHREAD 
	rt_thread_init(nam2num("KERCAL"), 0, 1, SCHED_FIFO, 0xF);
#endif

	start_time = rtai_rdtsc();
	resume_time = start_time + 5*period;
	rt_task_make_periodic(NULL, resume_time, period);
	for (calok = loop = 1; loop <= MAX_LOOPS; loop++) {
		resume_time += period;
		if (!rt_task_wait_period()) {
			y = (long)(rtai_rdtsc() - resume_time);
		} else {
			y = xe;
		}
		if (y < xm) xm = y;
		xp  = xe;				 // xp = xe
		pp  = rtai_simuldiv(pe, ALPHA, SG) + q;  // pp = ALPHA*pe + q
		g   = rtai_simuldiv(pp, SG, pp + r);     // g = pp/(pp + r)
		xe  = xp + rtai_simuldiv(y - xp, g, SG); // xe = xp + g*(y - xp)
		ppe = pe;				 // ppe = pe
		pe  = rtai_simuldiv(SG - g, pp, SG);     // pe = (1.0 - g)*pp

#if DIAG_KF_LAT_EVAL
		rt_printk("loop %d, xp %d, xe %d, y %d, pp %d, pe %d, g %d, r %d.\n", loop, xp, xe, y, pp, pe, g, r);
#endif

		if (abs(xe - xp) < abs(xe)/1000 && abs(pe - ppe) < abs(pe)/1000) {
			if (calok++ > 250) break;
		} else {
			calok = 1;
		}
	}
	rt_printk("KERNEL SPACE LATENCY ENDED AT CYCLE: %d, LATENCY = %d, VARIANCE = %d/%d, GAIN = %d/%d, LEAST = %d.\n", loop - 1 , xe, pe, SV*SV, g, SG, xm);

	KernelLatency = CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS == 1 ? xe : (CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS == 2 ? xm : (xe + xm)/2);
	end_kernel_lat_cal = 1;
}
#else
static void kernel_lat_cal(long period)
{
#define WARMUP 50
	int loop, max_overn_loop;
	long latency, ovrns;
	RTIME start_time, resume_time;

	max_overn_loop  = 0;
	latency = ovrns = 0;

#if CAL_WITH_KTHREAD 
	rt_thread_init(nam2num("KERCAL"), 0, 1, SCHED_FIFO, 0xF);
#endif

	start_time = rtai_rdtsc();
	resume_time = start_time + 5*period;
	rt_task_make_periodic(NULL, resume_time, period);
	for (loop = 1; loop <= (MAX_LOOPS + WARMUP); loop++) {
		resume_time += period;
		if (!rt_task_wait_period()) {
			latency += (long)(rtai_rdtsc() - resume_time);
			if (loop == WARMUP) {
				latency = 0;
			}
		} else {
			ovrns++;
			max_overn_loop = loop;
		}
	}

	if (ovrns) {
		printk("KERNEL SPACE CALIBRATION: OVERRUNS %ld, MAX OVERRUN LOOP %d, NUMBER OF WARMUP LOOOP %d.\n", ovrns, max_overn_loop, WARMUP);
	}
	KernelLatency = latency/MAX_LOOPS;
	end_kernel_lat_cal = 1;
}
#endif

static int recalibrate = 0;
module_param(recalibrate, int, S_IRUGO);

#define sign(i) (((i) >= 0) ? 1 : -1)

static void calibrate_latencies(void)
{
#define NUM_ARGSIZE   15
	char env0[] = RTAI_INSTALL_DIR"/calibration";
	char arg0[] = RTAI_INSTALL_DIR"/calibration/calibrate";
	char arg1[] = RTAI_INSTALL_DIR"/calibration/latencies";
	char arg2[NUM_ARGSIZE];
	char arg3[NUM_ARGSIZE] = "-1";
	char *envp[] = { env0, "TERM=linux", "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", NULL };
	char *argv[] = { arg0, arg1, arg2, arg3, NULL };
	int cpuid, period = nano2count(1000000000/CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ);

	tuned.sched_latency = 0;
	satdelay = oneshot_span;
	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		tuned.timers_tol[cpuid] = rt_half_tick = 0;
	}

	if (!kernel_latency || !user_latency) {
		sprintf(arg2, "%d", recalibrate ? -1 : 0);
		printk("USERMODE CHECK: %s.\n", !call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC) ? "OK" : "ERROR");
		printk("USERMODE CHECK PROVIDED (ns): KernelLatency %d, UserLatency %d.\n", KernelLatency > 0 ? (int)count2nano(KernelLatency) : KernelLatency, UserLatency > 0 ? (int)count2nano(UserLatency) : UserLatency);
	}

	if (kernel_latency > 0) {
		KernelLatency = nano2count(kernel_latency);
	} else if (KernelLatency < 0) {
		RTIME t = rtai_rdtsc();
#if CAL_WITH_KTHREAD 
		rt_thread_create(kernel_lat_cal, (void *)(long)period, 0);
		while (!end_kernel_lat_cal) msleep(100);
#else
		RT_TASK *task;
		task = kmalloc(sizeof(RT_TASK), GFP_KERNEL);
		rt_task_init(task, kf_lat_eval, period, 4096, 0, 1, 0);
		rt_task_resume(task);
		while (!end_kernel_lat_cal) msleep(100);
		kfree(task);
#endif
		printk("AFTER KERNEL CALIBRATION (WITH %s, ns): KernelLatency %d, UserLatency %d (CALIBRATION: PERIOD %d (ns), TIME %lld (ns)).\n", CAL_WITH_KTHREAD ? "KTHREAD" : "RTAI TASK", (int)count2nano(KernelLatency), UserLatency > 0 ? (int)count2nano(UserLatency) : UserLatency, CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ, count2nano(rtai_rdtsc() - t));
	}

	if (user_latency > 0) {
		UserLatency = nano2count(user_latency);
	} else if (UserLatency < 0) {
		RTIME t = rtai_rdtsc();
		sprintf(arg2, "%d", period);
		sprintf(arg3, "%d", KernelLatency);
		printk("USERMODE USER SPACE CALIBRATION: %s.\n", !call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC) ? "OK" : "ERROR");
		printk("AFTER USER CALIBRATION (ns): KernelLatency %d, UserLatency %d (CALIBRATION: PERIOD %d (ns), TIME %lld (ns)).\n", (int)count2nano(KernelLatency), (int)count2nano(UserLatency), CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ, count2nano(rtai_rdtsc() - t));
	}

	printk("FINAL CALIBRATION SUMMARY (ns): KernelLatency %d, UserLatency %d.\n", (int)count2nano(KernelLatency), (int)count2nano(UserLatency));
}

extern int rt_registry_alloc(void);
extern void rt_registry_free(void);
extern int kthread_server(void *);
static struct task_struct *kthread_server_thread;

static int __rtai_lxrt_init(void)
{
	int cpuid, retval;
	
#ifdef CONFIG_RTAI_MALLOC
	rtai_kstack_heap_size = (rtai_kstack_heap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	if (rtheap_init(&rtai_kstack_heap, NULL, rtai_kstack_heap_size, PAGE_SIZE, GFP_KERNEL)) {
		printk(KERN_INFO "RTAI[malloc]: failed to initialize the kernel stacks heap (size=%d bytes).\n", rtai_kstack_heap_size);
		return 1;
	}
#endif
	sched_mem_init();

	rt_registry_alloc();

	for (cpuid = 0; cpuid < RTAI_NR_CPUS; cpuid++) {
		rt_linux_task.uses_fpu = 1;
		rt_linux_task.magic = 0;
		rt_linux_task.policy = rt_linux_task.is_hard = 0;
		rt_linux_task.runnable_on_cpus = cpuid;
		rt_linux_task.state = RT_SCHED_READY;
		rt_linux_task.msg_queue.prev = &(rt_linux_task.msg_queue);      
		rt_linux_task.msg_queue.next = &(rt_linux_task.msg_queue);      
		rt_linux_task.msg_queue.task = &rt_linux_task;    
		rt_linux_task.msg = 0;  
		rt_linux_task.ret_queue.prev = &(rt_linux_task.ret_queue);
		rt_linux_task.ret_queue.next = &(rt_linux_task.ret_queue);
		rt_linux_task.ret_queue.task = NULL;
		rt_linux_task.priority = RT_SCHED_LINUX_PRIORITY;
		rt_linux_task.base_priority = RT_SCHED_LINUX_PRIORITY;
		rt_linux_task.signal = 0;
		rt_linux_task.prev = &rt_linux_task;
                rt_linux_task.resume_time = RTAI_TIME_LIMIT;
                rt_linux_task.periodic_resume_time = RTAI_TIME_LIMIT;
                rt_linux_task.tprev = rt_linux_task.tnext =
                rt_linux_task.rprev = rt_linux_task.rnext = &rt_linux_task;
#ifdef CONFIG_RTAI_LONG_TIMED_LIST
		rt_linux_task.rbr.rb_node = NULL;
#endif
		rt_linux_task.next = 0;
		rt_linux_task.lnxtsk = current;
		rt_smp_current[cpuid] = &rt_linux_task;
		rt_smp_fpu_task[cpuid] = &rt_linux_task;
		linux_cr0 = 0;
		rt_linux_task.resq.prev = rt_linux_task.resq.next = &rt_linux_task.resq;
		rt_linux_task.resq.task = NULL;
		rt_linux_task.schedlat = UserLatency;
	}
	tuned.sched_latency = 0;
#if RTAI_KERN_BUSY_ALIGN_RET_DELAY > 0
	tuned.kern_latency_busy_align_ret_delay = rtai_imuldiv(RTAI_KERN_BUSY_ALIGN_RET_DELAY, tuned.clock_freq, 1000000000);
#endif
#if RTAI_USER_BUSY_ALIGN_RET_DELAY > 0
	tuned.user_latency_busy_align_ret_delay = rtai_imuldiv(RTAI_USER_BUSY_ALIGN_RET_DELAY, tuned.clock_freq, 1000000000);
#endif
	SetupTimeTIMER = rtai_calibrate_hard_timer();
	tuned.setup_time_TIMER_UNIT = rtai_imuldiv(SetupTimeTIMER, TIMER_FREQ, 1000000000);
	if (tuned.setup_time_TIMER_UNIT < 1) {
		tuned.setup_time_TIMER_UNIT = 1;
		tuned.setup_time_TIMER_CPUNIT = (tuned.clock_freq + TIMER_FREQ/2)/TIMER_FREQ;
	} else {
		tuned.setup_time_TIMER_CPUNIT = rtai_imuldiv(SetupTimeTIMER, tuned.clock_freq, 1000000000);
	}
	if (tuned.sched_latency < tuned.setup_time_TIMER_CPUNIT) {
		tuned.sched_latency = tuned.setup_time_TIMER_CPUNIT;
	}
	tuned.timers_tol[0] = 0;
	oneshot_span = ONESHOT_SPAN;
	satdelay = oneshot_span - tuned.sched_latency;
	if (rtai_proc_sched_register()) {
		retval = 1;
		goto mem_end;
	}

// 0x7dd763ad == nam2num("MEMSRQ").
	if ((frstk_srq.srq = rt_request_srq(0x7dd763ad, frstk_srq_handler, 0)) < 0) {
		printk("MEM SRQ: no sysrq available.\n");
		retval = frstk_srq.srq;
		goto proc_unregister;
	}

	frstk_srq.in = frstk_srq.out = 0;
	if ((retval = rt_request_sched_ipi()) != 0)
		goto free_srq;

	if ((retval = lxrt_init()) != 0)
		goto free_sched_ipi;

	register_reboot_notifier(&lxrt_reboot_notifier);

#ifdef CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL
	printk(", <uses LINUX SYSCALLs>");
#endif
#ifdef CONFIG_RTAI_MALLOC
	printk("kstacks pool size = %d bytes", rtai_kstack_heap_size);
#endif
	printk(KERN_INFO "RTAI[sched]: hard timer type/freq = %s/%d(Hz)", TIMER_NAME, (int)TIMER_FREQ);
#ifdef CONFIG_RTAI_LONG_TIMED_LIST
	printk("black/red timed lists.\n");
#else
	printk("linear timed lists.\n");
#endif
	printk(KERN_INFO "RTAI[sched]: Linux timer freq = %d (Hz), TimeBase freq = %lu hz.\n", HZ, (unsigned long)tuned.clock_freq);
	printk(KERN_INFO "RTAI[sched]: timer setup = %d ns, resched latency = %d ns.\n", (int)rtai_imuldiv(tuned.setup_time_TIMER_CPUNIT, 1000000000, tuned.clock_freq), (int)rtai_imuldiv(tuned.sched_latency - tuned.setup_time_TIMER_CPUNIT, 1000000000, tuned.clock_freq));

	kthread_server_thread = kthread_run(kthread_server, NULL, "KTHREAD_SERVER");

	retval = rtai_init_features(); /* see rtai_schedcore.h */

exit:
#if CONFIG_RTAI_RTC_FREQ == 0
	rt_linux_hrt_next_shot = _rt_linux_hrt_next_shot;
#endif
	rt_start_timers();
	calibrate_latencies();

	return retval;
free_sched_ipi:
	rt_free_sched_ipi();
free_srq:
	rt_free_srq(frstk_srq.srq);
proc_unregister:
	rtai_proc_sched_unregister();
mem_end:
	sched_mem_end();
#ifdef CONFIG_RTAI_MALLOC
	rtheap_destroy(&rtai_kstack_heap, GFP_KERNEL);
#endif
	rt_registry_free();
	goto exit;
}

static void __rtai_lxrt_exit(void)
{
	unregister_reboot_notifier(&lxrt_reboot_notifier);

#if CONFIG_RTAI_RTC_FREQ == 0
	rt_linux_hrt_next_shot = NULL;
#endif

	rtai_tskext(kthread_server_thread, TSKEXT3) = (void *)1;
	rt_task_resume(rtai_tskext_t(kthread_server_thread, TSKEXT0));
	while (rtai_tskext(kthread_server_thread, TSKEXT3)) msleep(5);

	lxrt_killall();

	krtai_objects_release();

	lxrt_exit();

	rtai_cleanup_features();

        rtai_proc_sched_unregister();

	while (frstk_srq.out != frstk_srq.in);
	if (rt_free_srq(frstk_srq.srq) < 0) {
		printk("MEM SRQ: frstk_srq %d illegal or already free.\n", frstk_srq.srq);
	}
	rt_free_sched_ipi();
	sched_mem_end();
#ifdef CONFIG_RTAI_MALLOC
	rtheap_destroy(&rtai_kstack_heap, GFP_KERNEL);
#endif
	rt_registry_free();
	msleep(100);

#ifdef IPIPE_NOSTACK_FLAG
	ipipe_clear_foreign_stack(&rtai_domain);
#endif

	printk(KERN_INFO "RTAI[sched]: unloaded (forced hard/soft/hard transitions: traps %lu, syscalls %lu).\n", traptrans, systrans);
}

module_init(__rtai_lxrt_init);
module_exit(__rtai_lxrt_exit);

MODULE_ALIAS("rtai_up");
MODULE_ALIAS("rtai_mup");
MODULE_ALIAS("rtai_smp");
MODULE_ALIAS("rtai_ksched");
MODULE_ALIAS("rtai_lxrt");

EXPORT_SYMBOL(rt_fun_lxrt);
EXPORT_SYMBOL(clr_rtext);
EXPORT_SYMBOL(set_rtext);
EXPORT_SYMBOL(get_min_tasks_cpuid);
EXPORT_SYMBOL(put_current_on_cpu);
EXPORT_SYMBOL(rt_schedule_soft);
EXPORT_SYMBOL(rt_do_force_soft);
EXPORT_SYMBOL(rt_schedule_soft_tail);
EXPORT_SYMBOL(rt_sched_timed);
#if CONFIG_RTAI_MONITOR_EXECTIME
EXPORT_SYMBOL(switch_time);
#endif
EXPORT_SYMBOL(lxrt_prev_task);
