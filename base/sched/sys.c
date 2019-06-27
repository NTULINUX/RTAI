/*
 * Copyright (C) 2001-2015  Paolo Mantegazza <mantegazza@aero.polimi.it>,
 * Copyright (C) 2019       Alec Ari <neotheuser@ymail.com>,
 * Copyright (C) 2001       Pierre Cloutier <pcloutier@poseidoncontrols.com>,
 * Copyright (C) 2001       Steve Papacharalambous <stevep@zentropix.com>,
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

/*
Nov. 2001, Jan Kiszka (Jan.Kiszka@web.de) fix a tiny bug in __task_init.
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/mman.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

#define DEFINE_FPU_FPREGS_OWNER_CTX
#include <rtai_sched.h>
#include <rtai_lxrt.h>
#include <rtai_sem.h>
#include <rtai_mbx.h>
#include <rtai_rwl.h>
#include <rtai_spl.h>

#include <asm/rtai_fpu.h>
#include <rtai_registry.h>
#include <rtai_proxies.h>
#include <rtai_msg.h>
#include <rtai_schedcore.h>

#define MAX_FUN_EXT  16
struct rt_fun_entry *rt_fun_ext[MAX_FUN_EXT];
EXPORT_SYMBOL(rt_fun_ext);

/* 
 * WATCH OUT for the default max expected size of messages from/to user space.
 */
#define USRLAND_MAX_MSG_SIZE  128  // Default max message size, used here only.

int get_min_tasks_cpuid(unsigned long cpus_allowed);

int clr_rtext(RT_TASK *task);

void steal_from_linux(RT_TASK *task);

void give_back_to_linux(RT_TASK *task, int);

void rt_schedule_soft(RT_TASK *task);

void *rt_get_lxrt_fun_entry(int index);

static inline void lxrt_typed_sem_init(SEM *sem, int count, int type)
{
	((RTAI_SYSCALL_MODE int (*)(SEM *, ...))rt_get_lxrt_fun_entry(TYPED_SEM_INIT))(sem, count, type);
}

static inline int lxrt_typed_mbx_init(MBX *mbx, int bufsize, int type)
{
	return ((RTAI_SYSCALL_MODE int (*)(MBX *, ...))rt_get_lxrt_fun_entry(TYPED_MBX_INIT))(mbx, bufsize, type);
}

static inline int lxrt_typed_rwl_init(RWL *rwl, int type)
{
	return ((RTAI_SYSCALL_MODE int (*)(RWL *, ...))rt_get_lxrt_fun_entry(RWL_INIT))(rwl, type);
}

static inline int lxrt_spl_init(SPL *spl)
{
	return ((RTAI_SYSCALL_MODE int (*)(SPL *, ...))rt_get_lxrt_fun_entry(SPL_INIT))(spl);
}

static inline int lxrt_Proxy_detach(pid_t pid)
{
	return ((RTAI_SYSCALL_MODE int (*)(int, ...))rt_get_lxrt_fun_entry(PROXY_DETACH))(pid);
}

static inline int GENERIC_DELETE(int index, void *object)
{
	return ((RTAI_SYSCALL_MODE int (*)(void *, ...))rt_get_lxrt_fun_entry(index))(object);
}
			 
#define lxrt_sem_delete(sem)        GENERIC_DELETE(SEM_DELETE, sem)
#define lxrt_named_sem_delete(sem)  GENERIC_DELETE(NAMED_SEM_DELETE, sem)
#define lxrt_rwl_delete(rwl)        GENERIC_DELETE(RWL_DELETE, rwl)
#define lxrt_named_rwl_delete(rwl)  GENERIC_DELETE(NAMED_RWL_DELETE, rwl)
#define lxrt_spl_delete(spl)        GENERIC_DELETE(SPL_DELETE, spl)
#define lxrt_named_spl_delete(spl)  GENERIC_DELETE(NAMED_SPL_DELETE, spl)
#define lxrt_mbx_delete(mbx)        GENERIC_DELETE(MBX_DELETE, mbx)
#define lxrt_named_mbx_delete(mbx)  GENERIC_DELETE(NAMED_MBX_DELETE, mbx)

extern void rt_schedule_soft_tail(RT_TASK *, int);
static inline void lxrt_fun_call(RT_TASK *task, void *fun, int narg, long *arg)
{
	if (likely(task->is_hard > 0)) {
		task->retval = ((RTAI_SYSCALL_MODE long long (*)(unsigned long, ...))fun)(RTAI_FUN_ARGS);
		if (unlikely(!task->is_hard)) {
			rt_schedule_soft_tail(task, task->runnable_on_cpus);
		}
	} else {
		struct fun_args *funarg;
		memcpy(funarg = (void *)task->fun_args, arg, narg);
		funarg->fun = fun;
		rt_schedule_soft(task);
	}
}

