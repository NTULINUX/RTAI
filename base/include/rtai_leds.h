/*
 * rtai_leds.c - mini-driver for generic control of digital signals
 *
 * Copyright (C) 2000  Pierre Cloutier <pcloutier@PoseidonControls.com>
 * Copyright (C) 2001  David A. Schleef <ds@schleef.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RTAI_LEDS_H
#define _RTAI_LEDS_H

#include <rtai_types.h>

#if defined(CONFIG_RTAI_INTERNAL_LEDS_SUPPORT) && defined(CONFIG_RTAI_LEDS)

#include <asm/rtai_leds.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_leds_init(void);

void __rtai_leds_exit(void);

void rt_leds_set_mask(unsigned int mask,
		      unsigned int value);

void rt_toggle_leds(unsigned int l);

void rt_reset_leds(unsigned int l);

void rt_set_leds(unsigned int l);

void rt_clear_leds(void);

unsigned int rt_get_leds(void);

void rt_set_leds_port(int p);

void rt_config_leds(unsigned int type,
		    void (*func)(unsigned long p, unsigned int l),
		    unsigned long p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONFIG_RTAI_INTERNAL_LEDS_SUPPORT */

#endif /* !_RTAI_LEDS_H */
