/* rtai/include/asm-arm/arch-pxa/rtai_arch.h
-------------------------------------------------------------
DON´T include directly - it's included through asm-arm/rtai.h
-------------------------------------------------------------
COPYRIGHT (C) 2002 Guennadi Liakhovetski, DSA GmbH (gl@dsa-ac.de)
COPYRIGHT (C) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
Copyright (c) 2005 Luca Pizzi, (lucapizzi@hotmail.com)
Copyright (c) 2005 Stefano Gafforelli, (stefano.gafforelli@tiscali.it)
Copyright (c) 2006 Torsten Koschorrek, (koschorrek@synertronixx.de)

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
- port to imx architecture using both
  arch-pxa/rtai_arch.h and arch-ep9301/rtai_arch.h
  20060613: Torsten Koschorrek (koschorrek@synertronixx.de)
*/

#ifndef _ASM_ARCH_RTAI_ARCH_H_
#define _ASM_ARCH_RTAI_ARCH_H_

#include <asm/leds.h>

#define RTAI_TSC_FREQ		200000000 /* 32768 kHz */

/* interrupt-timer related values
 * ============================== */

/* irq number of timer interrupt */
#define RTAI_TIMER_IRQ		TIM1_INT

/* name of timer */
#define RTAI_TIMER_NAME		"TIMER1"

/* maximal timer load value */
#define RTAI_TIMER_MAXVAL	(CLOCK_TICK_RATE / 100)

/* clock frequency of timer that generates timer-interrupt [Hz] */
#define RTAI_TIMER_FREQ		CLOCK_TICK_RATE /* 32768 kHz */

/* - oneshot timer latency (is subtracted from oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution (because it is used this way in the scheduler)) */
#define RTAI_TIMER_LATENCY \
0
//((int)((6 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

/* - oneshot timer setup delay (i.e. minimal oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution) */
#define RTAI_TIMER_SETUP_TIME \
0
//((int)((3 * 1000000000LL + RTAI_TSC_FREQ/2) / (long long)RTAI_TSC_FREQ))

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
