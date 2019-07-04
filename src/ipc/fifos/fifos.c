/**
 * @ingroup fifos
 * @ingroup fifos_ipc
 * @ingroup fifos_sem
 * @file
 *
 * Implementation of the @ref fifos "RTAI FIFO module".
 *
 * @author Paolo Mantegazza
 *
 * @note Copyright &copy; 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * @note Copyright &copy; 2019 Alec Ari <neotheuser@ymail.com>
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
ACKNOWLEDGEMENTS: 
- nice proc file contributed by Steve Papacharalambous (stevep@zentropix.com);
- added proc handler info contributed by Rich Walker (rw@shadow.org.uk)
- 11-19-2001, Truxton Fulton (trux@truxton.com) fixed a race in mbx_get.
- 10-23-2003 added atomic send contributed by Jan Kiszka 
  (kiszka@rts.uni-hannover.de) and expanded it to rtf_get_if.
- 12-10-2003 a fix of rtf_resize odds contributed by Abramo Bagnara
  (abramo.bagnara@tin.it).
*/

/**
 * @defgroup fifos RTAI FIFO module
 *
 * See @ref fifos_overview "the general overview of RTAI fifos".
 */

/**
 * @ingroup fifos
 * @defgroup fifos_ipc Inter-process communications.
 *
 * RTAI FIFO communication functions.
 *
 * RTAI fifos maintain full compatibility with those available in NMT_RTLinux
 * while adding many other useful services that avoid the clumsiness of
 * Unix/Linux calls. So if you need portability you should bent yourself to the
 * use of select for timing out IO operations, while if you have not to satisfy
 * such constraints use the available simpler, and more direct, RTAI fifos
 * specific services.
 *
 * In the table below the standard Unix/Linux services in user space are
 * enclosed in []. See standard Linux man pages if you want to use them, they
 * need not be explained here.
 *
 * <CENTER><TABLE>
 * <TR><TD> Called from RT task </TD><TD> Called from Linux process </TD></TR>
 * <TR><TD> #rtf_create         </TD><TD> #rtf_open_sized           <BR>
 *                                         [open]                   </TD></TR>
 * <TR><TD> #rtf_destroy        </TD><TD>  [close]                  </TD></TR>
 * <TR><TD> #rtf_reset          </TD><TD> #rtf_reset                </TD></TR>
 * <TR><TD> #rtf_resize         </TD><TD> #rtf_resize               </TD></TR>
 * <TR><TD> #rtf_get            </TD><TD>  [read]                   <BR>
 *                                        #rtf_read_timed           <BR>
 *                                        #rtf_read_all_at_once     </TD></TR>
 * <TR><TD> #rtf_put            </TD><TD>  [write]                  <BR>
 *                                        #rtf_write_timed          </TD></TR>
 * <TR><TD> #rtf_create_handler </TD><TD>                           </TD></TR>
 * <TR><TD>                     </TD><TD> #rtf_suspend_timed        </TD></TR>
 * <TR><TD>                     </TD><TD> #rtf_set_async_sig        </TD></TR>
 * </TABLE></CENTER>
 *
 * In Linux, fifos have to be created by :
 * @verbatim $ mknod /dev/rtf<x> c 150 <x> @endverbatim
 * where \<x\> is the minor device number, from 0 to 63; thus on the Linux side
 * RTL fifos can be used as standard character devices. As it was said above to
 * use standard IO operations on such devices there is no need to explain
 * anything, go directly to Linux man pages. RTAI fifos specific services
 * available in kernel and user space are instead explained here.
 *
 * What is important to remember is that in the user space side you address
 * fifos through the file descriptor you get at fifo device opening while in
 * kernel space you directly address them by their minor number. So you will
 * mate the @a fd you get in user space by using
 * @verbatim open(/dev/rtfxx,...) @endverbatim
 * to the integer @p xx you will use in kernel space.
 *
 * @note RTAI fifos should be used just with applications that use only real
 * time interrupt handlers, so that no RTAI scheduler is installed, or if you
 * need compatibility with NMT RTL.  If you are working with any RTAI scheduler
 * already installed you are strongly invited to think about avoiding them, use
 * LXRT instead.
 *
 * It is far better and flexible, and if you really like it the fifos way
 * mailboxes are a one to one, more effective, substitute.   After all RTAI
 * fifos are implemented on top of them.
 */

/**
 * @ingroup fifos
 * @defgroup fifos_sem Semaphores.
 *
 * RTAI FIFO semaphore functions.
 *
 * Fifos have an embedded synchronization capability, however using them only
 * for such a purpose can be clumsy. So RTAI fifos have binary semaphores for
 * that purpose. Note that, as for put and get fifos functions, only nonblocking
 * functions are available in kernel space.
 * 
 * <CENTER><TABLE>
 * <TR><TD> Called from RT task </TD><TD> Called from Linux process </TD></TR>
 * <TR><TD> #rtf_sem_init       </TD><TD> #rtf_sem_init             </TD></TR>
 * <TR><TD> #rtf_sem_post       </TD><TD> #rtf_sem_post             </TD></TR>
 * <TR><TD> #rtf_sem_trywait    </TD><TD> #rtf_sem_wait             <BR>
 *                                        #rtf_sem_trywait          <BR>
 *                                        #rtf_sem_timed_wait       </TD></TR>
 * <TR><TD> #rtf_sem_destroy    </TD><TD> #rtf_sem_destroy          </TD></TR>
 * </TABLE></CENTER>
 *
 * To add a bit of confusion (J), with respect to RTAI schedulers semaphore
 * functions, fifos semaphore functions names follow the POSIX mnemonics.
 *
 * It should be noted that semaphores are associated to a fifo for
 * identification purposes. So it is once more important to remember is that
 * in the user space side you address fifos through the file descriptor you get
 * at fifo device opening while in kernel space you directly address them by
 * their minor number.   So you will mate the fd  you get in user space by
 * @verbatim open(/dev/rtfxx,) @endverbatim to the integer @p xx youll use in
 * kernel space.
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/poll.h>
#include <linux/termios.h>
#include <linux/tty_driver.h>
#include <linux/console.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>

#include <rtai_fifos.h>
#include <rtai_trace.h>
#include <rtai_proc_fs.h>
#include <rtai_sched.h>
#include <rtai_lxrt.h>

MODULE_LICENSE("GPL");

#include <linux/sched/signal.h>
#define CURRENT_TIME current_time(inode)

/* these are copied from <rt/rt_compat.h> */
#define rtf_save_flags_and_cli(x)	do{x=rt_spin_lock_irqsave(&rtf_lock);}while(0)
#define rtf_restore_flags(x)		rt_spin_unlock_irqrestore((x),&rtf_lock)
#define rtf_spin_lock_irqsave(x,y)	do{x=rt_spin_lock_irqsave(&(y));}while(0)
#define rtf_spin_unlock_irqrestore(x,y)	rt_spin_unlock_irqrestore((x),&(y))
#define rtf_request_srq(x)		rt_request_srq(0, (x), 0)
#define rtf_free_srq(x)			rt_free_srq((x))
#define rtf_pend_srq(x)			rt_pend_linux_srq((x))
#define f_dentry  f_path.dentry

static int rtai_proc_fifo_register(void);
static void rtai_proc_fifo_unregister(void);

typedef struct lx_queue {
	struct lx_queue *prev;
	struct lx_queue *next;
	struct lx_task_struct *task;
} F_QUEUE;

typedef struct lx_semaphore {
	int free;
	int qtype;
	F_QUEUE queue;
} F_SEM;

typedef struct lx_task_struct {
	int blocked;
	int priority;
	F_QUEUE queue;
	struct task_struct *task;
} LX_TASK;

typedef struct lx_mailbox {
	int size;   // size of the entire buffer
	int fbyte;  // head
	int lbyte;  // tail
	int avbs;   // bytes available in the buffer
	int frbs;   // free bytes in the buffer
	char *bufadr;
	F_SEM sndsem, rcvsem;
	struct task_struct *waiting_task;
	spinlock_t buflock;
} F_MBX;

