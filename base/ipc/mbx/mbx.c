/** 
 * @file
 * Mailbox functions.
 * @author Paolo Mantegazza
 *
 * @note Copyright (C) 1999-2006 Paolo Mantegazza
 * <mantegazza@aero.polimi.it>
 *
 * @note Copyright (C) 2019 Alec Ari
 * <neotheuser@ymail.com> 
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
 *
 * @ingroup mbx
 */

/**
 * @ingroup sched
 * @defgroup mbx Mailbox functions
 *@{*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

#include <asm/uaccess.h>

#include <rtai_schedcore.h>
#include <rtai_prinher.h>

MODULE_LICENSE("GPL");

/* +++++++++++++++++++++++++++++ MAIL BOXES ++++++++++++++++++++++++++++++++ */

#define _mbx_signal(mbx, blckdon) \
do { \
	unsigned long flags; \
	RT_TASK *task; \
	flags = rt_global_save_flags_and_cli(); \
	if ((task = mbx->waiting_task)) { \
		rem_timed_task(task); \
		task->blocked_on  = blckdon; \
		mbx->waiting_task = NULL; \
		if (task->state != RT_SCHED_READY && (task->state &= ~(RT_SCHED_MBXSUSP | RT_SCHED_DELAYED)) == RT_SCHED_READY) { \
			enq_ready_task(task); \
			RT_SCHEDULE(task, rtai_cpuid()); \
		} \
	} \
	rt_global_restore_flags(flags); \
} while (0)

static void mbx_delete_signal(MBX *mbx)
{
	_mbx_signal(mbx, RTP_OBJREM);
}

static void mbx_signal(MBX *mbx)
{
	_mbx_signal(mbx, NULL);
}

static int mbx_wait(MBX *mbx, int *fravbs, RT_TASK *rt_current)
{
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	if (!(*fravbs)) {
		unsigned long retval;
		rt_current->state |= RT_SCHED_MBXSUSP;
		rem_ready_current(rt_current);
		rt_current->blocked_on = (void *)mbx;
		mbx->waiting_task = rt_current;
		rt_schedule();
		if (unlikely(retval = (unsigned long)rt_current->blocked_on)) {
			mbx->waiting_task = NULL;
			rt_global_restore_flags(flags);
			return retval;
		}
	}
	rt_global_restore_flags(flags);
	return 0;
}

static int mbx_wait_until(MBX *mbx, int *fravbs, RTIME time, RT_TASK *rt_current)
{
	unsigned long flags;

	flags = rt_global_save_flags_and_cli();
	if (!(*fravbs)) {
		void *retp;
		rt_current->blocked_on = (void *)mbx;
		mbx->waiting_task = rt_current;
		if ((rt_current->resume_time = time) > rt_smp_time_h[rtai_cpuid()]) {
			rt_current->state |= (RT_SCHED_MBXSUSP | RT_SCHED_DELAYED);
			rem_ready_current(rt_current);
			enq_timed_task(rt_current);
			rt_schedule();
		}
		if (unlikely((retp = rt_current->blocked_on) != NULL)) {
			mbx->waiting_task = NULL;
			rt_global_restore_flags(flags);
			return likely(retp > RTP_HIGERR) ? RTE_TIMOUT : (retp == RTP_UNBLKD ? RTE_UNBLKD : RTE_OBJREM);
		}
	}
	rt_global_restore_flags(flags);
	return 0;
}

#define MOD_SIZE(indx) ((indx) < mbx->size ? (indx) : (indx) - mbx->size)

static int mbxput(MBX *mbx, char **msg, int msg_size, int space)
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
		if (space) {
			memcpy(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		} else {
			rt_copy_from_user(mbx->bufadr + mbx->lbyte, *msg, tocpy);
		}
		flags = rt_spin_lock_irqsave(&(mbx->lock));
		mbx->frbs -= tocpy;
		mbx->avbs += tocpy;
		rt_spin_unlock_irqrestore(flags, &(mbx->lock));
		msg_size -= tocpy;
		*msg     += tocpy;
		mbx->lbyte = MOD_SIZE(mbx->lbyte + tocpy);
	}
	return msg_size;
}

