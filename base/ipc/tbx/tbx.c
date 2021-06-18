/*
 * Copyright (C) 2005 Paolo Mantegazza <mantegazza@aero.polimi.it>
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
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include <rtai_registry.h>
#include <rtai_schedcore.h>
#include <rtai_tbx.h>

MODULE_LICENSE("GPL");

static inline void enq_msg(RT_MSGQ *q, RT_MSGH *msg)
{
	RT_MSGH *prev, *next;

	for (prev = next = q->firstmsg; msg->priority >= next->priority; prev = next, next = next->next);
	if (next == prev) {
		msg->next = next;
		q->firstmsg = msg;
	} else {
		msg->next = prev->next;
		prev->next = msg;
	}
}

RTAI_SYSCALL_MODE int rt_msgq_init(RT_MSGQ *mq, int nmsg, int msg_size)
{
	int i;
        void *p;

	if (!(mq->slots = rt_malloc((msg_size + RT_MSGH_SIZE + sizeof(void *))*nmsg + RT_MSGH_SIZE))) {
		return -ENOMEM;
	}
	mq->nmsg  = nmsg;
	mq->fastsize = msg_size;
	mq->slot = 0;
	p = mq->slots + nmsg;
	for (i = 0; i < nmsg; i++) {
                mq->slots[i] = p;
		((RT_MSGH *)p)->priority = 0;
		p += (msg_size + RT_MSGH_SIZE);
	}
        ((RT_MSGH *)(mq->firstmsg = p))->priority = (0xFFFFFFFF/2);
	rt_typed_sem_init(&mq->receivers, 1, RES_SEM);
	rt_typed_sem_init(&mq->senders, 1, RES_SEM);
	rt_typed_sem_init(&mq->received, 0, CNT_SEM);
	rt_typed_sem_init(&mq->freslots, nmsg, CNT_SEM);
	spin_lock_init(&mq->lock);
	return 0;
}

RTAI_SYSCALL_MODE int rt_msgq_delete(RT_MSGQ *mq)
{
	int err;
	err = rt_sem_delete(&mq->receivers) || rt_sem_delete(&mq->senders) || rt_sem_delete(&mq->received) || rt_sem_delete(&mq->freslots);
	rt_sem_delete(&mq->broadcast);
	rt_free(mq->slots);
	return err ? -EFAULT : 0;
}

RTAI_SYSCALL_MODE RT_MSGQ *_rt_named_msgq_init(unsigned long msgq_name, int nmsg, int msg_size)
{
	RT_MSGQ *msgq;

	if ((msgq = rt_get_adr_cnt(msgq_name))) {
		return msgq;
	}
	if ((msgq = rt_malloc(sizeof(RT_MSGQ)))) {
		rt_msgq_init(msgq, nmsg, msg_size);
		if (rt_register(msgq_name, msgq, IS_MBX, 0)) {
			return msgq;
		}
		rt_msgq_delete(msgq);
	}
	rt_free(msgq);
	return NULL;
}

RTAI_SYSCALL_MODE int rt_named_msgq_delete(RT_MSGQ *msgq)
{
	int ret;
	if (!(ret = rt_drg_on_adr_cnt(msgq))) {
		if (!rt_msgq_delete(msgq)) {
			rt_free(msgq);
			return 0;
		} else {
			return -EFAULT;
		}
	}
	return ret;
}

static int _send(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int space)
{
	unsigned long flags;
	RT_MSG *msg_ptr;
	void *p;

	if (msg_size > mq->fastsize) {
		if (!(p = rt_malloc(msg_size))) {
			rt_sem_signal(&mq->freslots);
			rt_sem_signal(&mq->senders);
			return -ENOMEM;
		}
	} else {
		p = NULL;
	}
	flags = rt_spin_lock_irqsave(&mq->lock);
	msg_ptr = mq->slots[mq->slot++];
	rt_spin_unlock_irqrestore(flags, &mq->lock);
	msg_ptr->hdr.size = msg_size;
	msg_ptr->hdr.priority = msgpri;
	msg_ptr->hdr.malloc = p;
	msg_ptr->hdr.broadcast = 0;
	if (space) {
		memcpy(p ? p : msg_ptr->msg, msg, msg_size);
	} else {
		rt_copy_from_user(p ? p : msg_ptr->msg, msg, msg_size);
	}
	flags = rt_spin_lock_irqsave(&mq->lock);
	enq_msg(mq, &msg_ptr->hdr);
	rt_spin_unlock_irqrestore(flags, &mq->lock);
	rt_sem_signal(&mq->received);
	rt_sem_signal(&mq->senders);
	return 0;
}

#define TBX_RET(msg_size, retval) \
        (CONFIG_RTAI_USE_NEWERR ? retval : msg_size)

RTAI_SYSCALL_MODE int _rt_msg_send(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int space)
{
	int retval;

	if ((retval = rt_sem_wait(&mq->senders)) >= RTE_LOWERR) {
		return TBX_RET(msg_size, retval);
        }
	if (rt_sem_wait(&mq->freslots) >= RTE_LOWERR) {
		rt_sem_signal(&mq->senders);
		return TBX_RET(msg_size, retval);
	}
	return _send(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_send_if(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int space)
{
	if (rt_sem_wait_if(&mq->senders) <= 0) {
                return msg_size;
        }
	if (rt_sem_wait_if(&mq->freslots) <= 0) {
		rt_sem_signal(&mq->senders);
		return msg_size;
	}
	return _send(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_send_until(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, RTIME until, int space)
{
	int retval;
	if ((retval = rt_sem_wait_until(&mq->senders, until)) >= RTE_LOWERR) {
		return TBX_RET(msg_size, retval);
        }
	if ((retval = rt_sem_wait_until(&mq->freslots, until)) >= RTE_LOWERR) {
		rt_sem_signal(&mq->senders);
		return TBX_RET(msg_size, retval);
	}
	return _send(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_send_timed(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, RTIME delay, int space)
{
	return _rt_msg_send_until(mq, msg, msg_size, msgpri, get_time() + delay, space);
}

static int _receive(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, int space)
{
	int size;
	RT_MSG *msg_ptr;
	void *p;

	size = min((msg_ptr = mq->firstmsg)->hdr.size, msg_size);
	if (space) {
		memcpy(msg, (p = msg_ptr->hdr.malloc) ? p : msg_ptr->msg, size);
		if (msgpri) {
			*msgpri = msg_ptr->hdr.priority;
		}
	} else {
		rt_copy_to_user(msg, (p = msg_ptr->hdr.malloc) ? p : msg_ptr->msg, size);
		if (msgpri) {
			rt_put_user(msg_ptr->hdr.priority, msgpri);
		}
	}

	if (msg_ptr->hdr.broadcast) {
		if (!--msg_ptr->hdr.broadcast) {
			rt_sem_wait_barrier(&mq->broadcast);
			goto relslot;
		} else {
			rt_sem_signal(&mq->received);
			rt_sem_signal(&mq->receivers);
			rt_sem_wait_barrier(&mq->broadcast);
		}
	} else {
		unsigned long flags;
relslot:	flags = rt_spin_lock_irqsave(&mq->lock);
		mq->firstmsg = msg_ptr->hdr.next;
		mq->slots[--mq->slot] = msg_ptr;
		rt_spin_unlock_irqrestore(flags, &mq->lock);
		rt_sem_signal(&mq->freslots);
		rt_sem_signal(&mq->receivers);
		if (p) {
			rt_free(p);
		}
	}
	return msg_size - size;
}

RTAI_SYSCALL_MODE int _rt_msg_receive(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, int space)
{
	int retval;
	if ((retval = rt_sem_wait(&mq->receivers)) >= RTE_LOWERR) {
		return TBX_RET(msg_size, retval);
	}
	if ((retval = rt_sem_wait(&mq->received)) >= RTE_LOWERR) { ;
		rt_sem_signal(&mq->receivers);
		return TBX_RET(msg_size, retval);
	}
	return _receive(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_if(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, int space)
{
	if (rt_sem_wait_if(&mq->receivers) <= 0) {
		return msg_size;
	}
	if (rt_sem_wait_if(&mq->received) <= 0) { ;
		rt_sem_signal(&mq->receivers);
		return msg_size;
	}
	return _receive(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_until(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, RTIME until, int space)
{
	int retval;
	if ((retval = rt_sem_wait_until(&mq->receivers, until)) >= RTE_LOWERR) {
		return TBX_RET(msg_size, retval);
	}
	if ((retval = rt_sem_wait_until(&mq->received, until)) >= RTE_LOWERR) {
		rt_sem_signal(&mq->receivers);
		return TBX_RET(msg_size, retval);
	}
	return _receive(mq, msg, msg_size, msgpri, space);
}

RTAI_SYSCALL_MODE int _rt_msg_receive_timed(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, RTIME delay, int space)
{
	return _rt_msg_receive_until(mq, msg, msg_size, msgpri, get_time() + delay, space);
}

RTAI_SYSCALL_MODE int _rt_msg_evdrp(RT_MSGQ *mq, void *msg, int msg_size, int *msgpri, int space)
{
	int size;
	RT_MSG *msg_ptr;
	void *p;

	size = min((msg_ptr = mq->firstmsg)->hdr.size, msg_size);
	if (space) {
		memcpy(msg, (p = msg_ptr->hdr.malloc) ? p : msg_ptr->msg, size);
		if (msgpri) {
			*msgpri = msg_ptr->hdr.priority;
		}
	} else {
		rt_copy_to_user(msg, (p = msg_ptr->hdr.malloc) ? p : msg_ptr->msg, size);
		if (msgpri) {
			rt_put_user(msg_ptr->hdr.priority, msgpri);
		}
	}
	return 0;
}

static int _broadcast(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int broadcast, int space)
{
	unsigned long flags;
	RT_MSG *msg_ptr;
	void *p;

	if (msg_size > mq->fastsize) {
		if (!(p = rt_malloc(msg_size))) {
			rt_sem_signal(&mq->freslots);
			rt_sem_signal(&mq->senders);
			return -ENOMEM;
		}
	} else {
		p = NULL;
	}
	flags = rt_spin_lock_irqsave(&mq->lock);
	msg_ptr = mq->slots[mq->slot++];
	rt_spin_unlock_irqrestore(flags, &mq->lock);
	msg_ptr->hdr.size = msg_size;
	msg_ptr->hdr.priority = msgpri;
	msg_ptr->hdr.malloc = p;
	if (space) {
		memcpy(p ? p : msg_ptr->msg, msg, msg_size);
	} else {
		rt_copy_from_user(p ? p : msg_ptr->msg, msg, msg_size);
	}
	rt_typed_sem_init(&mq->broadcast, broadcast + 1, CNT_SEM | PRIO_Q);
	msg_ptr->hdr.broadcast = broadcast; 
	flags = rt_spin_lock_irqsave(&mq->lock);
	enq_msg(mq, &msg_ptr->hdr);
	rt_spin_unlock_irqrestore(flags, &mq->lock);
	rt_sem_signal(&mq->received);
	rt_sem_wait_barrier(&mq->broadcast);
	rt_sem_signal(&mq->senders);
	return broadcast;
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int space)
{
	int retval;

	if ((retval = rt_sem_wait(&mq->senders)) >= RTE_LOWERR) {
		return TBX_RET(0, retval);
        }
	if (mq->received.count >= 0) {
		rt_sem_signal(&mq->senders);
		return 0;
	}
	if ((retval = rt_sem_wait(&mq->freslots)) >= RTE_LOWERR) {
		rt_sem_signal(&mq->senders);
		return TBX_RET(0, retval);
	}
	return _broadcast(mq, msg, msg_size, msgpri, -(mq->received.count + mq->receivers.count), space);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_if(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, int space)
{
	if (rt_sem_wait_if(&mq->senders) <= 0) {
                return 0;
        }
	if (mq->received.count >= 0) {
		rt_sem_signal(&mq->senders);
		return 0;
	}
	if (rt_sem_wait_if(&mq->freslots) <= 0) {
		rt_sem_signal(&mq->senders);
                return 0;
	}
	return _broadcast(mq, msg, msg_size, msgpri, -(mq->received.count + mq->receivers.count), space);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_until(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, RTIME until, int space)
{
	int retval;

	if ((retval = rt_sem_wait_until(&mq->senders, until)) >= RTE_LOWERR) {
		return TBX_RET(0, retval);
        }
	if (mq->received.count >= 0) {
		rt_sem_signal(&mq->senders);
		return 0;
	}
	if ((retval = rt_sem_wait_until(&mq->freslots, until)) >= RTE_LOWERR) {
		rt_sem_signal(&mq->senders);
		return TBX_RET(0, retval);
	}
	return _broadcast(mq, msg, msg_size, msgpri, -(mq->received.count + mq->receivers.count), space);
}

RTAI_SYSCALL_MODE int _rt_msg_broadcast_timed(RT_MSGQ *mq, void *msg, int msg_size, int msgpri, RTIME delay, int space)
{
	return _rt_msg_broadcast_until(mq, msg, msg_size, msgpri, get_time() + delay, space);
}

struct rt_native_fun_entry rt_msg_queue_entries[] = {
	{ { 0, rt_msgq_init },  	       	MSGQ_INIT },
        { { 1, rt_msgq_delete },		MSGQ_DELETE },
	{ { 0, _rt_named_msgq_init },  	       	NAMED_MSGQ_INIT },
        { { 1, rt_named_msgq_delete },		NAMED_MSGQ_DELETE },
        { { 1, _rt_msg_send },    	     	MSG_SEND },
        { { 1, _rt_msg_send_if },  	     	MSG_SEND_IF },
        { { 1, _rt_msg_send_until }, 		MSG_SEND_UNTIL },
        { { 1, _rt_msg_send_timed }, 		MSG_SEND_TIMED },
        { { 1, _rt_msg_receive },          	MSG_RECEIVE },
        { { 1, _rt_msg_receive_if },       	MSG_RECEIVE_IF },
        { { 1, _rt_msg_receive_until },		MSG_RECEIVE_UNTIL },
        { { 1, _rt_msg_receive_timed },		MSG_RECEIVE_TIMED },
        { { 1, _rt_msg_broadcast },          	MSG_BROADCAST },
        { { 1, _rt_msg_broadcast_if },       	MSG_BROADCAST_IF },
        { { 1, _rt_msg_broadcast_until },	MSG_BROADCAST_UNTIL },
        { { 1, _rt_msg_broadcast_timed }, 	MSG_BROADCAST_TIMED },
        { { 1, _rt_msg_evdrp }, 		MSG_EVDRP },
	{ { 0, 0 },  		      		000 }
};

extern int set_rt_fun_entries(struct rt_native_fun_entry *entry);
extern void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

int __rtai_msg_queue_init(void) 
{
	printk(KERN_INFO "RTAI[rtai_msgq]: loaded.\n");
	return set_rt_fun_entries(rt_msg_queue_entries);
}

void __rtai_msg_queue_exit(void) 
{
	reset_rt_fun_entries(rt_msg_queue_entries);
	printk(KERN_INFO "RTAI[rtai_msgq]: unloaded.\n");
}

#ifndef CONFIG_RTAI_TBX_BUILTIN
module_init(__rtai_msg_queue_init);
module_exit(__rtai_msg_queue_exit);
#endif /* !CONFIG_RTAI_TBX_BUILTIN */

#ifdef CONFIG_KBUILD
EXPORT_SYMBOL(rt_msgq_init);
EXPORT_SYMBOL(rt_msgq_delete);
EXPORT_SYMBOL(_rt_named_msgq_init);
EXPORT_SYMBOL(rt_named_msgq_delete);
EXPORT_SYMBOL(_rt_msg_send);
EXPORT_SYMBOL(_rt_msg_send_if);
EXPORT_SYMBOL(_rt_msg_send_until);
EXPORT_SYMBOL(_rt_msg_send_timed);
EXPORT_SYMBOL(_rt_msg_receive);
EXPORT_SYMBOL(_rt_msg_receive_if);
EXPORT_SYMBOL(_rt_msg_receive_until);
EXPORT_SYMBOL(_rt_msg_receive_timed);
EXPORT_SYMBOL(_rt_msg_broadcast);
EXPORT_SYMBOL(_rt_msg_broadcast_if);
EXPORT_SYMBOL(_rt_msg_broadcast_until);
EXPORT_SYMBOL(_rt_msg_broadcast_timed);
EXPORT_SYMBOL(_rt_msg_evdrp);
#endif /* CONFIG_KBUILD */