typedef struct rt_fifo_struct {
	F_MBX mbx;		// MUST BE THE FIRST!
	int opncnt;
	int malloc_type;
	int pol_asyn_pended;
	wait_queue_head_t pollq;
	struct fasync_struct *asynq;
	rtf_handler_t handler;
	F_SEM sem;
	char name[RTF_NAMELEN+1];
} FIFO;

static int fifo_srq, async_sig;
static DEFINE_SPINLOCK(rtf_lock);
static DEFINE_SPINLOCK(rtf_name_lock);

#define MAX_FIFOS 64
//static FIFO fifo[MAX_FIFOS] = {{{0}}};
static FIFO *fifo;

#define MAXREQS 64	// KEEP IT A POWER OF 2!!!
static struct { int in, out; struct task_struct *task[MAXREQS]; } taskq;
static struct { int in, out; FIFO *fifo[MAXREQS]; } pol_asyn_q;

static int do_nothing(unsigned int arg, int rw) { return 0; }

static inline void enqueue_blocked(LX_TASK *task, F_QUEUE *queue, int qtype, int priority)
{
	F_QUEUE *q;

	task->blocked = 1;
	q = queue;
	if (!qtype) {
		while ((q = q->next) != queue && (q->task)->priority >= priority);
	}
	q->prev = (task->queue.prev = q->prev)->next  = &(task->queue);
	task->queue.next = q;
}

static inline void dequeue_blocked(LX_TASK *task)
{
	task->blocked = 0;
	(task->queue.prev)->next = task->queue.next;
	(task->queue.next)->prev = task->queue.prev;
}

static inline void mbx_sem_signal(F_SEM *sem, FIFO *fifop)
{
	unsigned long flags;
	LX_TASK *task;

	rtf_save_flags_and_cli(flags);
	if ((task = (sem->queue.next)->task)) {
		dequeue_blocked(task);
		taskq.task[taskq.in] = task->task;
		taskq.in = (taskq.in + 1) & (MAXREQS - 1);
		rtf_pend_srq(fifo_srq);
	} else {
		sem->free = 1;
		if (fifop && !(fifop->pol_asyn_pended) &&
		    (((F_MBX *)fifop)->avbs || ((F_MBX *)fifop)->frbs) &&
		    (waitqueue_active(&fifop->pollq) || fifop->asynq)) {
			fifop->pol_asyn_pended = 1;
			pol_asyn_q.fifo[pol_asyn_q.in] = fifop;
			pol_asyn_q.in = (pol_asyn_q.in + 1) & (MAXREQS - 1);
			rtf_pend_srq(fifo_srq);
		}
	}
	rtf_restore_flags(flags);
	return;
}

static inline void mbx_signal(F_MBX *mbx)
{
	unsigned long flags;
	struct task_struct *task;

	rtf_save_flags_and_cli(flags);
	if ((task = mbx->waiting_task)) {
		mbx->waiting_task = 0;
		taskq.task[taskq.in] = task;
		taskq.in = (taskq.in + 1) & (MAXREQS - 1);
		rtf_pend_srq(fifo_srq);
	}
	rtf_restore_flags(flags);
	return;
}

static inline int mbx_sem_wait_if(F_SEM *sem)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (sem->free) {
		sem->free = 0;
		rtf_restore_flags(flags);
		return 1;
	}
	rtf_restore_flags(flags);
	return 0;
}

static inline int mbx_sem_wait(F_SEM *sem)
{
	unsigned long flags;
	LX_TASK task;
	int ret;

	ret = 0;
	rtf_save_flags_and_cli(flags);
	if (!sem->free) {
		task.queue.task = &task;
		task.priority = current->rt_priority;
		enqueue_blocked(&task, &sem->queue, sem->qtype, task.priority);
		task.task = current;
		rtf_restore_flags(flags);
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
		}
		rtf_save_flags_and_cli(flags);
		if (task.blocked) { 
			dequeue_blocked(&task);
			if (!(sem->queue.next)->task) {
				sem->free = 1;
			}
			if (!ret) {
				ret = -1;
			}
		}
	} else {
		sem->free = 0;
	}
	rtf_restore_flags(flags);
	return ret;
}

static inline int mbx_wait(F_MBX *mbx, int *fravbs)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (!(*fravbs)) {
		mbx->waiting_task = current;
		current->state = TASK_INTERRUPTIBLE;
		rtf_restore_flags(flags);
		schedule();
		if (signal_pending(current)) {
			return -ERESTARTSYS;
		}
		rtf_save_flags_and_cli(flags);
		if (mbx->waiting_task == current) {
			mbx->waiting_task = 0;
			rtf_restore_flags(flags);
			return -1;
		}
	}
	rtf_restore_flags(flags);
	return 0;
}

static inline int mbx_sem_wait_timed(F_SEM *sem, int delay)
{
	unsigned long flags;
	LX_TASK task;

	rtf_save_flags_and_cli(flags);
	if (!sem->free) {
		task.queue.task = &task;
		task.priority = current->rt_priority;
		enqueue_blocked(&task, &sem->queue, sem->qtype, task.priority);
		task.task = current;
		rtf_restore_flags(flags);
		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(delay);
		if (signal_pending(current)) {
			return -ERESTARTSYS;
		}
		rtf_save_flags_and_cli(flags);
		if (task.blocked) { 
			dequeue_blocked(&task);
			if (!((sem->queue.next)->task)) {
				sem->free = 1;
			}
			rtf_restore_flags(flags);
			return -1;
		}
	} else {
		sem->free = 0;
	}
	rtf_restore_flags(flags);
	return 0;
}

static inline int mbx_wait_timed(F_MBX *mbx, int *fravbs, int delay)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (!(*fravbs)) {
		mbx->waiting_task = current;
		rtf_restore_flags(flags);
		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(delay);
		if (signal_pending(current)) {
			return -ERESTARTSYS;
		}
		rtf_save_flags_and_cli(flags);
		if (mbx->waiting_task == current) {;
			mbx->waiting_task = 0;
			rtf_restore_flags(flags);
			return -1;
		}
	}
	rtf_restore_flags(flags);
	return 0;
}

#define MOD_SIZE(indx) ((indx) < mbx->size ? (indx) : (indx) - mbx->size)

