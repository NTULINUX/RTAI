/* 020222 asm-arm/arch-at91/timer.h - ARM/AT91 specific timer
Don't include directly - it's included through asm-arm/rtai.h

Copyright (c) 2005 Luca Pizzi, (lucapizzi@hotmail.com)
Copyright (c) 2005 Stefano Gafforelli, (stefano.gafforelli@tiscali.it)
COPYRIGHT (C) 2002 Guennadi Liakhovetski, DSA GmbH (gl@dsa-ac.de)
COPYRIGHT (C) 2002 Wolfgang Müller (wolfgang.mueller@dsa-ac.de)
Copyright (c) 2001 Alex Züpke, SYSGO RTS GmbH (azu@sysgo.de)
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

#ifndef _ASM_ARCH_RTAI_TIMER_H_
#define _ASM_ARCH_RTAI_TIMER_H_

#include <linux/time.h>
#include <linux/timer.h>
#include <asm/mach/irq.h>
#include <linux/timex.h>

#include <asm/arch/rtai_arch.h>
#include <asm/arch/at91_tc.h>

/* specific at91 tc registers read/write functions */

static inline unsigned int at91_tc_read(unsigned int reg_offset)
{
	unsigned long addr =
		(AT91_VA_BASE_TCB0 + 0x40 * CONFIG_IPIPE_AT91_TC);

	return readl((void __iomem *)(addr + reg_offset));
}

static inline void at91_tc_write(unsigned int reg_offset, unsigned long value)
{
	unsigned long addr =
		(AT91_VA_BASE_TCB0 + 0x40 * CONFIG_IPIPE_AT91_TC);

	writel(value, (void __iomem *)(addr + reg_offset));
}

extern notrace unsigned long long __ipipe_get_tsc(void);
extern notrace void __ipipe_set_tsc(unsigned long long value);

static inline void rtai_at91_update_tsc(void)
{
	__ipipe_set_tsc(__ipipe_get_tsc()+rt_times.periodic_tick);
};

extern unsigned int rt_periodic;

static inline RTIME rtai_rdtsc(void)
{
	if(!rt_periodic)
		/*
		 * one-shot mode : use ipipe native get tsc
		 */
		return __ipipe_mach_get_tsc();
	else
		/*
		 * periodic mode : use specific get tsc
		 * tsc has been updated since last shot by rtai_at91_update_tsc
		 */
		return __ipipe_get_tsc()+at91_tc_read(AT91_TC_CV);
}

static inline void rt_set_timer_delay(unsigned long delay)
{
	if (delay) {
		/*
		 * one-shot mode : reprogramm timer
		 */
		__ipipe_mach_set_dec(delay);
	} else {
		/*
		 * periodic mode: at91's TC reload itself 
		 * so nothing to do
		 */
	}
}
#endif /* !_ASM_ARCH_RTAI_TIMER_H_ */
