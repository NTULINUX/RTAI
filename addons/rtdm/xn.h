/*
 * Copyright (C) 2005-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


// Wrappers and inlines to avoid too much an editing of RTDM code. 
// The core stuff is just RTAI in disguise.

#ifndef _RTAI_XNSTUFF_H
#define _RTAI_XNSTUFF_H

#include <linux/compat.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/mman.h>

#include <rtai_schedcore.h>

#define XNARCH_NR_IRQS              RTHAL_NR_IRQS
#define CONFIG_XENO_OPT_RTDM_FILDES CONFIG_RTAI_RTDM_FD_MAX
#define XNHEAP_DEV_NAME             "/dev/rtai_shm"

#define CONFIG_XENO_OPT_PERVASIVE

#ifdef CONFIG_PROC_FS
#define CONFIG_XENO_OPT_VFILE
#endif

#ifdef CONFIG_RTAI_RTDM_SELECT
#define CONFIG_XENO_OPT_SELECT
#define CONFIG_XENO_OPT_RTDM_SELECT
#define CONFIG_RTDM_SELECT
#endif

#ifdef CONFIG_RTAI_RTDM_SHIRQ
#define CONFIG_XENO_OPT_SHIRQ 
#endif

#define CONFIG_XENO_DEBUG_RTDM_APPL 0
#define CONFIG_XENO_DEBUG_RTDM      0

#define XENO_DEBUG(subsystem)   (CONFIG_XENO_DEBUG_##subsystem > 0)

#define XENO_ASSERT(subsystem, cond, action)  do { \
    if (unlikely(CONFIG_XENO_DEBUG_##subsystem > 0 && !(cond))) { \
        xnlogerr("assertion failed at %s:%d (%s)\n", __FILE__, __LINE__, (#cond)); \
        action; \
    } \
} while(0)

#define XENO_BUGON(subsystem, cond)  do { /*\
	if (unlikely(CONFIG_XENO_DEBUG_##subsystem > 0 && (cond))) \
		xnpod_fatal("bug at %s:%d (%s)", __FILE__, __LINE__, (#cond)); */ \
 } while(0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,16,0)
#define smp_mb__before_atomic()  smp_mb()
#endif

// with what above we let some assertion diagnostic, below we keep knowledge of
// specific assertions we care of

#define xnpod_root_p()          (!rtai_tskext(current, TSKEXT0) || !rtai_tskext_t(current, TSKEXT0)->is_hard)
#define xnshadow_thread(t)      ((xnthread_t *)rtai_tskext(current, TSKEXT0))
#define rthal_local_irq_test()  (!rtai_save_flags_irqbit())
#define rthal_local_irq_enable  rtai_sti 
#define rthal_domain            rtai_domain

#define rthal_local_irq_disabled()                              \
({                                                              \
        unsigned long __flags, __ret;                           \
        local_irq_save_hw_smp(__flags);                         \
        __ret = ipipe_test_head();			        \
        local_irq_restore_hw_smp(__flags);                      \
        __ret;                                                  \
})

#define compat_module_param_array(name, type, count, perm) \
        module_param_array(name, type, NULL, perm)

#define trace_mark(ev, fmt, args...)  do { } while (0)

// recursive smp locks, as for RTAI global lock stuff but with an own name

#define nklock (*((xnlock_t *)rtai_cpu_lock))

#define XNARCH_LOCK_UNLOCKED  { { { 0, __ARCH_SPIN_LOCK_UNLOCKED } } }

typedef unsigned long spl_t;
typedef struct { struct global_lock lock[1]; } xnlock_t;

#ifndef list_first_entry
#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)
#endif

#ifndef local_irq_save_hw_smp
#ifdef CONFIG_SMP
#define local_irq_save_hw_smp(flags)    do { (flags) = hard_local_irq_save(); } while(0)
#define local_irq_restore_hw_smp(flags) hard_local_irq_restore(flags)
#else /* !CONFIG_SMP */
#define local_irq_save_hw_smp(flags)    do { (void)(flags); } while (0)
#define local_irq_restore_hw_smp(flags) do { } while (0)
#endif /* !CONFIG_SMP */
#endif /* !local_irq_save_hw_smp */

#ifdef CONFIG_SMP