static inline void lxrt_fun_call_wbuf(RT_TASK *rt_task, void *fun, int narg, long *arg, unsigned long type)
{
	int rsize, r2size, wsize, w2size, msg_size;
	long *wmsg_adr, *w2msg_adr, *fun_args;
		
	rsize = r2size = wsize = w2size = 0 ;
	wmsg_adr = w2msg_adr = NULL;
	fun_args = arg - 1;
	if (NEED_TO_R(type)) {			
		rsize = USP_RSZ1(type);
		rsize = rsize ? fun_args[rsize] : sizeof(long);
		if (NEED_TO_R2ND(type)) {
			r2size = USP_RSZ2(type);
			r2size = r2size ? fun_args[r2size] : sizeof(long);
		}
	}
	if (NEED_TO_W(type)) {
		wsize = USP_WSZ1(type);
		wsize = wsize ? fun_args[wsize] : sizeof(long);
		if (NEED_TO_W2ND(type)) {
			w2size = USP_WSZ2(type);
			w2size = w2size ? fun_args[w2size] : sizeof(long);
		}
	}
	if ((msg_size = rsize > wsize ? rsize : wsize) > 0) {
		if (msg_size > rt_task->max_msg_size[0]) {
			rt_free(rt_task->msg_buf[0]);
			rt_task->max_msg_size[0] = (msg_size << 7)/100;
			rt_task->msg_buf[0] = rt_malloc(rt_task->max_msg_size[0]);
		}
		if (rsize) {			
			long *buf_arg = fun_args + USP_RBF1(type);
			if (buf_arg[0]) {
				rt_copy_from_user(rt_task->msg_buf[0], (long *)buf_arg[0], rsize);
				buf_arg[0] = (long)rt_task->msg_buf[0];
			}
		}
		if (wsize) {
			long *buf_arg = fun_args + USP_WBF1(type);
			wmsg_adr = (long *)buf_arg[0];
			buf_arg[0] = (long)rt_task->msg_buf[0];
		}
	}
	if ((msg_size = r2size > w2size ? r2size : w2size) > 0) {
		if (msg_size > rt_task->max_msg_size[1]) {
			rt_free(rt_task->msg_buf[1]);
			rt_task->max_msg_size[1] = (msg_size << 7)/100;
			rt_task->msg_buf[1] = rt_malloc(rt_task->max_msg_size[1]);
		}
		if (r2size) {
			long *buf_arg = fun_args + USP_RBF2(type);
			if (buf_arg[0]) {
				rt_copy_from_user(rt_task->msg_buf[1], (long *)buf_arg[0], r2size);
				buf_arg[0] = (long)rt_task->msg_buf[1];
       			}
       		}
		if (w2size) {
			long *buf_arg = fun_args + USP_WBF2(type);
			w2msg_adr = (long *)buf_arg[0];
       	        	buf_arg[0] = (long)rt_task->msg_buf[1];
       		}
	}
	lxrt_fun_call(rt_task, fun, narg, arg);
	if (wsize) {
		rt_copy_to_user(wmsg_adr, rt_task->msg_buf[0], wsize);
		if (w2size) {
			rt_copy_to_user(w2msg_adr, rt_task->msg_buf[1], w2size);
		}
	}
}

void put_current_on_cpu(int cpuid);
void rt_set_task_pid(RT_TASK *);

