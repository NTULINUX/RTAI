/*
 * This file contains the necessary definitions for the RTAI tracer.
 *
 * Copyright (C) 2000, Karim Yaghmour <karym@opersys.com>
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

#ifndef _RTAI_TRACE_H
#define _RTAI_TRACE_H

#include <rtai_types.h>

#if defined(CONFIG_RTAI_TRACE) && defined(__KERNEL__)

#include <linux/trace.h>

/* Is RTAI tracing enabled */

/* The functions to the tracer management code */
int rt_register_tracer
       (tracer_call   /* The tracer function */);
int rt_unregister_tracer
       (tracer_call   /* The tracer function */);
int rt_trace_event
       (uint8_t       /* Event ID (as defined in this header file) */,
	void*         /* Structure describing the event */);

/* Generic macros */
#define RT_TRACE_EVENT(ID, DATA) rt_trace_event(ID, DATA)

#define TRACE_RTAI_START TRACE_EV_MAX

/* Traced events */
#define TRACE_RTAI_EV_MOUNT              TRACE_RTAI_START +  1   /* The RTAI subsystem was mounted */
#define TRACE_RTAI_EV_UMOUNT             TRACE_RTAI_START +  2   /* The RTAI subsystem was unmounted */
#define TRACE_RTAI_EV_GLOBAL_IRQ_ENTRY   TRACE_RTAI_START +  3   /* Entry in a global IRQ */
#define TRACE_RTAI_EV_GLOBAL_IRQ_EXIT    TRACE_RTAI_START +  4   /* Exit from a global IRQ */
#define TRACE_RTAI_EV_OWN_IRQ_ENTRY      TRACE_RTAI_START +  5   /* Entry in a CPU own IRQ */
#define TRACE_RTAI_EV_OWN_IRQ_EXIT       TRACE_RTAI_START +  6   /* Exit from a CPU own IRQ */
#define TRACE_RTAI_EV_TRAP_ENTRY         TRACE_RTAI_START +  7   /* Entry in a trap */
#define TRACE_RTAI_EV_TRAP_EXIT          TRACE_RTAI_START +  8   /* Exit from a trap */
#define TRACE_RTAI_EV_SRQ_ENTRY          TRACE_RTAI_START +  9   /* Entry in a SRQ */
#define TRACE_RTAI_EV_SRQ_EXIT           TRACE_RTAI_START + 10   /* Exit from a SRQ */
#define TRACE_RTAI_EV_SWITCHTO_LINUX     TRACE_RTAI_START + 11   /* Switch a CPU to Linux */
#define TRACE_RTAI_EV_SWITCHTO_RT        TRACE_RTAI_START + 12   /* Switch a CPU to real-time */
#define TRACE_RTAI_EV_SCHED_CHANGE       TRACE_RTAI_START + 13   /* A scheduling change has occured */
#define TRACE_RTAI_EV_TASK               TRACE_RTAI_START + 14   /* Hit key part of task services */
#define TRACE_RTAI_EV_TIMER              TRACE_RTAI_START + 15   /* Hit key part of timer services */
#define TRACE_RTAI_EV_SEM                TRACE_RTAI_START + 16   /* Hit key part of semaphore services */
#define TRACE_RTAI_EV_MSG                TRACE_RTAI_START + 17   /* Hit key part of message services */
#define TRACE_RTAI_EV_RPC                TRACE_RTAI_START + 18   /* Hit key part of RPC services */
#define TRACE_RTAI_EV_MBX                TRACE_RTAI_START + 19   /* Hit key part of mail box services */
#define TRACE_RTAI_EV_FIFO               TRACE_RTAI_START + 20   /* Hit key part of FIFO services */
#define TRACE_RTAI_EV_SHM                TRACE_RTAI_START + 21   /* Hit key part of shared memory services */
#define TRACE_RTAI_EV_LXRT               TRACE_RTAI_START + 22   /* Hit key part of LXRT services */
#define TRACE_RTAI_EV_LXRTI              TRACE_RTAI_START + 23   /* Hit key part of LXRT-Informed services */

/* Max number of traced events */
#define TRACE_RTAI_EV_MAX               TRACE_RTAI_EV_LXRTI

