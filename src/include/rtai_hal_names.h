/*
 * Copyright 2015 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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


#ifndef _RTAI_HAL_NAMES_H
#define _RTAI_HAL_NAMES_H

#include <linux/version.h>

#define TSKEXT0  0
#define TSKEXT1  1
#define TSKEXT2  2
#define TSKEXT3  3

#define hal_processor_id      ipipe_processor_id

#define hal_domain_struct     ipipe_domain 
#define hal_root_domain       ipipe_root_domain 

#define hal_critical_enter    ipipe_critical_enter
#define hal_critical_exit     ipipe_critical_exit

#define hal_sysinfo_struct    ipipe_sysinfo
#define hal_get_sysinfo       ipipe_get_sysinfo

#define hal_set_irq_affinity  ipipe_set_irq_affinity

#define hal_alloc_irq         ipipe_alloc_virq
#define hal_free_irq          ipipe_free_virq

#define hal_reenter_root()  __ipipe_reenter_root() 

#endif /* _RTAI_HAL_NAMES_H */
