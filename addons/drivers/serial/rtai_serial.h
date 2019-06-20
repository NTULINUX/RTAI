/*
 * Copyright (C) 2002,2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2002,2003 Giuseppe Renoldi <giuseppe@renoldi.org>
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

#ifndef _RTAI_SERIAL_H
#define _RTAI_SERIAL_H

#define TTY0  0
#define TTY1  1
#define COM1  TTY0
#define COM2  TTY1

#define RT_SP_NO_HAND_SHAKE  0x00
#define RT_SP_DSR_ON_TX      0x01
#define RT_SP_HW_FLOW	     0x02

#define RT_SP_PARITY_EVEN    0x18
#define RT_SP_PARITY_NONE    0x00
#define RT_SP_PARITY_ODD     0x08
#define RT_SP_PARITY_HIGH    0x28
#define RT_SP_PARITY_LOW     0x38

#define RT_SP_FIFO_DISABLE   0x00
#define RT_SP_FIFO_SIZE_1    0x00
#define RT_SP_FIFO_SIZE_4    0x40
#define RT_SP_FIFO_SIZE_8    0x80
#define RT_SP_FIFO_SIZE_14   0xC0

#define RT_SP_FIFO_SIZE_DEFAULT  RT_SP_FIFO_SIZE_8

#define RT_SP_DTR            0x01
#define RT_SP_RTS            0x02 

#define RT_SP_CTS            0x10
#define RT_SP_DSR            0x20
#define RT_SP_RI             0x40
#define RT_SP_DCD            0x80

#define RT_SP_BUFFER_FULL    0x01
#define RT_SP_OVERRUN_ERR    0x02
#define RT_SP_PARITY_ERR     0x04
#define RT_SP_FRAMING_ERR    0x08
#define RT_SP_BREAK          0x10
#define RT_SP_BUFFER_OVF     0x20

#define DELAY_FOREVER  0x3FFFFFFFFFFFFFFFLL

#define  FUN_EXT_RTAI_SP  14

#define _SPOPEN      	  	 0 
#define _SPCLOSE    	  	 1	 
#define _SPREAD     	  	 2
#define _SPEVDRP      	  	 3
#define _SPWRITE     	  	 4
#define _SPCLEAR_RX  	  	 5
#define _SPCLEAR_TX  	  	 6
#define _SPGET_MSR   	  	 7
#define _SPSET_MCR   	  	 8
#define _SPGET_ERR  	  	 9
#define _SPSET_MODE	 	10
#define _SPSET_FIFOTRIG	 	11
#define _SPGET_RXAVBS	 	12
#define _SPGET_TXFRBS	 	13
#define _SPSET_THRS 	 	14
#define _SPSET_CALLBACK  	15
#define _SPSET_ERR_CALLBACK 	16
#define _SPWAIT_USR_CALLBACK 	17
#define _SPREAD_TIMED	 	18
#define _SPWRITE_TIMED  	19

#ifdef __KERNEL__

#include <rtai.h>

RTAI_SYSCALL_MODE int rt_spopen(unsigned int tty, unsigned int baud, unsigned int numbits, unsigned int stopbits, unsigned int parity, int mode, int fifotrig);

RTAI_SYSCALL_MODE int rt_spclose(unsigned int tty);

RTAI_SYSCALL_MODE int rt_spread(unsigned int tty, char *msg, int msg_size);

RTAI_SYSCALL_MODE int rt_spevdrp(unsigned int tty, char *msg, int msg_size);

RTAI_SYSCALL_MODE int rt_spwrite(unsigned int tty, char *msg, int msg_size);

RTAI_SYSCALL_MODE int rt_spget_rxavbs(unsigned int tty);

RTAI_SYSCALL_MODE int rt_spget_txfrbs(unsigned int tty);

RTAI_SYSCALL_MODE int rt_spclear_rx(unsigned int tty);

RTAI_SYSCALL_MODE int rt_spclear_tx(unsigned int tty);

RTAI_SYSCALL_MODE int rt_spset_mcr(unsigned int tty, int mask, int setbits);

RTAI_SYSCALL_MODE int rt_spget_msr(unsigned int tty, int mask);

RTAI_SYSCALL_MODE int rt_spset_mode(unsigned int tty, int mode);

RTAI_SYSCALL_MODE int rt_spset_fifotrig(unsigned int tty, int fifotrig);

RTAI_SYSCALL_MODE int rt_spget_err(unsigned int tty);

long rt_spset_callback_fun(unsigned int tty, void (*callback_fun)(int, int), int rxthrs, int txthrs);

RTAI_SYSCALL_MODE int rt_spset_thrs(unsigned int tty, int rxthrs, int txthrs);

long rt_spset_err_callback_fun(unsigned int tty, void (*err_callback_fun)(int));

RTAI_SYSCALL_MODE int rt_spset_callback_fun_usr(unsigned int tty, unsigned long callback_fun, int rxthrs, int txthrs, int code, void *task);

RTAI_SYSCALL_MODE int rt_spset_err_callback_fun_usr(unsigned int tty, unsigned long err_callback_fun, int dummy1, int dummy2, int code, void *task);

RTAI_SYSCALL_MODE void rt_spwait_usr_callback(unsigned int tty, unsigned long *retvals);

RTAI_SYSCALL_MODE int rt_spread_timed(unsigned int tty, char *msg, int msg_size, RTIME delay);

RTAI_SYSCALL_MODE int rt_spwrite_timed(unsigned int tty, char *msg, int msg_size, RTIME delay);

/*
 * rt_com compatibility functions.
 */ 

