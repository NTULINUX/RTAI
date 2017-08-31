/*
 *   ARTI -- RTAI-compatible Adeos-based Real-Time Interface. Based on
 *   the original RTAI layer for x86.
 *
 *   Original RTAI/x86 layer implementation:
 *   Copyright (C) 2000-2013 Paolo Mantegazza,
 *   Copyright (C) 2000 Steve Papacharalambous,
 *   Copyright (C) 2000 Stuart Hughes,
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos:
 *   Copyright (C) 2002 Philippe Gerum.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _RTAI_ASM_X86_VECTORS_H
#define _RTAI_ASM_X86_VECTORS_H

#ifdef __KERNEL__

#include <rtai_hal_names.h>
#include <rtai_config.h>

#define RTAI_APIC_HIGH_VECTOR  HAL_APIC_HIGH_VECTOR
#define RTAI_APIC_LOW_VECTOR   HAL_APIC_LOW_VECTOR

#define RTAI_APIC_HIGH_IPI     ipipe_apic_vector_irq(RTAI_APIC_HIGH_VECTOR)
#define RTAI_APIC_LOW_IPI      ipipe_apic_vector_irq(RTAI_APIC_LOW_VECTOR)
#define LOCAL_TIMER_IPI	ipipe_apic_vector_irq(LOCAL_TIMER_VECTOR)

#endif /* !__KERNEL_ */

#define __rtai_stringize0(_s_) #_s_
#define __rtai_stringize(_s_)  __rtai_stringize0(_s_)
#define __rtai_trap_call(_t_)  _t_
#define __rtai_do_trap0(_t_)   __rtai_stringize(int $ _t_)
#define __rtai_do_trap(_t_)    __rtai_do_trap0(__rtai_trap_call(_t_))

#define RTAI_DO_TRAP(v, r, a1, a2)  do { __asm__ __volatile__ ( __rtai_do_trap(v): : "a" (a1), "c" (a2), "d" (&r): "memory"); } while (0)

#endif /* !_RTAI_ASM_X86_VECTORS_H */