static inline int mbx_put(F_MBX *mbx, char **msg, int msg_size, int lnx)
{
	unsigned long flags;
	int tocpy;

	while (msg_size > 0 && mbx->frbs) {
		if ((tocpy = mbx->size - mbx->lbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > mbx->frbs) {
			tocpy = mbx->frbs;
		}
		if (lnx) {
			rt_copy_from_user(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		} else {
			memcpy(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		}
		rtf_spin_lock_irqsave(flags, mbx->buflock);
		mbx->lbyte = MOD_SIZE(mbx->lbyte + tocpy);
		mbx->frbs -= tocpy;
		mbx->avbs += tocpy;
		rtf_spin_unlock_irqrestore(flags, mbx->buflock);
		msg_size -= tocpy;
		*msg     += tocpy;
	}
	return msg_size;
}

static inline int mbx_ovrwr_put(F_MBX *mbx, char **msg, int msg_size, int lnx)
{
	unsigned long flags;
	int tocpy,n;

	if ((n = msg_size - mbx->size) > 0) {
		*msg += n;
		msg_size -= n;
	}		
	while (msg_size > 0) {
		if (mbx->frbs) {	
			if ((tocpy = mbx->size - mbx->lbyte) > msg_size) {
				tocpy = msg_size;
			}
			if (tocpy > mbx->frbs) {
				tocpy = mbx->frbs;
			}
			if (lnx) {
				rt_copy_from_user(mbx->bufadr + mbx->lbyte, *msg, tocpy);
			} else {
				memcpy(mbx->bufadr + mbx->lbyte, *msg, tocpy);
			}
			rtf_spin_lock_irqsave(flags, mbx->buflock);
			mbx->frbs -= tocpy;
			mbx->avbs += tocpy;
			rtf_spin_unlock_irqrestore(flags, mbx->buflock);
			msg_size -= tocpy;
			*msg     += tocpy;
			mbx->lbyte = MOD_SIZE(mbx->lbyte + tocpy);
		}	
		if (msg_size) {
			while ((n = msg_size - mbx->frbs) > 0) {
				if ((tocpy = mbx->size - mbx->fbyte) > n) {
					tocpy = n;
				}
				if (tocpy > mbx->avbs) {
					tocpy = mbx->avbs;
				}
				rtf_spin_lock_irqsave(flags, mbx->buflock);
				mbx->frbs  += tocpy;
				mbx->avbs  -= tocpy;
				rtf_spin_unlock_irqrestore(flags, mbx->buflock);
				mbx->fbyte = MOD_SIZE(mbx->fbyte + tocpy);
			}
		}		
	}
	return 0;
}

static inline int mbx_get(F_MBX *mbx, char **msg, int msg_size, int lnx)
{
	unsigned long flags;
	int tocpy;

	while (msg_size > 0 && mbx->avbs) {
		if ((tocpy = mbx->size - mbx->fbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > mbx->avbs) {
			tocpy = mbx->avbs;
		}
		if (lnx) {
			rt_copy_to_user(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		} else {
			memcpy(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		}
		rtf_spin_lock_irqsave(flags, mbx->buflock);
		mbx->fbyte = MOD_SIZE(mbx->fbyte + tocpy);
		mbx->frbs += tocpy;
		mbx->avbs -= tocpy;
		rtf_spin_unlock_irqrestore(flags, mbx->buflock);
		msg_size  -= tocpy;
		*msg      += tocpy;
	}
	return msg_size;
}

static inline int mbx_evdrp(F_MBX *mbx, char **msg, int msg_size, int lnx)
{
	int tocpy, fbyte, avbs;

	fbyte = mbx->fbyte;
	avbs  = mbx->avbs;
	while (msg_size > 0 && avbs) {
		if ((tocpy = mbx->size - fbyte) > msg_size) {
			tocpy = msg_size;
		}
		if (tocpy > avbs) {
			tocpy = avbs;
		}
		if (lnx) {
			rt_copy_to_user(*msg, mbx->bufadr + fbyte, tocpy);
		} else {
			memcpy(*msg, mbx->bufadr + fbyte, tocpy);
		}
		avbs     -= tocpy;
		msg_size -= tocpy;
		*msg     += tocpy;
		fbyte = MOD_SIZE(fbyte + tocpy);
	}
	return msg_size;
}

static inline void mbx_sem_init(F_SEM *sem, int value)
{
	sem->free  = value;
	sem->qtype = 0;
	sem->queue.prev = &(sem->queue);
	sem->queue.next = &(sem->queue);
	sem->queue.task = 0;
}

static inline int mbx_sem_delete(F_SEM *sem)
{
	unsigned long flags;
	LX_TASK *task;

	rtf_save_flags_and_cli(flags);
	while ((task = (sem->queue.next)->task)) {
		sem->queue.next = task->queue.next;
		(task->queue.next)->prev = &(sem->queue);
		taskq.task[taskq.in] = task->task;
		taskq.in = (taskq.in + 1) & (MAXREQS - 1);
		rtf_pend_srq(fifo_srq);
	}
	rtf_restore_flags(flags);
	return 0;
}

static inline void mbx_init(F_MBX *mbx, int size, char *bufadr)
{
	mbx_sem_init(&(mbx->sndsem), 1);
	mbx_sem_init(&(mbx->rcvsem), 1);
	mbx->waiting_task = 0;
	mbx->bufadr = bufadr;
	mbx->size = mbx->frbs = size;
	mbx->fbyte = mbx->lbyte = mbx->avbs = 0;
#ifdef CONFIG_SMP
        spin_lock_init(&mbx->buflock);
#endif
	spin_lock_init(&(mbx->buflock));
}

static inline int mbx_delete(F_MBX *mbx)
{
	mbx_signal(mbx);
	if (mbx_sem_delete(&(mbx->sndsem)) || mbx_sem_delete(&(mbx->rcvsem))) {
		return -EFAULT;
	}
	return 0;
}

static inline int mbx_send(F_MBX *mbx, const char *msg, int msg_size, int lnx)
{
	if (mbx_sem_wait(&(mbx->sndsem))) {
		return msg_size;
	}
	while (msg_size) {
		if (mbx_wait(mbx, &mbx->frbs)) {
			mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
			return msg_size;
		}
		msg_size = mbx_put(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
	}
	mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
	return 0;
}

static inline int mbx_send_wp(F_MBX *mbx, const char *msg, int msg_size, int lnx)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (mbx->sndsem.free && mbx->frbs) {
		mbx->sndsem.free = 0;
		rtf_restore_flags(flags);
		msg_size = mbx_put(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
		mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
	} else {
		rtf_restore_flags(flags);
	}
	return msg_size;
}

static inline int mbx_send_if(F_MBX *mbx, const char *msg, int msg_size, int lnx)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (mbx->sndsem.free && (mbx->frbs >= msg_size)) {
		mbx->sndsem.free = 0;
		rtf_restore_flags(flags);
		msg_size = mbx_put(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
		mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
	} else {
		rtf_restore_flags(flags);
	}
	return msg_size;
}

static int mbx_send_timed(F_MBX *mbx, const char *msg, int msg_size, int delay, int lnx)
{
	if (mbx_sem_wait_timed(&(mbx->sndsem), delay)) {
		return msg_size;
	}
	while (msg_size) {
		if (mbx_wait_timed(mbx, &(mbx->frbs), delay)) {
			mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
			return msg_size;
		}
		msg_size = mbx_put(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
	}
	mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
	return 0;
}

static inline int mbx_receive(F_MBX *mbx, void *msg, int msg_size, int lnx)
{
	if (mbx_sem_wait(&(mbx->rcvsem))) {
		return msg_size;
	}
	while (msg_size) {
		if (mbx_wait(mbx, &mbx->avbs)) {
			mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
			return msg_size;
		}
		msg_size = mbx_get(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
	}
	mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
	return 0;
}

static inline int mbx_receive_wjo(F_MBX *mbx, void *msg, int msg_size, int lnx)
{
	if (mbx_sem_wait(&(mbx->rcvsem))) {
		return msg_size;
	}
	if (msg_size) {
		if (mbx_wait(mbx, &mbx->avbs)) {
			mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
			return msg_size;
		}
		msg_size = mbx_get(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
	}
	mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
	return msg_size;
}

static inline int mbx_receive_wp(F_MBX *mbx, void *msg, int msg_size, int lnx)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (mbx->rcvsem.free && mbx->avbs) {
		mbx->rcvsem.free = 0;
		rtf_restore_flags(flags);
		msg_size = mbx_get(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
		mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
	} else {
		rtf_restore_flags(flags);
	}
	return msg_size;
}

static inline int mbx_receive_if(F_MBX *mbx, void *msg, int msg_size, int lnx)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (mbx->rcvsem.free && mbx->avbs >= msg_size) {
		mbx->rcvsem.free = 0;
		rtf_restore_flags(flags);
		msg_size = mbx_get(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
		mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
	} else {
		rtf_restore_flags(flags);
	}
	return msg_size;
}

static int mbx_receive_timed(F_MBX *mbx, void *msg, int msg_size, int delay, int lnx)
{
	if (mbx_sem_wait_timed(&(mbx->rcvsem), delay)) {
		return msg_size;
	}
	while (msg_size) {
		if (mbx_wait_timed(mbx, &(mbx->avbs), delay)) {
			mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
			return msg_size;
		}
		msg_size = mbx_get(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
	}
	mbx_sem_signal(&(mbx->rcvsem), (FIFO *)mbx);
	return 0;
}

static inline int mbx_ovrwr_send(F_MBX *mbx, void *msg, int msg_size, int lnx)
{
	unsigned long flags;

	rtf_save_flags_and_cli(flags);
	if (mbx->sndsem.free) {
		mbx->sndsem.free = 0;
		rtf_restore_flags(flags);
		msg_size = mbx_ovrwr_put(mbx, (char **)(&msg), msg_size, lnx);
		mbx_signal(mbx);
		mbx_sem_signal(&(mbx->sndsem), (FIFO *)mbx);
	} else {
		rtf_restore_flags(flags);
	}
	return msg_size;
}

static void rtf_sysrq_handler(void)
{
	FIFO *fifop;
	while (taskq.out != taskq.in) {
		if (taskq.task[taskq.out]->state == TASK_INTERRUPTIBLE) {
			wake_up_process(taskq.task[taskq.out]);
		}
		taskq.out = (taskq.out + 1) & (MAXREQS - 1);
	}

	while (pol_asyn_q.out != pol_asyn_q.in) {
		fifop = pol_asyn_q.fifo[pol_asyn_q.out];
		fifop->pol_asyn_pended = 0;
		if (waitqueue_active(&(fifop = pol_asyn_q.fifo[pol_asyn_q.out])->pollq)) {
			wake_up_interruptible(&(fifop->pollq));
		}
		if (fifop->asynq) { 
			kill_fasync(&fifop->asynq, async_sig, POLL_IN);
		}
		pol_asyn_q.out = (pol_asyn_q.out + 1) & (MAXREQS - 1);
	}
	set_tsk_need_resched(current);
}

#define VALID_FIFO	if (minor >= MAX_FIFOS) { return -ENODEV; } \
			if (!(fifo[minor].opncnt)) { return -EINVAL; }

/**
 * @ingroup fifos_ipc
 * Reset a real-time FIFO
 *
 * rtf_reset resets RT-FIFO @a fd_fifo by setting its buffer pointers to zero,
 * so that any existing data is discarded and the fifo started anew like at its
 * creations.   It can be used both in kernel and user space.
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space.
 *
 * @retval 0 on succes.
 * @retval ENODEV if @a fd_fifo is greater than or equal to RTF_NO.
 * @retval EINVAL if @a fd_fifo refers to a not opened fifo.
 * @retval EFAULT if the operation was unsuccessful.
 */
RTAI_SYSCALL_MODE int rtf_reset(unsigned int minor)
{
	int semval;
	F_MBX *mbx;
	
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_RESET, minor, 0);

	mbx = &(fifo[minor].mbx);
	if (!mbx_sem_wait(&(mbx->rcvsem))) {
		while (!(semval = mbx_sem_wait_if(&mbx->sndsem)) && !mbx->waiting_task) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(1);
		}
	} else {
		return -EBADF;
	}
	mbx->frbs = mbx->size;
	mbx->fbyte = mbx->lbyte = mbx->avbs = 0;
	if (semval) {
		mbx_sem_signal(&mbx->sndsem, (FIFO *)mbx);
	} else {
		mbx_signal(mbx);
	}
	mbx_sem_signal(&mbx->rcvsem, (FIFO *)mbx);
	return 0;
}


/**
 * @ingroup fifos_ipc
 * Resize a real-time FIFO
 *
 * rtf_resize modifies the real-time fifo fifo, previously created with,
 * rtf_create(), to have a new size of @a size. Any data in the fifo is
 * discarded.
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space.
 *
 * @param size is the requested new size.
 *
 * @retval size on success.
 * @retval -ENODEV if @a fifo is greater than or equal to RTF_NO.
 * @retval -EINVAL if @a fifo refers to a not opened fifo.
 * @retval -ENOMEM if @a size bytes could not be allocated for the RT-FIFO. Fifo
 * @retval -EBUSY if @a size is smaller than actual content
 * size is unchanged.
 */
RTAI_SYSCALL_MODE int rtf_resize(unsigned int minor, int size)
{
	void *oldbuf, *newbuf;
	int old_malloc_type, new_malloc_type, semval;
	F_MBX *mbx;
	
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_RESIZE, minor, size);

	mbx = &(fifo[minor].mbx);
	if (!size) {
		return -EINVAL;
	}
	if (size <= PAGE_SIZE*32) {
		if (!(newbuf = kmalloc(size, GFP_KERNEL))) {
			return -ENOMEM;
		}
		new_malloc_type = 'k';
	} else {
		if (!(newbuf = vmalloc(size))) {
			return -ENOMEM;
		}
		new_malloc_type = 'v';
	}
	if (!mbx_sem_wait(&(mbx->rcvsem))) {
		while (!(semval = mbx_sem_wait_if(&mbx->sndsem)) && !mbx->waiting_task) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(1);
		}
	} else {
		return -EBADF;
	}
	if (size < mbx->avbs) {
		if (semval) {
			mbx_sem_signal(&mbx->sndsem, (FIFO *)mbx);
		}
		mbx_sem_signal(&mbx->rcvsem, (FIFO *)mbx);
		new_malloc_type == 'k' ? kfree(newbuf) : vfree(newbuf);
		return -EBUSY;
	}
	oldbuf = newbuf;
	mbx->frbs = mbx_get(mbx, (char **)(&oldbuf), size, 0);
	mbx->avbs = mbx->lbyte = size - mbx->frbs;
	mbx->fbyte = 0;
	oldbuf = mbx->bufadr;
	mbx->bufadr = newbuf;
	old_malloc_type = fifo[minor].malloc_type;
	fifo[minor].malloc_type = new_malloc_type;
	if (semval) {
		mbx->size = size;
		mbx_sem_signal(&mbx->sndsem, (FIFO *)mbx);
	} else if (size > mbx->size) {
		mbx->size = size;
		mbx_signal(mbx);
	}
	mbx_sem_signal(&mbx->rcvsem, (FIFO *)mbx);
	old_malloc_type == 'k' ? kfree(oldbuf) : vfree(oldbuf);
	return size;
}


/**
 * @ingroup fifos_ipc
 * Create a real-time FIFO
 *
 * rtf_create creates a real-time fifo (RT-FIFO) of initial size @a size and
 * assigns it the identifier @a fifo.  It must be used only in kernel space.
 *
 * @param minor is a positive integer that identifies the fifo on further
 * operations. It has to be less than RTF_NO.
 *
 * @param size is the requested size for the fifo. 
 * 
 * @a fifo may refer to an existing RT-FIFO. In this case the size is adjusted
 * if necessary.
 *
 * The RT-FIFO is a character based mechanism to communicate among real-time
 * tasks and ordinary Linux processes. The rtf_* functions are used by the
 * real-time tasks; Linux processes use standard character device access
 * functions such as read, write, and select.
 *
 * If this function finds an existing fifo of lower size it resizes it to the
 * larger new size. Note that the same condition apply to the standard Linux
 * device open, except that when it does not find any already existing fifo it
 * creates it with a default size of 1K bytes.
 *
 * It must be remarked that practically any fifo size can be asked for. In
 * fact if @a size is within the constraint allowed by kmalloc such a function
 * is used, otherwise vmalloc is called, thus allowing any size that can fit
 * into the available core memory.
 *
 * Multiple calls of this function are allowed, a counter is kept internally to
 * track their number, and avoid destroying/closing a fifo that is still used.
 *
 * @retval 0 on success
 * @retval ENODEV if fifo is greater than or equal to RTF_NO
 * @retval ENOMEM if the necessary size could not be allocated for the RT-FIFO.
 *
 */
RTAI_SYSCALL_MODE int rtf_create(unsigned int minor, int size)
{
	void *buf;

	if (minor >= MAX_FIFOS) {
		return -ENODEV;
	}
	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_CREATE, minor, size);
	if (!atomic_cmpxchg((atomic_t *)&fifo[minor].opncnt, 0, 1)) {
		if (size <= PAGE_SIZE*32) {
			if (!(buf = kmalloc(size, GFP_KERNEL))) {
				fifo[minor].opncnt = 0;
				return -ENOMEM;
			}
			fifo[minor].malloc_type = 'k';
		} else {
			if (!(buf = vmalloc(size))) {
				fifo[minor].opncnt = 0;
				return -ENOMEM;
			}
			fifo[minor].malloc_type = 'v';
		}
		fifo[minor].handler = do_nothing;
		mbx_init(&(fifo[minor].mbx), size, buf);
		mbx_sem_init(&(fifo[minor].sem), 0);
		fifo[minor].pol_asyn_pended = 0;
		fifo[minor].asynq = 0;
	} else {
		if (size > fifo[minor].mbx.size) {
			rtf_resize(minor, size);
		}
		atomic_inc((atomic_t *)&fifo[minor].opncnt);
	}
	return 0;
}

/**
 * @ingroup fifos_ipc
 * Close a real-time FIFO
 *
 * rtf_destroy closes, in kernel space, a real-time fifo previously
 * created or reopened with rtf_create() or rtf_open_sized(). An internal
 * mechanism counts how many times a fifo was opened. Opens and closes must be
 * in pair. rtf_destroy should be called as many times as rtf_create was.
 * After the last close the fifo is really destroyed.
 *
 * No need for any particular function for the same service in user space,
 * simply use the standard Unix close.
 *
 * @return a non-negative value on success. Actually it is the open counter, that
 * means how many times rtf_destroy should be called yet to destroy the fifo.
 *
 * @return a a negative value is returned as described below.
 * @retval ENODEV if @a fifo is greater than or equal to RTF_NO.
 * @retval EINVAL if @a fifo refers to a not opened fifo.
 *
 * @note The equivalent of rtf_destroy in user space is the standard UNIX
 * close.
 */
RTAI_SYSCALL_MODE int rtf_destroy(unsigned int minor)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_DESTROY, minor, 0);

	if (atomic_dec_and_test((atomic_t *)&fifo[minor].opncnt)) {
		if (fifo[minor].malloc_type == 'k') {
			kfree(fifo[minor].mbx.bufadr); 
		} else {
			vfree(fifo[minor].mbx.bufadr); 
		}
		fifo[minor].handler = do_nothing;
		mbx_delete(&(fifo[minor].mbx));
		fifo[minor].pol_asyn_pended = 0;
		fifo[minor].asynq = 0;
		fifo[minor].name[0] = 0;
	}
	return fifo[minor].opncnt;
}

/**
 * @ingroup fifos_ipc
 * Install a FIFO handler function.
 *
 * rtf_create_handler installs a handler which is executed when data is written
 * to or read from a real-time fifo.
 *
 * @param minor is an RT-FIFO that must have previously been created with a call
 * to rtf_create().
 *
 * @param handler is a pointer on a function wich will be called whenever a
 * Linux process accesses that fifo.
 *
 * rtf_create_handler is often used in conjunction with rtf_get() to process
 * data acquired asynchronously from a Linux process. The installed handler
 * calls rtf_get() when data is present. Because the handler is only executed
 * when there is activity on the fifo, polling is not necessary.
 *
 * @retval 0 on success.
 * @retval EINVAL if @a fifo is greater than or equal to RTF_NO, or handler is
 * @c NULL.
 *
 * @note rtf_create_handler does not check if FIFO referred by @a fifo is open
 * or not. The next call of rtf_create will uninstall the handler just
 * "installed".
 */
int rtf_create_handler(unsigned int minor, void *handler)
{
	if (minor >= MAX_FIFOS || !handler) {
		return -EINVAL;
	}

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_CREATE_HANDLER, minor, handler);

	fifo[minor].handler = handler;
	return 0;
}

/**
 * @ingroup fifos_ipc
 * Write data to FIFO
 *
 * rtf_put tries to write a block of data to a real-time fifo previously created
 * with rtf_create().
 *
 * @param minor is the ID with which the RT-FIFO was created.
 * @param buf points the block of data to be written.
 * @param count is the size of the block in bytes.
 *
 * This mechanism is available only in kernel space, i.e. either in real-time
 * tasks or handlers; Linux processes use a write to the corresponding
 * /dev/fifo\<n\> device to enqueue data to a fifo. Similarly, Linux processes
 * use read or similar functions to read the data previously written via rtf_put
 * by a real-time task.
 *
 * @return the number of bytes written on succes. Note that this value may
 * be less than @a count if @a count bytes of free space is not available in the
 * fifo.
 * @retval ENODEV if @a fifo is greater than or equal to RTF_NO.
 * @retval EINVAL if @a fifo refers to a not opened fifo.
 *
 * @note The equivalent of rtf_put in user space is the standard UNIX write,
 * which can be either blocking or nonblocking according to how you opened the
 * related device.
 */
RTAI_SYSCALL_MODE int rtf_put(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_PUT, minor, count);

	count -= mbx_send_wp(&(fifo[minor].mbx), buf, count, 0);
	return count;
}

RTAI_SYSCALL_MODE int rtf_ovrwr_put(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return mbx_ovrwr_send(&(fifo[minor].mbx), buf, count, 0);
}

RTAI_SYSCALL_MODE int rtf_put_if(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;

	count -= mbx_send_if(&(fifo[minor].mbx), buf, count, 0);
	return count;
}

RTAI_SYSCALL_MODE int rtf_get_avbs(unsigned int minor)
{
	VALID_FIFO;
	return fifo[minor].mbx.avbs;
}

RTAI_SYSCALL_MODE int rtf_get_frbs(unsigned int minor)
{
	VALID_FIFO;
	return fifo[minor].mbx.frbs;
}

/**
 * @ingroup fifos_ipc
 * Read data from FIFO
 *
 * rtf_get tries to read a block of data from a real-time fifo previously
 * created with a call to rtf_create().
 *
 * @param minor is the ID with which the RT-FIFO was created.
 * @param buf points a buffer provided by the caller.
 * @param count is the size of @a buf in bytes.
 *
 * This mechanism is available only to real-time tasks; Linux processes use a
 * read from the corresponding fifo device to dequeue data from a fifo.
 * Similarly, Linux processes use write or similar functions to write the data
 * to be read via rtf_put() by a real-time task.
 *
 * rtf_get is often used in conjunction with rtf_create_handler() to process
 * data received asynchronously from a Linux process. A handler is installed
 * via rtf_create_handler(); this handler calls rtf_get to receive any data
 * present in the RT-FIFO as it becomes available. In this way, polling is not
 * necessary; the handler is called only when data is present in the fifo.
 *
 * @return the size of the received data block on success. Note that this
 * value may be less than count if count bytes of data is not available in the
 * fifo.
 * @retval ENODEV if @a fifo is greater than or equal to RTF_NO.
 * @retval EINVAL if @a fifo refers to a not opened fifo.
 *
 * @note The equivalent of rtf_get in user space is the standard UNIX read,
 * which can be either blocking or nonblocking according to how you opened the
 * related device.
 */
RTAI_SYSCALL_MODE int rtf_get(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_GET, minor, count);

	count -= mbx_receive_wp(&(fifo[minor].mbx), buf, count, 0);
	return count;
}

int rtf_evdrp(unsigned int minor, void *msg, int msg_size)
{
	VALID_FIFO;

	return msg_size - mbx_evdrp(&(fifo[minor].mbx), (char **)(&msg), msg_size, 0);
}

RTAI_SYSCALL_MODE int rtf_get_if(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;

	return count - mbx_receive_if(&(fifo[minor].mbx), buf, count, 0);
}

/**
 * @ingroup fifos_sem
 * Initialize a binary semaphore
 *
 * rtf_sem_init initializes a semaphore identified by the file descriptor or
 * fifo number @a fd_fifo.
 *
 * A fifo semaphore can be used for communication and synchronization between
 * kernel and user space.
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space. In fact
 * fifos semaphores must be associated to a fifo for identification purposes.
 * @param value is the initial value of the semaphore, it must be either 0 or
 * 1.
 *
 * rt_sem_init can be used both in kernel and user space.
 *
 * @retval 0 on success.
 * @retval EINVAL if @a fd_fifo refers to an invalid file descriptor or fifo.
 */
RTAI_SYSCALL_MODE int rtf_sem_init(unsigned int minor, int value)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_INIT, minor, value);

	mbx_sem_init(&(fifo[minor].sem), value);
	return 0;
}