#define DECLARE_XNLOCK(lock)              xnlock_t lock
#define DECLARE_EXTERN_XNLOCK(lock)       extern xnlock_t lock
#define DEFINE_XNLOCK(lock)               xnlock_t lock = XNARCH_LOCK_UNLOCKED
#define DEFINE_PRIVATE_XNLOCK(lock)       static DEFINE_XNLOCK(lock)

static inline void xnlock_init(xnlock_t *lock)
{
	lock->lock[0] = (struct global_lock) { 0, __ARCH_SPIN_LOCK_UNLOCKED };
}

static inline void xnlock_get(xnlock_t *lock)
{
	barrier();
	rtai_cli();
	if (!test_and_set_bit(rtai_cpuid(), &lock->lock[0].mask)) {
		rt_spin_lock(&lock->lock[0].lock);
	}
	barrier();
}

static inline void xnlock_put(xnlock_t *lock)
{
	barrier();
	rtai_cli();
	if (test_and_clear_bit(rtai_cpuid(), &lock->lock[0].mask)) {
		rt_spin_unlock(&lock->lock[0].lock);
	}
	barrier();
}

static inline spl_t __xnlock_get_irqsave(xnlock_t *lock)
{
	unsigned long flags;

	barrier();
	flags = rtai_save_flags_irqbit_and_cli();
	if (!test_and_set_bit(rtai_cpuid(), &lock->lock[0].mask)) {
		rt_spin_lock(&lock->lock[0].lock);
		barrier();
		return flags | 1;
	}
	barrier();
	return flags;
}

#define xnlock_get_irqsave(lock, flags)  \
	do { flags = __xnlock_get_irqsave(lock); } while (0)

static inline void xnlock_put_irqrestore(xnlock_t *lock, spl_t flags)
{
	barrier();
	if (test_and_clear_bit(0, &flags)) {
		xnlock_put(lock);
	} else {
		xnlock_get(lock);
	}
	if (flags) {
		rtai_sti();
	}
	barrier();
}

#else /* !CONFIG_SMP */

#define DECLARE_XNLOCK(lock)
#define DECLARE_EXTERN_XNLOCK(lock)
#define DEFINE_XNLOCK(lock)
#define DEFINE_PRIVATE_XNLOCK(lock)

#define xnlock_init(lock)                   do { } while(0)
#define xnlock_get(lock)                    rtai_cli()
#define xnlock_put(lock)                    rtai_sti()
#define xnlock_get_irqsave(lock, flags)     rtai_save_flags_and_cli(flags)
#define xnlock_put_irqrestore(lock, flags)  rtai_restore_flags(flags)

#endif /* CONFIG_SMP */

// memory allocation

#define xnmalloc  rt_malloc
#define xnfree    rt_free
#define xnarch_fault_range(vma)

// in kernel printing (taken from RTDM pet system)

#define XNARCH_PROMPT "RTDM: "

#define xnprintf(fmt, args...)  printk(KERN_INFO XNARCH_PROMPT fmt, ##args)
#define xnlogerr(fmt, args...)  printk(KERN_ERR  XNARCH_PROMPT fmt, ##args)
#define xnlogwarn               xnlogerr

// user space access (taken from Linux)

//#define __xn_access_ok(task, type, addr, size) (access_ok(type, addr, size))
#define access_rok(addr, size)  access_ok(VERIFY_READ, (addr), (size))
#define access_wok(addr, size)  access_ok(VERIFY_WRITE, (addr), (size))

#define __xn_copy_from_user(dstP, srcP, n)      __copy_from_user_inatomic(dstP, srcP, n)
#define __xn_copy_to_user(dstP, srcP, n)        __copy_to_user_inatomic(dstP, srcP, n)
#define __xn_put_user(src, dstP)                __put_user(src, dstP)
#define __xn_get_user(dst, srcP)                __get_user(dst, srcP)

#if !defined CONFIG_M68K || defined CONFIG_MMU
#define __xn_strncpy_from_user(dstP, srcP, n) \
	({ long err = rt_strncpy_from_user(dstP, srcP, n); err; })
/*	({ long err = __strncpy_from_user(dstP, srcP, n); err; }) */
#else
#define __xn_strncpy_from_user(dstP, srcP, n) \
	({ long err = strncpy_from_user(dstP, srcP, n); err; })
#endif /* CONFIG_M68K */


#include <rtai_shm.h>

#define __va_to_kva(adr)  UVIRT_TO_KVA(adr)

