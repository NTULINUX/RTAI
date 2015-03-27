/**
 *   @ingroup hal
 *   @file
 *
 *   Copyright: 2013 Paolo Mantegazza.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _RTAI_ASM_X86_HAL_H
#define _RTAI_ASM_X86_HAL_H

#define RTAI_KERN_BUSY_ALIGN_RET_DELAY  CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY
#define RTAI_USER_BUSY_ALIGN_RET_DELAY  CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY

#include <rtai_types.h>

#ifdef CONFIG_SMP
#define RTAI_NR_CPUS  CONFIG_RTAI_CPUS
#else /* !CONFIG_SMP */
#define RTAI_NR_CPUS  1
#endif /* CONFIG_SMP */

struct calibration_data {
	unsigned long cpu_freq;
	unsigned long apic_freq;
	int latency;
	int kern_latency_busy_align_ret_delay;
	int user_latency_busy_align_ret_delay;
	int setup_time_TIMER_CPUNIT;
	int setup_time_TIMER_UNIT;
	int timers_tol[RTAI_NR_CPUS];
};

int rtai_calibrate_hard_timer(void);

#ifdef __i386__
#include "rtai_hal_32.h"
#else
#include "rtai_hal_64.h"
#endif

#ifdef CONFIG_RTAI_TSC
static inline RTIME rt_get_tscnt(void)
{
#ifdef __i386__
        unsigned long long t;
        __asm__ __volatile__ ("rdtsc" : "=A" (t));
       return t;
#else
        union { unsigned int __ad[2]; RTIME t; } t;
        __asm__ __volatile__ ("rdtsc" : "=a" (t.__ad[0]), "=d" (t.__ad[1]));
        return t.t;
#endif
}
#else
#define rt_get_tscnt  rt_get_time
#endif

#endif /* !_RTAI_ASM_X86_HAL_H */