/* Structures and macros for traced events */
/*  TRACE_RTAI_MOUNT */
#define TRACE_RTAI_MOUNT()    rt_trace_event(TRACE_RTAI_EV_MOUNT, NULL)

/*  TRACE_RTAI_UMOUNT */
#define TRACE_RTAI_UMOUNT()   rt_trace_event(TRACE_RTAI_EV_UMOUNT, NULL)

/*  TRACE_RTAI_GLOBAL_IRQ_ENTRY */
typedef struct _trace_rtai_global_irq_entry
{
  uint8_t  irq_id;      /* IRQ number */
  uint8_t  kernel;      /* Are we executing kernel code */
} LTT_PACKED_STRUCT trace_rtai_global_irq_entry;
#if CONFIG_X86
#define TRACE_RTAI_GLOBAL_IRQ_ENTRY(ID, __dummy) \
           do \
           {\
           uint32_t                    eflags, xcs; \
           trace_rtai_global_irq_entry irq_entry;\
           irq_entry.irq_id = ID;\
           __asm__ __volatile__("pushfl; pop %0": "=g" (eflags)); \
           __asm__ __volatile__("pushl %%cs; pop %0": "=g" (xcs)); \
           irq_entry.kernel = !((VM_MASK & eflags) || (3 & xcs));\
           rt_trace_event(TRACE_RTAI_EV_GLOBAL_IRQ_ENTRY, &irq_entry);\
           } while(0)
#endif

/*  TRACE_RTAI_GLOBAL_IRQ_EXIT */
#define TRACE_RTAI_GLOBAL_IRQ_EXIT() rt_trace_event(TRACE_RTAI_EV_GLOBAL_IRQ_EXIT, NULL)

/*  TRACE_RTAI_OWN_IRQ_ENTRY */
typedef struct _trace_rtai_own_irq_entry
{
  uint8_t  irq_id;      /* IRQ number */
  uint8_t  kernel;      /* Are we executing kernel code */
} LTT_PACKED_STRUCT trace_rtai_own_irq_entry;
#if CONFIG_X86
#define TRACE_RTAI_OWN_IRQ_ENTRY(ID) \
           do \
           {\
           uint32_t                 eflags, xcs; \
           trace_rtai_own_irq_entry irq_entry;\
           irq_entry.irq_id = ID;\
           __asm__ __volatile__("pushfl; pop %0": "=g" (eflags)); \
           __asm__ __volatile__("pushl %%cs; pop %0": "=g" (xcs)); \
           irq_entry.kernel = !((VM_MASK & eflags) || (3 & xcs));\
           rt_trace_event(TRACE_RTAI_EV_OWN_IRQ_ENTRY, &irq_entry);\
           } while(0)
#endif

/*  TRACE_RTAI_OWN_IRQ_EXIT */
#define TRACE_RTAI_OWN_IRQ_EXIT() rt_trace_event(TRACE_RTAI_EV_OWN_IRQ_EXIT, NULL)

/*  TRACE_RTAI_TRAP_ENTRY */
typedef struct _trace_rtai_trap_entry
{
  uint8_t   trap_id;        /* Trap number */
  uint32_t  address;        /* Address where trap occured */
} LTT_PACKED_STRUCT trace_rtai_trap_entry;
#define TRACE_RTAI_TRAP_ENTRY(ID,ADDR) \
           do \
           {\
           trace_rtai_trap_entry trap_event;\
           trap_event.trap_id = ID;\
           trap_event.address = ADDR; \
           rt_trace_event(TRACE_RTAI_EV_TRAP_ENTRY, &trap_event);\
	   } while(0)
/*
           uint32_t              eip; \
           __asm__ __volatile__("pushl %%ip; pop %0": "=g" (eip)); \
           trap_event.address = eip;\
*/

/*  TRACE_RTAI_TRAP_EXIT */
#define TRACE_RTAI_TRAP_EXIT() rt_trace_event(TRACE_RTAI_EV_TRAP_EXIT, NULL)