/**
 * @ingroup fifos_sem
 * Posting (signaling) a semaphore.
 *
 * rtf_sem_post signal an event to a semaphore. The semaphore value is set to
 * one and the first process, if any, in semaphore's waiting queue is allowed to
 * run.
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space. In fact
 * fifos semaphores must be associated to a fifo for identification purposes.
 *
 * Since it is not blocking rtf_sem_post can be used both in kernel and user
 * space.
 *
 * @retval 0 on success.
 * @retval EINVAL if @a fd_fifo refers to an invalid file descriptor or fifo.
 */
RTAI_SYSCALL_MODE int rtf_sem_post(unsigned int minor)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_POST, minor, 0);

	mbx_sem_signal(&(fifo[minor].sem), 0);
	return 0;
}

/**
 * @ingroup fifos_sem
 * Take a semaphore, only if the calling task is not blocked.
 *
 * rtf_sem_trywait is a version of the semaphore wait operation is similar to
 * rtf_sem_wait() but it is never blocks the caller.   If the semaphore is not
 * free, rtf_sem_trywait returns immediately and the semaphore value remains
 * unchanged.
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space. In fact
 * fifos semaphores must be associated to a fifo for identification purposes.
 *
 * Since it is not blocking rtf_sem_trywait can be used both in kernel and user
 * space.
 *
 * @retval 0 on success.
 * @retval EINVAL if @a fd_fifo  refers to an invalid file descriptor or fifo.
 */
