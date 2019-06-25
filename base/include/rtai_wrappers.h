/*
 * Copyright (C) 2004-2017 Philippe Gerum <rpm@xenomai.org>
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

#ifndef _RTAI_WRAPPERS_H
#define _RTAI_WRAPPERS_H

#ifdef __KERNEL__

#include <linux/version.h>
#ifndef __cplusplus
#include <linux/module.h>
#endif /* !__cplusplus */

#include <linux/moduleparam.h>

#define RTAI_MODULE_PARM(name, type) \
	module_param(name, type, 0444)

#ifndef DEFINE_SPINLOCK
#define DEFINE_SPINLOCK(x) spinlock_t x = SPIN_LOCK_UNLOCKED
#endif

#ifndef DECLARE_MUTEX_LOCKED
#ifndef __DECLARE_SEMAPHORE_GENERIC
#define DECLARE_MUTEX_LOCKED(name) \
        struct semaphore name = __SEMAPHORE_INITIALIZER(name, 0)
#else
#define DECLARE_MUTEX_LOCKED(name) __DECLARE_SEMAPHORE_GENERIC(name,0)
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define IRQF_SHARED  SA_SHIRQ
#endif

#ifndef cpu_online_map
#define cpu_online_map (*(cpumask_t *)cpu_online_mask)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#include <linux/bitops.h>

typedef int clockid_t;

#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME  0x01
#endif

typedef int timer_t;

#ifndef __deprecated
#define container_of(ptr, type, member) \
	({ const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef __deprecated
#define __deprecated  __attribute__((deprecated))
#endif

#define RTAI_MODULE_PARM_ARRAY(name, type, addr, size) \
        static inline void *__check_existence_##name(void) { return &name; } \
	MODULE_PARM(name, "1-" __MODULE_STRING(size) _MODULE_PARM_STRING_ ## type);
#define _MODULE_PARM_STRING_charp "s"

#define PID_MAX_LIMIT     PID_MAX
#define num_online_cpus() smp_num_cpus
#define mm_remap_page_range(vma,from,to,size,prot) remap_page_range(from,to,size,prot)
#define __user

#define set_tsk_need_resched(t) do { \
    (t)->need_resched = 1; \
} while(0)

#define clear_tsk_need_resched(t) do { \
    (t)->need_resched = 0; \
} while(0)

#define set_need_resched() set_tsk_need_resched(current)

#define LIST_POISON1  NULL
#define LIST_POISON2  NULL

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23) || __cplusplus
typedef void irqreturn_t;
#endif  /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23) || __cplusplus */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,28)
#define msleep(delay) \
do { \
	set_current_state(TASK_RTAISRVSLEEP); \
	schedule_timeout(((delay)*HZ)/1000); \
} while(0)
#endif

#define get_tsk_addr_limit(t) ((t)->addr_limit.seg)

#define task_cpu(t) ((t)->processor)

#define self_daemonize(name) do { \
   strcpy(current->comm,"gatekeeper"); \
   daemonize(); \
} while(0)

#define get_thread_ptr(t)  (t)

#define RTAI_LINUX_IRQ_HANDLED	/* i.e. "void" return */

static inline unsigned long hweight_long(unsigned long w)
{
        return sizeof(w) == 4 ? hweight32(w) : hweight32(w);
}

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#ifndef init_MUTEX_LOCKED 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
#define init_MUTEX_LOCKED(sem)  sema_init(sem, 0)
#endif
#endif

#define RTAI_MODULE_PARM_ARRAY(name, type, addr, size) \
	module_param_array(name, type, addr, 0400);

/* Basic class macros */

#ifdef CONFIG_SYSFS
#include <linux/device.h>
#if  LINUX_VERSION_CODE > KERNEL_VERSION(2,6,25)
typedef struct class class_t;

#if  LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
#define CLASS_DEVICE_CREATE(cls, devt, device, fmt, arg...)  device_create(cls, NULL, devt, NULL, fmt, ##arg)
#else  /* < 2.6.27 */
#define CLASS_DEVICE_CREATE(cls, devt, device, fmt, arg...)  device_create(cls, NULL, devt, fmt, ##arg)
#endif  /* > 2.6.26 */

#define class_device_destroy(a, b)  device_destroy(a, b)
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
typedef struct class class_t;
#define CLASS_DEVICE_CREATE(cls, devt, device, fmt, arg...) class_device_create(cls, NULL, devt, device, fmt, ## arg)
#else /* LINUX 2.6.0 - 2.6.14 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13) /* LINUX 2.6.13 - 2.6.14 */
typedef struct class class_t;
#define CLASS_DEVICE_CREATE class_device_create
#else /* LINUX 2.6.0 - 2.6.12, class_simple */
typedef struct class_simple class_t;
#define CLASS_DEVICE_CREATE class_simple_device_add
#define class_create class_simple_create
#define class_destroy class_simple_destroy
#define class_device_destroy(a, b) class_simple_device_remove(b)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,13) */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) */
#endif
#endif

#define mm_remap_page_range(vma,from,to,size,prot) remap_page_range(vma,from,to,size,prot)

#define get_tsk_addr_limit(t) ((t)->thread_info->addr_limit.seg)

#define self_daemonize(name) daemonize(name)

#define get_thread_ptr(t)  ((t)->thread_info)

#define RTAI_LINUX_IRQ_HANDLED IRQ_HANDLED

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(s)	/* For really old 2.4 kernels. */
#endif /* !MODULE_LICENSE */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,7)
#define CPUMASK_T(name) (name)
#define CPUMASK(name)   (name)
#else /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7) */
#define CPUMASK_T(name)  ((cpumask_t){ { name } })
#define CPUMASK(name)    (name.bits[0])
#endif /* LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,7) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)

#include <linux/pid.h>

#define find_task_by_pid(nr) \
	find_task_by_pid_ns(nr, &init_pid_ns)
#define kill_proc(pid, sig, priv)       \
  kill_proc_info(sig, (priv) ? SEND_SIG_PRIV : SEND_SIG_NOINFO, pid)

#endif /* LINUX_VERSION_CODE <= 2.6.26 */

#ifndef CONFIG_SYSFS
typedef void * class_t;
#define class_create(a,b)  ((void *)1)
#define CLASS_DEVICE_CREATE(a, b, c, d, ...)  ((void *)1) 
#define class_device_destroy(a, b)
#define class_destroy(a)
#endif

#endif /* __KERNEL__ */

#endif /* !_RTAI_WRAPPERS_H */
