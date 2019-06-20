/*
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   ARTI for x86 and RTHAL for ARM. This file provides user-visible
 *   definitions for compatibility purpose with the legacy RTHAL. Must
 *   be included from rtai_hal.h only.
 *
 * Original RTAI/x86 layer implementation:
 *   Copyright (c) 2000 Paolo Mantegazza (mantegazza@aero.polimi.it)
 *   Copyright (c) 2000 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2000 Stuart Hughes
 *   and others.
 *
 * RTAI/x86 rewrite over Adeos:
 *   Copyright (c) 2002 Philippe Gerum (rpm@xenomai.org)
 *
 * Original RTAI/ARM RTHAL implementation:
 *   Copyright (c) 2000 Pierre Cloutier (pcloutier@poseidoncontrols.com)
 *   Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
 *   Copyright (c) 2002 Guennadi Liakhovetski DSA GmbH (gl@dsa-ac.de)
 *   Copyright (c) 2002 Steve Papacharalambous (stevep@zentropix.com)
 *   Copyright (c) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
 *   Copyright (c) 2003 Bernard Haible, Marconi Communications
 *   Copyright (c) 2003 Thomas Gleixner (tglx@linutronix.de)
 *   Copyright (c) 2003 Philippe Gerum (rpm@xenomai.org)
 *
 * RTAI/ARM over Adeos rewrite:
 *   Copyright (c) 2004-2005 Michael Neuhauser, Firmix Software GmbH (mike@firmix.at)
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _RTAI_ASM_ARM_OLDNAMES_H
#define _RTAI_ASM_ARM_OLDNAMES_H

#ifdef __KERNEL__

#define hard_cli()                   	rtai_cli()
#define hard_sti()                   	rtai_sti()
#define hard_save_flags_and_cli(x)   	rtai_save_flags_and_cli(x)
#define hard_save_flags_cli(x)		rtai_save_flags_and_cli(x)
#define hard_restore_flags(x)        	rtai_restore_flags(x)
#define hard_save_flags(x)           	rtai_save_flags(x)
#define hard_cpu_id                  	hal_processor_id
#define this_rt_task                 	ptd

#endif /* __KERNEL__ */

#ifndef __RTAI_HAL__

#define tuned          			rtai_tunables
#define NR_RT_CPUS     			RTAI_NR_CPUS
#define RT_TIME_END    			RTAI_TIME_LIMIT

#define CPU_FREQ       			RTAI_CPU_FREQ
#define TIMER_8254_IRQ 			RTAI_TIMER_IRQ
#define FREQ_8254      			RTAI_TIMER_FREQ
#define LATENCY_8254   			RTAI_TIMER_LATENCY
#define SETUP_TIME_8254			RTAI_TIMER_SETUP_TIME

#define FREQ_APIC       		RTAI_TIMER_FREQ
#define LATENCY_APIC    		RTAI_TIMER_LATENCY
#define SETUP_TIME_APIC 		RTAI_TIMER_SETUP_TIME		
#define RTAI_FREQ_APIC			RTAI_TIMER_FREQ

#define CALIBRATED_APIC_FREQ  		RTAI_CALIBRATED_APIC_FREQ
#define CALIBRATED_CPU_FREQ   		RTAI_CALIBRATED_CPU_FREQ

#ifdef __KERNEL__

#undef  rdtsc
#define rdtsc()     			rtai_rdtsc()
#define rd_CPU_ts() 			rtai_rdtsc()

#define rt_set_rtai_trap_handler  	rt_set_trap_handler
#define rt_mount_rtai   		rt_mount
#define rt_umount_rtai  		rt_umount
#define calibrate_8254  		rtai_calibrate_TC

#define ulldiv(a,b,c)  			rtai_ulldiv(a,b,c)
#define imuldiv(a,b,c) 			rtai_imuldiv(a,b,c)
#define llimd(a,b,c)   			rtai_llimd(a,b,c)

#define rt_reset_irq_to_sym_mode(irq)
#define rt_assign_irq_to_cpu(irq, cpu)

#ifndef __cplusplus

#include <linux/irq.h>

extern inline int
rt_request_cpu_own_irq(unsigned irq, rt_irq_handler_t handler)
{
    return rt_request_irq(irq, handler, NULL, 0);
}

extern inline int
rt_free_cpu_own_irq(unsigned irq)
{
    return rt_release_irq(irq);
}

#endif /* !__cplusplus */

#endif /* __KERNEL__ */

#endif /* !__RTAI_HAL__ */

#endif /* !_RTAI_ASM_ARM_OLDNAMES_H */
