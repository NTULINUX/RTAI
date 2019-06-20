/* rtai/arch/arm/mach-at91/at91-timer.c
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

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <asm/mach/irq.h>
#include <asm/system.h>
#include <rtai.h>
#include <asm/arch/timex.h>
#include <asm/arch/rtai_timer.h>
#include <rtai_trace.h>

#include <asm/arch/rtai_timer.h>

extern int (*extern_timer_isr)(struct pt_regs *regs);
extern int rtai_timer_handler(struct pt_regs *regs);

unsigned int rt_periodic;
EXPORT_SYMBOL(rt_periodic);

int rt_request_timer (void (*handler)(void), unsigned tick, int use_apic)
{
	unsigned long flags;

	flags = rtai_critical_enter(NULL);

	__ipipe_mach_timerstolen = 1;		// no need to reprogram timer on timer_tick() call

	rt_times.tick_time = rtai_rdtsc();
	rt_times.linux_tick = __ipipe_mach_ticks_per_jiffy;
	if (tick > 0) {
		rt_periodic = 1;

		/* Periodic setup --
		Use the built-in Adeos service directly. */
		if (tick > __ipipe_mach_ticks_per_jiffy) {
			tick = __ipipe_mach_ticks_per_jiffy;
		}
		rt_times.intr_time = rt_times.tick_time + tick;
		rt_times.linux_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.periodic_tick = tick;

		/* Prepare TCx to reload automaticly on RC compare */
		at91_tc_write(AT91_TC_CCR, AT91_TC_CLKDIS);
		at91_tc_write(AT91_TC_CMR, AT91_TC_TIMER_CLOCK3 | AT91_TC_WAVESEL_UP_AUTO | AT91_TC_WAVE);
		at91_tc_write(AT91_TC_RC, rt_times.periodic_tick);
		at91_tc_write(AT91_TC_CCR, AT91_TC_CLKEN | AT91_TC_SWTRG);
	} else {
		rt_periodic = 0;

		/* Oneshot setup. */
		rt_times.intr_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.linux_time = rt_times.tick_time + rt_times.linux_tick;
		rt_times.periodic_tick = rt_times.linux_tick;

		/* Prepare TCx behaviour as oneshot timer */
		at91_tc_write(AT91_TC_CMR, AT91_TC_TIMER_CLOCK3);
		rt_set_timer_delay(rt_times.periodic_tick);
	}

        rt_release_irq(RTAI_TIMER_IRQ);

	rt_request_irq(RTAI_TIMER_IRQ, (rt_irq_handler_t)handler, NULL, 0);
	extern_timer_isr = rtai_timer_handler;	// shunt for ipipe.c __ipipe_grab_irq

	rtai_critical_exit(flags);

        return 0;
}

void rt_free_timer (void)
{
	unsigned long flags;

	rt_periodic = 0;
	__ipipe_mach_timerstolen = 0;		// ipipe can reprogram timer for Linux now
	at91_tc_write(AT91_TC_CMR, AT91_TC_TIMER_CLOCK3); // back to oneshot mode
	rt_set_timer_delay(__ipipe_mach_ticks_per_jiffy); // regular timer delay
	rt_release_irq(RTAI_TIMER_IRQ);		// free this irq
	rtai_save_flags_and_cli(flags);		// critical section
	extern_timer_isr = NULL;		// let ipipe run as normally
	rtai_restore_flags(flags);		// end of critical section
}

int rtai_calibrate_TC (void)
{
	unsigned long flags;
	RTIME t, dt;
	int i;

	flags = rtai_critical_enter(NULL);
	rt_set_timer_delay(LATCH);
	t = rtai_rdtsc();
	for (i = 0; i < 10000; i++) { 
		rt_set_timer_delay(LATCH);
	}
	dt = rtai_rdtsc() - t;
	rtai_critical_exit(flags);

	return rtai_imuldiv(dt, 100000, RTAI_CPU_FREQ);
}