static inline int xnarch_remap_io_page_range(struct file *filp, struct vm_area_struct *vma, unsigned long from, unsigned long to, unsigned long size, pgprot_t prot)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	return remap_pfn_range(vma, from, to >> PAGE_SHIFT, size, pgprot_noncached(prot));
}

static inline int xnarch_remap_kmem_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long to, unsigned long size, pgprot_t prot)
{
	return remap_pfn_range(vma, from, to >> PAGE_SHIFT, size, prot);
}

#ifdef CONFIG_MMU

static inline int xnarch_remap_vm_page(struct vm_area_struct *vma, unsigned long from, unsigned long to)
{
	return vm_insert_page(vma, from, vmalloc_to_page((void *)to));
}

#endif

// interrupt setup/management (adopted_&|_adapted from RTDM pet system)

#define RTHAL_NR_IRQS  IPIPE_NR_XIRQS

#define XN_ISR_NONE       0x1
#define XN_ISR_HANDLED    0x2

#define XN_ISR_PROPAGATE  0x100
#define XN_ISR_NOENABLE   0x200
#define XN_ISR_BITMASK    ~0xff

#define XN_ISR_SHARED     0x1
#define XN_ISR_EDGE       0x2

#define XN_ISR_ATTACHED   0x10000

struct xnintr;

typedef int (*xnisr_t)(struct xnintr *intr);

typedef int (*xniack_t)(unsigned irq);

typedef unsigned long xnflags_t;

typedef atomic_t atomic_counter_t;

typedef RTIME xnticks_t;

typedef struct xnstat_exectime { xnticks_t start; xnticks_t total; } xnstat_exectime_t;

typedef struct xnstat_counter { int counter; } xnstat_counter_t;

#define xnstat_counter_inc(c)  ((c)->counter++)

typedef struct xnintr {
#ifdef CONFIG_XENO_OPT_SHIRQ
    struct xnintr *next;
#endif /* CONFIG_XENO_OPT_SHIRQ */
    unsigned unhandled;
    xnisr_t isr;
    void *cookie;
    xnflags_t flags;
    unsigned irq;
    xniack_t iack;
    const char *name;
    struct {
	xnstat_counter_t  hits;
	xnstat_exectime_t account;
	xnstat_exectime_t sum;
    } stat[RTAI_NR_CPUS];
} xnintr_t;

#define xnsched_cpu(sched)  rtai_cpuid()

int xnintr_shirq_attach(xnintr_t *intr, void *cookie);
int xnintr_shirq_detach(xnintr_t *intr);
int xnintr_init (xnintr_t *intr, const char *name, unsigned irq, xnisr_t isr, xniack_t iack, xnflags_t flags);
int xnintr_destroy (xnintr_t *intr);
int xnintr_attach (xnintr_t *intr, void *cookie);
int xnintr_detach (xnintr_t *intr);
int xnintr_enable (xnintr_t *intr);
int xnintr_disable (xnintr_t *intr);

/* Atomic operations are already serializing on x86 */
#ifdef smp_mb__before_atomic_dec
#define xnarch_before_atomic_dec()   smp_mb__before_atomic_dec()
#else
#define xnarch_before_atomic_dec()   smp_mb__before_atomic()
#endif
#define xnarch_after_atomic_dec()    smp_mb__after_atomic_dec()
#define xnarch_before_atomic_inc()   smp_mb__before_atomic_inc()
#define xnarch_after_atomic_inc()    smp_mb__after_atomic_inc()

#define xnarch_memory_barrier()      smp_mb()
#define xnarch_atomic_get(pcounter)  atomic_read(pcounter)
#define xnarch_atomic_inc(pcounter)  atomic_inc(pcounter)
#define xnarch_atomic_dec(pcounter)  atomic_dec(pcounter)

#define   testbits(flags, mask)  ((flags) & (mask))
#define __testbits(flags, mask)  ((flags) & (mask))
#define __setbits(flags, mask)   do { (flags) |= (mask);  } while(0)
#define __clrbits(flags, mask)   do { (flags) &= ~(mask); } while(0)

#define xnarch_chain_irq   rt_pend_linux_irq
#define xnarch_end_irq     rt_end_irq

#define xnarch_hook_irq(irq, handler, iack, intr) \
	rt_request_irq_wack(irq, (void *)handler, intr, 0, (void *)iack);
#define xnarch_release_irq(irq) \
	rt_release_irq(irq);

#define xnarch_get_irq_cookie(irq)  (rtai_domain.irqs[irq].cookie)

