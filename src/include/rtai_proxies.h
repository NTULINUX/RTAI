/*
 * Copyright (C) 2000  Pierre Cloutier  <pcloutier@poseidoncontrols.com>
 * Copyright (C) 2000  Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_PROXIES_H
#define _RTAI_PROXIES_H

#include <rtai_sched.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Create a generic proxy task.
RT_TASK *__rt_proxy_attach(void (*func)(long),
			   RT_TASK *task,
			   void *msg,
			   int nbytes,
			   int priority);

// Create a raw proxy task.
RTAI_SYSCALL_MODE RT_TASK *rt_proxy_attach(RT_TASK *task,
			 void *msg,
			 int nbytes,
			 int priority);

// Delete a proxy task (a simplified specific rt_task_delete).
RTAI_SYSCALL_MODE int rt_proxy_detach(RT_TASK *proxy);

//Trigger a proxy.
RTAI_SYSCALL_MODE RT_TASK *rt_trigger(RT_TASK *proxy);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_RTAI_PROXIES_H */
