#ifndef _RTAI_PQUEUE_WRAPPER_H
#define _RTAI_PQUEUE_WRAPPER_H
//////////////////////////////////////////////////////////////////////////////
//
//      Copyright (©) 1999 Zentropic Computing, All rights reserved
//
// Authors:             Trevor Woolven (trevw@zentropix.com)
// Original date:       Thu 15 Jul 1999
// Id:                  @(#)$Id: rtai_pqueue_wrapper.h,v 1.4 2013/10/22 14:54:14 ando Exp $
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, see <http://www.gnu.org/licenses/>.
// 
//
// pqueues interface for Real Time Linux.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include "linux_wrapper.h"
#include "rtai.h"
#include "rtai_pthread_wrapper.h"
#include "zdefs.h"

// ----------------------------------------------------------------------------
#define TASK_FPU_DISABLE 0
#define TASK_FPU_ENABLE 1

//Posix definitions
#define	MQ_OPEN_MAX	8	//Maximum number of message queues per process
#define	MQ_PRIO_MAX	32	//Maximum number of message priorities
#define	MQ_BLOCK	0	//Flag to set queue into blocking mode
#define	MQ_NONBLOCK	1	//Flag to set queue into non-blocking mode
#define MQ_NAME_MAX	80	//Maximum length of a queue name string

#define MQ_MIN_MSG_PRIORITY 0		//Lowest priority message
#define MQ_MAX_MSG_PRIORITY MQ_PRIO_MAX //Highest priority message

//Definitions to support higher-level applications
typedef enum {FIFO_BASED, PRIORITY_BASED} QUEUEING_POLICY;
typedef enum {POSIX, VxWORKS} QUEUE_TYPE;

// ----------------------------------------------------------------------------
//Posix Queue Attributes
struct mq_attr {
    long mq_maxmsg;		//Maximum number of messages in queue
    long mq_msgsize;		//Maximum size of a message (in bytes)
    long mq_flags;		//Blocking/Non-blocking behaviour specifier
				// not used in mq_open only relevant for 
				// mq_getattrs and mq_setattrs
    long mq_curmsgs;		//Number of messages currently in queue
};
typedef struct mq_attr MQ_ATTR;

typedef union sigval {
	int sival_int;
	void *sival_ptr;
} sigval_t;

/*
 * sigevent definitions
 * 
 * It seems likely that SIGEV_THREAD will have to be handled from 
 * userspace, libpthread transmuting it to SIGEV_SIGNAL, which the
 * thread manager then catches and does the appropriate nonsense.
 * However, everything is written out here so as to not get lost.
 */
#define SIGEV_SIGNAL	0	/* notify via signal */
#define SIGEV_NONE	1	/* other notification: meaningless */
#define SIGEV_THREAD	2	/* deliver via thread creation */

#define SIGEV_MAX_SIZE	64
#define SIGEV_PAD_SIZE	((SIGEV_MAX_SIZE/sizeof(int)) - 3)

typedef struct sigevent {
	sigval_t sigev_value;
	int sigev_signo;
	int sigev_notify;
	union {
		int _pad[SIGEV_PAD_SIZE];

		struct {
			void (*_function)(sigval_t);
			void *_attribute;	/* really pthread_attr_t */
		} _sigev_thread;
	} _sigev_un;
} sigevent_t;

//Notification data
struct notify {
    RT_TASK *task;
    struct sigevent data;
};

// ----------------------------------------------------------------------------
//Generic Message format
//TODO: Consider moving this definition into rtai_utils.h
struct msg_hdr {
    BOOL in_use;
    size_t size;		//Actual message size
    uint priority;		//Usage priority (message/task)
    void *next;			//Pointer to next message on queue
};
typedef struct msg_hdr MSG_HDR;

//Generic queue header 
struct queue_control {
    void *base;		//Pointer to the base of the queue in memory
    void *head;		//Pointer to the element at the front of the queue
    void *tail;		//Pointer to the element at the back of the queue
    MQ_ATTR attrs;	//Queue attributes
};
typedef struct queue_control Q_CTRL;

//Data messages 
struct msg {
    MSG_HDR hdr;
    char data;			//Anchor point for message data
};
typedef struct msg MSG;

