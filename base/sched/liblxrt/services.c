/*
 * Copyright (C) Pierre Cloutier <pcloutier@PoseidonControls.com>
 * Copyright (C) Alec Ari <neotheuser@ymail.com>
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


/* dummy defines to avoid annoying warning about here unused stuff */
#define read_cr4()  0
#define write_cr4(x)
/* end of dummy defines to avoid annoying warning about here unused stuff */

#define CONFIG_RTAI_LXRT_INLINE 0

#include <rtai_lxrt.h>
#include <rtai_signal.h>
#include <rtai_version.h>
#include <rtai_sched.h>
#include <rtai_malloc.h>
#include <rtai_trace.h>
#include <rtai_sem.h>
#include <rtai_rwl.h>
#include <rtai_spl.h>
#include <rtai_scb.h>
#include <rtai_mbx.h>
#include <rtai_msg.h>
#include <rtai_tasklets.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>
#ifdef CONFIG_RTAI_TASKLETS
#include <rtai_tasklets.h>
#endif /* CONFIG_RTAI_TASKLETS */