static int mbxovrwrput(MBX *mbx, char **msg, int msg_size, int space)
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
			if (space) {
				memcpy(mbx->bufadr + mbx->lbyte, *msg, tocpy);
			} else {
				rt_copy_from_user(mbx->bufadr + mbx->lbyte, *msg, tocpy);
			}
			flags = rt_spin_lock_irqsave(&(mbx->lock));
			mbx->frbs -= tocpy;
			mbx->avbs += tocpy;
        		rt_spin_unlock_irqrestore(flags, &(mbx->lock));
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
	        		flags = rt_spin_lock_irqsave(&(mbx->lock));
				mbx->frbs  += tocpy;
				mbx->avbs  -= tocpy;
        			rt_spin_unlock_irqrestore(flags, &(mbx->lock));
				mbx->fbyte = MOD_SIZE(mbx->fbyte + tocpy);
			}
		}		
	}
	return 0;
}

static int mbxget(MBX *mbx, char **msg, int msg_size, int space)
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
		if (space) {
			memcpy(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		} else {
			rt_copy_to_user(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		}
		flags = rt_spin_lock_irqsave(&(mbx->lock));
		mbx->frbs  += tocpy;
		mbx->avbs  -= tocpy;
		rt_spin_unlock_irqrestore(flags, &(mbx->lock));
		msg_size -= tocpy;
		*msg     += tocpy;
		mbx->fbyte = MOD_SIZE(mbx->fbyte + tocpy);
	}
	return msg_size;
}

static int mbxevdrp(MBX *mbx, char **msg, int msg_size, int space)
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
		if (space) {
			memcpy(*msg, mbx->bufadr + fbyte, tocpy);
		} else {
			rt_copy_to_user(*msg, mbx->bufadr + mbx->fbyte, tocpy);
		}
		avbs     -= tocpy;
		msg_size -= tocpy;
		*msg     += tocpy;
		fbyte = MOD_SIZE(fbyte + tocpy);
	}
	return msg_size;
}


/**
 * @brief Receives bytes as many as possible leaving the message
 * available for another receive.
 *
 * rt_mbx_evdrp receives at most @e msg_size of bytes of message
 * from the mailbox @e mbx and then returns immediately.
 * Does what rt_mbx_receive_wp does while keeping the message in the mailbox buffer.
 * Useful if one needs to just preview the mailbox content, without actually
 * receiving it.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message to be received.
 *
 * @return The number of bytes not received is returned.
 */
RTAI_SYSCALL_MODE int _rt_mbx_evdrp(MBX *mbx, void *msg, int msg_size, int space)
{
	return mbxevdrp(mbx, (char **)(&msg), msg_size, space);
}


#define CHK_MBX_MAGIC \
do { if (!mbx || mbx->magic != RT_MBX_MAGIC) return (CONFIG_RTAI_USE_NEWERR ? RTE_OBJINV : -EINVAL); } while (0)

#define MBX_RET(msg_size, retval) \
	(CONFIG_RTAI_USE_NEWERR ? retval : msg_size)

/**
 * @brief Initializes a fully typed mailbox queueing tasks
 * according to the specified type.
 *
 * rt_typed_mbx_init initializes a mailbox of size @e size. @e mbx must
 * point to a user allocated MBX structure. Tasks are queued in FIFO
 * order (FIFO_Q), priority order (PRIO_Q) or resource order (RES_Q).
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param size corresponds to the size of the mailbox.
 *
 * @param type corresponds to the queueing policy: FIFO_Q, PRIO_Q or RES_Q.
 *
 * @return On success 0 is returned. On failure, a special value is
 * returned as indicated below:
 * - @b ENOMEM: Space could not be allocated for the mailbox buffer.
 *
 * See also: notes under rt_mbx_init().
 */