//static inline RT_TASK* __task_init(unsigned long name, int prio, int stack_size, int max_msg_size, int cpus_allowed)
RT_TASK* __task_init(unsigned long name, int prio, int stack_size, int max_msg_size, int cpus_allowed)
{
	void *msg_buf0, *msg_buf1;
	RT_TASK *rt_task;

	if ((rt_task = rtai_tskext_t(current, TSKEXT0))) {
		if (num_online_cpus() > 1 && cpus_allowed) {
	    		cpus_allowed = hweight32(cpus_allowed) > 1 ? get_min_tasks_cpuid(cpus_allowed) : ffnz(cpus_allowed);
		} else {
			cpus_allowed = rtai_cpuid();
		}
		put_current_on_cpu(cpus_allowed);
		return rt_task;
	}
	if (rt_get_adr(name)) {
		return 0;
	}
	if (prio > RT_SCHED_LOWEST_PRIORITY) {
		prio = RT_SCHED_LOWEST_PRIORITY;
	}
	if (!max_msg_size) {
		max_msg_size = USRLAND_MAX_MSG_SIZE;
	}
	if (!(msg_buf0 = rt_malloc(max_msg_size))) {
		return 0;
	}
	if (!(msg_buf1 = rt_malloc(max_msg_size))) {
		rt_free(msg_buf0);
		return 0;
	}
	rt_task = rt_malloc(sizeof(RT_TASK) + 3*sizeof(struct fun_args)); 
	if (rt_task) {
	    rt_task->magic = 0;
	    if (num_online_cpus() > 1 && cpus_allowed) {
			cpus_allowed = hweight32(cpus_allowed) > 1 ? get_min_tasks_cpuid(cpus_allowed) : ffnz(cpus_allowed);
	    } else {
			cpus_allowed = rtai_cpuid();
	    }
	    if (!set_rtext(rt_task, prio, 0, 0, cpus_allowed)) {
	        rt_task->fun_args = (long *)((struct fun_args *)(rt_task + 1));
		rt_task->msg_buf[0] = msg_buf0;
		rt_task->msg_buf[1] = msg_buf1;
		rt_task->max_msg_size[0] =
		rt_task->max_msg_size[1] = max_msg_size;
		if (rt_register(name, rt_task, IS_TASK, 0)) {
			rt_task->state = 0;
			__ipipe_share_current(0);
			ipipe_enable_notifier(current);
			__ipipe_disable_ondemand_mappings(current);
			RTAI_OOM_DISABLE();
			rt_set_task_pid(rt_task);
			return rt_task;
		} else {
			clr_rtext(rt_task);
		}
	    }
	    rt_free(rt_task);
	}
	rt_free(msg_buf0);
	rt_free(msg_buf1);
	return 0;
}
EXPORT_SYMBOL(__task_init);

static int __task_delete(RT_TASK *rt_task)
{
	struct task_struct *lnxtsk;

	if (current != (lnxtsk = rt_task->lnxtsk)) {
		return -EPERM;
	}
	rtai_tskext(lnxtsk, TSKEXT0) = rtai_tskext(lnxtsk, TSKEXT1) = 0;
	if (rt_task->is_hard > 0) {
		give_back_to_linux(rt_task, 0);
	}
	if (clr_rtext(rt_task)) {
		return -EFAULT;
	}
	rt_free(rt_task->msg_buf[0]);
	rt_free(rt_task->msg_buf[1]);
	rt_free(rt_task);
	return (!rt_drg_on_adr(rt_task)) ? -ENODEV : 0;
}

//#define ECHO_SYSW
#ifdef ECHO_SYSW
#define SYSW_DIAG_MSG(x) x
#else
#define SYSW_DIAG_MSG(x)
#endif

#include <linux/cred.h>
static inline void set_lxrt_perm(int perm)
{
	struct cred *cred;
	if ((cred = prepare_creds())) {
		cap_raise(cred->cap_effective, perm);
		commit_creds(cred);
	}
}

void rt_make_hard_real_time(RT_TASK *task)
{
	if (task && task->magic == RT_TASK_MAGIC && !task->is_hard) {
		steal_from_linux(task);
	}
}
EXPORT_SYMBOL(rt_make_hard_real_time);

void rt_make_soft_real_time(RT_TASK *task)
{
	if (task && task->magic == RT_TASK_MAGIC && task->is_hard) {
		if (task->is_hard > 0) {
			give_back_to_linux(task, 0);
		} else {
			task->is_hard = 0;
		}
	}
}
EXPORT_SYMBOL(rt_make_soft_real_time);

#if 0
static long kernel_calibrator_spv(long period, long loops, RT_TASK *task);
#endif