/*  TRACE_RTAI_SRQ_ENTRY */
typedef struct _trace_rtai_srq_entry
{
  uint8_t  srq_id;      /* SRQ number */
  uint8_t  kernel;      /* Are we executing kernel code */
} LTT_PACKED_STRUCT trace_rtai_srq_entry;
#if CONFIG_X86
#define TRACE_RTAI_SRQ_ENTRY(ID) \
           do \
           {\
           uint32_t             eflags, xcs; \
           trace_rtai_srq_entry srq_entry;\
           srq_entry.srq_id = ID;\
           __asm__ __volatile__("pushfl; pop %0": "=g" (eflags)); \
           __asm__ __volatile__("pushl %%cs; pop %0": "=g" (xcs)); \
           srq_entry.kernel = !((VM_MASK & eflags) || (3 & xcs));\
           rt_trace_event(TRACE_RTAI_EV_SRQ_ENTRY, &srq_entry);\
           } while(0)
#endif

/*  TRACE_RTAI_SRQ_EXIT */
#define TRACE_RTAI_SRQ_EXIT() rt_trace_event(TRACE_RTAI_EV_SRQ_EXIT, NULL)

/*  TRACE_RTAI_SWITCHTO_LINUX */
typedef struct _trace_rtai_switchto_linux
{
  uint8_t   cpu_id;         /* The CPUID being switched to Linux */
} LTT_PACKED_STRUCT trace_rtai_switchto_linux;
#define TRACE_RTAI_SWITCHTO_LINUX(ID) \
           do \
           {\
           trace_rtai_switchto_linux switch_event; \
           switch_event.cpu_id = (uint8_t) ID; \
           rt_trace_event(TRACE_RTAI_EV_SWITCHTO_LINUX, &switch_event); \
           } while(0)

/*  TRACE_RTAI_SWITCHTO_RT */
typedef struct _trace_rtai_switchto_rt
{
  uint8_t   cpu_id;         /* The CPUID being switched to RT */
} LTT_PACKED_STRUCT trace_rtai_switchto_rt;
#define TRACE_RTAI_SWITCHTO_RT(ID) \
           do \
           {\
           trace_rtai_switchto_rt switch_event; \
           switch_event.cpu_id = (uint8_t) ID; \
           rt_trace_event(TRACE_RTAI_EV_SWITCHTO_RT, &switch_event); \
           } while(0)

/*  TRACE_RTAI_SCHED_CHANGE */ 
typedef struct _trace_rtai_sched_change
{
  uint32_t  out;         /* Outgoing process */
  uint32_t  in;          /* Incoming process */
  uint32_t  out_state;   /* Outgoing process' state */
} LTT_PACKED_STRUCT trace_rtai_sched_change;
#define TRACE_RTAI_SCHED_CHANGE(OUT, IN, OUT_STATE) \
           do \
           {\
           trace_rtai_sched_change sched_event;\
           sched_event.out       = (uint32_t) OUT;\
           sched_event.in        = (uint32_t) IN;\
           sched_event.out_state = (uint32_t) OUT_STATE; \
           rt_trace_event(TRACE_RTAI_EV_SCHED_CHANGE, &sched_event);\
           } while(0)

/*  TRACE_RTAI_TASK */
#define TRACE_RTAI_EV_TASK_INIT                    1     /* Initialize task */
#define TRACE_RTAI_EV_TASK_DELETE                  2     /* Delete task */
#define TRACE_RTAI_EV_TASK_SIG_HANDLER             3     /* Set signal handler */
#define TRACE_RTAI_EV_TASK_YIELD                   4     /* Yield CPU control */
#define TRACE_RTAI_EV_TASK_SUSPEND                 5     /* Suspend task */
#define TRACE_RTAI_EV_TASK_RESUME                  6     /* Resume task */
#define TRACE_RTAI_EV_TASK_MAKE_PERIOD_RELATIVE    7     /* Make task periodic relative in nanoseconds */
#define TRACE_RTAI_EV_TASK_MAKE_PERIOD             8     /* Make task periodic */
#define TRACE_RTAI_EV_TASK_WAIT_PERIOD             9     /* Wait until the next period */
#define TRACE_RTAI_EV_TASK_BUSY_SLEEP             10     /* Busy sleep */
#define TRACE_RTAI_EV_TASK_SLEEP                  11     /* Sleep */
#define TRACE_RTAI_EV_TASK_SLEEP_UNTIL            12     /* Sleep until */
typedef struct _trace_rtai_task
{
  uint8_t   event_sub_id;  /* Task event ID */
  uint32_t  event_data1;   /* Event data */
  uint64_t  event_data2;   /* Event data 2 */
  uint64_t  event_data3;   /* Event data 3 */
} LTT_PACKED_STRUCT trace_rtai_task;
#define TRACE_RTAI_TASK(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_task task_event;\
           task_event.event_sub_id = (uint8_t)  ID;\
           task_event.event_data1  = (uint32_t) DATA1; \
           task_event.event_data2  = (uint64_t) DATA2; \
           task_event.event_data3  = (uint64_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_TASK, &task_event);\
           } while(0)