RTAI_SYSCALL_MODE int rtf_sem_trywait(unsigned int minor)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_TRY_WAIT, minor, 0);

	return mbx_sem_wait_if(&(fifo[minor].sem));
}

/**
 * @ingroup fifos_sem
 * Delete a semaphore
 *
 * rtf_sem_destroy deletes a semaphore previously created with rtf_sem_init().
 *
 * @param minor is a file descriptor returned by standard UNIX open in user
 * space while it is directly the chosen fifo number in kernel space. In fact
 * fifos semaphores must be associated to a fifo for identification purposes.
 *
 * Any tasks blocked on this semaphore is returned in error and allowed to run
 * when semaphore is destroyed.
 *
 * rtf_sem_destroy can be used both in kernel and user space.
 *
 * @retval 0 on sucess.
 * @retval EINVAL if @a fd_fifo refers to an invalid file descriptor or fifo.
 */
RTAI_SYSCALL_MODE int rtf_sem_destroy(unsigned int minor)
{
	VALID_FIFO;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_DESTROY, minor, 0);

	return mbx_sem_delete(&(fifo[minor].sem));
}

static int rtf_open(struct inode *inode, struct file *filp)
{
#define DEFAULT_SIZE 1000
	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_OPEN, MINOR(inode->i_rdev), DEFAULT_SIZE);

	return rtf_create(MINOR(inode->i_rdev), DEFAULT_SIZE);
}

