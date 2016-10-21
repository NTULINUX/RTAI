/*
 * Copyright (C) 2005-2010 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2014-2016 Alec Ari <neotheuser@ymail.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


// Wrappers and inlines to avoid too much an editing of RTDM code.
// The core stuff is just RTAI in disguise.

#ifndef _RTAI_XNSTUFF_H
#define _RTAI_XNSTUFF_H

#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/mman.h>

#include <rtai_schedcore.h>

#define RTAI_DEBUG(subsystem)   (CONFIG_RTAI_DEBUG_##subsystem > 0)

#define RTAI_ASSERT(subsystem, cond, action)  do { \
    if (unlikely(CONFIG_RTAI_DEBUG_##subsystem > 0 && !(cond))) { \
	xnlogerr("assertion failed at %s:%d (%s)\n", __FILE__, __LINE__, (#cond)); \
	action; \
    } \
} while(0)

#define RTAI_BUGON(subsystem, cond)  do { /*\
	if (unlikely(CONFIG_RTAI_DEBUG_##subsystem > 0 && (cond))) \
		xnpod_fatal("bug at %s:%d (%s)", __FILE__, __LINE__, (#cond)); */ \
 } while(0)

/*
  With what above we let some assertion diagnostic. Here below we keep knowledge
  of specific assertions we care of.
 */

#define xnpod_root_p()	  (!current->rtai_tskext(TSKEXT0) || !((RT_TASK *)(current->rtai_tskext(TSKEXT0)))->is_hard)
#define xnshadow_thread(t)      ((xnthread_t *)current->rtai_tskext(TSKEXT0))
#define rthal_local_irq_test()  (!rtai_save_flags_irqbit())
#define rthal_local_irq_enable  rtai_sti
#define rthal_domain rtai_domain
#define rthal_local_irq_disabled()			      \
({							      \
	unsigned long __flags, __ret;			   \
	local_irq_save_hw_smp(__flags);			 \
	__ret = ipipe_test_pipeline_from(&rthal_domain);	\
	local_irq_restore_hw_smp(__flags);		      \
	__ret;						  \
})

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define _MODULE_PARM_STRING_charp "s"
#define compat_module_param_array(name, type, count, perm) \
	static inline void *__check_existence_##name(void) { return &name; } \
	MODULE_PARM(name, "1-" __MODULE_STRING(count) _MODULE_PARM_STRING_##type)

typedef unsigned long phys_addr_t;

#else

#define compat_module_param_array(name, type, count, perm) \
	module_param_array(name, type, NULL, perm)

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#define trace_mark(ev, fmt, args...)  do { } while (0)
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
#include <linux/marker.h>
#endif
#define trace_mark(ev, fmt, args...)  do { } while (0)
#endif

//recursive smp locks, as for RTAI global lock stuff but with an own name

#define nklock (*((xnlock_t *)rtai_cpu_lock))

#define XNARCH_LOCK_UNLOCKED  (xnlock_t) { { 0, 0 } }

typedef unsigned long spl_t;
typedef struct { volatile unsigned long lock[2]; } xnlock_t;

#ifndef list_first_entry
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)
#endif

#ifndef local_irq_save_hw_smp
#ifdef CONFIG_SMP
#define local_irq_save_hw_smp(flags)    local_irq_save_hw(flags)
#define local_irq_restore_hw_smp(flags) local_irq_restore_hw(flags)
#else /* !CONFIG_SMP */
#define local_irq_save_hw_smp(flags)    do { (void)(flags); } while (0)
#define local_irq_restore_hw_smp(flags) do { } while (0)
#endif /* !CONFIG_SMP */
#endif /* !local_irq_save_hw_smp */

#ifdef CONFIG_SMP

#define DECLARE_XNLOCK(lock)	      xnlock_t lock
#define DECLARE_EXTERN_XNLOCK(lock)       extern xnlock_t lock
#define DEFINE_XNLOCK(lock)	       xnlock_t lock = XNARCH_LOCK_UNLOCKED
#define DEFINE_PRIVATE_XNLOCK(lock)       static DEFINE_XNLOCK(lock)

static inline void xnlock_init(xnlock_t *lock)
{
	*lock = XNARCH_LOCK_UNLOCKED;
}

static inline void xnlock_get(xnlock_t *lock)
{
	barrier();
	rtai_cli();
	if (!test_and_set_bit(hal_processor_id(), &lock->lock[0])) {
		rtai_spin_glock(&lock->lock[0]);
	}
	barrier();
}

static inline void xnlock_put(xnlock_t *lock)
{
	barrier();
	rtai_cli();
	if (test_and_clear_bit(hal_processor_id(), &lock->lock[0])) {
		rtai_spin_gunlock(&lock->lock[0]);
	}
	barrier();
}