static inline long long handle_lxrt_request (unsigned int lxsrq, long *uarg, RT_TASK *task)
{
#define larg ((struct arg *)arg)

	union {unsigned long name; RT_TASK *rt_task; SEM *sem; MBX *mbx; RWL *rwl; SPL *spl; int i; void *p; long long ll; } arg0;
	int srq;

#ifdef CONFIG_RTAI_USE_STACK_ARGS
	long *arg = uarg;
#else
	int argsize = lxsrq & 0x7FF;
	long arg[argsize];
	rt_copy_from_user(arg, uarg, argsize);
#endif

	if (likely((srq = SRQ(lxsrq)) < MAX_LXRT_FUN)) {
		unsigned long type;
		struct rt_fun_entry *funcm;
/*
 * The next two lines of code do a lot. It makes possible to extend the use of
 * USP to any other real time module service in user space, both for soft and
 * hard real time. Concept contributed and copyrighted by: Giuseppe Renoldi 
 * (giuseppe@renoldi.org).
 */
		if (unlikely(!(funcm = rt_fun_ext[INDX(lxsrq)]))) {
			rt_printk("BAD: null rt_fun_ext, no module for extension %d?\n", INDX(lxsrq));
			return -ENOSYS;
		}
		if (!(type = funcm[srq].type)) {
			return ((RTAI_SYSCALL_MODE long long (*)(unsigned long, ...))funcm[srq].fun)(RTAI_FUN_ARGS);
		}
		if (unlikely(NEED_TO_RW(type))) {
			lxrt_fun_call_wbuf(task, funcm[srq].fun, LXRT_NARG(lxsrq), arg, type);
		} else {
			lxrt_fun_call(task, funcm[srq].fun, LXRT_NARG(lxsrq), arg);
	        }
		return task->retval;
	}

	arg0.name = arg[0];
	switch (srq) {
		case LXRT_GET_ADR: {
			arg0.p = rt_get_adr(arg0.name);
			return arg0.ll;
		}

		case LXRT_GET_NAME: {
			arg0.name = rt_get_name(arg0.p);
			return arg0.ll;
		}

		case LXRT_TASK_INIT: {
			struct arg { unsigned long name; long prio, stack_size, max_msg_size, cpus_allowed; };
			arg0.rt_task = __task_init(arg0.name, larg->prio, larg->stack_size, larg->max_msg_size, larg->cpus_allowed);
			return arg0.ll;
		}

		case LXRT_TASK_DELETE: {
			arg0.i = __task_delete(arg0.rt_task ? arg0.rt_task : task);
			return arg0.ll;
		}

		case LXRT_SEM_INIT: {
			if (rt_get_adr(arg0.name)) {
				return 0;
			}
			if ((arg0.sem = rt_malloc(sizeof(SEM)))) {
				struct arg { unsigned long name; long cnt; long typ; };
				lxrt_typed_sem_init(arg0.sem, larg->cnt, larg->typ);
				if (rt_register(larg->name, arg0.sem, IS_SEM, current)) {
					return arg0.ll;
				} else {
					rt_free(arg0.sem);
				}
			}
			return 0;
		}

		case LXRT_SEM_DELETE: {
			if (lxrt_sem_delete(arg0.sem)) {
				arg0.i = -EFAULT;
				return arg0.ll;
			}
			rt_free(arg0.sem);
			arg0.i = rt_drg_on_adr(arg0.sem);
			return arg0.ll;
		}

		case LXRT_MBX_INIT: {
			if (rt_get_adr(arg0.name)) {
				return 0;
			}
			if ((arg0.mbx = rt_malloc(sizeof(MBX)))) {
				struct arg { unsigned long name; long size; int qtype; };
				if (lxrt_typed_mbx_init(arg0.mbx, larg->size, larg->qtype) < 0) {
					rt_free(arg0.mbx);
					return 0;
				}
				if (rt_register(larg->name, arg0.mbx, IS_MBX, current)) {
					return arg0.ll;
				} else {
					rt_free(arg0.mbx);
				}
			}
			return 0;
		}

		case LXRT_MBX_DELETE: {
			if (lxrt_mbx_delete(arg0.mbx)) {
				arg0.i = -EFAULT;
				return arg0.ll;
			}
			rt_free(arg0.mbx);
			arg0.i = rt_drg_on_adr(arg0.mbx);
			return arg0.ll;
		}

		case LXRT_RWL_INIT: {
			if (rt_get_adr(arg0.name)) {
				return 0;
			}
			if ((arg0.rwl = rt_malloc(sizeof(RWL)))) {
				struct arg { unsigned long name; long type; };
				lxrt_typed_rwl_init(arg0.rwl, larg->type);
				if (rt_register(larg->name, arg0.rwl, IS_SEM, current)) {
					return arg0.ll;
				} else {
					rt_free(arg0.rwl);
				}
			}
			return 0;
		}

		case LXRT_RWL_DELETE: {
			if (lxrt_rwl_delete(arg0.rwl)) {
				arg0.i = -EFAULT;
				return arg0.ll;
			}
			rt_free(arg0.rwl);
			arg0.i = rt_drg_on_adr(arg0.rwl);
			return arg0.ll;
		}

		case LXRT_SPL_INIT: {
			if (rt_get_adr(arg0.name)) {
				return 0;
			}
			if ((arg0.spl = rt_malloc(sizeof(SPL)))) {
				struct arg { unsigned long name; };
				lxrt_spl_init(arg0.spl);
				if (rt_register(larg->name, arg0.spl, IS_SEM, current)) {
					return arg0.ll;
				} else {
					rt_free(arg0.spl);
				}
			}
			return 0;
		}

		case LXRT_SPL_DELETE: {
			if (lxrt_spl_delete(arg0.spl)) {
				arg0.i = -EFAULT;
				return arg0.ll;
			}
			rt_free(arg0.spl);
			arg0.i = rt_drg_on_adr(arg0.spl);
			return arg0.ll;
		}

		case MAKE_HARD_RT: {
			rt_make_hard_real_time(task);
			return 0;
			if (!task || task->is_hard) {
				 return 0;
			}
			steal_from_linux(task);
			return 0;
		}

		case MAKE_SOFT_RT: {
			rt_make_soft_real_time(task);
			return 0;
			if (!task || !task->is_hard) {
				return 0;
			}
			if (task->is_hard < 0) {
				task->is_hard = 0;
			} else {
				give_back_to_linux(task, 0);
			}
			return 0;
		}
		case PRINT_TO_SCREEN:
		case PRINTK: {
			struct arg { char *display; long nch; };
#ifdef CONFIG_RTAI_USE_STACK_ARGS
			char *ldisplay = larg->display;
#else
			char ldisplay[larg->nch + 1];
			rt_copy_from_user(ldisplay, larg->display, larg->nch);
			ldisplay[larg->nch] = 0;
#endif
			arg0.i = rt_printk("%s", ldisplay);
			return arg0.ll;
		}

		case NONROOT_HRT: {
			set_lxrt_perm(CAP_IPC_LOCK);
			set_lxrt_perm(CAP_SYS_RAWIO);
			set_lxrt_perm(CAP_SYS_NICE);
			return 0;
		}

		case RT_BUDDY: {
			arg0.rt_task = task && rtai_tskext(current, TSKEXT1) == current ? task : NULL;
			return arg0.ll;
		}

		case HRT_USE_FPU: {
			struct arg { RT_TASK *task; long use_fpu; };
			if(!larg->use_fpu) {
				clear_lnxtsk_uses_fpu((larg->task)->lnxtsk);
			} else {
				init_fpu((larg->task)->lnxtsk);
			}
			return 0;
		}

                case GET_USP_FLAGS: {
                        arg0.name = arg0.rt_task->usp_flags;
			return arg0.ll;
                }
                case SET_USP_FLAGS: {
                        struct arg { RT_TASK *task; unsigned long flags; };
                        arg0.rt_task->usp_flags = larg->flags;
                        arg0.rt_task->force_soft = (arg0.rt_task->is_hard > 0) && (larg->flags & arg0.rt_task->usp_flags_mask & FORCE_SOFT);
                        return 0;
                }

                case GET_USP_FLG_MSK: {
                        arg0.name = arg0.rt_task->usp_flags_mask;
			return arg0.ll;
                }

                case SET_USP_FLG_MSK: {
                        task->usp_flags_mask = arg0.name;
                        task->force_soft = (task->is_hard > 0) && (task->usp_flags & arg0.name & FORCE_SOFT);
                        return 0;
                }

                case HARD_SOFT_TOGGLER: {
			if (arg0.rt_task && arg0.rt_task->lnxtsk) {
				return (arg0.rt_task->lnxtsk)->pid;
			} 
#ifdef CONFIG_RTAI_HARD_SOFT_TOGGLER
			  else if (task) {
				rtai_cli();
				if (task->is_hard > 0) {
					rt_make_soft_real_time(task); 
				} else {
					rt_make_hard_real_time(task);
				}
				rtai_sti();
			}
#endif
                        return 0;
                }

		case IS_HARD: {
			arg0.ll = arg0.rt_task || (arg0.rt_task = rtai_tskext_t(current, TSKEXT0)) ? arg0.rt_task->is_hard : 0;
			return arg0.ll;
		}
		case GET_EXECTIME: {
			struct arg { RT_TASK *task; RTIME *exectime; };
			RTIME exectime[3];
			if ((larg->task)->exectime[0] && (larg->task)->exectime[1]) {
				exectime[0] = (larg->task)->exectime[0]; 
				exectime[1] = (larg->task)->exectime[1]; 
				exectime[2] = rtai_rdtsc(); 
				rt_copy_to_user(larg->exectime, exectime, sizeof(exectime));
			}
                        return 0;
		}
		case GET_TIMEORIG: {
			struct arg { RTIME *time_orig; };
			if (larg->time_orig) {
				RTIME time_orig[2];
				rt_gettimeorig(time_orig);
				rt_copy_to_user(larg->time_orig, time_orig, sizeof(time_orig));
			} else {
				rt_gettimeorig(NULL);
			}
                        return 0;
		}

		case LINUX_SERVER: {
			struct arg { struct linux_syscalls_list syscalls; };
			if (larg->syscalls.nr) {
				if (larg->syscalls.task->linux_syscall_server) {
					RT_TASK *serv;
					rt_get_user(serv, &larg->syscalls.serv);
					rt_task_masked_unblock(serv, ~RT_SCHED_READY);
				}
				larg->syscalls.task->linux_syscall_server = larg->syscalls.serv;
				rtai_set_linux_task_priority(current, (larg->syscalls.task)->lnxtsk->policy, (larg->syscalls.task)->lnxtsk->rt_priority);
				arg0.rt_task = __task_init((unsigned long)larg->syscalls.task, larg->syscalls.task->base_priority >= BASE_SOFT_PRIORITY ? larg->syscalls.task->base_priority - BASE_SOFT_PRIORITY : larg->syscalls.task->base_priority, 0, 0, 1 << larg->syscalls.task->runnable_on_cpus);

				larg->syscalls.task->linux_syscall_server = arg0.rt_task;
				arg0.rt_task->linux_syscall_server = larg->syscalls.serv;

				return arg0.ll;
			} else {
				if (!larg->syscalls.task) {
					larg->syscalls.task = RT_CURRENT;
				}
				if ((arg0.rt_task = larg->syscalls.task->linux_syscall_server)) {
					larg->syscalls.task->linux_syscall_server = NULL;
					arg0.rt_task->suspdepth = -RTE_HIGERR;
					rt_task_masked_unblock(arg0.rt_task, ~RT_SCHED_READY);
				}
			}
			return 0;
		}

		case SCHED_LATENCIES: {
			extern int KernelLatency, UserLatency;
			struct arg { long klat, ulat, period; };
			KernelLatency = larg->klat;
			UserLatency   = larg->ulat;
			return 0;
		}

		case GET_CPU_FREQ: {
			extern struct calibration_data rtai_tunables;
			return rtai_tunables.clock_freq;
		}

	        default: {
		    rt_printk("RTAI/LXRT: Unknown srq #%d\n", srq);
		    arg0.i = -ENOSYS;
		    return arg0.ll;
		}
	}
	return 0;
}

