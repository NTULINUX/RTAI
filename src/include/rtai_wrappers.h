/*
 * Copyright (C) 2004-2017 Philippe Gerum <rpm@xenomai.org>
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

#ifndef cpu_online_map
#define cpu_online_map (*(cpumask_t *)cpu_online_mask)
#endif

#ifndef init_MUTEX_LOCKED 
#define init_MUTEX_LOCKED(sem)  sema_init(sem, 0)
#endif

#define RTAI_MODULE_PARM_ARRAY(name, type, addr, size) \
	module_param_array(name, type, addr, 0400);

/* Basic class macros */

#include <linux/device.h>
typedef struct class class_t;
#define CLASS_DEVICE_CREATE(cls, devt, device, fmt, arg...)  device_create(cls, NULL, devt, NULL, fmt, ##arg)
#define class_device_destroy(a, b)  device_destroy(a, b)

#define mm_remap_page_range(vma,from,to,size,prot) remap_page_range(vma,from,to,size,prot)

#define get_tsk_addr_limit(t) ((t)->thread_info->addr_limit.seg)

#define self_daemonize(name) daemonize(name)

#define get_thread_ptr(t)  ((t)->thread_info)

#define RTAI_LINUX_IRQ_HANDLED IRQ_HANDLED

#define CPUMASK_T(name)  ((cpumask_t){ { name } })
#define CPUMASK(name)    (name.bits[0])

#include <linux/pid.h>

#define find_task_by_pid(nr) \
	find_task_by_pid_ns(nr, &init_pid_ns)
#define kill_proc(pid, sig, priv)       \
  kill_proc_info(sig, (priv) ? SEND_SIG_PRIV : SEND_SIG_NOINFO, pid)

#endif /* __KERNEL__ */

#endif /* !_RTAI_WRAPPERS_H */