/*  TRACE_RTAI_TIMER */
#define TRACE_RTAI_EV_TIMER_REQUEST                1     /* Request timer */
#define TRACE_RTAI_EV_TIMER_FREE                   2     /* Free timer */
#define TRACE_RTAI_EV_TIMER_REQUEST_APIC           3     /* Request APIC timers */
#define TRACE_RTAI_EV_TIMER_APIC_FREE              4     /* Free APIC timers */
#define TRACE_RTAI_EV_TIMER_HANDLE_EXPIRY          5     /* Handle timer expiry */
typedef struct _trace_rtai_timer
{
  uint8_t   event_sub_id;  /* Timer event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
} LTT_PACKED_STRUCT trace_rtai_timer;
#define TRACE_RTAI_TIMER(ID, DATA1, DATA2) \
           do \
           {\
           trace_rtai_timer timer_event; \
           timer_event.event_sub_id = (uint8_t)  ID; \
           timer_event.event_data1  = (uint32_t) DATA1; \
           timer_event.event_data2  = (uint32_t) DATA2; \
           rt_trace_event(TRACE_RTAI_EV_TIMER, &timer_event); \
           } while(0)

/*  TRACE_RTAI_SEM */
#define TRACE_RTAI_EV_SEM_INIT                     1     /* Initialize semaphore */
#define TRACE_RTAI_EV_SEM_DELETE                   2     /* Delete semaphore */
#define TRACE_RTAI_EV_SEM_SIGNAL                   3     /* Signal semaphore */
#define TRACE_RTAI_EV_SEM_WAIT                     4     /* Wait on semaphore */
#define TRACE_RTAI_EV_SEM_WAIT_IF                  5     /* Take semaphore if possible */
#define TRACE_RTAI_EV_SEM_WAIT_UNTIL               6     /* Wait on semaphore until a certain time */
typedef struct _trace_rtai_sem
{
  uint8_t   event_sub_id;  /* Semaphore event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint64_t  event_data2;   /* Event data 2 */
} LTT_PACKED_STRUCT trace_rtai_sem;
#define TRACE_RTAI_SEM(ID, DATA1, DATA2) \
           do \
           {\
           trace_rtai_sem sem_event; \
           sem_event.event_sub_id = (uint8_t)  ID; \
           sem_event.event_data1  = (uint32_t) DATA1; \
           sem_event.event_data2  = (uint64_t) DATA2; \
           rt_trace_event(TRACE_RTAI_EV_SEM, &sem_event); \
           } while(0)

/*  TRACE_RTAI_MSG */
#define TRACE_RTAI_EV_MSG_SEND                      1    /* Send a message */
#define TRACE_RTAI_EV_MSG_SEND_IF                   2    /* Send if possible */
#define TRACE_RTAI_EV_MSG_SEND_UNTIL                3    /* Try sending until a certain time */
#define TRACE_RTAI_EV_MSG_RECV                      4    /* Receive a message */
#define TRACE_RTAI_EV_MSG_RECV_IF                   5    /* Receive if possible */
#define TRACE_RTAI_EV_MSG_RECV_UNTIL                6    /* Try receiving until a certain time */
typedef struct _trace_rtai_msg
{
  uint8_t   event_sub_id;  /* Message event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint64_t  event_data3;   /* Event data 3 */
} LTT_PACKED_STRUCT trace_rtai_msg;
#define TRACE_RTAI_MSG(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_msg msg_event; \
           msg_event.event_sub_id = (uint8_t)  ID; \
           msg_event.event_data1  = (uint32_t) DATA1; \
           msg_event.event_data2  = (uint32_t) DATA2; \
           msg_event.event_data3  = (uint64_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_MSG, &msg_event); \
           } while(0)