static inline void check_to_soften_harden(RT_TASK *task)
{
	if (unlikely(task->force_soft)) {
		if (task->is_hard > 0) {
			give_back_to_linux(task, 0);
		} else {
			task->is_hard = 0;
		}
		task->force_soft = 0;
		task->usp_flags &= ~FORCE_SOFT;
	} else if (unlikely(task->is_hard < 0)) {
		SYSW_DIAG_MSG(rt_printk("GOING BACK TO HARD (SYSLXRT, DIRECT), PID = %d.\n", current->pid););
		steal_from_linux(task);
		SYSW_DIAG_MSG(rt_printk("GONE BACK TO HARD (SYSLXRT),  PID = %d.\n", current->pid););
	}
}

long long rtai_lxrt_invoke (unsigned int lxsrq, void *arg, RT_TASK *task)
{
	if (likely(task)) {
		long long retval;
		check_to_soften_harden(task);
		retval = handle_lxrt_request(lxsrq, arg, task);
		check_to_soften_harden(task);
		if (unlikely(task->unblocked)) {
			if (task->is_hard > 0) {
				give_back_to_linux(task, -1);
			}
			task->unblocked = 0;
		}
		return retval;
	} 
	return handle_lxrt_request(lxsrq, arg, NULL);
}

int set_rt_fun_ext_index(struct rt_fun_entry *fun, int idx)
{
	if (idx > 0 && idx < MAX_FUN_EXT && !rt_fun_ext[idx]) {
		rt_fun_ext[idx] = fun;
		return 0;
	}
	return -EACCES;
}