static inline int rt_com_setup(unsigned int tty,
			       int baud, int mode,
			       unsigned int parity,
			       unsigned int stopbits,
			       unsigned int numbits,
			       int fifotrig)
{
	return baud <= 0 ? rt_spclose(tty) : rt_spopen(tty, baud, numbits, stopbits, parity, mode, fifotrig);
}

static inline int rt_com_read(unsigned int tty,
			      char *msg,
			      int msg_size)
{
	int notrd;
	if ((notrd = rt_spread(tty, msg, msg_size)) >= 0) {
		return msg_size - notrd;
	}
	return notrd;
}

static inline int rt_com_write(unsigned int tty,
			       char *msg,
			       int msg_size)
{
	int notwr;
	if ((notwr = rt_spwrite(tty, msg, msg_size)) >= 0) {
		return msg_size - notwr;
	}
	return notwr;
}

#define rt_com_clear_input(indx) rt_spclear_rx(indx)
#define rt_com_clear_output(indx) rt_spclear_tx(indx)

#define rt_com_write_modem(indx, mask, op) rt_spset_mcr(indx, mask, op)
#define rt_com_read_modem(indx, mask) rt_spget_msr(indx, mask)

#define rt_com_set_mode(indx, mode) rt_spset_mode(indx, mode)
#define rt_com_set_fifotrig(indx, fifotrig) rt_spset_fifotrig(indx, fifotrig)

#define rt_com_error(indx) rt_spget_err(indx)

#else /* !__KERNEL__ */

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include <rtai_lxrt.h>

RTAI_PROTO(int, rt_spopen, (unsigned int tty, unsigned int baud, unsigned int numbits, unsigned int stopbits, unsigned int parity, int mode, int fifotrig))
{
	struct { unsigned long tty, baud, numbits, stopbits, parity; long mode, fifotrig; } arg = { tty, baud, numbits, stopbits, parity, mode, fifotrig }; 
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPOPEN, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spclose, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty }; 
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPCLOSE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spread, (unsigned int tty, char *msg, int msg_size))
{
	struct { unsigned long tty; char *msg; long msg_size; } arg = { tty, msg, msg_size };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPREAD, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spevdrp, (unsigned int tty, char *msg, int msg_size))
{
	struct { unsigned long tty; char *msg; long msg_size; } arg = { tty, msg, msg_size };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPEVDRP, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spwrite, (unsigned int tty, char *msg, int msg_size))
{
	struct { unsigned long tty; char *msg; long msg_size; } arg = { tty, msg, msg_size };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPWRITE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spread_timed, (unsigned int tty, char *msg, int msg_size, RTIME delay))
{
	struct { unsigned long tty; char *msg; long msg_size; RTIME delay; } arg = { tty, msg, msg_size, delay };
	return msg_size > 0 ? rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPREAD_TIMED, &arg).i[LOW] : msg_size;
}

RTAI_PROTO(int, rt_spwrite_timed, (unsigned int tty, char *msg, int msg_size, RTIME delay))
{
	struct { unsigned long tty; char *msg; long msg_size; RTIME delay; } arg = { tty, msg, msg_size, delay };
	return msg_size > 0 ? rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPWRITE_TIMED, &arg).i[LOW] : msg_size;
}

RTAI_PROTO(int, rt_spclear_rx, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPCLEAR_RX, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spclear_tx, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPCLEAR_TX, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spget_msr, (unsigned int tty, int mask))
{
	struct { unsigned long tty; long mask; } arg = { tty, mask };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPGET_MSR, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spset_mcr, (unsigned int tty, int mask, int setbits))
{
	struct { unsigned long tty; long mask, setbits; } arg = { tty, mask, setbits };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_MCR, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spget_err, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPGET_ERR, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spset_mode, (unsigned int tty, int mode))
{
	struct { unsigned long tty; long mode; } arg = { tty, mode };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_MODE, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spset_fifotrig, (unsigned int tty, int fifotrig))
{
	struct { unsigned long tty; long fifotrig; } arg = { tty, fifotrig };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_FIFOTRIG, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spget_rxavbs, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPGET_RXAVBS, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spget_txfrbs, (unsigned int tty))
{
	struct { unsigned long tty; } arg = { tty };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPGET_TXFRBS, &arg).i[LOW];
}

