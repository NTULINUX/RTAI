/*
 * ARM/EP9301 specific timer code
 *
 * COPYRIGHT (C) 2004 Michael Neuhauser, Firmix Software GmbH <mike@firmix.at>
 *
 * Acknowledgements:
 *	Paolo Mantegazza <mantegazza@aero.polimi.it>, creator of RTAI 
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <rtai.h>
#include <rtai_trace.h>
#include <asm/arch/ep93xx_tsc.h>

/*
 * Install a timer interrupt handler as a real-time ISR and set hardware-timer
 * to requested period.
 * tick = 0: oneshot mode
 * tick > 0: periodic mode (and tick specifies the period in timer-ticks)
 */
int
rt_request_timer(rt_timer_irq_handler_t handler, unsigned int tick, int i386_legacy_dummy)
{
    unsigned long flags;
    int is_oneshot = (tick == 0);

    TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_REQUEST, handler, tick);

    /* initial one-shot delay is 1/HZ seconds */
    if (is_oneshot)
	tick = LATCH;

    flags = rtai_critical_enter(NULL);

    /* stop timer & set reload value */
    outl(0, TIMER1CONTROL);
    outl(tick - 1, TIMER1LOAD);

    /* sync to jiffy clock phase (still neccessary with Adeos?) */
    rt_times.tick_time = ({
	ep93xx_tsc_t t0;
	unsigned long target_offset;
	unsigned long t;

	/* spin until we are not to close (~ 10 µs) before jiffy time-point */
	do {
	    t0.ll = ep93xx_rdtsc();
	    unsigned long long const realtime_jiffies = (long long)(t0.ll * HZ) / (long)RTAI_TSC_FREQ;
	    unsigned long const current_offset = t0.ll - (realtime_jiffies * RTAI_TSC_FREQ) / HZ;
	    target_offset = ((RTAI_TSC_FREQ + HZ/2) / HZ) - current_offset;
	} while (target_offset < 10);

	/* spin until we have passed jiffy time-point */
	do {
	    t = inl(TIMER4VALUELOW);
	} while ((t - t0.u.low) < target_offset);

	/* quickly start timer (508.469 kHz clock, free runing or periodic mode) */
	outl((is_oneshot) ? 0x88 : 0xc8, TIMER1CONTROL);

	/* set t0 to current time */
	if (t < t0.u.low)
	    ++t0.u.high;
	t0.u.low = t;

	t0.ll;
    });

    /* ack interrupt (timer may have underflowed since lock) */
    outl(1, TIMER1CLEAR);

    /* set up rt_times structure */
    if (is_oneshot) {
	/* oneshot mode: unit is TSC-tick */
	rt_times.linux_time	= rtai_llimd(ep93xx_jiffies_done * LATCH, RTAI_TSC_FREQ, RTAI_TIMER_FREQ);
	rt_times.linux_tick	= rtai_imuldiv(LATCH, RTAI_TSC_FREQ, RTAI_TIMER_FREQ);
	rt_times.periodic_tick	= rt_times.linux_tick;
    } else {
	/* periodic mode: unit is irq-timer-tick */
	rt_times.tick_time	= rtai_llimd(rt_times.tick_time, RTAI_TIMER_FREQ, RTAI_TSC_FREQ);
	rt_times.linux_time	= ep93xx_jiffies_done * LATCH;
	rt_times.linux_tick	= LATCH;
	rt_times.periodic_tick	= tick;
    }
    rt_times.intr_time	= rt_times.tick_time + rt_times.periodic_tick;

    rt_release_irq(RTAI_TIMER_IRQ);
    if (rt_request_irq(RTAI_TIMER_IRQ, (rt_irq_handler_t)handler, NULL, 0) < 0) {
	rtai_critical_exit(flags);
	return -EINVAL;
    }

    /* Note that it is not necessary to change the linux handler of the
     * timer-irq as it was done on RTAI/ARM before Adeos. This was done to
     * enable/disable hardware ack. Now Adeos will do all for us, using the
     * irq_des[].mask_ack function (which has to be set up accordingly in
     * arch/arm/mach-MACH/irq.c:MACH_init_irq()).
     */

    rtai_critical_exit(flags);
    return 0;
}

/*
 * Uninstall a timer handler previously set by rt_request_timer() and reset
 * hardware-timer to Linux HZ-tick.
 */
void
rt_free_timer(void)
{
    unsigned long flags;

    TRACE_RTAI_TIMER(TRACE_RTAI_EV_TIMER_FREE, 0, 0);

    flags = rtai_critical_enter(NULL);

    /* no need to sync with linux jiffy clock because jiffy clock is synced to
     * TSC (i.e. absolute time) by linux timer interrupt handlers */

    /* stop timer for re-programming */
    outl(0, TIMER1CONTROL);
    /* set reload value */
    outl(LATCH - 1, TIMER1LOAD);
    /* set timer to 508 kHz clock, periodic mode and start it */
    outl(0xc8, TIMER1CONTROL);
    /* ack interrupt that may have occured */
    outl(1, TIMER1CLEAR);

    rt_release_irq(RTAI_TIMER_IRQ);

    rtai_critical_exit(flags);
}