void reset_rt_fun_ext_index( struct rt_fun_entry *fun, int idx)
{
	if (idx > 0 && idx < MAX_FUN_EXT && rt_fun_ext[idx] == fun) {
		rt_fun_ext[idx] = 0;
	}
}

void linux_process_termination(void)

{
	extern int max_slots;
	unsigned long numid;
	char name[8];
	RT_TASK *task2delete;
	struct rt_registry_entry entry;
	int slot;
/*
 * Linux is just about to schedule current out of existence. With this feature, 
 * LXRT frees the real time resources allocated to it.
*/
	if (!(numid = is_process_registered(current))) {
		return;
	}
	for (slot = 1; slot <= max_slots; slot++) {
		if (!rt_get_registry_slot(slot, &entry) || entry.tsk != current || rt_drg_on_name_cnt(entry.name) <= 0) {
			continue;
		}
		num2nam(entry.name, name);
		entry.tsk = 0;
       		switch (entry.type) {
			case IS_SEM:
				rt_printk("LXRT releases SEM %s\n", name);
				lxrt_sem_delete(entry.adr);
				rt_free(entry.adr);
				break;
			case IS_RWL:
				rt_printk("LXRT releases RWL %s\n", name);
				lxrt_rwl_delete(entry.adr);
				rt_free(entry.adr);
				break;
			case IS_SPL:
				rt_printk("LXRT releases SPL %s\n", name);
				lxrt_spl_delete(entry.adr);
				rt_free(entry.adr);
				break;
			case IS_MBX:
				rt_printk("LXRT releases MBX %s\n", name);
				lxrt_mbx_delete(entry.adr);
				rt_free(entry.adr);
				break;
			case IS_PRX:
				numid = rttask2pid(entry.adr);
				rt_printk("LXRT releases PROXY PID %lu\n", numid);
				lxrt_Proxy_detach(numid);
				break;
			case IS_TASK:
				rt_printk("LXRT deregisters task %s %d\n", name, ((RT_TASK *)entry.adr)->lnxtsk->pid);
				break;
		}
	}
	if ((task2delete = rtai_tskext_t(current, TSKEXT0))) {
		if (!clr_rtext(task2delete)) {
			rt_drg_on_adr(task2delete); 
			rt_printk("LXRT releases PID %d (ID: %s).\n", current->pid, current->comm);
			rt_free(task2delete->msg_buf[0]);
			rt_free(task2delete->msg_buf[1]);
			rt_free(task2delete);
			rtai_tskext(current, TSKEXT0) = rtai_tskext(current, TSKEXT0) = 0;
		}
	}
}