static int rtf_fasync(int fd, struct file *filp, int mode)
{	
	int minor;
	minor = MINOR((filp->f_dentry->d_inode)->i_rdev);

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_FASYNC, minor, fd);

	return fasync_helper(fd, filp, mode, &(fifo[minor].asynq));
	if (!mode) {
		fifo[minor].asynq = 0;
	}
}

static int rtf_release(struct inode *inode, struct file *filp)
{
	int minor;
	minor = MINOR(inode->i_rdev);

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_RELEASE, minor, 0);

	if (waitqueue_active(&(fifo[minor].pollq))) {
		wake_up_interruptible(&(fifo[minor].pollq));
	}
	rtf_fasync(-1, filp, 0);
	set_tsk_need_resched(current);
	return rtf_destroy(minor);
}

static ssize_t rtf_read(struct file *filp, char *buf, size_t count, loff_t* ppos)
{
	struct inode *inode = filp->f_dentry->d_inode;
	unsigned int minor = MINOR(inode->i_rdev);
	int handler_ret;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_READ, minor, count);

	if (filp->f_flags & O_NONBLOCK) {
		count -= mbx_receive_wp(&(fifo[minor].mbx), buf, count, 1);
		if (!count) {
			return -EAGAIN;
		}
	} else {
		count -= mbx_receive_wjo(&(fifo[minor].mbx), buf, count, 1);
	}

	if (count) {
		inode->i_atime = CURRENT_TIME;
		if ((handler_ret = (fifo[minor].handler)(minor, 'r')) < 0) {
			return handler_ret;
		}
	}

	return count;
}

static ssize_t rtf_write(struct file *filp, const char *buf, size_t count, loff_t* ppos)
{
	struct inode *inode = filp->f_dentry->d_inode;
	unsigned int minor = MINOR(inode->i_rdev);
	int handler_ret;

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_WRITE, minor, count);

	if (filp->f_flags & O_NONBLOCK) {
		count -= mbx_send_wp(&(fifo[minor].mbx), (char *)buf, count, 1);
		if (!count) {
			return -EAGAIN;
		}
	} else {
		count -= mbx_send(&(fifo[minor].mbx), (char *)buf, count, 1);
	}

	inode->i_ctime = inode->i_mtime = CURRENT_TIME;
	if ((handler_ret = (fifo[minor].handler)(minor, 'w')) < 0) {
		return handler_ret;
	}

	return count;
}

#define DELAY(x) (((x)*HZ + 500)/1000)

#ifdef HAVE_UNLOCKED_IOCTL
static long rtf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode = filp->f_dentry->d_inode;
#else
static int rtf_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
#endif
	unsigned int minor;
	FIFO *fifop;

	fifop = fifo + (minor = MINOR(inode->i_rdev));

	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_IOCTL, minor, cmd);

	switch(cmd) {
		case RESET: {
			return rtf_reset(minor);
		}
		case RESIZE: {
			return rtf_resize(minor, arg);
		}
		case SUSPEND_TIMED: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SUSPEND_TIMED, DELAY(arg), 0);
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(DELAY(arg));
			if (signal_pending(current)) {
				return -ERESTARTSYS;
			}
			return 0;
		}
		case OPEN_SIZED: {
			return rtf_create(minor, arg);
		}
		case READ_ALL_AT_ONCE: {
			struct { char *buf; int count; } args;
			int handler_ret;
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_READ_ALLATONCE, 0, 0);
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			args.count -= mbx_receive(&(fifop->mbx), args.buf, args.count, 1);
			if (args.count) {
				inode->i_atime = CURRENT_TIME;
				if ((handler_ret = (fifo[minor].handler)(minor,
'r')) < 0) {
					return handler_ret;
				}
				return args.count;
			}
			return 0;
		}
		case EAVESDROP: {
			struct { char *buf; int count; } args;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			return args.count - mbx_evdrp(&(fifop->mbx), (char **)&args.buf, args.count, 1);
		}
		case READ_TIMED: {
			struct { char *buf; int count, delay; } args;
			int handler_ret;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_READ_TIMED, args.count, DELAY(args.delay));
			if (!args.delay) {
				args.count -= mbx_receive_wp(&(fifop->mbx), args.buf, args.count, 1);
				if (!args.count) {
					return -EAGAIN;
				}
			} else {
				args.count -= mbx_receive_timed(&(fifop->mbx), args.buf, args.count, DELAY(args.delay), 1);
			}
			if (args.count) {
				inode->i_atime = CURRENT_TIME;
//				if ((handler_ret = (fifop->handler)(minor)) < 0) {
				if ((handler_ret = (fifop->handler)(minor, 'r')) < 0) {
					return handler_ret;
				}
				return args.count;
			}
			return 0;
		}
		case READ_IF: {
			struct { char *buf; int count; } args;
			int handler_ret;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			args.count -= mbx_receive_if(&(fifop->mbx), args.buf, args.count, 1);
			if (args.count) {
				inode->i_atime = CURRENT_TIME;
				if ((handler_ret = (fifo[minor].handler)(minor, 'r')) < 0) {
					return handler_ret;
				}
				return args.count;
			}
			return 0;
		}
		case WRITE_TIMED: {
			struct { char *buf; int count, delay; } args;
			int handler_ret;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_WRITE_TIMED, args.count, DELAY(args.delay));
			if (!args.delay) {
				args.count -= mbx_send_wp(&(fifop->mbx), args.buf, args.count, 1);
				if (!args.count) {
					return -EAGAIN;
				}
			} else {
				args.count -= mbx_send_timed(&(fifop->mbx), args.buf, args.count, DELAY(args.delay), 1);
			}
			inode->i_ctime = inode->i_mtime = CURRENT_TIME;