static inline spl_t __xnlock_get_irqsave(xnlock_t *lock)
{
	unsigned long flags;

	barrier();
	flags = rtai_save_flags_irqbit_and_cli();
	if (!test_and_set_bit(hal_processor_id(), &lock->lock[0])) {
		rtai_spin_glock(&lock->lock[0]);
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

#define xnlock_init(lock)		   do { } while(0)
#define xnlock_get(lock)		    rtai_cli()
#define xnlock_put(lock)		    rtai_sti()
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
#define xnlogwarn	       xnlogerr

// user space access (taken from Linux)

#define __xn_access_ok(task, type, addr, size) \
	(access_ok(type, addr, size))

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define __xn_copy_from_user(task, dstP, srcP, n) \
	({ long err = __copy_from_user(dstP, srcP, n); err; })

#define __xn_copy_to_user(task, dstP, srcP, n) \
	({ long err = __copy_to_user(dstP, srcP, n); err; })
#else
#define __xn_copy_from_user(task, dstP, srcP, n) \
	({ long err = __copy_from_user_inatomic(dstP, srcP, n); err; })

#define __xn_copy_to_user(task, dstP, srcP, n) \
	({ long err = __copy_to_user_inatomic(dstP, srcP, n); err; })
#endif

#if !defined CONFIG_M68K || defined CONFIG_MMU
#define __xn_strncpy_from_user(task, dstP, srcP, n) \
	({ long err = rt_strncpy_from_user(dstP, srcP, n); err; })
/*	({ long err = __strncpy_from_user(dstP, srcP, n); err; }) */
#else
#define __xn_strncpy_from_user(task, dstP, srcP, n) \
	({ long err = strncpy_from_user(dstP, srcP, n); err; })
#endif /* CONFIG_M68K */

static inline int xnarch_remap_io_page_range(struct file *filp, struct vm_area_struct *vma, unsigned long from, unsigned long to, unsigned long size, pgprot_t prot)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(from, to, size, prot);

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
	return remap_pfn_range(vma, from, (to) >> PAGE_SHIFT, size, prot);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	return remap_pfn_range(vma, from, (to) >> PAGE_SHIFT, size, prot);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) */
	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(vma, from, to, size, prot);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
}

#define wrap_remap_kmem_page_range(vma,from,to,size,prot) ({ \
    vma->vm_flags |= VM_RESERVED; \
    remap_page_range(from,to,size,prot); \
})

static inline int xnarch_remap_kmem_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long to, unsigned long size, pgprot_t prot)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(from, to, size, prot);

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) && defined(CONFIG_MMU)
	return remap_pfn_range(vma, from, to >> PAGE_SHIFT, size, prot);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	return remap_pfn_range(vma, from, to >> PAGE_SHIFT, size, prot);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) */
	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(from, to, size, prot);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
}

#include <rtai_shm.h>
#define __va_to_kva(adr)  UVIRT_TO_KVA(adr)

#ifdef CONFIG_MMU

static inline int xnarch_remap_vm_page(struct vm_area_struct *vma, unsigned long from, unsigned long to)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(from, virt_to_phys((void *)__va_to_kva(to)), PAGE_SIZE, PAGE_SHARED);

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#ifndef VM_RESERVED
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) && defined(CONFIG_MMU)
	vma->vm_flags |= VM_RESERVED;
	return vm_insert_page(vma, from, vmalloc_to_page((void *)to));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	return remap_pfn_range(vma, from, virt_to_phys((void *)__va_to_kva(to)) >> PAGE_SHIFT, PAGE_SHIFT, PAGE_SHARED);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10) */
	vma->vm_flags |= VM_RESERVED;
	return remap_page_range(from, virt_to_phys((void *)__va_to_kva(to)), PAGE_SIZE, PAGE_SHARED);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
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

#if !defined(CONFIG_PPC) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,32) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)))

#define rthal_virtualize_irq(dom, irq, isr, cookie, ackfn, mode) \
	ipipe_virtualize_irq(dom, irq, isr, ackfn, mode)

#else

#define rthal_virtualize_irq(dom, irq, isr, cookie, ackfn, mode) \
	ipipe_virtualize_irq(dom, irq, isr, cookie, ackfn, mode)

#endif

struct xnintr;

typedef int (*xnisr_t)(struct xnintr *intr);

typedef int (*xniack_t)(unsigned irq);

typedef unsigned long xnflags_t;

typedef atomic_t atomic_counter_t;

typedef RTIME xnticks_t;

typedef struct xnstat_exectime {
	xnticks_t start;
	xnticks_t total;
} xnstat_exectime_t;

typedef struct xnstat_counter {
	int counter;
} xnstat_counter_t;
#define xnstat_counter_inc(c)  ((c)->counter++)

