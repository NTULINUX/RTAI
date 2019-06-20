/* 020222 asm-arm/arch-pxa/timer.h - ARM/PXA255 specific timer
Don't include directly - it's included through asm-arm/rtai.h

Copyright (c) 2006 Torsten Koschorrek, (koschorrek@synertronixx.de)
Copyright (c) 2005 Luca Pizzi, (lucapizzi@hotmail.com)
Copyright (c) 2005 Stefano Gafforelli, (stefano.gafforelli@tiscali.it)
COPYRIGHT (C) 2002 Guennadi Liakhovetski, DSA GmbH (gl@dsa-ac.de)
COPYRIGHT (C) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)

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
  arch-pxa/rtai_timer.h and arch-ep9301/rtai_timer.h
  20060613: Torsten Koschorrek (koschorrek@synertronixx.de)
*/

#ifndef _ASM_ARCH_RTAI_TIMER_H_
#define _ASM_ARCH_RTAI_TIMER_H_

#include <linux/time.h>
#include <linux/timer.h>
#include <asm/mach/irq.h>
#include <linux/timex.h>
#include <asm/arch/imx-regs.h>

static inline int rtai_timer_irq_ack( void )
{
	if (IMX_TSTAT(0))
		IMX_TSTAT(0) = 0x0;

	rt_unmask_irq(RTAI_TIMER_IRQ);

	if ( (int)(IMX_TCN(0) - IMX_TCMP(0)) < 0 ) {
		/* This is what just happened: we were setting a next timer
		   interrupt in the scheduler, while a previously configured
		   timer-interrupt happened, so, just restore the correct value
		   of the match register and proceed as usual. Yes, we will
		   have to re-calculate the next interrupt. */
		IMX_TCMP(0) = IMX_TCN(0) - 1;
	}

	return 0;
}

union rtai_tsc {
	unsigned long long tsc;
	unsigned long hltsc[2];
};
extern volatile union rtai_tsc rtai_tsc;

static inline RTIME rtai_rdtsc(void)
{
	RTIME ts;
	unsigned long flags, count;

	rtai_save_flags_and_cli(flags);

	if ((count = IMX_TCN(0)) < rtai_tsc.hltsc[0])
		rtai_tsc.hltsc[1]++;

	rtai_tsc.hltsc[0] = count;
	ts = rtai_tsc.tsc;

	rtai_restore_flags(flags);

	return ts;
}

#define PROTECT_TIMER
#define SETUP_TIME_TICKS (0)

static inline void rt_set_timer_delay(int delay)
{
	unsigned long flags;
	unsigned long next_match;

#ifdef PROTECT_TIMER
	if ( delay > LATCH )
		delay = LATCH;
#endif /* PROTECT_TIMER */

	rtai_save_flags_and_cli(flags);

	if ( delay )
		next_match = IMX_TCMP(0) = delay + IMX_TCN(0);
	else
		next_match = ( IMX_TCMP(0) += rt_times.periodic_tick );

#ifdef PROTECT_TIMER
	while ((int)(next_match - IMX_TCN(0)) < 2 * SETUP_TIME_TICKS ) {
		next_match = IMX_TCMP(0) = IMX_TCN(0) + 4 * SETUP_TIME_TICKS;
	}
#endif /* PROTECT_TIMER */

	rtai_restore_flags(flags);
}

#endif /* _ASM_ARCH_RTAI_TIMER_H_ */