RTAI_SYSCALL_MODE int rt_typed_mbx_init(MBX *mbx, int size, int type)
{
	if (!(mbx->bufadr = rt_malloc(size))) { 
		return -ENOMEM;
	}
	rt_typed_sem_init(&(mbx->sndsem), 1, type & 3 ? type : BIN_SEM | type);
	rt_typed_sem_init(&(mbx->rcvsem), 1, type & 3 ? type : BIN_SEM | type);
	mbx->magic = RT_MBX_MAGIC;
	mbx->size = mbx->frbs = size;
	mbx->owndby = mbx->waiting_task = NULL;
	mbx->fbyte = mbx->lbyte = mbx->avbs = 0;
        spin_lock_init(&(mbx->lock));
#ifdef CONFIG_RTAI_RT_POLL
	mbx->poll_recv.pollq.prev = mbx->poll_recv.pollq.next = &(mbx->poll_recv.pollq);
	mbx->poll_send.pollq.prev = mbx->poll_send.pollq.next = &(mbx->poll_send.pollq);
	mbx->poll_recv.pollq.task = mbx->poll_send.pollq.task = NULL;
        spin_lock_init(&(mbx->poll_recv.pollock));
        spin_lock_init(&(mbx->poll_send.pollock));
#endif
	return 0;
}


/**
 * @brief Initializes a mailbox.
 *
 * rt_mbx_init initializes a mailbox of size @e size. @e mbx must
 * point to a user allocated MBX structure.
 * Using mailboxes is a flexible method for inter task
 * communications. Tasks are allowed to send arbitrarily sized
 * messages by using any mailbox buffer size. There is even no need to
 * use a buffer sized at least as the largest message you envisage,
 * even if efficiency is likely to suffer from such a
 * decision. However if you expect a message larger than the average
 * message size very rarely you can use a smaller buffer without much
 * loss of efficiency. In such a way you can set up your own mailbox
 * usage protocol, e.g. using fix sized messages with a buffer that is
 * an integer multiple of such a size guarantees maximum efficiency by
 * having each message sent/received atomically to/from the
 * mailbox. Multiple senders and receivers are allowed and each will
 * get the service it requires in turn, according to its priority. 
 * Thus mailboxes provide a flexible mechanism to allow you to freely
 * implement your own policy.
 *
 * rt_mbx_init is equivalent to rt_typed_mbx_init(mbx, size, PRIO_Q).
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param size corresponds to the size of the mailbox.
 *
 * @return On success 0 is returned. On failure, a special value is
 * returned as indicated below:
 * - @b ENOMEM: Space could not be allocated for the mailbox buffer.
 *
 * See also: notes under rt_typed_mbx_init().
 */
int rt_mbx_init(MBX *mbx, int size)
{
	return rt_typed_mbx_init(mbx, size, PRIO_Q);
}


/**
 *
 * @brief Deletes a mailbox.
 * 
 * rt_mbx_delete removes a mailbox previously created with rt_mbx_init().
 *
 * @param mbx is the pointer to the structure used in the corresponding call
 * to rt_mbx_init.
 *
 * @return 0 is returned on success. On failure, a negative value is
 * returned as described below:
 * - @b EINVAL: @e mbx points to an invalid mailbox.
 * - @b EFAULT: mailbox data were found in an invalid state.
 */
RTAI_SYSCALL_MODE int rt_mbx_delete(MBX *mbx)
{
	CHK_MBX_MAGIC;
	mbx->magic = 0;
	rt_wakeup_pollers(&mbx->poll_recv, RTE_OBJREM);
	rt_wakeup_pollers(&mbx->poll_send, RTE_OBJREM);
	if (rt_sem_delete(&mbx->sndsem) || rt_sem_delete(&mbx->rcvsem)) {
		return -EFAULT;
	}
	while (mbx->waiting_task) {
		mbx_delete_signal(mbx);
	}
	rt_free(mbx->bufadr); 
	return 0;
}