/*  TRACE_RTAI_RPC */
#define TRACE_RTAI_EV_RPC_MAKE                       1    /* Make a remote procedure call */
#define TRACE_RTAI_EV_RPC_MAKE_IF                    2    /* Make RPC if receiver is ready */
#define TRACE_RTAI_EV_RPC_MAKE_UNTIL                 3    /* Try making an RPC until a certain time */
#define TRACE_RTAI_EV_RPC_RETURN                     4    /* Send result of RPC back to caller */
typedef struct _trace_rtai_rpc
{
  uint8_t   event_sub_id;  /* RPC event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint64_t  event_data3;   /* Event data 3 */
} LTT_PACKED_STRUCT trace_rtai_rpc;
#define TRACE_RTAI_RPC(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_rpc rpc_event; \
           rpc_event.event_sub_id = (uint8_t)  ID; \
           rpc_event.event_data1  = (uint32_t) DATA1; \
           rpc_event.event_data2  = (uint32_t) DATA2; \
           rpc_event.event_data3  = (uint64_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_RPC, &rpc_event); \
           } while(0)

/*  TRACE_RTAI_MBX */
#define TRACE_RTAI_EV_MBX_INIT                       1    /* Initialize Message BoX */
#define TRACE_RTAI_EV_MBX_DELETE                     2    /* Delete message box */
#define TRACE_RTAI_EV_MBX_SEND                       3    /* Send a message to a message box */
#define TRACE_RTAI_EV_MBX_SEND_WP                    4    /* Send as many bytes as possible */
#define TRACE_RTAI_EV_MBX_SEND_IF                    5    /* Send a message if possible */
#define TRACE_RTAI_EV_MBX_SEND_UNTIL                 6    /* Try sending until a certain time */
#define TRACE_RTAI_EV_MBX_RECV                       7    /* Receive a message */
#define TRACE_RTAI_EV_MBX_RECV_WP                    8    /* Receive as many bytes as possible */
#define TRACE_RTAI_EV_MBX_RECV_IF                    9    /* Receive a message if available */
#define TRACE_RTAI_EV_MBX_RECV_UNTIL                10    /* Try receiving until a certain time */
typedef struct _trace_rtai_mbx
{
  uint8_t   event_sub_id;  /* Message Box event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint64_t  event_data3;   /* Event data 3 */  
} LTT_PACKED_STRUCT trace_rtai_mbx;
#define TRACE_RTAI_MBX(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_mbx mbx_event; \
           mbx_event.event_sub_id = (uint8_t)  ID; \
           mbx_event.event_data1  = (uint32_t) DATA1; \
           mbx_event.event_data2  = (uint32_t) DATA2; \
           mbx_event.event_data3  = (uint64_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_MBX, &mbx_event); \
           } while(0)