extern unsigned long IsolCpusMask;
#define xnarch_set_irq_affinity(irq, nkaffinity) \
	rt_assign_irq_to_cpu(irq, IsolCpusMask)

// support for RTDM timers

#define RTDM_TIMER_NAMELEN 32
struct rtdm_timer_struct {
        struct rtdm_timer_struct *next, *prev;
        int priority, cpuid;
        RTIME firing_time, period;
        void (*handler)(unsigned long);
        unsigned long data;
	char name[RTDM_TIMER_NAMELEN];
#ifdef  CONFIG_RTAI_LONG_TIMED_LIST
        rb_root_t rbr;
        rb_node_t rbn;
#endif
};

struct xntbase;

typedef struct rtdm_timer_struct xntimer_t;

RTAI_SYSCALL_MODE int rt_timer_insert(struct rtdm_timer_struct *timer, int priority, RTIME firing_time, RTIME period, void (*handler)(unsigned long), unsigned long data);

RTAI_SYSCALL_MODE void rt_timer_remove(struct rtdm_timer_struct *timer);

#define XN_INFINITE  (0)

/* Timer modes */
typedef enum xntmode {
        XN_RELATIVE,
        XN_ABSOLUTE,
        XN_REALTIME
} xntmode_t;

#define xntbase_ns2ticks(base, t)      nano2count(t)
#define xntbase_ns2ticks_ceil(base, t) ({ t; })

static inline void xntimer_init(xntimer_t *timer, struct xntbase *base, void (*handler)(xntimer_t *))
{
        memset(timer, 0, sizeof(struct rtdm_timer_struct));
        timer->handler = (void *)handler;
        timer->data    = (unsigned long)timer;
	timer->next    =  timer->prev = timer;
}

static inline void xntimer_set_name(xntimer_t *timer, const char *name)
{
	if (name != NULL) {
		strncpy(timer->name, name, sizeof(timer->name));
	}
}


extern struct epoch_struct boot_epoch;

static inline int xntimer_start(xntimer_t *timer, xnticks_t value, xnticks_t interval, int mode)
{
	if (mode == XN_RELATIVE) {
		value += rt_get_time_ns();
	}
	if (mode == XN_REALTIME) {
		value += boot_epoch.time[boot_epoch.touse][0];
	}
	return rt_timer_insert(timer, 0, nano2count(value), nano2count(interval), timer->handler, (unsigned long)timer);
}

static inline void xntimer_stop(xntimer_t *timer)
{
        rt_timer_remove(timer);
}

static inline void xntimer_destroy(xntimer_t *timer)
{
        rt_timer_remove(timer);
}

// support for use in RTDM usage testing found in RTAI SHOWROOM CVS

static inline unsigned long long xnarch_ulldiv(unsigned long long ull, unsigned
long uld, unsigned long *r)
{
        unsigned long rem = do_div(ull, uld);
        if (r) {
                *r = rem;
        }
        return ull;
}

// support for RTDM select

typedef struct xnholder { struct xnholder *next; struct xnholder *prev; } xnholder_t;

typedef xnholder_t xnqueue_t;

#define DEFINE_XNQUEUE(q) xnqueue_t q = { { &(q), &(q) } }

#define inith(holder) \
	do { *(holder) = (xnholder_t) { holder, holder }; } while (0)

#define initq(queue) \
	do { inith(queue); } while (0)

#define appendq(queue, holder) \
do { \
	(holder)->prev = (queue); \
	((holder)->next = (queue)->next)->prev = holder; \
	(queue)->next = holder; \
} while (0)

#define removeq(queue, holder) \
do { \
	(holder)->prev->next = (holder)->next; \
	(holder)->next->prev = (holder)->prev; \
} while (0)

static inline xnholder_t *getheadq(xnqueue_t *queue)
{
	xnholder_t *holder = queue->next;
	return holder == queue ? NULL : holder;
}

static inline xnholder_t *getq(xnqueue_t *queue)
{
	xnholder_t *holder;
	if ((holder = getheadq(queue))) {
		removeq(queue, holder);
	}
	return holder;
}

static inline xnholder_t *nextq(xnqueue_t *queue, xnholder_t *holder)
{
	xnholder_t *nextholder = holder->next;
	return nextholder == queue ? NULL : nextholder;
}

static inline int emptyq_p(xnqueue_t *queue)
{
	return queue->next == queue;
}

#include "rtai_taskq.h"