void init_fun_ext (void)
{
	rt_fun_ext[0] = rt_fun_lxrt;
}

#if 0
/* SUPPORT FOR CALIBRATING SCHEDULING LATENCIES FOR KERNEL SPACE TASKS */

struct kern_cal_arg { long period, loops; RT_TASK *task; };

static void kernel_calibrator(struct kern_cal_arg *calpar)
{
	RTIME expected;
	int average = 0;
	double s = 0;

	expected = rt_get_time() + 10*calpar->period;
	rt_task_make_periodic(NULL, expected, calpar->period);
	while (calpar->loops--) {
		expected += calpar->period;
		rt_task_wait_period();
		average += rt_get_time() - expected;
		s += 3.14;
        }
	calpar->period = average;
	rt_task_resume(calpar->task);
}

struct calsup { struct kern_cal_arg calpar; RT_TASK rtask; };

long kernel_calibrator_spv(long period, long loops, RT_TASK *task)
{
	struct calsup *calsup;
	calsup = kmalloc(sizeof(struct calsup), GFP_KERNEL);
	calsup->calpar = (struct kern_cal_arg) { period, loops, task };
	rt_task_init_cpuid(&calsup->rtask, (void *)kernel_calibrator, (long)&calsup->calpar, 4096, 0, 1, 0, rtai_cpuid());
	rt_task_resume(&calsup->rtask);
	task->fun_args[0] = (long)task;
	((struct fun_args *)task->fun_args)->fun = (void *)rt_task_suspend;
	rt_schedule_soft(task);
	period = calsup->calpar.period;
	kfree(calsup);
	return period;
}
#endif

/* SUPPORT FOR KERNEL THREADS, IN SOFT-HARD REAL TIME 
 * MODE, TO BE USED THE SAME WAY AS IN USER SPACED.              
 */

#include <linux/kthread.h>

#ifndef MAX_RT_PRIO
#define MAX_RT_PRIO 99
#endif 

struct kthread_fun_args { void *fun; void *args; struct task_struct **thread; };
struct klist_t kthread_to_create;
static RT_TASK *kthread_server_task;
static spinlock_t kthread_server_spinlock;

void rt_thread_create(void *fun, void *args, struct task_struct **thread)
{
	struct task_struct *lnxkthrd;

	if (!RT_CURRENT->is_hard) {
		lnxkthrd = kthread_run((void *)fun, args, "RTAI_KTHREAD");
		if (thread) {
			*thread = lnxkthrd;
		}
	} else {
		struct kthread_fun_args *fun_args;
		fun_args = rt_malloc(sizeof(struct kthread_fun_args));
		*fun_args = (struct kthread_fun_args){ fun, args, thread };
		rt_spin_lock_irq(&kthread_server_spinlock);
		kthread_to_create.task[kthread_to_create.in++ & (MAX_WAKEUP_SRQ - 1)] = fun_args;
		rt_spin_unlock_irq(&kthread_server_spinlock);
		rt_task_resume(kthread_server_task);
	}
	return;
}
EXPORT_SYMBOL(rt_thread_create);
	