//			if ((handler_ret = (fifop->handler)(minor)) < 0) {
			if ((handler_ret = (fifop->handler)(minor, 'w')) < 0) {
				return handler_ret;
			}
			return args.count;
		}
		case WRITE_IF: {
			struct { char *buf; int count, delay; } args;
			int handler_ret;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			if (args.count) {
				args.count -= mbx_send_wp(&(fifop->mbx), args.buf, args.count, 1);
				if (!args.count) {
					return -EAGAIN;
				}
			}
			inode->i_ctime = inode->i_mtime = CURRENT_TIME;
			if ((handler_ret = (fifo[minor].handler)(minor, 'w')) <
0) {
				return handler_ret;
			}
			return args.count;
		}
		case OVRWRITE: {
			struct { char *buf; int count; } args;
			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			return mbx_ovrwr_send(&(fifop->mbx), (char **)&args.buf, args.count, 1);
		}
		case RTF_SEM_INIT: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_INIT, minor, arg);
			mbx_sem_init(&(fifop->sem), arg);
			return 0;
		}
		case RTF_SEM_WAIT: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_WAIT, minor, 0);
			return mbx_sem_wait(&(fifop->sem));
		}
		case RTF_SEM_TRYWAIT: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_TRY_WAIT, minor, 0);
			return mbx_sem_wait_if(&(fifop->sem));
		}
		case RTF_SEM_TIMED_WAIT: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_TIMED_WAIT, minor, DELAY(arg));
			return mbx_sem_wait_timed(&(fifop->sem), DELAY(arg));
		}
		case RTF_SEM_POST: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_POST, minor, 0);
			mbx_sem_signal(&(fifop->sem), 0);
			return 0;
		}
		case RTF_SEM_DESTROY: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SEM_DESTROY, minor, 0);
			mbx_sem_delete(&(fifop->sem));
			return 0;
		}
		case SET_ASYNC_SIG: {
			TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_SET_ASYNC_SIG, arg, 0);
			async_sig = arg;
			return 0;
		}
		case FIONREAD: {
			return put_user(fifo[minor].mbx.avbs, (int *)arg);
	        }
		/* 
		 * Support for named FIFOS : Ian Soanes (ians@zentropix.com)
		 * Based on ideas from Stuart Hughes and David Schleef
		 */
		case RTF_GET_N_FIFOS: {
			return MAX_FIFOS;
		}
		case RTF_GET_FIFO_INFO: {
			struct rt_fifo_get_info_struct req = { 0, 0, NULL };
			int i, n;

			rt_copy_from_user(&req, (void *)arg, sizeof(req));
			for ( i = req.fifo, n = 0; 
			      i < MAX_FIFOS && n < req.n; 
			      i++, n++
			      ) {
				struct rt_fifo_info_struct info;

				info.fifo_number = i;
				info.size        = fifo[i].mbx.size;
				info.opncnt      = fifo[i].opncnt;
				info.avbs        = fifo[i].mbx.avbs;
				info.frbs        = fifo[i].mbx.frbs;
				strncpy(info.name, fifo[i].name, RTF_NAMELEN+1);
				rt_copy_to_user(req.ptr + n, &info, sizeof(info));
			}
			return n;
		}
		case RTF_NAMED_CREATE: {
			struct { char name[RTF_NAMELEN+1]; int size; } args;

			rt_copy_from_user(&args, (void *)arg, sizeof(args));
			return rtf_named_create(args.name, args.size);
	        }
		case RTF_CREATE_NAMED: {
			char name[RTF_NAMELEN+1];

			rt_copy_from_user(name, (void *)arg, RTF_NAMELEN+1);
			return rtf_create_named(name);
	        }
		case RTF_NAME_LOOKUP: {
			char name[RTF_NAMELEN+1];

			rt_copy_from_user(name, (void *)arg, RTF_NAMELEN+1);
			return rtf_getfifobyname(name);
		}
	        case TCGETS:
		        /* Keep isatty() probing silent */
		        return -ENOTTY;

		default : {
			printk("RTAI-FIFO: cmd %d is not implemented\n", cmd);
			return -EINVAL;
	}
	}
	return 0;
}

static unsigned int rtf_poll(struct file *filp, poll_table *wait)
{
	unsigned int retval, minor;

	retval = 0;
	minor = MINOR((filp->f_dentry->d_inode)->i_rdev);
	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_POLL, minor, 0);
	poll_wait(filp, &(fifo[minor].pollq), wait);
	if (fifo[minor].mbx.avbs) {
		retval |= POLLIN | POLLRDNORM;
	}
	if (fifo[minor].mbx.frbs) {
		retval |= POLLOUT | POLLWRNORM;
	}
	return retval;
}

static loff_t rtf_llseek(struct file *filp, loff_t offset, int origin)
{
	TRACE_RTAI_FIFO(TRACE_RTAI_EV_FIFO_LLSEEK, MINOR((filp->f_dentry->d_inode)->i_rdev), offset);

	return rtf_reset(MINOR((filp->f_dentry->d_inode)->i_rdev));
}

static struct file_operations rtf_fops =
{
	owner:		THIS_MODULE,
	llseek:		rtf_llseek,
	read:		rtf_read,
	write:		rtf_write,
	poll:		rtf_poll,
#ifdef HAVE_UNLOCKED_IOCTL
	unlocked_ioctl:	rtf_ioctl,
#else
	ioctl:		rtf_ioctl,
#endif
	open:		rtf_open,
	release:	rtf_release,
	fasync:		rtf_fasync,
};

RTAI_SYSCALL_MODE int rtf_ovrwr_put_user(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return mbx_ovrwr_send(&(fifo[minor].mbx), buf, count, 1);
}

RTAI_SYSCALL_MODE int rtf_put_user(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return count - mbx_send_wp(&(fifo[minor].mbx), buf, count, 1);
}

RTAI_SYSCALL_MODE int rtf_get_user(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return count - mbx_receive_wp(&(fifo[minor].mbx), buf, count, 1);
}

RTAI_SYSCALL_MODE int rtf_put_if_user(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return count - mbx_send_if(&(fifo[minor].mbx), buf, count, 1);
}

RTAI_SYSCALL_MODE int rtf_get_if_user(unsigned int minor, void *buf, int count)
{
	VALID_FIFO;
	return count - mbx_receive_if(&(fifo[minor].mbx), buf, count, 1);
}

static int MaxFifos = MAX_FIFOS;
RTAI_MODULE_PARM(MaxFifos, int);

#define LXRTEXT  /* undef it to allow using fifos without any RTAI scheduler */

#ifdef LXRTEXT

static struct rt_fun_entry rtai_fifos_fun[] = {
	[_CREATE]       = { 0, rtf_create },
	[_DESTROY]      = { 0, rtf_destroy },
	[_PUT]          = { 0, rtf_put_user },
	[_GET]          = { 0, rtf_get_user },
	[_RESET]        = { 0, rtf_reset },
	[_RESIZE]       = { 0, rtf_resize },
	[_SEM_INIT]     = { 0, rtf_sem_init },
	[_SEM_DESTRY]   = { 0, rtf_sem_destroy },
	[_SEM_POST]     = { 0, rtf_sem_post },
	[_SEM_TRY]      = { 0, rtf_sem_trywait },
	[_CREATE_NAMED] = { 0, rtf_create_named },
	[_GETBY_NAME]   = { 0, rtf_getfifobyname },
	[_OVERWRITE]    = { 0, rtf_ovrwr_put_user },
	[_PUT_IF]       = { 0, rtf_put_if_user },
	[_GET_IF]       = { 0, rtf_get_if_user },
	[_AVBS]         = { 0, rtf_get_avbs },
	[_FRBS]         = { 0, rtf_get_frbs }
};

static int register_lxrt_fifos_support(void)
{
	if (set_rt_fun_ext_index(rtai_fifos_fun, FUN_FIFOS_LXRT_INDX)) {
		printk("LXRT EXTENSION SLOT FOR FIFOS (%d) ALREADY USED\n", FUN_FIFOS_LXRT_INDX);
		return -EACCES;
	}
	return 0;
}