#define xnpod_schedule  rt_schedule_readied

#define xnthread_t            RT_TASK
#define xnpod_current_thread  _rt_whoami
#define xnthread_test_info    rt_task_test_taskq_retval
#define xnthread_test_state(task, flags) ({ !task->magic; }) // attenzione, vale solo per XNZOMBIE 

#define xnsynch_t                   TASKQ
#define xnsynch_init(s, f, p)       rt_taskq_init(s, f)
#define xnsynch_destroy             rt_taskq_delete
#define xnsynch_wakeup_one_sleeper  rt_taskq_ready_one
#define xnsynch_flush               rt_taskq_ready_all
static inline void xnsynch_sleep_on(void *synch, xnticks_t timeout, xntmode_t timeout_mode)
{
	if (timeout == XN_INFINITE) {
		rt_taskq_wait(synch);
	} else {
		timeout = nano2count(timeout);
		if (timeout_mode == XN_RELATIVE) {
			timeout += rtai_rdtsc();
		} else if (timeout_mode == XN_REALTIME) {
			timeout -= boot_epoch.time[boot_epoch.touse][0];
		}
		rt_taskq_wait_until(synch, timeout);
	}
}
#define xnsynch_test_flags(synchp, flags)   ((synchp)->status & (flags))
#define xnsynch_set_flags(synchp, flags)    ((synchp)->status |= flags)
#define xnsynch_clear_flags(synchp, flags)  ((synchp)->status &= ~(flags))

#define XNSYNCH_SPARE0  0x01000000
#define XNSYNCH_SPARE1  0x02000000
#define XNSYNCH_NOPIP    0
#define XNSYNCH_PRIO     TASKQ_PRIO
#define XNSYNCH_FIFO     TASKQ_FIFO
#define XNSYNCH_RESCHED  1

#define rthal_apc_alloc(name, handler, cookie) \
	rt_request_srq(nam2num(name), (void *)(handler), NULL);

#define rthal_apc_free(apc) \
	rt_free_srq((apc))

#define __rthal_apc_schedule(apc) \
	hal_pend_uncond(apc, rtai_cpuid())

#define rthal_apc_schedule(apc) \
	rt_pend_linux_srq((apc))

#define DECLARE_WORK_NODATA(f, n)       DECLARE_WORK(f, n)
#define DECLARE_WORK_FUNC(f)            void f(struct work_struct *work)
#define DECLARE_DELAYED_WORK_NODATA(n, f) DECLARE_DELAYED_WORK(n, f)

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
#define user_msghdr msghdr
#endif

typedef struct xntbase { } xntbase_t;

typedef struct xnshadow_ppd_t { } xnshadow_ppd_t;

/* Call with nklock locked irqs off. */
static inline xnshadow_ppd_t *xnshadow_ppd_get(unsigned muxid)
{
        return NULL;
}

typedef long long xnsticks_t;

#define xnarch_get_cpu_tsc rtai_rdtsc

static inline xnticks_t xntbase_get_jiffies(xntbase_t *base)
{
        return rt_get_time_ns();
}

#define xnarch_ns_to_tsc nano2count 

#define xnpod_active_p() (1)

#ifdef CONFIG_XENO_OPT_VFILE
#define wrap_f_inode(file)	((file)->f_path.dentry->d_inode)
#define wrap_proc_dir_entry_owner(entry) do { (void)entry; } while(0)
#endif

typedef cpumask_t xnarch_cpumask_t;

#ifdef CONFIG_SMP
#define xnarch_cpu_online_map (*cpu_online_mask)
#else
#define xnarch_cpu_online_map cpumask_of_cpu(0)
#endif
#define xnarch_cpu_set(cpu, mask)          cpu_set(cpu, (mask))
#define xnarch_cpu_clear(cpu, mask)        cpu_clear(cpu, (mask))
#define xnarch_cpu_isset(cpu, mask)        cpu_isset(cpu, (mask))
#define xnarch_cpu_test_and_set(cpu, mask) cpu_test_and_set(cpu, (mask))

#define XNKCOUT         0x80000000      /*!< Sched callout context */
#define XNINTCK         0x40000000      /*!< In master tick handler context */
#define XNINSW          0x20000000      /*!< In context switch */
#define XNRESCHED       0x10000000      /*!< Needs rescheduling */

