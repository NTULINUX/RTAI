/* rtai/arch/arm/mach-pxa/pxa-timer.c
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

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <asm/mach/irq.h>
#include <asm/system.h>
#include <rtai.h>
#define do_leds()
#define do_set_rtc()
#define do_profile(x)
extern struct irqaction timer_irq;
extern unsigned long (*gettimeoffset)(void);
extern int (*set_rtc)(void);
#include <asm-arm/arch-pxa/timex.h>
#include <asm-arm/arch-pxa/rtai_timer.h>
#include <rtai_trace.h>

volatile union rtai_tsc rtai_tsc;
EXPORT_SYMBOL(rtai_tsc);
extern volatile u32 soft_timer_match;
extern void timer_tick(struct pt_regs *);
static int		(*saved_adeos_timer_handler)(int irq, void *dev_id, struct pt_regs *regs);


void rtai_pxa_GPIO_2_80_demux( int irq, void *dev_id, struct pt_regs *regs )
{
	/* No pending to linux! */
}


volatile union rtai_tsc lx_timer;

unsigned long split_timer (void) {
	lx_timer.tsc = rt_times.linux_time;
	return lx_timer.hltsc[0];
}

int soft_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	long flags;

	rdtsc(); /* update tsc - make sure we don't miss a wrap */

	local_irq_save(flags);

	timer_tick(regs);
	soft_timer_match = split_timer();

	local_irq_restore(flags);

	return IRQ_HANDLED;
}

int	rt_request_timer(rt_timer_irq_handler_t handler, unsigned tick, int use_apic)
{
	RTIME t;
	unsigned long flags;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_REQUEST, handler, tick);
	flags = rtai_critical_enter(NULL);

/*
 * sync w/jiffie, wait for a OS timer 0 match and clear the match bit
 */
	rtai_tsc.tsc = 0;
	/* wait for a timer match 0 and clear the match bit */

	do {
//		t = rdtsc();
 } while ( (signed long)(OSMR0 - OSCR) >= 0 );
        OSSR = OSSR_M0;

	/* set up rt_times structure */
	rt_times.linux_tick = LATCH;
	rt_times.periodic_tick = tick > 0 && tick < (RTIME)rt_times.linux_tick ? tick : rt_times.linux_tick;
	rt_times.tick_time  = t = rdtsc();
	rt_times.intr_time  = t + (RTIME)rt_times.periodic_tick;
	rt_times.linux_time = t + (RTIME)rt_times.linux_tick;

	/* Trick the scheduler - set this our way. */
//	tuned.setup_time_TIMER_CPUNIT = (int)(~(~0 >> 1)) + 1; /* smallest negative + 1 - for extra safety:-) */

	/* update Match-register */
	rt_set_timer_match_reg(rt_times.periodic_tick);

	saved_adeos_timer_handler = xchg(&irq_desc[TIMER_8254_IRQ].action->handler,(void *)soft_timer_interrupt);

	rt_free_global_irq(TIMER_8254_IRQ);

	rt_request_global_irq(TIMER_8254_IRQ, handler);

/*
 * pend linux timer irq to handle current jiffie
 */
 	rt_pend_linux_irq(TIMER_8254_IRQ);
	rtai_critical_exit(flags);

	return 0;
}

void rt_free_timer(void)
{
	unsigned long flags;

	TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_FREE, 0, 0);

	flags =  rtai_critical_enter(NULL);

	rt_free_global_irq(TIMER_8254_IRQ);
	irq_desc[TIMER_8254_IRQ].action->handler =(void*)saved_adeos_timer_handler ;

	rtai_critical_exit(flags);
}
