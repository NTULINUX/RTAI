/* rtai/include/asm-arm/arch-pxa/rtai_arch.h
-------------------------------------------------------------
DON´T include directly - it's included through asm-arm/rtai.h
-------------------------------------------------------------
COPYRIGHT (C) 2002 Guennadi Liakhovetski, DSA GmbH (gl@dsa-ac.de)
COPYRIGHT (C) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
Copyright (c) 2005 Luca Pizzi, (lucapizzi@hotmail.com)
Copyright (c) 2005 Stefano Gafforelli, (stefano.gafforelli@tiscali.it)

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
--------------------------------------------------------------------------
Acknowledgements
- Paolo Mantegazza	(mantegazza@aero.polimi.it)
	creator of RTAI
*/

#ifndef _ASM_ARCH_RTAI_ARCH_H_
#define _ASM_ARCH_RTAI_ARCH_H_

#define FREQ_SYS_CLK       3686400
#define LATENCY_MATCH_REG     2000
#define SETUP_TIME_MATCH_REG   600
#define LATENCY_TICKS    (LATENCY_MATCH_REG/(1000000000/FREQ_SYS_CLK))
#define SETUP_TIME_TICKS (SETUP_TIME_MATCH_REG/(1000000000/FREQ_SYS_CLK))

#define RTAI_TIMER_IRQ		IRQ_OST0

#define ARCH_MUX_IRQ		IRQ_GPIO_2_80
#define RTAI_TIMER_MAXVAL   0xFFFF

/* clock frequency of timer that generates timer-interrupt [Hz] */
#define RTAI_TIMER_FREQ		CLOCK_TICK_RATE
#include <asm/arch/irq.h>

#define RTAI_TSC_FREQ		FREQ_SYS_CLK

#define RTAI_CALIBRATED_CPU_FREQ  	RTAI_TSC_FREQ

/* name of timer */
#define RTAI_TIMER_NAME		"TIMER1"

/* - oneshot timer latency (is subtracted from oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution (because it is used this way in the scheduler)) */
#define RTAI_TIMER_LATENCY \
0
//    ((int)((6 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

/* - oneshot timer setup delay (i.e. minimal oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution) */
#define RTAI_TIMER_SETUP_TIME \
0
//    ((int)((3 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

static unsigned long rtai_cpufreq_arg = RTAI_CALIBRATED_CPU_FREQ;

extern struct calibration_data		rtai_tunables;

void rtai_pxa_GPIO_2_80_demux( int irq, void *dev_id, struct pt_regs *regs );

#if 0
static inline void arch_mount_rtai( void )
{
	/* Let's take care about our "special" IRQ11 */
//	free_irq( IRQ_GPIO_2_80 );
	rt_request_irq( IRQ_GPIO_2_80, (void *) rtai_pxa_GPIO_2_80_demux, 0, 0);
}

static inline void arch_umount_rtai( void )
{
	rt_release_irq( IRQ_GPIO_2_80 );
//	request_irq( IRQ_GPIO_2_80, pxa_GPIO_2_80_demux, SA_INTERRUPT, "GPIO 2-80", NULL );
}
#endif

extern inline void
rtai_archdep_init(void)
{
	if (rtai_cpufreq_arg == 0) {
		adsysinfo_t sysinfo;
		adeos_get_sysinfo(&sysinfo);
		rtai_cpufreq_arg = (unsigned long)sysinfo.cpufreq;
	}
}

extern inline void
rtai_archdep_exit(void)
{
    /* nothing to do */
}

/* Check, if this is a demultiplexed irq */
#define isdemuxirq(irq) (irq >= IRQ_GPIO(2))

#endif