/**
 * @brief Sends a message unconditionally.
 *
 * rt_mbx_send sends a message @e msg of @e msg_size bytes to the
 * mailbox @e mbx. The caller will be blocked until the whole message
 * is copied into the mailbox or an error occurs. Even if the message
 * can be sent in a single shot, the sending task can be blocked if
 * there is a task of higher priority waiting to receive from the
 * mailbox.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg corresponds to the message to be sent.
 *
 * @param msg_size is the size of the message.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all sending tasks.
 * - @b EINVAL: mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_send(MBX *mbx, void *msg, int msg_size, int space)
{
	RT_TASK *rt_current = RT_CURRENT;
	int retval;

	CHK_MBX_MAGIC;
	if ((retval = rt_sem_wait(&mbx->sndsem)) > 1) {
		return MBX_RET(msg_size, retval);
	}
	while (msg_size) {
		if ((retval = mbx_wait(mbx, &mbx->frbs, rt_current))) {
			rt_sem_signal(&mbx->sndsem);
			retval = MBX_RET(msg_size, retval);
			rt_wakeup_pollers(&mbx->poll_recv, retval);
			return retval;
		}
		msg_size = mbxput(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
	}
	rt_sem_signal(&mbx->sndsem);
	rt_wakeup_pollers(&mbx->poll_recv, 0);
	return 0;
}


/**
 * @brief Sends as many bytes as possible without blocking the calling task.
 *
 * rt_mbx_send_wp atomically sends as many bytes of message @e msg as
 * possible to the mailbox @e mbx then returns immediately.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg corresponds to the message to be sent.
 * 
 * @param msg_size is the size of the message.
 *
 * @return On success, the number of unsent bytes is returned. On
 * failure a negative value is returned as described below:
 * - @b EINVAL: @e mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_send_wp(MBX *mbx, void *msg, int msg_size, int space)
{
	unsigned long flags;
	RT_TASK *rt_current = RT_CURRENT;
	int size = msg_size;

	CHK_MBX_MAGIC;
	flags = rt_global_save_flags_and_cli();
	if (mbx->sndsem.count > 0 && mbx->frbs) {
		mbx->sndsem.count = 0;
		if (mbx->sndsem.type > 0) {
			mbx->sndsem.owndby = rt_current;
			enqueue_resqel(&mbx->sndsem.resq, rt_current);
		}
		rt_global_restore_flags(flags);
		msg_size = mbxput(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
		rt_sem_signal(&mbx->sndsem);
	} else {
		rt_global_restore_flags(flags);
	}
	if (msg_size < size) {
		rt_wakeup_pollers(&mbx->poll_recv, 0);
	}
	return msg_size;
}


/**
 * @brief Sends a message, only if the whole message can be passed
 * without blocking the calling task.
 *
 * rt_mbx_send_if tries to atomically send the message @e msg of @e
 * msg_size bytes to the mailbox @e mbx. It returns immediately and
 * the caller is never blocked.
 *
 * @return On success, the number of unsent bytes (0 or @e msg_size)
 * is returned. On failure a negative value is returned as described
 * below:
 * - @b EINVAL: @e mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_send_if(MBX *mbx, void *msg, int msg_size, int space)
{
	unsigned long flags;
	RT_TASK *rt_current = RT_CURRENT;

	CHK_MBX_MAGIC;
	flags = rt_global_save_flags_and_cli();
	if (mbx->sndsem.count > 0 && msg_size <= mbx->frbs) {
		mbx->sndsem.count = 0;
		if (mbx->sndsem.type > 0) {
			mbx->sndsem.owndby = rt_current;
			enqueue_resqel(&mbx->sndsem.resq, rt_current);
		}
		rt_global_restore_flags(flags);
		mbxput(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
		rt_sem_signal(&mbx->sndsem);
		rt_wakeup_pollers(&mbx->poll_recv, 0);
		return 0;
	}
	rt_global_restore_flags(flags);
	return msg_size;
}


/**
 * @brief Sends a message with absolute timeout.
 *
 * rt_mbx_send_until sends a message @e msg of @e msg_size bytes to
 * the mailbox @e mbx. The caller will be blocked until all bytes of
 * message is enqueued, timeout expires or an error occurs.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg is the message to be sent.
 *
 * @param msg_size corresponds to the size of the message.
 *
 * @param time is an absolute value for the timeout.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all sending tasks or the timeout has expired.
 * - @b EINVAL: mbx points to an invalid mailbox.
 *
 * See also: notes under @ref _rt_mbx_send_timed().
 */