/*  TRACE_RTAI_FIFO */
#define TRACE_RTAI_EV_FIFO_CREATE                     1   /* Create FIFO */
#define TRACE_RTAI_EV_FIFO_DESTROY                    2   /* Destroy FIFO */
#define TRACE_RTAI_EV_FIFO_RESET                      3   /* Reset FIFO */
#define TRACE_RTAI_EV_FIFO_RESIZE                     4   /* Resize FIFO */
#define TRACE_RTAI_EV_FIFO_PUT                        5   /* Write data to FIFO */
#define TRACE_RTAI_EV_FIFO_GET                        6   /* Get data from FIFO */
#define TRACE_RTAI_EV_FIFO_CREATE_HANDLER             7   /* Install FIFO handler */
#define TRACE_RTAI_EV_FIFO_OPEN                       8   /* Open FIFO */
#define TRACE_RTAI_EV_FIFO_RELEASE                    9   /* Release FIFO */
#define TRACE_RTAI_EV_FIFO_READ                      10   /* Read from FIFO */
#define TRACE_RTAI_EV_FIFO_WRITE                     11   /* Write to FIFO */
#define TRACE_RTAI_EV_FIFO_READ_TIMED                12   /* Read with time limit */
#define TRACE_RTAI_EV_FIFO_WRITE_TIMED               13   /* Write with time limit */
#define TRACE_RTAI_EV_FIFO_READ_ALLATONCE            14   /* Read all the data from FIFO */
#define TRACE_RTAI_EV_FIFO_LLSEEK                    15   /* Seek position into FIFO */
#define TRACE_RTAI_EV_FIFO_FASYNC                    16   /* Asynchronous notification */
#define TRACE_RTAI_EV_FIFO_IOCTL                     17   /* IO control on FIFO */
#define TRACE_RTAI_EV_FIFO_POLL                      18   /* Poll FIFO */
#define TRACE_RTAI_EV_FIFO_SUSPEND_TIMED             19   /* Suspend task for given period */
#define TRACE_RTAI_EV_FIFO_SET_ASYNC_SIG             20   /* Set asynchrounous signal */
#define TRACE_RTAI_EV_FIFO_SEM_INIT                  21   /* Initialize semaphore */
#define TRACE_RTAI_EV_FIFO_SEM_POST                  22   /* Post semaphore */
#define TRACE_RTAI_EV_FIFO_SEM_WAIT                  23   /* Wait on semaphore */
#define TRACE_RTAI_EV_FIFO_SEM_TRY_WAIT              24   /* Try waiting on semaphore */
#define TRACE_RTAI_EV_FIFO_SEM_TIMED_WAIT            25   /* Wait on semaphore until a certain time */
#define TRACE_RTAI_EV_FIFO_SEM_DESTROY               26   /* Destroy semaphore  */
typedef struct _trace_rtai_fifo
{
  uint8_t   event_sub_id;  /* FIFO event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
} LTT_PACKED_STRUCT trace_rtai_fifo;
#define TRACE_RTAI_FIFO(ID, DATA1, DATA2) \
           do \
           {\
           trace_rtai_fifo fifo_event; \
           fifo_event.event_sub_id = (uint8_t)  ID; \
           fifo_event.event_data1  = (uint32_t) DATA1; \
           fifo_event.event_data2  = (uint32_t) DATA2; \
           rt_trace_event(TRACE_RTAI_EV_FIFO, &fifo_event); \
           } while(0)

/*  TRACE_RTAI_SHM */
#define TRACE_RTAI_EV_SHM_MALLOC                       1  /* Allocate shared memory */
#define TRACE_RTAI_EV_SHM_KMALLOC                      2  /* Allocate shared memory in kernel space */
#define TRACE_RTAI_EV_SHM_GET_SIZE                     3  /* Get the size of the shared memory area */ 
#define TRACE_RTAI_EV_SHM_FREE                         4  /* Free shared memory */
#define TRACE_RTAI_EV_SHM_KFREE                        5  /* Free kernel space shared memory */
typedef struct _trace_rtai_shm
{
  uint8_t   event_sub_id;  /* SHared Memory event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint32_t  event_data3;   /* Event data 3 */
} LTT_PACKED_STRUCT trace_rtai_shm;
#define TRACE_RTAI_SHM(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_shm shm_event; \
           shm_event.event_sub_id = (uint8_t)  ID; \
           shm_event.event_data1  = (uint32_t) DATA1; \
           shm_event.event_data2  = (uint32_t) DATA2; \
           shm_event.event_data3  = (uint32_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_SHM, &shm_event); \
           } while(0)

/*  TRACE_RTAI_LXRT */
#define TRACE_RTAI_EV_LXRT_RTAI_SYSCALL_ENTRY             1  /* Entry in LXRT syscall */
#define TRACE_RTAI_EV_LXRT_RTAI_SYSCALL_EXIT              2  /* Exit from LXRT syscall */
#define TRACE_RTAI_EV_LXCHANGE                   3  /* Scheduling change */
#define TRACE_RTAI_EV_LXRT_STEAL_TASK                     4  /* Take task control from Linux */
#define TRACE_RTAI_EV_LXRT_GIVE_BACK_TASK                 5  /* Give task control back to Linux */
#define TRACE_RTAI_EV_LXRT_SUSPEND                        6  /* Suspend a task */
#define TRACE_RTAI_EV_LXRT_RESUME                         7  /* Resume task's execution */
#define TRACE_RTAI_EV_LXRT_HANDLE                         8  /* Handle a request for an RTAI service */
typedef struct _trace_rtai_lxrt
{
  uint8_t   event_sub_id;  /* LXRT event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint32_t  event_data3;   /* Event data 3 */  
} LTT_PACKED_STRUCT trace_rtai_lxrt;
#define TRACE_RTAI_LXRT(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_lxrt lxrt_event; \
           lxrt_event.event_sub_id = (uint8_t)  ID; \
           lxrt_event.event_data1  = (uint32_t) DATA1; \
           lxrt_event.event_data2  = (uint32_t) DATA2; \
           lxrt_event.event_data3  = (uint32_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_LXRT, &lxrt_event); \
           } while(0)

