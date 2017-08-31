/*
 * Copyright 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
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


#ifndef _RTAI_HAL_NAMES_H
#define _RTAI_HAL_NAMES_H

#include <linux/version.h>

#define TSKEXT0  (HAL_ROOT_NPTDKEYS - 4)
#define TSKEXT1  (HAL_ROOT_NPTDKEYS - 3)
#define TSKEXT2  (HAL_ROOT_NPTDKEYS - 2)
#define TSKEXT3  (HAL_ROOT_NPTDKEYS - 1)

#define HAL_VERSION_STRING   IPIPE_VERSION_STRING

#define HAL_NR_CPUS	  IPIPE_NR_CPUS
#define HAL_NR_FAULTS	IPIPE_NR_FAULTS
#define HAL_NR_EVENTS	IPIPE_NR_EVENTS
#define HAL_ROOT_NPTDKEYS    IPIPE_ROOT_NPTDKEYS

#define HAL_APIC_HIGH_VECTOR  IPIPE_HRTIMER_VECTOR //IPIPE_SERVICE_VECTOR3
#define HAL_APIC_LOW_VECTOR   IPIPE_RESCHEDULE_VECTOR //IPIPE_SERVICE_VECTOR2

#define HAL_SCHEDULE_HEAD     IPIPE_EVENT_SCHEDULE
#define HAL_SCHEDULE_TAIL     (IPIPE_FIRST_EVENT - 2)  // safely unused, cared
#define HAL_SYSCALL_PROLOGUE  IPIPE_EVENT_SYSCALL
#define HAL_SYSCALL_EPILOGUE  1000000 // invalid for sure, will be rejected, OK
#define HAL_EXIT_PROCESS      IPIPE_EVENT_EXIT
#define HAL_KICK_PROCESS      IPIPE_EVENT_SIGWAKE

#define hal_pipeline	__ipipe_pipeline
#define hal_domain_struct   ipipe_domain
#define hal_root_domain     ipipe_root_domain

#define hal_current_domain(cpuid)  per_cpu(ipipe_percpu_domain, cpuid)

#define hal_propagate_irq   ipipe_propagate_irq
#define hal_schedule_irq    ipipe_schedule_irq

#define hal_critical_enter  ipipe_critical_enter
#define hal_critical_exit   ipipe_critical_exit

#define hal_clear_irq(a, b)

#define hal_lock_irq    __ipipe_lock_irq
#define hal_unlock_irq  __ipipe_unlock_irq

#define hal_std_irq_dtype	__ipipe_std_irq_dtype
#define hal_ipipe_std_irq_dtype  __adeos_std_irq_dtype

#define hal_tick_regs  __ipipe_tick_regs
#define hal_tick_irq   __ipipe_tick_irq

#define hal_sync_stage  __ipipe_sync_stage

#define hal_set_irq_affinity  ipipe_set_irq_affinity

#define hal_propagate_event  ipipe_propagate_event

#define hal_get_sysinfo  ipipe_get_sysinfo

#define HAL_TYPE  "IPIPE-NOTHREADS"

#define INTERCEPT_WAKE_UP_TASK(data)  ((struct task_struct *)data)

#define FIRST_LINE_OF_RTAI_DOMAIN_ENTRY  static void rtai_domain_entry(void) { if (1)
#define LAST_LINE_OF_RTAI_DOMAIN_ENTRY   }

#define hal_suspend_domain()  break

#define hal_alloc_irq       ipipe_alloc_virq
#define hal_free_irq	ipipe_free_virq

#define hal_virtualize_irq(d, n, h, a, m) \
	ipipe_virtualize_irq(d, n, (void *)h, NULL, a, m)
#define hal_irq_hits_pp(irq, domain, cpuid) \
do { domain->cpudata[cpuid].irq_counters[irq].pending_hits++; } while (0)

#define hal_sysinfo_struct	 ipipe_sysinfo
#define hal_attr_struct	    ipipe_domain_attr
#define hal_init_attr	      ipipe_init_attr
#define hal_register_domain	ipipe_register_domain
#define hal_unregister_domain      ipipe_unregister_domain
#define hal_catch_event	    ipipe_catch_event
#define hal_event_handler	  ipipe_event_handler
#define hal_event_handler_fun(e)   legacy.handlers[e] //evhand[e]

#define hal_set_printk_sync   ipipe_set_printk_sync
#define hal_set_printk_async  ipipe_set_printk_async

#define hal_schedule_back_root(prev) \
do { \
	if ((prev)->rtai_tskext(HAL_ROOT_NPTDKEYS - 1)) { \
		ipipe_reenter_root((prev)->rtai_tskext(HAL_ROOT_NPTDKEYS - 1), (prev)->policy, (prev)->rt_priority); \
		(prev)->rtai_tskext(HAL_ROOT_NPTDKEYS - 1) = NULL; \
	} else { \
		ipipe_reenter_root(prev, (prev)->policy, (prev)->rt_priority); \
	} \
} while (0)

#define hal_processor_id  ipipe_processor_id

#define hal_hw_cli		local_irq_disable_hw
#define hal_hw_sti		local_irq_enable_hw
#define hal_hw_local_irq_save     local_irq_save_hw
#define hal_hw_local_irq_restore  local_irq_restore_hw
#define hal_hw_local_irq_flags    local_save_flags_hw

#define hal_set_timer(ns)  ipipe_tune_timer(ns, 0)
#define hal_reset_timer()  ipipe_tune_timer(0, IPIPE_RESET_TIMER)

#define hal_unstall_pipeline_from  ipipe_unstall_pipeline_from

#define hal_ack_system_irq  __ipipe_ack_apic

#define hal_irq_handler     rtai_irq_handler

#define hal_tskext  ptd

#define hal_set_linux_task_priority  ipipe_setscheduler_root

#if defined(IPIPE_ROOT_NPTDKEYS) && TSKEXT0 < 0
#error *** TSKEXTs WILL CAUSE MEMORY LEAKS, CHECK BOUNDS IN HAL PATCHES ***
#endif

#endif