RTAI_PROTO(int, rt_spset_thrs, (unsigned int tty, int rxthrs, int txthrs))
{
	struct { unsigned long tty; long rxthrs, txthrs; } arg = { tty, rxthrs, txthrs };
	return rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_THRS, &arg).i[LOW];
}

RTAI_PROTO(void, rt_spwait_usr_callback, (unsigned int tty, unsigned long *retvals))
{
	struct { unsigned long tty; unsigned long *retvals; long size; } arg = { tty, retvals, 6*sizeof(unsigned long) };
	rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPWAIT_USR_CALLBACK, &arg);
	return;
}

#ifndef __CALLBACK_THREAD__
#define __CALLBACK_THREAD__
static void *callback_thread(void *farg)
{
	unsigned long retvals[6];
	struct farg_t { long tty; void *callback_fun; long rxthrs, txthrs, code; RT_TASK *task; } *arg;

	arg = (struct farg_t *)farg;
	if (!(arg->task = rt_task_init_schmod((unsigned long)arg, 0, 0, 0, SCHED_FIFO, 0xF))) {
		printf("CANNOT INIT USER SPACE CALLBACK SUPPORT\n");
		return (void *)1;
	}
	retvals[0] = arg->code;
	arg->code = 0;
	if (rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, retvals[0], arg).i[LOW] < 0) {
		printf("CANNOT SET USER SPACE CALLBACK SUPPORT\n");
		rt_task_delete(arg->task);
		free(arg);
		return (void *)1;
	}
	mlockall(MCL_CURRENT | MCL_FUTURE);

	rt_make_hard_real_time();
	while(1) {
		rt_spwait_usr_callback(arg->tty, retvals);
		if (!retvals[5]) break;
		if (retvals[0]) {
			((void(*)(unsigned long, unsigned long))retvals[0])(retvals[2], retvals[3]);
		}
		if (retvals[1]) {
			((void(*)(unsigned long))retvals[1])(retvals[4]);
		}
	}
	rt_make_soft_real_time();

	rt_task_delete(arg->task);
	free(arg);
	return (void *)0;
}
#endif

RTAI_PROTO(int, rt_spset_callback_fun, (unsigned int tty, void (*callback_fun)(int, int), int rxthrs, int txthrs))
{
	int ret;
	pthread_t thread;
	struct { long tty; void (*callback_fun)(int, int); long rxthrs, txthrs, code; void *task; } arg = { tty, callback_fun, rxthrs, txthrs, _SPSET_CALLBACK, 0 };
	if ((ret = rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_CALLBACK, &arg).i[LOW]) == EINVAL) {
		void *argp;
		argp = (void *)malloc(sizeof(arg));
		memcpy(argp, &arg, sizeof(arg));
		return pthread_create(&thread, NULL, callback_thread, argp);
	}
	return ret;
}

RTAI_PROTO(int, rt_spset_err_callback_fun, (unsigned int tty, void (*err_callback_fun)(int)))
{
	int ret;
	pthread_t thread;
	struct { long tty; void (*err_callback_fun)(int); long dummy1, dummy2, code; void *task; } arg = { tty, err_callback_fun, 0, 0, _SPSET_ERR_CALLBACK, 0 };
	if ((ret = rtai_lxrt(FUN_EXT_RTAI_SP, SIZARG, _SPSET_ERR_CALLBACK, &arg).i[LOW]) == EINVAL) {
		void *argp;
		argp = (void *)malloc(sizeof(arg));
		memcpy(argp, &arg, sizeof(arg));
		return pthread_create(&thread, NULL, callback_thread, argp);
	}
	return ret;
}

static inline int rt_com_setup(unsigned int tty, int baud, int mode, unsigned int parity, unsigned int stopbits, unsigned int numbits, int fifotrig)
{
	return baud <= 0 ? rt_spclose(tty) : rt_spopen(tty, baud, numbits, stopbits, parity, mode, fifotrig);
}

static inline int rt_com_read(unsigned int tty, char *msg, int msg_size)
{
	int notrd;
	if ((notrd = rt_spread(tty, msg, msg_size)) >= 0) {
		return abs(msg_size) - notrd;
	}
	return notrd;
}

static inline int rt_com_write(unsigned int tty, char *msg, int msg_size)
{
	int notwr;
	if ((notwr = rt_spwrite(tty, msg, msg_size)) >= 0) {
		return abs(msg_size) - notwr;
	}
	return notwr;
}

#define rt_com_clear_input(indx) rt_spclear_rx(indx)
#define rt_com_clear_output(indx) rt_spclear_tx(indx)

#define rt_com_write_modem(indx, mask, op) rt_spset_mcr(indx, mask, op)
#define rt_com_read_modem(indx, mask) rt_spget_msr(indx, mask)

#define rt_com_set_mode(indx, mode) rt_spset_mode(indx, mode)
#define rt_com_set_fifotrig(indx, fifotrig) rt_spset_fifotrig(indx, fifotrig)

#define rt_com_error(indx) rt_spget_err(indx)

#endif /* __KERNEL__ */

#endif /* !_RTAI_SERIAL_H */
