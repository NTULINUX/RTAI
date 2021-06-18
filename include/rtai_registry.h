/**
 * @ingroup lxrt
 * @file
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999  Paolo Mantegazza <mantegazza@aero.polimi.it>,
 * @note Copyright &copy; 2019  Alec Ari <neotheuser@ymail.com,
 * extensions for user space modules are jointly copyrighted (2000) with:
 *            Pierre Cloutier <pcloutier@poseidoncontrols.com>,
 *     	      Steve Papacharalambous <stevep@zentropix.com>.
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

#ifndef _RTAI_REGISTRY_H
#define _RTAI_REGISTRY_H

#include <rtai_nam2num.h>

struct task_struct;

struct rt_registry_entry {
	unsigned long name;	 // Numerical representation of resource name
	void *adr;		 // Physical rt memory address of resource
	struct task_struct *tsk; // Linux task owner of the resource
	int type;		 // Type of resource
        unsigned short count;	 // Usage registry
	unsigned short alink;
	unsigned short nlink;
};

#define MAX_SLOTS  CONFIG_RTAI_SCHED_LXRT_NUMSLOTS // Max number of registered objects

#define IS_TASK  0               // Used to identify registered resources
#define IS_SEM   1
#define IS_RWL   2
#define IS_SPL   3
#define IS_MBX   4
#define IS_PRX   5
#define IS_HPCK  6

#ifdef __KERNEL__

#include <rtai.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

unsigned long is_process_registered(struct task_struct *tsk);

int rt_register(unsigned long nam,
		void *adr,
		int typ,
		struct task_struct *tsk);

int rt_drg_on_name(unsigned long name);

int rt_drg_on_name_cnt(unsigned long name);

int rt_drg_on_adr(void *adr);

int rt_drg_on_adr_cnt(void *adr);

RTAI_SYSCALL_MODE unsigned long rt_get_name(void *adr);

RTAI_SYSCALL_MODE void *rt_get_adr(unsigned long name);

void *rt_get_adr_cnt(unsigned long name);

int rt_get_type(unsigned long name);

int rt_get_registry_slot(int slot, struct rt_registry_entry *entry);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#define exist(name)  rt_get_adr(nam2num(name))

#endif /* !_RTAI_REGISTRY_H */