/*  TRACE_RTAI_LXRTI */
#define TRACE_RTAI_EV_LXRTI_NAME_ATTACH                    1  /* Register current process as name */
#define TRACE_RTAI_EV_LXRTI_NAME_LOCATE                    2  /* Locate a given process usint it's name */
#define TRACE_RTAI_EV_LXRTI_NAME_DETACH                    3  /* Detach process from name */
#define TRACE_RTAI_EV_LXRTI_SEND                           4  /* Send message to PID */
#define TRACE_RTAI_EV_LXRTI_RECV                           5  /* Receive message */
#define TRACE_RTAI_EV_LXRTI_CRECV                          6  /* Non-blocking receive */
#define TRACE_RTAI_EV_LXRTI_REPLY                          7  /* Reply to message received */
#define TRACE_RTAI_EV_LXRTI_PROXY_ATTACH                   8  /* Attach proxy to process */
#define TRACE_RTAI_EV_LXRTI_PROXY_DETACH                   9  /* Detach proxy from process */
#define TRACE_RTAI_EV_LXRTI_TRIGGER                       10  /* Trigger proxy */
typedef struct _trace_rtai_lxrti
{
  uint8_t   event_sub_id;  /* LXRT event ID */
  uint32_t  event_data1;   /* Event data 1 */
  uint32_t  event_data2;   /* Event data 2 */
  uint64_t  event_data3;   /* Event data 3 */  
} LTT_PACKED_STRUCT trace_rtai_lxrti;
#define TRACE_RTAI_LXRTI(ID, DATA1, DATA2, DATA3) \
           do \
           {\
           trace_rtai_lxrti lxrti_event; \
           lxrti_event.event_sub_id = (uint8_t)  ID; \
           lxrti_event.event_data1  = (uint32_t) DATA1; \
           lxrti_event.event_data2  = (uint32_t) DATA2; \
           lxrti_event.event_data3  = (uint64_t) DATA3; \
           rt_trace_event(TRACE_RTAI_EV_LXRTI, &lxrti_event); \
           } while(0)

#else /* !(CONFIG_RTAI_TRACE && __KERNEL__) */
#define RT_TRACE_EVENT(ID, DATA)
#define TRACE_RTAI_MOUNT()
#define TRACE_RTAI_UMOUNT()
#define TRACE_RTAI_GLOBAL_IRQ_ENTRY(ID,X)
#define TRACE_RTAI_GLOBAL_IRQ_EXIT()
#define TRACE_RTAI_OWN_IRQ_ENTRY(ID)
#define TRACE_RTAI_OWN_IRQ_EXIT()
#define TRACE_RTAI_TRAP_ENTRY(ID,ADDR)
#define TRACE_RTAI_TRAP_EXIT()
#define TRACE_RTAI_SRQ_ENTRY(a)
#define TRACE_RTAI_SRQ_EXIT()
#define TRACE_RTAI_SWITCHTO_LINUX(ID)
#define TRACE_RTAI_SWITCHTO_RT(ID)
#define TRACE_RTAI_SCHED_CHANGE(OUT, IN, OUT_STATE)
#define TRACE_RTAI_TASK(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_TIMER(ID, DATA1, DATA2)
#define TRACE_RTAI_SEM(ID, DATA1, DATA2)
#define TRACE_RTAI_MSG(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_RPC(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_MBX(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_FIFO(ID, DATA1, DATA2)
#define TRACE_RTAI_SHM(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_LXRT(ID, DATA1, DATA2, DATA3)
#define TRACE_RTAI_LXRTI(ID, DATA1, DATA2, DATA3)
#endif /* CONFIG_RTAI_TRACE && __KERNEL__ */

#endif /* !_RTAI_TRACE_H */