// ----------------------------------------------------------------------------
//Posix Queue Descriptors
struct _pqueue_descr_struct {
    RT_TASK *owner;		//Task that created the queue
    int open_count;		//Count of the number of tasks that have
				// 'opened' the queue for access
    char q_name[MQ_NAME_MAX];	//Name supplied for queue
    uint q_id;			//Queue Id (index into static list of queues)
    BOOL marked_for_deletion;	//Queue can be deleted once all tasks have 
				// closed it	
    Q_CTRL data;		//Data queue (real messages)
    mode_t permissions;		//Permissions granted by creator (ugo, rwx)
    struct notify notify;	//Notification data (empty -> !empty)
    pthread_cond_t  emp_cond;   //For blocking on empty queue
    pthread_cond_t  full_cond;  //For blocking on full queue
    pthread_mutex_t mutex;      //For synchronisation of queue
};
typedef struct _pqueue_descr_struct MSG_QUEUE;

// ----------------------------------------------------------------------------
//Task-related Posix Queue data
//A task can open up to MQ_OPEN_MAX pqueues, each with 'potentially'
//different permissions (read, write, blocking/non-blocking etc)
//
struct _pqueue_access_data {
    int q_id;
    int oflags;			//Queue access permissions & blocking spec
};

struct _pqueue_access_struct {
    RT_TASK *this_task;
    int n_open_pqueues;
    struct _pqueue_access_data q_access[MQ_OPEN_MAX];
};
typedef struct _pqueue_access_struct *QUEUE_CTRL;

// ----------------------------------------------------------------------------
typedef unsigned long mqd_t;
#define	INVALID_PQUEUE	0

typedef enum {FOR_READ, FOR_WRITE} Q_ACCESS;

///////////////////////////////////////////////////////////////////////////////
//      ACCESS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////
QUEUEING_POLICY get_task_queueing_policy(void);
QUEUEING_POLICY set_task_queuing_policy(QUEUEING_POLICY policy);

QUEUE_TYPE get_queue_type(void);
QUEUE_TYPE set_queue_type(QUEUE_TYPE type);

///////////////////////////////////////////////////////////////////////////////
//      POSIX MESSAGE QUEUES API
//
// Note that error returns represent the appropriate macro found in errno.h
// eg: -EINVAL, -EBADF etc
//
///////////////////////////////////////////////////////////////////////////////

//--------------------------------< mq_open >----------------------------------
// Create and/or open a message queue
// 
// Return codes:	>= 0 	valid Posix Queue Id
//			< 0	error
//
extern mqd_t mq_open(char *mq_name, int oflags, mode_t permissions, 
						struct mq_attr *mq_attr);

//------------------------------< mq_receive >---------------------------------
// Receive a message from a message queue
//
// Return codes:	>= 0	number of bytes received
//			< 0	error
//
extern size_t mq_receive(mqd_t mq, char *msg_buffer, 
				size_t buflen, unsigned int *msgprio);

//--------------------------------< mq_send >----------------------------------
// Send a message to a queue
//
// Return codes:	>= 0	number of bytes sent
//			< 0	error
extern int mq_send(mqd_t mq, const char *msg, size_t msglen, 
						unsigned int msgprio);

//--------------------------------< mq_close >---------------------------------
// Close a message queue (note that the queue remains in existance!)
//
// Return codes:	= 0	pQueue closed OK
//			< 0	error
//
extern int mq_close(mqd_t mq);

//-------------------------------< mq_getattr >--------------------------------
// Get the attributes of a message queue
//
// Return codes:	= 0	attributes copied successfully
//			< 0	error
//
extern int mq_getattr(mqd_t mq, struct mq_attr *attrbuf);

//-------------------------------< mq_setattr >--------------------------------
// Set a subset of a message queue's attributes
//
// Return codes:	= 0	attributes set successfully
//			< 0	error
//
extern int mq_setattr(mqd_t mq, const struct mq_attr *new_attrs,
				struct mq_attr *old_attrs);

//-------------------------------< mq_notify >---------------------------------
// Register a request to be notified whenever a message arrives on an empty
// queue
// Note that setting the 'notification' parameter to NULL cancels the task's
// earlier notification request
//
// Return codes:	= 0 	notification set/cleared successfully
//			< 0	error
//
extern int mq_notify(mqd_t mq, const struct sigevent *notification);

//-------------------------------< mq_unlink >---------------------------------
// Destroy a message queue
//
// Returns:		= 0 	queue was successfully unlinked
//			< 0	error 
//			> 0 	'n' tasks still have the queue 'open'
//
extern int mq_unlink(char *mq_name);

// ---------------------------------< eof >------------------------------------

#ifdef __cplusplus
}
#endif


#endif  // _RTAI_PQUEUE_WRAPPER_H_