RTAI_SYSCALL_MODE int _rt_mbx_send_until(MBX *mbx, void *msg, int msg_size, RTIME time, int space)
{
	RT_TASK *rt_current = RT_CURRENT;
	int retval;

	CHK_MBX_MAGIC;
	if ((retval = rt_sem_wait_until(&mbx->sndsem, time)) > 1) {
		return MBX_RET(msg_size, retval);
	}
	while (msg_size) {
		if ((retval = mbx_wait_until(mbx, &mbx->frbs, time, rt_current))) {
			rt_sem_signal(&mbx->sndsem);
			retval = MBX_RET(msg_size, retval);
			rt_wakeup_pollers(&mbx->poll_recv, retval);
			return retval;
		}
		msg_size = mbxput(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
	}
	rt_sem_signal(&mbx->sndsem);
	rt_wakeup_pollers(&mbx->poll_recv, 0);
	return 0;
}


/**
 * @brief Sends a message with relative timeout.
 *
 * rt_mbx_send_timed send a message @e msg of @e msg_size bytes to the
 * mailbox @e mbx. The caller will be blocked until all bytes of message 
 * is enqueued, timeout expires or an error occurs.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg is the message to be sent.
 *
 * @param msg_size corresponds to the size of the message.
 *
 * @param delay is the timeout value relative to the current time.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all sending tasks or the timeout has expired.
 * - @b EINVAL: mbx points to an invalid mailbox.
 *
 * See also: notes under @ref _rt_mbx_send_until().
 */
RTAI_SYSCALL_MODE int _rt_mbx_send_timed(MBX *mbx, void *msg, int msg_size, RTIME delay, int space)
{
	return _rt_mbx_send_until(mbx, msg, msg_size, get_time() + delay, space);
}


/**
 * @brief Receives a message unconditionally.
 *
 * rt_mbx_receive receives a message of @e msg_size bytes from the
 * mailbox @e mbx. The caller will be blocked until all bytes of the
 * message arrive or an error occurs.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message to be received.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all receiving tasks.
 * - @b EINVAL: mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_receive(MBX *mbx, void *msg, int msg_size, int space)
{
	RT_TASK *rt_current = RT_CURRENT;
	int retval;

	CHK_MBX_MAGIC;
	if ((retval = rt_sem_wait(&mbx->rcvsem)) > 1) {
		return msg_size;
	}
	while (msg_size) {
		if ((retval = mbx_wait(mbx, &mbx->avbs, rt_current))) {
			rt_sem_signal(&mbx->rcvsem);
			retval = MBX_RET(msg_size, retval);
			rt_wakeup_pollers(&mbx->poll_recv, retval);
			return retval;
			return MBX_RET(msg_size, retval);
		}
		msg_size = mbxget(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
	}
	rt_sem_signal(&mbx->rcvsem);
	rt_wakeup_pollers(&mbx->poll_send, 0);
	return 0;
}


/**
 * @brief Receives bytes as many as possible, without blocking the
 * calling task.
 *
 * rt_mbx_receive_wp receives at most @e msg_size of bytes of message
 * from the mailbox @e mbx then returns immediately.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message to be received.
 *
 * @return On success, the number of bytes not received is returned. On
 * failure a negative value is returned as described below:
 * - @b EINVAL: mbx points to not a valid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_receive_wp(MBX *mbx, void *msg, int msg_size, int space)
{
	unsigned long flags;
	RT_TASK *rt_current = RT_CURRENT;
	int size = msg_size;

	CHK_MBX_MAGIC;
	flags = rt_global_save_flags_and_cli();
	if (mbx->rcvsem.count > 0 && mbx->avbs) {
		mbx->rcvsem.count = 0;
		if (mbx->rcvsem.type > 0) {
			mbx->rcvsem.owndby = rt_current;
			enqueue_resqel(&mbx->rcvsem.resq, rt_current);
		}
		rt_global_restore_flags(flags);
		msg_size = mbxget(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
		rt_sem_signal(&mbx->rcvsem);
	} else {
		rt_global_restore_flags(flags);
	}
	if (msg_size < size) {
		rt_wakeup_pollers(&mbx->poll_send, 0);
	}
	return msg_size;
}


/**
 * @brief Receives a message only if the whole message can be passed
 * without blocking the calling task.
 *
 * rt_mbx_receive_if receives a message from the mailbox @e mbx if the
 * whole message of @e msg_size bytes is available immediately.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message to be received.
 *
 * @return On success, the number of bytes not received (0 or @e msg_size)
 * is returned. On failure a negative value is returned as described
 * below:
 * - @b EINVAL: mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_receive_if(MBX *mbx, void *msg, int msg_size, int space)
{
	unsigned long flags;
	RT_TASK *rt_current = RT_CURRENT;

	CHK_MBX_MAGIC;
	flags = rt_global_save_flags_and_cli();
	if (mbx->rcvsem.count > 0 && msg_size <= mbx->avbs) {
		mbx->rcvsem.count = 0;
		if (mbx->rcvsem.type > 0) {
			mbx->rcvsem.owndby = rt_current;
			enqueue_resqel(&mbx->rcvsem.resq, rt_current);
		}
		rt_global_restore_flags(flags);
		mbxget(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
		rt_sem_signal(&mbx->rcvsem);
		rt_wakeup_pollers(&mbx->poll_send, 0);
		return 0;
	}
	rt_global_restore_flags(flags);
	return msg_size;
}


/**
 * @brief Receives a message with absolute timeout.
 *
 * rt_mbx_receive_until receives a message of @e msg_size bytes from
 * the mailbox @e mbx. The caller will be blocked until all bytes of
 * the message arrive, timeout expires or an error occurs.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message received.
 *
 * @param time is an absolute value of the timeout.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all receiving tasks or the timeout has expired.
 * - @b EINVAL: mbx points to an invalid mailbox.
 *
 * See also: notes under rt_mbx_received_timed().
 */
RTAI_SYSCALL_MODE int _rt_mbx_receive_until(MBX *mbx, void *msg, int msg_size, RTIME time, int space)
{
	RT_TASK *rt_current = RT_CURRENT;
	int retval;

	CHK_MBX_MAGIC;
	if ((retval = rt_sem_wait_until(&mbx->rcvsem, time)) > 1) {
		return MBX_RET(msg_size, retval);
	}
	while (msg_size) {
		if ((retval = mbx_wait_until(mbx, &mbx->avbs, time, rt_current))) {
			rt_sem_signal(&mbx->rcvsem);
			retval = MBX_RET(msg_size, retval);
			rt_wakeup_pollers(&mbx->poll_recv, retval);
			return retval;
		}
		msg_size = mbxget(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
	}
	rt_sem_signal(&mbx->rcvsem);
	rt_wakeup_pollers(&mbx->poll_send, 0);
	return 0;
}


/**
 * @brief Receives a message with relative timeout.
 *
 * rt_mbx_receive_timed receives a message of @e msg_size bytes from
 * the mailbox @e mbx. The caller will be blocked until all bytes of 
 * the message arrive, timeout expires or an error occurs.
 *
 * @param mbx is a pointer to a user allocated mailbox structure.
 *
 * @param msg points to a buffer provided by the caller.
 *
 * @param msg_size corresponds to the size of the message received.
 *
 * @param delay is the timeout value relative to the current time.
 *
 * @return On success, 0 is returned.
 * On failure a value is returned as described below:
 * - the number of bytes not received: an error is occured 
 *   in the queueing of all receiving tasks or the timeout has expired.
 * - @b EINVAL: mbx points to an invalid mailbox.
 *
 * See also: notes under rt_mbx_received_until().
 */
RTAI_SYSCALL_MODE int _rt_mbx_receive_timed(MBX *mbx, void *msg, int msg_size, RTIME delay, int space)
{
	return _rt_mbx_receive_until(mbx, msg, msg_size, get_time() + delay, space);
}


/**
 * @brief Sends a message overwriting what already in the buffer
 * if there is no place for the message.
 *
 * rt_mbx_ovrwr_send sends the message @e msg of @e msg_size bytes
 * to the mailbox @e mbx overwriting what already in the mailbox
 * buffer if there is no place for the message. Useful for logging
 * purposes. It returns immediately and the caller is never blocked.
 *
 * @return On success, 0 is returned. On failure a negative value
 * is returned as described below:
 * - @b EINVAL: @e mbx points to an invalid mailbox.
 */
RTAI_SYSCALL_MODE int _rt_mbx_ovrwr_send(MBX *mbx, void *msg, int msg_size, int space)
{
	unsigned long flags;
	RT_TASK *rt_current = RT_CURRENT;

	CHK_MBX_MAGIC;

	flags = rt_global_save_flags_and_cli();
	if (mbx->sndsem.count > 0) {
		mbx->sndsem.count = 0;
		if (mbx->sndsem.type > 0) {
			mbx->sndsem.owndby = rt_current;
			enqueue_resqel(&mbx->sndsem.resq, rt_current);
		}
		rt_global_restore_flags(flags);
		msg_size = mbxovrwrput(mbx, (char **)(&msg), msg_size, space);
		mbx_signal(mbx);
		rt_sem_signal(&mbx->sndsem);
	} else {
		rt_global_restore_flags(flags);
	}
	return msg_size;
}

/* ++++++++++++++++++++++++++ NAMED MAIL BOXES ++++++++++++++++++++++++++++++ */

#include <rtai_registry.h>


/**
 * @brief Initializes a specifically typed (fifo queued, priority queued
 * or resource queued) mailbox identified by a name.
 *
 * _rt_typed_named_mbx_init initializes a mailbox of type @e qtype
 * and size @e size identified by @e name. Named mailboxed
 * are useful for use among different processes, kernel/user space and
 * in distributed applications.
 *
 * @param mbx_name is the mailbox name; since it can be a clumsy identifier,
 * services are provided to convert 6 characters identifiers to unsigned long
 * (see nam2num()).
 *
 * @param size corresponds to the size of the mailbox.
 *
 * @param qtype corresponds to the queueing policy: FIFO_Q, PRIO_Q or RES_Q.
 *
 * @return On success the pointer to the allocated mbx is returned.
 * On failure, NULL is returned.
 *
 * See also: notes under rt_mbx_init() and rt_typed_mbx_init().
 */
RTAI_SYSCALL_MODE MBX *_rt_typed_named_mbx_init(unsigned long mbx_name, int size, int qtype)
{
	MBX *mbx;

	if ((mbx = rt_get_adr_cnt(mbx_name))) {
		return mbx;
	}
	if ((mbx = rt_malloc(sizeof(MBX)))) {
		rt_typed_mbx_init(mbx, size, qtype);
		if (rt_register(mbx_name, mbx, IS_MBX, 0)) {
			return mbx;
		}
		rt_mbx_delete(mbx);
	}
	rt_free(mbx);
	return (MBX *)0;
}


/**
 *
 * @brief Deletes a named mailbox.
 * 
 * rt_named_mbx_delete removes a mailbox previously created
 * with _rt_typed_named_mbx_init().
 *
 * @param mbx is the pointer to the structure returned by a corresponding
 * call to _rt_typed_named_mbx_init.
 *
 * As it is done by all the named allocation functions delete calls have just
 * the effect of decrementing a usage count till the last is done, as that is
 * the one that really frees the object.
 *
 * @return On success, an integer >=0 is returned.
 * On failure, a negative value is returned as described below:
 * - @b EFAULT: deletion of mailbox failed.
 *
 * See also: notes under rt_mbx_delete().
 */
RTAI_SYSCALL_MODE int rt_named_mbx_delete(MBX *mbx)
{
	int ret;
	if (!(ret = rt_drg_on_adr_cnt(mbx))) {
		if (!rt_mbx_delete(mbx)) {
			rt_free(mbx);
			return 0;
		} else {
			return -EFAULT;
		}
	}
	return ret;
}

/* +++++++++++++++++++++++++ MAIL BOXES ENTRIES +++++++++++++++++++++++++++++ */

struct rt_native_fun_entry rt_mbx_entries[] = {

	{ { 0, rt_typed_mbx_init }, 	      	TYPED_MBX_INIT },
	{ { 0, rt_mbx_delete }, 	      	MBX_DELETE },
	{ { 1, _rt_mbx_send }, 	       		MBX_SEND },
	{ { 1, _rt_mbx_send_wp },      		MBX_SEND_WP },
	{ { 1, _rt_mbx_send_if },      	 	MBX_SEND_IF },
	{ { 1, _rt_mbx_send_until },    	MBX_SEND_UNTIL },
	{ { 1, _rt_mbx_send_timed },    	MBX_SEND_TIMED },
	{ { 1, _rt_mbx_ovrwr_send },    	MBX_OVRWR_SEND },
        { { 1, _rt_mbx_evdrp },         	MBX_EVDRP },
	{ { 1, _rt_mbx_receive },       	MBX_RECEIVE },
	{ { 1, _rt_mbx_receive_wp },    	MBX_RECEIVE_WP },
	{ { 1, _rt_mbx_receive_if },    	MBX_RECEIVE_IF },
	{ { 1, _rt_mbx_receive_until }, 	MBX_RECEIVE_UNTIL },
	{ { 1, _rt_mbx_receive_timed }, 	MBX_RECEIVE_TIMED },
	{ { 0, _rt_typed_named_mbx_init },  	NAMED_MBX_INIT },
	{ { 0, rt_named_mbx_delete },		NAMED_MBX_DELETE },
	{ { 0, 0 },  		      	       	000 }
};

extern int set_rt_fun_entries(struct rt_native_fun_entry *entry);
extern void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

static int recv_blocks(void *mbx) { return ((MBX *)mbx)->avbs <= 0; }
static int send_blocks(void *mbx) { return ((MBX *)mbx)->frbs <= 0; }

int __rtai_mbx_init (void)
{
	rt_poll_ofstfun[RT_POLL_MBX_RECV].topoll = recv_blocks;
	rt_poll_ofstfun[RT_POLL_MBX_SEND].topoll = send_blocks;
	return set_rt_fun_entries(rt_mbx_entries);
}

void __rtai_mbx_exit (void)
{
	rt_poll_ofstfun[RT_POLL_MBX_RECV].topoll = NULL;
	rt_poll_ofstfun[RT_POLL_MBX_SEND].topoll = NULL;
	reset_rt_fun_entries(rt_mbx_entries);
}

/*@}*/

#ifndef CONFIG_RTAI_MBX_BUILTIN
module_init(__rtai_mbx_init);
module_exit(__rtai_mbx_exit);
#endif /* !CONFIG_RTAI_MBX_BUILTIN */

EXPORT_SYMBOL(_rt_mbx_evdrp);
EXPORT_SYMBOL(rt_typed_mbx_init);
EXPORT_SYMBOL(rt_mbx_init);
EXPORT_SYMBOL(rt_mbx_delete);
EXPORT_SYMBOL(_rt_mbx_send);
EXPORT_SYMBOL(_rt_mbx_send_wp);
EXPORT_SYMBOL(_rt_mbx_send_if);
EXPORT_SYMBOL(_rt_mbx_send_until);
EXPORT_SYMBOL(_rt_mbx_send_timed);
EXPORT_SYMBOL(_rt_mbx_receive);
EXPORT_SYMBOL(_rt_mbx_receive_wp);
EXPORT_SYMBOL(_rt_mbx_receive_if);
EXPORT_SYMBOL(_rt_mbx_receive_until);
EXPORT_SYMBOL(_rt_mbx_receive_timed);
EXPORT_SYMBOL(_rt_mbx_ovrwr_send);
EXPORT_SYMBOL(_rt_typed_named_mbx_init);
EXPORT_SYMBOL(rt_named_mbx_delete);