RT_TASK *rt_thread_init(unsigned long name, int priority, int make_hard, int policy, int cpus_allowed)
{
	int linux_rt_priority;
	RT_TASK *task;
	char namestr[20] = "RT_KTHRD_";

        if (policy == SCHED_NORMAL) {
                linux_rt_priority = 0;
        } else if ((linux_rt_priority = MAX_RT_PRIO - 1 - priority) < 1) {
                linux_rt_priority = 1;
	}
	rtai_set_linux_task_priority(current, policy, linux_rt_priority);
	init_fpu(current);
	
	if ((task = __task_init(name ? name : rt_get_name(NULL), priority, 0, 0, cpus_allowed)) && make_hard > 0) {
		rt_make_hard_real_time(task);
	} 
	num2nam(name, namestr + 9);
	strlcpy(current->comm, namestr, sizeof(current->comm)); // race chances?
	return task;
}
EXPORT_SYMBOL(rt_thread_init);

int rt_thread_delete(RT_TASK *rt_task)
{
	return __task_delete(rt_task);
}
EXPORT_SYMBOL(rt_thread_delete);

static inline void soft_kthread_server_suspend(RT_TASK *task)
{
	task->fun_args[0] = (unsigned long)task;
	((struct fun_args *)task->fun_args)->fun = (void *)rt_task_suspend;
	rt_schedule_soft(task);
	return;
}

int kthread_server(void *args)
{
	struct kthread_fun_args *fun_args;
	struct task_struct *lnxthread;

	kthread_server_task = rt_thread_init(nam2num("THRSRV"), 0, 0, SCHED_FIFO, 0xF);
	strlcpy(current->comm, "RTAI_KTHRD_SRVR", sizeof(current->comm));
	do {
		soft_kthread_server_suspend(kthread_server_task);
		while (kthread_to_create.out != kthread_to_create.in) {
			fun_args = kthread_to_create.task[kthread_to_create.out++ & (MAX_WAKEUP_SRQ - 1)];
			lnxthread = kthread_run(fun_args->fun, fun_args->args, "RTAI_KTHREAD");
			if (fun_args->thread) {
				*fun_args->thread = lnxthread;
			}
			rt_free(fun_args);
		}
	} while (!rtai_tskext(current, TSKEXT3));
	__task_delete(kthread_server_task);
	rtai_tskext(current, TSKEXT3) = NULL;
        return 0;
}

#if 0

struct kthread_fun_args { void *fun; void *args; struct semaphore *sem; };

static int kthread_fun(struct kthread_fun_args *args)
{
	int (*fun)(void *) = args->fun;
	void *fun_args     = args->args;
	RT_TASK *task;

	up(args->sem);
	fun(fun_args);
	if ((task = rtai_tskext_t(current, TSKEXT0))) {
		rt_make_soft_real_time(task);
		__task_delete(task);
	}
        return 0;
}

long rt_thread_create(void *fun, void *args, int stack_size)
{
	RT_TASK *task;
	struct task_struct *lnxkthrd;
	struct semaphore sem;
	int make_hard = 0;
	struct kthread_fun_args fun_args = { fun, args, &sem };

	init_MUTEX_LOCKED(&sem);	
	task = rtai_tskext_t(current, TSKEXT0);
	if (task) {
		if (!task->lnxtsk) {
			return (long)-EPERM;
		}
		if ((make_hard = task->is_hard) > 0) {
			rt_make_soft_real_time(task);
		}
	}
// wrapper kthread_fun and kernel sem grant new kthread is running at return
	lnxkthrd = kthread_run((void *)kthread_fun, &fun_args, "RTAI_KTHREAD");
	down(&sem);
	if (make_hard) {
		rt_make_hard_real_time(task);
	}
	return (long)lnxkthrd;
}
EXPORT_SYMBOL(rt_thread_create);
	
RT_TASK *rt_thread_init(unsigned long name, int priority, int hard, int policy, int cpus_allowed)
{
	int linux_rt_priority;
	RT_TASK *task;
	char namestr[10];

        if (policy == SCHED_NORMAL) {
                linux_rt_priority = 0;
        } else if ((linux_rt_priority = MAX_RT_PRIO - 1 - priority) < 1) {
                linux_rt_priority = 1;
	}
	rtai_set_linux_task_priority(current, policy, linux_rt_priority);
	init_fpu(current);
	
	if ((task = __task_init(name ? name : rt_get_name(NULL), priority, 0, hard, cpus_allowed)) && hard > 0) {
		rt_make_hard_real_time(task);
	} 
	num2nam(name, namestr);
	strlcpy(current->comm, namestr, sizeof(current->comm)); // race chances?
	return task;
}
EXPORT_SYMBOL(rt_thread_init);

int rt_thread_delete(RT_TASK *rt_task)
{
	return __task_delete(rt_task);
}
EXPORT_SYMBOL(rt_thread_delete);

#endif
