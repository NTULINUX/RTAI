/*
 * ARM/EP9301 specific timer related stuff
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
#ifndef _ASM_ARCH_RTAI_TIMER_H_
#define _ASM_ARCH_RTAI_TIMER_H_

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/ep93xx_tsc.h>

/* acknowledge & unmask timer interrupt */
extern inline int
rtai_timer_irq_ack(void)
{
    rt_unmask_irq(RTAI_TIMER_IRQ);
    /* *TODO* when return -1? (return < 0 => abort rt_timer_handler() for this interrupt) */
    return 0;
}

/* read time-stamp-counter */
#define rtai_rdtsc()		ep93xx_rdtsc()

/* set irq-timer's delay */
extern inline void
rt_set_timer_delay(unsigned int delay)
{
    ADEOS_PARANOIA_ASSERT(adeos_hw_irqs_disabled());
    if (delay) {
	/*
	 * one-shot mode: reprogramm timer
	 */
	outl(0, TIMER1CONTROL);		/* stop timer */
	outl(delay - 1, TIMER1LOAD);	/* set load value */
	outl(0x88, TIMER1CONTROL);	/* set 508 kHz clock, free running mode & enable timer */
    } else {
	/*
	 * periodic mode: nothing to do as the timer reloads itself
	 * with the value set in rt_request_timer()
	 */
    }
}

#endif /* _ASM_ARCH_RTAI_TIMER_H_ */
