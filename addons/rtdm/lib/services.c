/*
 * Copyright (C) Pierre Cloutier <pcloutier@PoseidonControls.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the version 2 of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */

//#define CONFIG_RTAI_LXRT_INLINE 0
#include <rtdm/rtdm.h>
#ifdef CONFIG_RTAI_DRIVERS_16550A
#include <rtdm/rtserial.h>
#endif /* CONFIG_RTAI_DRIVERS_16550A */
