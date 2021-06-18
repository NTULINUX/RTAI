/* rtai/include/asm-arm/arch-at91/rtai_arch.h
-------------------------------------------------------------
DONT include directly - it's included through asm-arm/rtai.h
-------------------------------------------------------------
COPYRIGHT (C) 2002 Guennadi Liakhovetski, DSA GmbH (gl@dsa-ac.de)
COPYRIGHT (C) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
Copyright (c) 2005 Luca Pizzi, (lucapizzi@hotmail.com)
Copyright (c) 2005 Stefano Gafforelli, (stefano.gafforelli@tiscali.it)
Copyright (C) 2007 Adeneo

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

#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>

/* irq number of timer interrupt */
#define RTAI_TIMER_IRQ		__ipipe_mach_timerint

/* clock frequency of timer that generates timer-interrupt [Hz] */
#define RTAI_TIMER_FREQ		CLOCK_TICK_RATE

/* maximal timer load value */
#define RTAI_TIMER_MAXVAL	0xFFFF

/* clock frequency of time-stamp-counter (TSC) (TC on AT91) [Hz] */
#define RTAI_TSC_FREQ		CLOCK_TICK_RATE

/* - oneshot timer latency (is subtracted from oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution (because it is used this way in the scheduler)) */
#define RTAI_TIMER_LATENCY	6000

/* - oneshot timer setup delay (i.e. minimal oneshot delay) [nanoseconds]
 *   (specify it with TSC resolution) */
#define RTAI_TIMER_SETUP_TIME	1500

/* name of timer */
#define RTAI_TIMER_NAME		"TIMER1"

#define RTAI_CALIBRATED_CPU_FREQ	0
#define RTAI_CALIBRATED_APIC_FREQ	0

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