static void unregister_lxrt_fifos_support(void)
{
	reset_rt_fun_ext_index(rtai_fifos_fun, FUN_FIFOS_LXRT_INDX);
}

#else

static int register_lxrt_fifos_support(void) { return 0; }
#define unregister_lxrt_fifos_support()

#endif

static class_t *fifo_class = NULL;

int __rtai_fifos_init(void)
{
	int minor;

	if (!(fifo = (FIFO *)kmalloc(MaxFifos*sizeof(FIFO), GFP_KERNEL))) {
		printk("RTAI-FIFO: cannot allocate memory for FIFOS structure.\n");
		return -ENOSPC;
	}
	memset(fifo, 0, MaxFifos*sizeof(FIFO));

	if ((fifo_class = class_create(THIS_MODULE, "rtai_fifos")) == NULL) {
		printk("RTAI-FIFO: cannot create class.\n");
		return -EBUSY;
	}
	for (minor = 0; minor < MAX_FIFOS; minor++) {
		if (CLASS_DEVICE_CREATE(fifo_class, MKDEV(RTAI_FIFOS_MAJOR, minor), NULL, "rtf%d", minor) == NULL) {
			printk("RTAI-FIFO: cannot attach class.\n");
			class_destroy(fifo_class);
                        return -EBUSY;
		}
	}

	if (register_chrdev(RTAI_FIFOS_MAJOR, "rtai_fifo", &rtf_fops)) {
		printk("RTAI-FIFO: cannot register major %d.\n", RTAI_FIFOS_MAJOR);
		return -EIO;
	}

	if ((fifo_srq = rtf_request_srq(rtf_sysrq_handler)) < 0) {
		printk("RTAI-FIFO: no srq available in rtai.\n");
		return fifo_srq;
	}
	taskq.in = taskq.out = pol_asyn_q.in = pol_asyn_q.out = 0;
	async_sig = SIGIO;

	for (minor = 0; minor < MAX_FIFOS; minor++) {
		fifo[minor].opncnt = fifo[minor].pol_asyn_pended = 0;
		init_waitqueue_head(&fifo[minor].pollq);
		fifo[minor].asynq = 0;;
		mbx_sem_init(&(fifo[minor].sem), 0);
	}
	rtai_proc_fifo_register();
	return register_lxrt_fifos_support();
}

void __rtai_fifos_exit(void)
{
	unregister_lxrt_fifos_support();
	unregister_chrdev(RTAI_FIFOS_MAJOR, "rtai_fifo");

{
	int minor;
	for (minor = 0; minor < MAX_FIFOS; minor++) {
		class_device_destroy(fifo_class, MKDEV(RTAI_FIFOS_MAJOR, minor));
	}
	class_destroy(fifo_class);
}

	if (rtf_free_srq(fifo_srq) < 0) {
		printk("RTAI-FIFO: rtai srq %d illegal or already free.\n", fifo_srq);
	}
        rtai_proc_fifo_unregister();
	kfree(fifo);
}

#ifndef CONFIG_RTAI_FIFOS_BUILTIN
module_init(__rtai_fifos_init);
module_exit(__rtai_fifos_exit);
#endif /* !CONFIG_RTAI_FIFOS_BUILTIN */

/* ----------------------< proc filesystem section >----------------------*/

static int PROC_READ_FUN(rtai_read_fifos) 
{
	int i;
	PROC_PRINT_VARS;

	PROC_PRINT("RTAI Real Time fifos status.\n\n");
	PROC_PRINT("Maximum number of FIFOS %d.\n\n", MaxFifos);
	PROC_PRINT("fifo No  Open Cnt  Buff Size  handler  malloc type");
	PROC_PRINT(" Name\n----------------");
	PROC_PRINT("-----------------------------------------\n");
/*
 * Display the status of all open RT fifos.
 */
	for (i = 0; i < MAX_FIFOS; i++) {
		if (fifo[i].opncnt > 0) {
			PROC_PRINT("%-8d %-9d %-10d %-10p %-12s", i,
                        	        fifo[i].opncnt, fifo[i].mbx.size,
					fifo[i].handler,
					fifo[i].malloc_type == 'v'
					    ? "vmalloc" : "kmalloc" 
					);
			PROC_PRINT("%s\n", fifo[i].name);
		} /* End if - fifo is open. */
	} /* End for loop - loop for all fifos. */
	return 0;

}  /* End function - rtai_read_fifos */

PROC_READ_OPEN_OPS(rtai_fifos_fops, rtai_read_fifos);

static int rtai_proc_fifo_register(void) 
{
        struct proc_dir_entry *proc_fifo_ent;
        proc_fifo_ent = CREATE_PROC_ENTRY("fifos", S_IFREG|S_IRUGO|S_IWUSR, rtai_proc_root, &rtai_fifos_fops);
        if (!proc_fifo_ent) {
                printk("Unable to initialize /proc/rtai/fifos\n");
                return(-1);
        }
	SET_PROC_READ_ENTRY(proc_fifo_ent, rtai_read_fifos);
	PROC_PRINT_DONE;
}

static void rtai_proc_fifo_unregister(void) 
{
	remove_proc_entry("fifos", rtai_proc_root);
}

/* ------------------< end of proc filesystem section >------------------*/

/* 
 * Support for named FIFOS : Ian Soanes (ians@zentropix.com)
 * Based on ideas from Stuart Hughes and David Schleef
 */
int rtf_named_create(const char *name, int size)
{
	int minor, err;
	unsigned long flags;

	if (strlen(name) > RTF_NAMELEN) {
	    	return -EINVAL;
	}
	rtf_spin_lock_irqsave(flags, rtf_name_lock);
	for (minor = 0; minor < MAX_FIFOS; minor++) {
	    	if (!strncmp(name, fifo[minor].name, RTF_NAMELEN)) {
			break;
		} else if (!fifo[minor].opncnt && !fifo[minor].name[0]) {
		        strncpy(fifo[minor].name, name, RTF_NAMELEN + 1);
			rtf_spin_unlock_irqrestore(flags, rtf_name_lock);
		        if ((err = rtf_create(minor, size)) < 0) {
		        	fifo[minor].name[0] = 0;
			        return err;
			}
			return minor;
		}
	}
	rtf_spin_unlock_irqrestore(flags, rtf_name_lock);
	return -EBUSY;
}

RTAI_SYSCALL_MODE int rtf_create_named(const char *name)
{
	return rtf_named_create(name, DEFAULT_SIZE);
}

RTAI_SYSCALL_MODE int rtf_getfifobyname(const char *name)
{
    	int minor;

	if (strlen(name) > RTF_NAMELEN) {
	    	return -EINVAL;
	}
	for (minor = 0; minor < MAX_FIFOS; minor++) {
	    	if ( fifo[minor].opncnt && 
		     !strncmp(name, fifo[minor].name, RTF_NAMELEN)
		     ) {
		    	return minor;
		}
	}
	return -ENODEV;
}

EXPORT_SYMBOL(rtf_create);
EXPORT_SYMBOL(rtf_create_handler);
EXPORT_SYMBOL(rtf_create_named);
EXPORT_SYMBOL(rtf_destroy);
EXPORT_SYMBOL(rtf_evdrp);
EXPORT_SYMBOL(rtf_get);
EXPORT_SYMBOL(rtf_get_if);
EXPORT_SYMBOL(rtf_getfifobyname);
EXPORT_SYMBOL(rtf_ovrwr_put);
EXPORT_SYMBOL(rtf_put);
EXPORT_SYMBOL(rtf_put_if);
EXPORT_SYMBOL(rtf_get_avbs);
EXPORT_SYMBOL(rtf_get_frbs);
EXPORT_SYMBOL(rtf_reset);
EXPORT_SYMBOL(rtf_resize);
EXPORT_SYMBOL(rtf_sem_destroy);
EXPORT_SYMBOL(rtf_sem_init);
EXPORT_SYMBOL(rtf_sem_post);
EXPORT_SYMBOL(rtf_sem_trywait);
EXPORT_SYMBOL(rtf_named_create);
