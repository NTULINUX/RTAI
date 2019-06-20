/*
 * ARM/EP9301 specific parameters
 *
 * Copyright (c) 2004-2005 Michael Neuhauser, Firmix Software GmbH (mike@firmix.at)
 *
 * Acknowledgements:
 *	Paolo Mantegazza <mantegazza@aero.polimi.it>, creator of RTAI 
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
#ifndef _ASM_ARCH_RTAI_ARCH_H_
#define _ASM_ARCH_RTAI_ARCH_H_

#include <asm/system.h>
#include <asm/arch/ep93xx_tsc.h>

/* clock frequency of time-stamp-counter (TSC) (timer 4 on EP9301) [Hz] */
#define RTAI_TSC_FREQ		FREQ_EP93XX_TSC

/* EP9301 ties linux-jiffies to TSC, so we don't need the jiffies-recover handler */
#define USE_LINUX_TIMER_WITHOUT_RECOVER	1

/* interrupt-timer related values
 * ============================== */

/* irq number of timer interrupt */
#define RTAI_TIMER_IRQ		IRQ_TIMER1

/* name of timer */
#define RTAI_TIMER_NAME		"TIMER1"

/* maximal timer load value */
#define RTAI_TIMER_MAXVAL	0xFFFF

/* clock frequency of timer that generates timer-interrupt [Hz] */
#define RTAI_TIMER_FREQ		CLOCK_TICK_RATE

/* - oneshot timer latency (is subtracted from oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution (because it is used this way in the scheduler)) */
#define RTAI_TIMER_LATENCY \
    ((int)((6 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

/* - oneshot timer setup delay (i.e. minimal oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution) */
#define RTAI_TIMER_SETUP_TIME \
    ((int)((3 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

/* this machine doesn't have multiplexed IRQs */
#define ARCH_MUX_IRQ		NO_IRQ
#define isdemuxirq(irq)		(0)

/*
 * hooks for architecture specific init/exit actions
 */

extern inline void
rtai_archdep_init(void)
{
    /* nothing to do */
}

extern inline void
rtai_archdep_exit(void)
{
    /* nothing to do */
}

#endif /* _ASM_ARCH_RTAI_ARCH_H_ */