#define XNHTICK         0x00008000      /*!< Host tick pending  */
#define XNINIRQ         0x00004000      /*!< In IRQ handling context */
#define XNHDEFER        0x00002000      /*!< Host tick deferred */
#define XNINLOCK        0x00001000      /*!< Scheduler locked */

typedef struct xnsched xnsched_t;

#define rthal_irq_ackfn_t ipipe_irq_ackfn_t

#define xntbase_ticks2ns(a, b) rt_get_time_ns()

#ifndef SPIN_LOCK_UNLOCKED
#define SPIN_LOCK_UNLOCKED {  { .rlock = { .raw_lock = __ARCH_SPIN_LOCK_UNLOCKED } } }
#endif
#define RTHAL_SPIN_LOCK_UNLOCKED SPIN_LOCK_UNLOCKED

#define rthal_spinlock_t spinlock_t

#define rthal_spin_lock_init spin_lock_init

#define rthal_spin_lock   rt_spin_lock
#define rthal_spin_unlock rt_spin_unlock

#define rthal_spin_lock_irqsave(lock, context) \
	do {  context = rt_spin_lock_irqsave(lock); } while (0)

#define rthal_local_irq_save rtai_save_flags_and_cli
#define rthal_local_irq_restore rtai_restore_flags

#define __xnpod_lock_sched   rt_sched_lock 
#define __xnpod_unlock_sched rt_sched_unlock 

static inline int ipipe_virtualize_irq(struct ipipe_domain *ipd, unsigned int irq, ipipe_irq_handler_t handler, void *cookie, ipipe_irq_ackfn_t ackfn, unsigned int unused)
{
	if (handler == NULL) {
		ipipe_free_irq(ipd, irq);
		return 0;
	}
	return ipipe_request_irq(ipd, irq, handler, cookie, ackfn);
}

static inline int ipipe_trigger_irq(unsigned int irq)
{
	ipipe_raise_irq(irq);
	return 1;
}

#define rthal_alloc_virq     ipipe_alloc_virq
#define rthal_virtualize_irq ipipe_virtualize_irq
#define rthal_free_virq      ipipe_free_virq
#define rthal_trigger_irq    ipipe_trigger_irq //hal_pend_uncond(irq, rtai_cpuid())

#define rthal_root_domain hal_root_domain

//#define rthal_trigger_irq(irq) hal_pend_uncond(irq, rtai_cpuid())

#define xnpod_asynch_p()      ({ 0; })
#define xnpod_unblockable_p() ({ 0; })
#define xnpod_unblockable_p() ({ 0; })

#define rthal_current_domain ipipe_current_domain

#define XNDELAY   0x00000004
static inline void xnpod_suspend_thread(xnthread_t *thread, xnflags_t mask, xnticks_t timeout, xntmode_t timeout_mode, xnsynch_t *wchan)
{
	xnticks_t now = rtai_rdtsc();
        unsigned long flags;
	
	if (timeout == XN_INFINITE) {
		thread->retval = rt_task_suspend(thread) == RTE_UNBLKD ? XNBREAK : 0;
		return;
	}
	timeout = nano2count(timeout);
	if (timeout_mode == XN_RELATIVE) {
		timeout += rtai_rdtsc();
	} else if (timeout_mode == XN_REALTIME) {
		timeout -= boot_epoch.time[boot_epoch.touse][0];
	}

        flags = rt_global_save_flags_and_cli();
	if ((thread->resume_time = timeout) > now) {
		void *blocked_on;
		thread->blocked_on = NULL;
		thread->state |= RT_SCHED_DELAYED;
                rem_ready_current(thread);
		enq_timed_task(thread);
		rt_schedule();
                blocked_on = thread->blocked_on;
                rt_global_restore_flags(flags);
                thread->retval = likely(!blocked_on) ? 0 : XNBREAK;
        	return;
        }
        rt_global_restore_flags(flags);
	thread->retval = XNTIMEO;
        return;
}

static inline int xnpod_wait_thread_period(unsigned long *overruns)
{
        if (_rt_whoami()->period <= 0) {
                return -EWOULDBLOCK;
        }
        if (!rt_task_wait_period()) {
        	if (overruns) {
			overruns = 0;
		}
                return 0;
        }
        if (_rt_whoami()->unblocked) {
                return -EINTR;
        }
        return rt_sched_timed ? -ETIMEDOUT : -EIDRM;
}

#define xnthread_time_base(a) ({ rtdm_tbase; })

#endif /* !_RTAI_XNSTUFF_H */
