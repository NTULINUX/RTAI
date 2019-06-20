/*
 *   Copyright (C) 2004 Paolo Mantegazza (mantegazza@aero.polimi.it)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or 
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


//#define EMULATE_TSC  /* to force emulation even if CONFIG_X86_TSC */

#ifndef _RTAI_ASM_EMULATE_TSC_H
#define _RTAI_ASM_EMULATE_TSC_H

#if defined(EMULATE_TSC) || !defined(CONFIG_X86_TSC)

#undef RTAI_CPU_FREQ
#undef RTAI_CALIBRATED_CPU_FREQ
#undef rdtsc
#undef rtai_rdtsc
#undef DECLR_8254_TSC_EMULATION
#undef TICK_8254_TSC_EMULATION
#undef SETUP_8254_TSC_EMULATION
#undef CLEAR_8254_TSC_EMULATION

#define RTAI_CPU_FREQ             RTAI_FREQ_8254
#define RTAI_CALIBRATED_CPU_FREQ  RTAI_FREQ_8254
#define rtai_rdtsc()              rd_8254_ts()
#define rdtsc()                   rd_8254_ts()

#define TICK_8254_TSC_EMULATION()  rd_8254_ts()

#include <linux/version.h>
#if defined(CONFIG_VT) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define DECLR_8254_TSC_EMULATION \
extern void *kd_mksound; \
static void *linux_mksound; \
static void rtai_mksound(void) { } \
const int TSC_EMULATION_GUARD_FREQ = 20; \
static struct timer_list timer;

#define SETUP_8254_TSC_EMULATION \
	do { \
		linux_mksound = kd_mksound; \
		kd_mksound = rtai_mksound; \
		rt_setup_8254_tsc(); \
		init_timer(&timer); \
		timer.function = timer_fun; \
		timer_fun(0); \
	} while (0)

#define CLEAR_8254_TSC_EMULATION \
	do { \
		del_timer(&timer); \
		if (linux_mksound) { \
			kd_mksound = linux_mksound; \
		} \
	} while (0)

#else /* !CONFIG_VT */

#define DECLR_8254_TSC_EMULATION \
const int TSC_EMULATION_GUARD_FREQ = 20; \
static struct timer_list timer;

#define SETUP_8254_TSC_EMULATION \
	do { \
		rt_setup_8254_tsc(); \
		init_timer(&timer); \
		timer.function = timer_fun; \
		timer_fun(0); \
	} while (0)

#define CLEAR_8254_TSC_EMULATION del_timer(&timer)

#endif /* !CONFIG_VT */

#endif

#endif /* !_RTAI_ASM_EMULATE_TSC_H */
