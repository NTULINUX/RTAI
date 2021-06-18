/*
 * pqueues interface for Real Time Linux.
 *
 * Copyright (©) 1999 Zentropic Computing, All rights reserved
 *  
 * Authors:             Trevor Woolven (trevw@zentropix.com)
 *
 * Original date:       Thu 15 Jul 1999
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
 * Timed services extension and user space integration for RTAI by
 * Paolo Mantegazza <mantegazza@aero.polimi.it>.
 * 2005, cleaned and revised Paolo Mantegazza <mantegazza@aero.polimi.it>.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
extern struct proc_dir_entry *rtai_proc_root;
#endif
#include <rtai_schedcore.h>
#include <rtai_proc_fs.h>
#include <rtai_signal.h>

MODULE_LICENSE("GPL");

#define mq_cond_t                   SEM
#define mq_mutex_t                  SEM
#define mq_mutex_init(mutex, attr)  rt_typed_sem_init(mutex, 1, BIN_SEM | PRIO_Q)
#define mq_mutex_unlock             rt_sem_signal
#define mq_mutex_lock(mutex) \
	do { \
		if (abs(rt_sem_wait(mutex)) >= RTE_LOWERR) { \
			return -EBADF; \
		} \
	} while (0)		
#define mq_mutex_timedlock(mutex, abstime) \
	do { \
		RTIME t = timespec2count(abstime); \
		int ret; \
		if ((ret = abs(rt_sem_wait_until(mutex, t))) >= RTE_LOWERR) { \
			return ret == RTE_TIMOUT ? -ETIMEDOUT : -EBADF; \
		} \
	} while (0)
#define mq_mutex_trylock            rt_sem_wait_if
#define mq_mutex_destroy            rt_sem_delete
#define mq_cond_init(cond, attr)    rt_sem_init(cond, 0)
#define mq_cond_wait(cond, mutex) \
	do { \
		rt_sem_signal(mutex); \
		if (abs(rt_sem_wait(cond)) >= RTE_LOWERR) { \
			return -EBADF; \
		} \
 		if (abs(rt_sem_wait(mutex)) >= RTE_LOWERR) { \
			rt_sem_signal(cond); \
			return -EBADF; \
		} \
	} while (0)
#define mq_cond_timedwait(cond, mutex, abstime) \
	do { \
		RTIME t = timespec2count(abstime); \
		int ret; \
		rt_sem_signal(mutex); \
		if ((ret = abs(rt_sem_wait_until(cond, t))) >= RTE_LOWERR) { \
			return ret == RTE_TIMOUT ? -ETIMEDOUT : -EBADF; \
		} \
		if ((ret = abs(rt_sem_wait_until(mutex, t))) >= RTE_LOWERR) { \
			rt_sem_signal(cond); \
			return ret == RTE_TIMOUT ? -ETIMEDOUT : -EBADF; \
		} \
	} while (0)
#define mq_cond_signal              rt_sem_signal
#define mq_cond_destroy             rt_sem_delete

#ifndef OK
#define OK  0
#endif
#ifndef ERROR
#define ERROR  -1
#endif

///////////////////////////////////////////////////////////////////////////////
//      LOCAL DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

#define MAX_RT_TASKS 128

///////////////////////////////////////////////////////////////////////////////
//      PACKAGE GLOBAL DATA
///////////////////////////////////////////////////////////////////////////////

static int num_pqueues = 0;
static struct _pqueue_descr_struct rt_pqueue_descr[MAX_PQUEUES] = {{0}};
static struct _pqueue_access_struct task_pqueue_access[MAX_RT_TASKS] = {{0}};
static MQ_ATTR default_queue_attrs = { MAX_MSGS, MAX_MSGSIZE, MQ_NONBLOCK, 0 };

static mq_mutex_t pqueue_mutex;

///////////////////////////////////////////////////////////////////////////////
//      LOCAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

static int name_to_id(char *name)
{
	int ind;
	for (ind = 0; ind < MAX_PQUEUES; ind++) {
		if (rt_pqueue_descr[ind].q_name[0] && !strcmp(rt_pqueue_descr[ind].q_name, name)) {
			return ind;
		}
	} 
	return ERROR;
}


static inline mq_bool_t is_empty(struct queue_control *q)
{
	return !q->attrs.mq_curmsgs;
}


static inline mq_bool_t is_full(struct queue_control *q)
{
	return q->attrs.mq_curmsgs == q->attrs.mq_maxmsg;
}


static inline MSG_HDR* getnode(Q_CTRL *queue)
{
	return queue->attrs.mq_curmsgs < queue->attrs.mq_maxmsg ? queue->nodes[queue->attrs.mq_curmsgs++] : NULL;
}

static inline int freenode(void *node, Q_CTRL *queue)
{
	if (queue->attrs.mq_curmsgs > 0) {
                queue->nodes[--queue->attrs.mq_curmsgs] = node;
                return 0;
        }
        return -EINVAL;
}


static void insert_message(Q_CTRL *q, MSG_HDR *this_msg)
{
//This function finds the appropriate point in a priority queue to
//insert the supplied message. It preserves FIFO order within each
//priority levela and can therefore be used for FIFO queuing policies
//simply by making the priority equal to the supplied message priority

	MSG_HDR *prev, *insertpt;

//Do a quick check in case the message at the back of the queue has
//a higher priority than this one, in which case this one can just
//go at the back of the queue. 
//Remember that Posix priorities increase from 0 to (at least) 32

	if (((MSG_HDR *)q->tail)->priority >= this_msg->priority) {
		((MSG_HDR*)q->tail)->next = this_msg;
		q->tail = this_msg;
	} else {
		prev = insertpt = q->head;
//POSIX queues preserve FIFO ordering of messages within
//a particular priority level
		while (insertpt->priority >= this_msg->priority) {
		        prev = insertpt;
		        insertpt = insertpt->next; 
		}
//We've now found a message (or messages) of equal or lower
//priority than the one we're trying to put onto the queue
 		if (insertpt == q->head) {
			this_msg->next = q->head;
			q->head = this_msg;
		} else {
			this_msg->next = prev->next;
			prev->next = this_msg;
		}   
	}
}

#undef mqueues
#define mqueues system_data_ptr

static mq_bool_t is_blocking(MSG_QUEUE *q)
{
	int q_ind;
	struct _pqueue_access_data *aces;

	aces = ((QUEUE_CTRL)_rt_whoami()->mqueues)->q_access;
	for (q_ind = 0; q_ind < MQ_OPEN_MAX; q_ind++) {
		if (aces[q_ind].q_id == q->q_id) {
			return !(aces[q_ind].oflags & O_NONBLOCK);
		}
	}
	return FALSE;
}


static mq_bool_t can_access(MSG_QUEUE *q, Q_ACCESS access)
{
	RT_TASK *caller = _rt_whoami();

	if (q->owner == caller ? (((access == FOR_READ) && (q->permissions & S_IRUSR)) || ((access == FOR_WRITE) && (q->permissions & S_IWUSR))) : (((access == FOR_READ) && (q->permissions & S_IRGRP)) || ((access == FOR_WRITE) && (q->permissions & S_IWGRP)))) {
		int q_ind;
		struct _pqueue_access_data *aces;
		struct _pqueue_access_struct *task_queue_data_ptr;
		int q_access_flags = 0;

		task_queue_data_ptr = (QUEUE_CTRL)caller->mqueues;
		if (task_queue_data_ptr == NULL) {
			return FALSE;
		}
		aces = task_queue_data_ptr->q_access;
		for (q_ind = 0; q_ind < MQ_OPEN_MAX; q_ind++) {
			if (aces[q_ind].q_id == q->q_id) {
				q_access_flags = aces[q_ind].oflags;
				goto set_mode;
			}
		}
		return FALSE;
set_mode:	if (access == FOR_WRITE) {
			if ((q_access_flags & O_WRONLY) || (q_access_flags & O_RDWR)) {
				return TRUE;
			}
		} else {
			return TRUE;
		}
	}
	return FALSE;
}


static inline void initialise_queue(Q_CTRL *q)
{
	int msg_size, msg_ind;
	void *msg_ptr;

	msg_size = q->attrs.mq_msgsize + sizeof(MSG_HDR);
	msg_ptr = q->base;
	q->nodes = msg_ptr + msg_size*q->attrs.mq_maxmsg; 
	for (msg_ind = 0; msg_ind < q->attrs.mq_maxmsg; msg_ind++) {
		q->nodes[msg_ind] = msg_ptr;
		((MSG_HDR *)msg_ptr)->size = 0;
		((MSG_HDR *)msg_ptr)->priority = MQ_MIN_MSG_PRIORITY;
		((MSG_HDR *)msg_ptr)->next = NULL;
		msg_ptr += msg_size;
	}
}


static void delete_queue(int q_index)
{
	rt_free(rt_pqueue_descr[q_index].data.base);

	rt_pqueue_descr[q_index].owner = NULL;
	rt_pqueue_descr[q_index].open_count = 0;
	strcpy(rt_pqueue_descr[q_index].q_name, "\0");
	rt_pqueue_descr[q_index].q_id = INVALID_PQUEUE;
	rt_pqueue_descr[q_index].data.base = NULL;
	rt_pqueue_descr[q_index].data.head = NULL;
	rt_pqueue_descr[q_index].data.tail = NULL;
	rt_pqueue_descr[q_index].data.attrs = (MQ_ATTR){ 0, 0, 0, 0 };
	rt_pqueue_descr[q_index].permissions = 0;

	mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	mq_mutex_destroy(&rt_pqueue_descr[q_index].mutex);
	mq_cond_destroy(&rt_pqueue_descr[q_index].emp_cond);
	mq_cond_destroy(&rt_pqueue_descr[q_index].full_cond);

	if (num_pqueues > 0) {	
		num_pqueues--;	
	}
}

static void signal_suprt_fun_mq(void *fun_arg)
{			
	struct suprt_fun_arg { RT_TASK *sigtask; RT_TASK *task; mqd_t mq; } arg = *(struct suprt_fun_arg *)fun_arg;
	
	arg.sigtask = RT_CURRENT;
	if (!rt_request_signal_(arg.sigtask, arg.task, (arg.mq + MAXSIGNALS))) {
		while (rt_wait_signal(arg.sigtask, arg.task)) {
			rt_pqueue_descr[arg.mq - 1].notify.data._sigev_un._sigev_thread._function((sigval_t)rt_pqueue_descr[arg.mq - 1].notify.data.sigev_value.sival_ptr);
		}
	} else {
		rt_task_resume(arg.task);
	}
}

int rt_request_signal_mq(mqd_t mq)
{
		RT_TASK *sigtask;
		struct suprt_fun_arg { RT_TASK *sigtask; RT_TASK *task; mqd_t mq; } arg = { NULL, rt_whoami(), mq };
		if ((sigtask = rt_malloc(sizeof(RT_TASK)))) {
			rt_task_init_cpuid(sigtask, (void *)signal_suprt_fun_mq, (long)&arg, SIGNAL_TASK_STACK_SIZE, arg.task->priority, 0, 0, RT_CURRENT->runnable_on_cpus);
			rt_task_resume(sigtask);
			rt_task_suspend(arg.task);
			return arg.task->retval;
		}
	return -EINVAL;
}

///////////////////////////////////////////////////////////////////////////////
//      POSIX MESSAGE QUEUES API
///////////////////////////////////////////////////////////////////////////////

RTAI_SYSCALL_MODE mqd_t _mq_open(char *mq_name, int oflags, mode_t permissions, struct mq_attr *mq_attr, long space)
{
	int q_index, t_index, q_ind;
	int spare_count = 0, first_spare = 0;
	mq_bool_t q_found = FALSE;
	RT_TASK *this_task = _rt_whoami();
	struct _pqueue_access_struct *task_data_ptr;

	task_data_ptr = (QUEUE_CTRL)this_task->mqueues;

	mq_mutex_lock(&pqueue_mutex);
	if ((q_index = name_to_id(mq_name)) >= 0) {
//========================
// OPEN AN EXISTING QUEUE
//========================
		if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
			mq_mutex_unlock(&pqueue_mutex);
			return -EEXIST;
		}
		if (task_data_ptr == NULL) {
			for (t_index = 0; t_index < MAX_RT_TASKS; t_index++) {
				if (task_pqueue_access[t_index].this_task == NULL) {
					task_data_ptr = &(task_pqueue_access[t_index]);
					task_data_ptr->this_task = this_task;
					this_task->mqueues = task_data_ptr;
					break;
				}
			}
			if (t_index == MAX_RT_TASKS) {
				mq_mutex_unlock(&pqueue_mutex);
				return -ENOMEM;
			}
		}
	//Now record that this task has opened this queue and
	//the access permissions required
	//Check first to see if this task has already opened this queue
	//and while doing so, record the number of spare 'slots' for this
	//task to have further opened queues
	mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
		for (q_ind = 0; q_ind < MQ_OPEN_MAX; q_ind++) {
			if (task_data_ptr->q_access[q_ind].q_id == rt_pqueue_descr[q_index].q_id) { 
				q_found = TRUE;
				break;
			} else if(task_data_ptr->q_access[q_ind].q_id == INVALID_PQUEUE) {
				if (spare_count == 0) {
					first_spare = q_ind;
				}			
				spare_count++;
			}
		}	
	//If the task has not already opened this queue and there are no
	//more available slots, can't do anymore...
		if (!q_found && spare_count == 0) {
			mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
			mq_mutex_unlock(&pqueue_mutex);
			return -EINVAL;
		}
	//Either the queue has already been opened and so we can re-use
	//it's slot, or a new one is being opened in an unused slot
		if (!q_found) {
	    //Open a new one, using the first free slot
			task_data_ptr->n_open_pqueues++;
			q_ind = first_spare;
		}
		task_data_ptr->q_access[q_ind].q_id = rt_pqueue_descr[q_index].q_id;
		task_data_ptr->q_access[q_ind].oflags = oflags;
		mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	} else if (oflags & O_CREAT) {
//================
// CREATE A QUEUE 
//================
		if(num_pqueues >= MAX_PQUEUES) {
			mq_mutex_unlock(&pqueue_mutex);
			return -ENOMEM;
		}
	//Check the size of the name
		if( strlen(mq_name) >= MQ_NAME_MAX) {
			mq_mutex_unlock(&pqueue_mutex);
			return -ENAMETOOLONG;
		}
	//Allocate a task pqueue access structure to this task, if necessary.
	//Otherwise, check that this task has not already opened too many 
	//queues
	//
		if (task_data_ptr == NULL) {
	    //Find a spare task pqueue access slot for this task
			for (t_index = 0; t_index < MAX_RT_TASKS; t_index++) {
				if (task_pqueue_access[t_index].this_task == NULL) {
					task_data_ptr = &task_pqueue_access[t_index];
					task_data_ptr->this_task = this_task;
					this_task->mqueues = task_data_ptr;
					break;
				}
			}
			if (t_index == MAX_RT_TASKS) {
				mq_mutex_unlock(&pqueue_mutex);
				return -ENOMEM;
			}
		} else if (task_data_ptr->n_open_pqueues >= MQ_OPEN_MAX) {
			mq_mutex_unlock(&pqueue_mutex);
			return -EINVAL;
 		}
	//Look for default queue attributes
		if (mq_attr == NULL) {
			mq_attr = &default_queue_attrs;
		}
	//Find a spare descriptor for this queue
		for (q_index = 0; q_index < MAX_PQUEUES; q_index++) {
			if (rt_pqueue_descr[q_index].q_id == INVALID_PQUEUE) {
				int msg_size, queue_size;
				void *mem_ptr;
		//Get memory for the data queue
				msg_size = mq_attr->mq_msgsize + sizeof(MSG_HDR);
				queue_size = (msg_size + sizeof(void *))*mq_attr->mq_maxmsg;
				mem_ptr = rt_malloc(queue_size);
				if(mem_ptr == NULL) {
					mq_mutex_unlock(&pqueue_mutex);
					return -ENOMEM;
				}
				rt_pqueue_descr[q_index].data.base = mem_ptr; 
		//Initialise the Message Queue descriptor
    				rt_pqueue_descr[q_index].owner = this_task;
		    		rt_pqueue_descr[q_index].open_count = 0;
				strcpy(rt_pqueue_descr[q_index].q_name, mq_name);
				rt_pqueue_descr[q_index].q_id = q_index + 1;
				rt_pqueue_descr[q_index].marked_for_deletion = FALSE;
				rt_pqueue_descr[q_index].data.head = 
				rt_pqueue_descr[q_index].data.tail = rt_pqueue_descr[q_index].data.base;
				rt_pqueue_descr[q_index].data.attrs = *(mq_attr);
				rt_pqueue_descr[q_index].data.attrs.mq_curmsgs = 0;
				rt_pqueue_descr[q_index].permissions = permissions;
		//Initialise conditional variables used for blocking
				mq_cond_init(&rt_pqueue_descr[q_index].emp_cond, NULL);
				mq_cond_init(&rt_pqueue_descr[q_index].full_cond, NULL);
				mq_mutex_init(&rt_pqueue_descr[q_index].mutex, NULL);

		//Clear the queue contents
				initialise_queue(&rt_pqueue_descr[q_index].data);
		//Initialise the Task Queue access descriptor
				q_ind = task_data_ptr->n_open_pqueues++;
				task_data_ptr->q_access[q_ind].q_id = q_index + 1;
				task_data_ptr->q_access[q_ind].oflags = oflags;
				break;
			}
        	}
		if(q_index >= MAX_PQUEUES) {
			mq_mutex_unlock(&pqueue_mutex);
			return -EMFILE;
		}
		num_pqueues++;
	} else {
//==============================
// OPENING A NON-EXISTANT QUEUE
//==============================
		mq_mutex_unlock(&pqueue_mutex);
		return -ENOENT;
	}
	
	// Return the message queue's id and mark it as open
	rt_pqueue_descr[q_index].open_count++;
	mq_mutex_unlock(&pqueue_mutex);
	
	// Prepare notify task 
	if ((oflags & O_NOTIFY_NP) && space == 0)	{
		rt_request_signal_mq(rt_pqueue_descr[q_index].q_id);
	}
	
	return (mqd_t)rt_pqueue_descr[q_index].q_id;
}
EXPORT_SYMBOL(_mq_open);

RTAI_SYSCALL_MODE size_t _mq_receive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, int space)
{
	int q_index = mq - 1, size;
	MQMSG *msg_ptr;
	MSG_QUEUE *q;

	if (q_index < 0 || q_index >= MAX_PQUEUES) { 
		return -EBADF;
	}
	q = &rt_pqueue_descr[q_index];
	if (buflen < q->data.attrs.mq_msgsize) {
		return -EMSGSIZE;
	}
	if (can_access(q, FOR_READ) == FALSE) {
		return -EINVAL;
	}
	if (is_blocking(q)) {
		mq_mutex_lock(&q->mutex);
	} else if (mq_mutex_trylock(&q->mutex) <= 0) {
		return -EAGAIN;
	}
    	while (is_empty(&q->data)) {
		if (is_blocking(q)) {
			mq_cond_wait(&q->emp_cond, &q->mutex);
		} else {
			mq_mutex_unlock(&q->mutex);
			return -EAGAIN;
		}
	}
    	msg_ptr = q->data.head;
        if (msg_ptr->hdr.size <= buflen) {
		size = msg_ptr->hdr.size;
		if (space) {
			memcpy(msg_buffer, &msg_ptr->data, size);
			if (msgprio) {
				*msgprio = msg_ptr->hdr.priority;
			}
		} else {
			rt_copy_to_user(msg_buffer, &msg_ptr->data, size);
			if (msgprio) {
				rt_put_user(msg_ptr->hdr.priority, msgprio);
			}
		}
	} else {
		size = ERROR;
	}
	q->data.head = msg_ptr->hdr.next;
	msg_ptr->hdr.size = 0;
    	msg_ptr->hdr.next = NULL;
    	freenode(msg_ptr, &q->data);
	if(q->data.head == NULL) {
		q->data.head = q->data.tail = q->data.nodes[0];
	}
	mq_cond_signal(&q->full_cond);
	mq_mutex_unlock(&q->mutex);
	return size;
}
EXPORT_SYMBOL(_mq_receive);

RTAI_SYSCALL_MODE size_t _mq_timedreceive(mqd_t mq, char *msg_buffer, size_t buflen, unsigned int *msgprio, const struct timespec *abstime, int space)
{
	int q_index = mq - 1, size;
	MQMSG *msg_ptr;
	MSG_QUEUE *q;
	struct timespec time;
	
	if (q_index < 0 || q_index >= MAX_PQUEUES) { 
		return -EBADF;
	}
	q = &rt_pqueue_descr[q_index];
	if (buflen < q->data.attrs.mq_msgsize) {
		return -EMSGSIZE;
	}
	if (can_access(q, FOR_READ) == FALSE) {
		return -EINVAL;
	}
	if (!space) {
		rt_copy_from_user(&time, abstime, sizeof(struct timespec));
		abstime = &time;
	}
	if (is_blocking(q)) {
		mq_mutex_timedlock(&q->mutex, abstime);
	} else if (mq_mutex_trylock(&q->mutex) <= 0) {
		return -EAGAIN;
	}
	while (is_empty(&q->data)) {
		if (is_blocking(q)) {
			mq_cond_timedwait(&q->emp_cond, &q->mutex, abstime);
		} else {
			return -EAGAIN;
		}
	}
	msg_ptr = q->data.head;
	if (msg_ptr->hdr.size <= buflen) {
		size = msg_ptr->hdr.size;
		if (space) {
			memcpy(msg_buffer, &msg_ptr->data, size);
			if (msgprio) {
				*msgprio = msg_ptr->hdr.priority;
			}
		} else {
			rt_copy_to_user(msg_buffer, &msg_ptr->data, size);
			if (msgprio) {
				rt_put_user(msg_ptr->hdr.priority, msgprio);
			}
		}
	} else {
		size = ERROR;
	}
	q->data.head = msg_ptr->hdr.next;
	msg_ptr->hdr.size = 0;
	msg_ptr->hdr.next = NULL;
    	freenode(msg_ptr, &q->data);
	if(q->data.head == NULL) {
		q->data.head = q->data.tail = q->data.nodes[0];
	}
	mq_cond_signal(&q->full_cond);
	mq_mutex_unlock(&q->mutex);
	return size;
}
EXPORT_SYMBOL(_mq_timedreceive);

RTAI_SYSCALL_MODE int _mq_send(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, int space)
{
	int q_index = mq - 1;
	MSG_QUEUE *q;
	MSG_HDR *this_msg;
	mq_bool_t q_was_empty;

	if (q_index < 0 || q_index >= MAX_PQUEUES) { 
		return -EBADF;
	}
	q = &rt_pqueue_descr[q_index];
	if( can_access(q, FOR_WRITE) == FALSE) {
		return -EINVAL;
	}
	if(msgprio > MQ_PRIO_MAX) {
		return -EINVAL;
	}
	if (is_blocking(q)) {
		mq_mutex_lock(&q->mutex);
	} else if (mq_mutex_trylock(&q->mutex) <= 0) {
		return -EAGAIN;
	}
	q_was_empty = is_empty(&q->data);
    while (is_full(&q->data)) {
		if (is_blocking(q)) {
			mq_cond_wait(&q->full_cond, &q->mutex);
		} else {
			mq_mutex_unlock(&q->mutex);
			return -EAGAIN;
		}
       	}
	if( (this_msg = getnode(&q->data)) == NULL) {
		mq_mutex_unlock(&q->mutex);
		return -ENOBUFS;
	}
	if (msglen > q->data.attrs.mq_msgsize) {
		mq_mutex_unlock(&q->mutex);
		return -EMSGSIZE;
	}
	this_msg->size = msglen;
	this_msg->priority = msgprio;
	if (space) {
		memcpy(&((MQMSG *)this_msg)->data, msg, msglen);
	} else {
		rt_copy_from_user(&((MQMSG *)this_msg)->data, msg, msglen);
	}
	insert_message(&q->data, this_msg);
	mq_cond_signal(&q->emp_cond);
	if(q_was_empty && rt_pqueue_descr[q_index].notify.task != NULL) {
		rt_trigger_signal((MAXSIGNALS + mq), rt_pqueue_descr[q_index].notify.task);
		rt_pqueue_descr[q_index].notify.task = NULL;
	}
	mq_mutex_unlock(&q->mutex);
	return msglen;
}
EXPORT_SYMBOL(_mq_send);

RTAI_SYSCALL_MODE int _mq_timedsend(mqd_t mq, const char *msg, size_t msglen, unsigned int msgprio, const struct timespec *abstime, int space)
{
	int q_index = mq - 1;
	MSG_QUEUE *q;
	MSG_HDR *this_msg;
	mq_bool_t q_was_empty;
	struct timespec time;

	if (q_index < 0 || q_index >= MAX_PQUEUES) { 
		return -EBADF;
	}
	q = &rt_pqueue_descr[q_index];
	if (can_access(q, FOR_WRITE) == FALSE) {
		return -EINVAL;
	}
	if (msgprio > MQ_PRIO_MAX) {
		return -EINVAL;
	}			
	if (!space) {
		rt_copy_from_user(&time, abstime, sizeof(struct timespec));
		abstime = &time;
	}
	if (is_blocking(q)) {
		mq_mutex_timedlock(&q->mutex, abstime);
	} else if (mq_mutex_trylock(&q->mutex) <= 0) {
		return -EAGAIN;
	}
	q_was_empty = is_empty(&q->data);
	while (is_full(&q->data)) {
		if (is_blocking(q)) {
			mq_cond_timedwait(&q->full_cond, &q->mutex, abstime);
		} else {
			mq_mutex_unlock(&q->mutex);
			return -EAGAIN;
		}
	}
	if ((this_msg = getnode(&q->data)) == NULL) {
		mq_mutex_unlock(&q->mutex);
		return -ENOBUFS;
	}
	if (msglen > q->data.attrs.mq_msgsize) {
		mq_mutex_unlock(&q->mutex);
		return -EMSGSIZE;
	}
	this_msg->size = msglen;
	this_msg->priority = msgprio;
	if (space) {
		memcpy(&((MQMSG *)this_msg)->data, msg, msglen);
	} else {
		rt_copy_from_user(&((MQMSG *)this_msg)->data, msg, msglen);
	}
	insert_message(&q->data, this_msg);
	mq_cond_signal(&q->emp_cond);
	if (q_was_empty && rt_pqueue_descr[q_index].notify.task != NULL) {
		rt_trigger_signal((MAXSIGNALS + mq), rt_pqueue_descr[q_index].notify.task);
		rt_pqueue_descr[q_index].notify.task = NULL;
	}
	mq_mutex_unlock(&q->mutex);
	return msglen;

}
EXPORT_SYMBOL(_mq_timedsend);

RTAI_SYSCALL_MODE int mq_close(mqd_t mq)
{
	int q_index = mq - 1;
	int q_ind;
	RT_TASK *this_task = _rt_whoami();
	struct _pqueue_access_struct *task_queue_data_ptr;

	if (q_index < 0 || q_index >= MAX_PQUEUES) { 
		return -EINVAL;
	}
	task_queue_data_ptr = (QUEUE_CTRL)this_task->mqueues;
	if (task_queue_data_ptr == NULL ) {
	    return -EINVAL;
	}
	mq_mutex_lock(&pqueue_mutex);
	for (q_ind = 0; q_ind < MQ_OPEN_MAX; q_ind++) {
		if (task_queue_data_ptr->q_access[q_ind].q_id == mq) {
			task_queue_data_ptr->q_access[q_ind].q_id = INVALID_PQUEUE;
			task_queue_data_ptr->q_access[q_ind].usp_notifier = NULL;
			rt_release_signal((mq + MAXSIGNALS), task_queue_data_ptr->this_task);
			task_queue_data_ptr->n_open_pqueues--;
			break;
	  	}
	}
	if (q_ind == MQ_OPEN_MAX) {
		mq_mutex_unlock(&pqueue_mutex);
		return -EINVAL;
	}
	mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
	if (rt_pqueue_descr[q_index].notify.task == this_task) {
		rt_pqueue_descr[q_index].notify.task = NULL;
	}
        if (--rt_pqueue_descr[q_index].open_count <= 0 &&
 	    rt_pqueue_descr[q_index].marked_for_deletion == TRUE ) {
		delete_queue(q_index);
	}
	mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	mq_mutex_unlock(&pqueue_mutex);
        return OK;
}
EXPORT_SYMBOL(mq_close);

RTAI_SYSCALL_MODE int mq_getattr(mqd_t mq, struct mq_attr *attrbuf)
{
	int q_index = mq - 1;

	if (0 <= q_index && q_index < MAX_PQUEUES) { 
		*attrbuf = rt_pqueue_descr[q_index].data.attrs;
		return OK;
	}
	return -EBADF;
}
EXPORT_SYMBOL(mq_getattr);

RTAI_SYSCALL_MODE int mq_setattr(mqd_t mq, const struct mq_attr *new_attrs, struct mq_attr *old_attrs)
{
	int q_index = mq - 1;
	int q_ind;
	RT_TASK *this_task = _rt_whoami();
	struct _pqueue_access_struct *task_queue_data_ptr;

	if (q_index < 0 || q_index >= MAX_PQUEUES) {
		return -EBADF;
	}
	if (old_attrs != NULL) { 
		*old_attrs = rt_pqueue_descr[q_index].data.attrs;
	}
	task_queue_data_ptr = (QUEUE_CTRL)this_task->mqueues;
	if (task_queue_data_ptr == NULL) {
		return -EINVAL;
	}
	for (q_ind = 0; q_ind < MQ_OPEN_MAX; q_ind++) {
		if (task_queue_data_ptr->q_access[q_ind].q_id == mq) {
			if(new_attrs->mq_flags == MQ_NONBLOCK) {
				task_queue_data_ptr->q_access[q_ind].oflags |= O_NONBLOCK;
			} else if (new_attrs->mq_flags == MQ_BLOCK) {
				task_queue_data_ptr->q_access[q_ind].oflags &= ~O_NONBLOCK;
			} else {
				return -EINVAL;
			}
		       	break;
	  	}
	}
	if (q_ind == MQ_OPEN_MAX) {
		return -EINVAL;
	}
	mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
	rt_pqueue_descr[q_index].data.attrs.mq_flags = new_attrs->mq_flags;
	mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	return OK;
}
EXPORT_SYMBOL(mq_setattr);

RTAI_SYSCALL_MODE int mq_reg_usp_notifier(mqd_t mq, RT_TASK *task, struct sigevent *usp_notification)
{
	mq_mutex_lock(&rt_pqueue_descr[mq -1].mutex);
	((QUEUE_CTRL)task->mqueues)->q_access[mq -1].usp_notifier = usp_notification;
	rt_copy_to_user(usp_notification, &rt_pqueue_descr[mq -1].notify.data, sizeof(struct sigevent));
	mq_mutex_unlock(&rt_pqueue_descr[mq -1].mutex);
	return 0;
}

RTAI_SYSCALL_MODE int _mq_notify(mqd_t mq, RT_TASK *task, long space, long rem, const struct sigevent *notification)
{
	int q_index = mq - 1;
	int rtn;
	if (q_index < 0 || q_index >= MAX_PQUEUES) {
		return -EBADF;
	}
	if (rem) {
		if (rt_pqueue_descr[q_index].notify.task == task) {
			mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
			rt_pqueue_descr[q_index].notify.task = NULL;
			mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
			return OK;
		} else {
			return -EBUSY;
		}
	}
	if (!space && !task->rt_signals) {
		rt_request_signal_mq(mq);
	} else if (!space && !((struct rt_signal_t *)task->rt_signals)[MAXSIGNALS + mq].sigtask) {
		rt_request_signal_mq(mq);
	}
	if (!space && (notification->sigev_notify != SIGEV_THREAD)){
		return ERROR;
	}
	mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
	if (rt_pqueue_descr[q_index].notify.task == NULL) {
        	rt_pqueue_descr[q_index].notify.task = task;
	        rt_pqueue_descr[q_index].notify.data = *notification;
	        if (space) {
	        	if (((QUEUE_CTRL)task->mqueues)->q_access[mq -1].usp_notifier) {
					rt_copy_to_user(((QUEUE_CTRL)task->mqueues)->q_access[mq -1].usp_notifier, &rt_pqueue_descr[mq -1].notify.data, sizeof(struct sigevent));
					rtn = OK;
	        	} else {
					rtn = O_NOTIFY_NP;
	        	}
	        } else {
				rtn = OK;
	        }
	} else {
		rtn = -EBUSY;
	}
	mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	return rtn;
}
EXPORT_SYMBOL(_mq_notify);

RTAI_SYSCALL_MODE int mq_unlink(char *mq_name)
{
	int q_index, rtn;

	mq_mutex_lock(&pqueue_mutex);
	q_index = name_to_id(mq_name);

	if (q_index < 0) {
		mq_mutex_unlock(&pqueue_mutex);
		return -ENOENT;
	}
	mq_mutex_lock(&rt_pqueue_descr[q_index].mutex);
	if (rt_pqueue_descr[q_index].open_count > 0) {
		strcpy(rt_pqueue_descr[q_index].q_name, "\0");
		rt_pqueue_descr[q_index].marked_for_deletion = TRUE;
		rtn = rt_pqueue_descr[q_index].open_count;
	} else {
		delete_queue(q_index);
		rtn = OK;
	}
	mq_mutex_unlock(&rt_pqueue_descr[q_index].mutex);
	mq_mutex_unlock(&pqueue_mutex);
	return rtn;
}
EXPORT_SYMBOL(mq_unlink);

///////////////////////////////////////////////////////////////////////////////
//      PROC FILESYSTEM SECTION
///////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_PROC_FS

static int PROC_READ_FUN(pqueue_read_proc)
{
int ind;
PROC_PRINT_VARS;

    PROC_PRINT("\nRTAI Posix Queue Status\n");
    PROC_PRINT("-----------------------\n\n");
    PROC_PRINT("MAX_PQUEUES = %2d (system wide)\n", MAX_PQUEUES);
    PROC_PRINT("MQ_OPEN_MAX = %2d (per RT task)\n", MQ_OPEN_MAX);
    PROC_PRINT("MQ_NAME_MAX = %d\n", MQ_NAME_MAX);

    PROC_PRINT("\nID  NOpen  NMsgs  MaxMsgs  MaxSz  Perms  Del  Name\n");
    PROC_PRINT("--------------------------------------------------------------------------------\n");
    for (ind = 0; ind < MAX_PQUEUES; ind++) {
	if (rt_pqueue_descr[ind].q_name[0] || rt_pqueue_descr[ind].open_count) {
	    PROC_PRINT( "%-3d %-6d ",
			rt_pqueue_descr[ind].q_id,
			rt_pqueue_descr[ind].open_count
			);
	    PROC_PRINT( "%-6ld %-6ld   %-5ld  ",
		        rt_pqueue_descr[ind].data.attrs.mq_curmsgs,
			rt_pqueue_descr[ind].data.attrs.mq_maxmsg,
			rt_pqueue_descr[ind].data.attrs.mq_msgsize
			);
	    PROC_PRINT( "%-4o   %c    %s\n",
			rt_pqueue_descr[ind].permissions,
			rt_pqueue_descr[ind].marked_for_deletion ? '*' : ' ',
			rt_pqueue_descr[ind].q_name
			);
	}
    }
    PROC_PRINT_DONE;
}

PROC_READ_OPEN_OPS(rtai_pqueue_fops, pqueue_read_proc)

static struct proc_dir_entry *proc_rtai_pqueue;

static int pqueue_proc_register(void)
{
    proc_rtai_pqueue = CREATE_PROC_ENTRY("pqueue", 0, rtai_proc_root, &rtai_pqueue_fops);
    SET_PROC_READ_ENTRY(proc_rtai_pqueue, pqueue_read_proc);
    return 0;
}

static int pqueue_proc_unregister(void)
{
    remove_proc_entry("pqueue", rtai_proc_root);
    return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//      MODULE CONTROL
///////////////////////////////////////////////////////////////////////////////

struct rt_native_fun_entry rt_pqueue_entries[] = {
	{ { UR1(1, 5) | UR2(4, 6), _mq_open }, 	        MQ_OPEN },
        { { 1, _mq_receive },  		                MQ_RECEIVE },
        { { 1, _mq_send },    		                MQ_SEND },
        { { 1, mq_close },                              MQ_CLOSE },
        { { UW1(2, 3), mq_getattr },                    MQ_GETATTR },
        { { UR1(2, 4) | UW1(3, 4), mq_setattr },	MQ_SETATTR },
        { { UR1(5, 6), _mq_notify },                     MQ_NOTIFY },
        { { UR1(1, 2), mq_unlink },                     MQ_UNLINK },      
        { { 1, _mq_timedreceive },		  	MQ_TIMEDRECEIVE },
        { { 1, _mq_timedsend }, 	       		MQ_TIMEDSEND },
        { { 1,	mq_reg_usp_notifier }, 	       		MQ_REG_USP_NOTIFIER },
	{ { 0, 0 },  		      	       		000 }
};

extern int set_rt_fun_entries(struct rt_native_fun_entry *entry);
extern void reset_rt_fun_entries(struct rt_native_fun_entry *entry);

int __rtai_mq_init(void) 
{
	num_pqueues = 0;
	mq_mutex_init(&pqueue_mutex, NULL);
#ifdef CONFIG_PROC_FS
	pqueue_proc_register();
#endif
	printk(KERN_INFO "RTAI[mq]: loaded.\n");
	return set_rt_fun_entries(rt_pqueue_entries);
	return OK;
}

void __rtai_mq_exit(void) 
{
	mq_mutex_destroy(&pqueue_mutex);
	reset_rt_fun_entries(rt_pqueue_entries);
#ifdef CONFIG_PROC_FS
	pqueue_proc_unregister();
#endif
	printk(KERN_INFO "RTAI[mq]: unloaded.\n");
}

#ifndef CONFIG_RTAI_MQ_BUILTIN
module_init(__rtai_mq_init);
module_exit(__rtai_mq_exit);
#endif /* !CONFIG_RTAI_MQ_BUILTIN */