typedef struct xnintr {
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
#define xnarch_before_atomic_dec()  smp_mb__before_atomic_dec()
#define xnarch_after_atomic_dec()    smp_mb__after_atomic_dec()
#define xnarch_before_atomic_inc()  smp_mb__before_atomic_inc()
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
#define xnarch_end_irq     rt_enable_irq

#define xnarch_hook_irq(irq, handler, iack, intr) \
	rt_request_irq_wack(irq, (void *)handler, intr, 0, (void *)iack);
#define xnarch_release_irq(irq) \
	rt_release_irq(irq);

extern struct rtai_realtime_irq_s rtai_realtime_irq[];
//#define xnarch_get_irq_cookie(irq)  (rtai_realtime_irq[irq].cookie)
#define xnarch_get_irq_cookie(irq)  (rtai_domain.irqs[irq].cookie)

extern unsigned long IsolCpusMask;
#define xnarch_set_irq_affinity(irq, nkaffinity) \
	rt_assign_irq_to_cpu(irq, IsolCpusMask)

// support for RTDM timers

struct rtdm_timer_struct {
	struct rtdm_timer_struct *next, *prev;
	int priority, cpuid;
	RTIME firing_time, period;
	void (*handler)(unsigned long);
	unsigned long data;
#ifdef  CONFIG_RTAI_LONG_TIMED_LIST
	rb_root_t rbr;
	rb_node_t rbn;
#endif
};

RTAI_SYSCALL_MODE void rt_timer_remove(struct rtdm_timer_struct *timer);

RTAI_SYSCALL_MODE int rt_timer_insert(struct rtdm_timer_struct *timer, int priority, RTIME firing_time, RTIME period, void (*handler)(unsigned long), unsigned long data);

typedef struct rtdm_timer_struct xntimer_t;

#define XN_INFINITE  (0)

/* Timer modes */
typedef enum xntmode {
	XN_RELATIVE,
	XN_ABSOLUTE,
	XN_REALTIME
} xntmode_t;

#define xntbase_ns2ticks(rtdm_tbase, expiry)  nano2count(expiry)

static inline void xntimer_init(xntimer_t *timer, void (*handler)(xntimer_t *))
{
	memset(timer, 0, sizeof(struct rtdm_timer_struct));
	timer->handler = (void *)handler;
	timer->data    = (unsigned long)timer;
	timer->next    =  timer->prev = timer;
}

#define xntimer_set_name(timer, name)

static inline int xntimer_start(xntimer_t *timer, xnticks_t value, xnticks_t interval, int mode)
{
	return rt_timer_insert(timer, 0, value, interval, timer->handler, (unsigned long)timer);
}

static inline void xntimer_destroy(xntimer_t *timer)
{
	rt_timer_remove(timer);
}

static inline void xntimer_stop(xntimer_t *timer)
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

typedef struct xnholder {
	struct xnholder *next;
	struct xnholder *prev;
} xnholder_t;

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

#define xnthread_t	    RT_TASK
#define xnpod_current_thread  _rt_whoami
#define xnthread_test_info    rt_task_test_taskq_retval

#define xnsynch_t		   TASKQ
#define xnsynch_init(s, f, p)       rt_taskq_init(s, f)
#define xnsynch_destroy	     rt_taskq_delete
#define xnsynch_wakeup_one_sleeper  rt_taskq_ready_one
#define xnsynch_flush	       rt_taskq_ready_all
static inline void xnsynch_sleep_on(void *synch, xnticks_t timeout, xntmode_t timeout_mode)
{
	if (timeout == XN_INFINITE) {
		rt_taskq_wait(synch);
	} else {
		rt_taskq_wait_until(synch, timeout_mode == XN_RELATIVE ? rt_get_time() + timeout : timeout);
	}
}

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

#define SELECT_SIGNAL(select_block, state)  do { } while (0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define __WORK_INITIALIZER(n,f,d) {			     \
	.list   = { &(n).list, &(n).list },		     \
	.sync = 0,					      \
	.routine = (f),					 \
	.data = (d),					    \
}

#define DECLARE_WORK(n,f,d)	     struct tq_struct n = __WORK_INITIALIZER(n, f, d)
#define DECLARE_WORK_NODATA(n, f)       DECLARE_WORK(n, f, NULL)
#define DECLARE_WORK_FUNC(f)	    void f(void *cookie)
#define DECLARE_DELAYED_WORK_NODATA(n, f) DECLARE_WORK(n, f, NULL)

#define schedule_delayed_work(work, delay) do {		 \
	if (delay) {					    \
		set_current_state(TASK_UNINTERRUPTIBLE);	\
		schedule_timeout(delay);			\
	}						       \
	schedule_task(work);				    \
} while (0)

#else

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
#define DECLARE_WORK_NODATA(f, n)       DECLARE_WORK(f, n, NULL)
#define DECLARE_WORK_FUNC(f)	    void f(void *cookie)
#define DECLARE_DELAYED_WORK_NODATA(n, f) DECLARE_DELAYED_WORK(n, f, NULL)
#else /* >= 2.6.20 */
#define DECLARE_WORK_NODATA(f, n)       DECLARE_WORK(f, n)
#define DECLARE_WORK_FUNC(f)	    void f(struct work_struct *work)
#define DECLARE_DELAYED_WORK_NODATA(n, f) DECLARE_DELAYED_WORK(n, f)
#endif /* >= 2.6.20 */

#endif

#endif /* !_RTAI_XNSTUFF_H */
