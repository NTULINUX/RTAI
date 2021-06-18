/*
 * Copyright (C) 2002 Thomas Leibner (leibner@t-online.de) (first complete writeup)
 * Copyright (C) 2002 David Schleef (ds@schleef.org) (COMEDI master)
 * Copyright (C) 2002 Lorenzo Dozio (dozio@aero.polimi.it) (made it all work)
 * Copyright (C) 2002-2017 Paolo Mantegazza (mantegazza@aero.polimi.it) (continued maintenance)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef _RTAI_COMEDI_H_
#define _RTAI_COMEDI_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <rtai_types.h>
#include <rtai_sem.h>
#include <rtai_netrpc.h>

#ifdef CONFIG_RTAI_USE_LINUX_COMEDI
// undef RTAI VERSION to avoid conflicting with the same macro in COMEDI
#undef VERSION
#endif

#define FUN_COMEDI_LXRT_INDX  9

#define _KCOMEDI_OPEN 			 0
#define _KCOMEDI_CLOSE 			 1
#define _KCOMEDI_LOCK 			 2
#define _KCOMEDI_UNLOCK 		 3
#define _KCOMEDI_CANCEL 		 4
#define _KCOMEDI_REGISTER_CALLBACK 	 5
#define _KCOMEDI_COMMAND 		 6
#define _KCOMEDI_COMMAND_TEST 		 7
#define _KCOMEDI_TRIGGER 		 8
#define _KCOMEDI_DATA_WRITE 		 9
#define _KCOMEDI_DATA_READ 		10
#define _KCOMEDI_DATA_READ_DELAYED	11
#define _KCOMEDI_DATA_READ_HINT         12
#define _KCOMEDI_DIO_CONFIG 		13
#define _KCOMEDI_DIO_READ 		14
#define _KCOMEDI_DIO_WRITE 		15
#define _KCOMEDI_DIO_BITFIELD 		16
#define _KCOMEDI_GET_N_SUBDEVICES 	17
#define _KCOMEDI_GET_VERSION_CODE 	18
#define _KCOMEDI_GET_DRIVER_NAME 	19
#define _KCOMEDI_GET_BOARD_NAME 	20
#define _KCOMEDI_GET_SUBDEVICE_TYPE 	21
#define _KCOMEDI_FIND_SUBDEVICE_TYPE	22
#define _KCOMEDI_GET_N_CHANNELS 	23
#define _KCOMEDI_GET_MAXDATA 		24
#define _KCOMEDI_GET_N_RANGES 		25
#define _KCOMEDI_DO_INSN 		26
#define _KCOMEDI_DO_INSN_LIST		27
#define _KCOMEDI_POLL 			28

/* DEPRECATED function
#define _KCOMEDI_GET_RANGETYPE 		29
*/

/* ALPHA functions */
#define _KCOMEDI_GET_SUBDEVICE_FLAGS 	30
#define _KCOMEDI_GET_LEN_CHANLIST 	31
#define _KCOMEDI_GET_KRANGE 		32
#define _KCOMEDI_GET_BUF_HEAD_POS	33
#define _KCOMEDI_SET_USER_INT_COUNT	34
#define _KCOMEDI_MAP 			35
#define _KCOMEDI_UNMAP 			36

/* RTAI specific callbacks from kcomedi to user space */
#define _KCOMEDI_WAIT        		37
#define _KCOMEDI_WAIT_IF     		38
#define _KCOMEDI_WAIT_UNTIL  		39
#define _KCOMEDI_WAIT_TIMED  		40

/* RTAI specific functions to allocate/free comedi_cmd */
#define _KCOMEDI_ALLOC_CMD  		41
#define _KCOMEDI_FREE_CMD  		42

#define _KCOMEDI_COMD_DATA_READ         43
#define _KCOMEDI_COMD_DATA_WREAD        44
#define _KCOMEDI_COMD_DATA_WREAD_IF     45
#define _KCOMEDI_COMD_DATA_WREAD_UNTIL  46
#define _KCOMEDI_COMD_DATA_WREAD_TIMED  47
#define _KCOMEDI_COMD_DATA_WRITE        48

/* Netrpced hooks for functions with special 32/64 bits conversions */
#define _RT_KCOMEDI_COMMAND  		49
#define _RT_KCOMEDI_DO_INSN_LIST 	50
#define _RT_KCOMEDI_COMD_DATA_WREAD 	51

#ifdef CONFIG_RTAI_USE_LINUX_COMEDI
typedef unsigned int lsampl_t;
typedef unsigned short sampl_t;
typedef struct comedi_cmd comedi_cmd;
typedef struct comedi_insn comedi_insn;
typedef struct comedi_insnlist comedi_insnlist;
typedef struct comedi_krange comedi_krange;
#endif

struct cmds_ofstlens { unsigned int cmd_chanlist_ofst, cmd_data_ofst, cmd_ofst, cmd_len, chanlist_ofst, chanlist_len, data_ofst, data_len; };

struct insns_ofstlens { unsigned int n_ofst, subdev_ofst, chanspec_ofst, insn_len, insns_ofst, data_ofsts, data_ofst; };

#if 1
#include <linux/comedi.h>
#else // new way to be improved by using what configured for RTAI
#include </home/mante/comedi/include/linux/comedi.h>
#endif

#ifdef __KERNEL__ /* For kernel module build. */

#include <linux/comedilib.h>

RTAI_SYSCALL_MODE long rt_comedi_wait(unsigned int *cbmask);

RTAI_SYSCALL_MODE long rt_comedi_wait_if(unsigned int *cbmask);

RTAI_SYSCALL_MODE long _rt_comedi_wait_until(unsigned int *cbmask, RTIME until);
static inline long rt_comedi_wait_until(RTIME until, unsigned int *cbmask)
{
	return _rt_comedi_wait_until(cbmask, until);
}

RTAI_SYSCALL_MODE long _rt_comedi_wait_timed(unsigned int *cbmask, RTIME delay);
static inline long rt_comedi_wait_timed(RTIME delay, unsigned int *cbmask)
{
	return _rt_comedi_wait_timed(cbmask, delay);
}

RTAI_SYSCALL_MODE char *rt_comedi_get_driver_name(void *dev, char *name);

RTAI_SYSCALL_MODE char *rt_comedi_get_board_name(void *dev, char *name);

RTAI_SYSCALL_MODE int rt_comedi_register_callback(void *dev, unsigned int subdev, unsigned int mask, int (*callback)(unsigned int, void *), void *arg);

RTAI_SYSCALL_MODE long rt_comedi_command_data_read (void *dev, unsigned int subdev, long nchans, lsampl_t *data);

RTAI_SYSCALL_MODE long rt_comedi_command_data_wread(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask);

RTAI_SYSCALL_MODE long rt_comedi_command_data_wread_if(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask);

RTAI_SYSCALL_MODE long _rt_comedi_command_data_wread_until(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask, RTIME until);
static inline long rt_comedi_command_data_wread_until(void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME until, unsigned int *cbmask)
{
	return _rt_comedi_command_data_wread_until(dev, subdev, nchans, data, cbmask, until);
}

RTAI_SYSCALL_MODE long _rt_comedi_command_data_wread_timed(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask, RTIME delay);
static inline long rt_comedi_command_data_wread_timed(void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME delay, unsigned int *cbmask)
{
	return _rt_comedi_command_data_wread_timed(dev, subdev, nchans, data, cbmask, delay);
}

RTAI_SYSCALL_MODE int rt_comedi_do_insnlist(void *dev, comedi_insnlist *ilist);

RTAI_SYSCALL_MODE int rt_comedi_trigger(void *dev, unsigned int subdev);

RTAI_SYSCALL_MODE long rt_comedi_command_data_write(void *dev, unsigned int subdev, long nchans, lsampl_t *data);

#else  /* __KERNEL__ not defined */

#include <string.h>
#include <asm/rtai_lxrt.h>
#include <rtai_msg.h>
#include <rtai_shm.h>

typedef void comedi_t;

#define COMEDI_LXRT_SIZARG sizeof(arg)

RTAI_PROTO(void *, comedi_open, (const char *filename))
{
        struct { const char *filename; } arg = { filename };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_OPEN, &arg).v[LOW];
}

RTAI_PROTO(int, comedi_close, (void *dev))
{
        struct { void *dev; } arg = { dev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_CLOSE, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_lock, (void *dev, unsigned int subdev))
{
        struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_LOCK, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_unlock, (void *dev, unsigned int subdev))
{
        struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_UNLOCK, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_cancel, (void *dev, unsigned int subdev))
{
        struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_CANCEL, &arg).i[LOW];
}

RTAI_PROTO(int, rt_comedi_register_callback, (void *dev, unsigned int subdevice, unsigned int mask, int (*cb)(unsigned int, void *), void *task))
{
        struct { void *dev; unsigned long subdevice; unsigned long mask; void *cb; void *task; } arg = { dev, subdevice, mask, NULL, task };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_REGISTER_CALLBACK, &arg).i[LOW];
}

#define comedi_register_callback(dev, subdev, mask, cb, arg)  rt_comedi_register_callback(dev, subdev, mask, NULL, arg)

RTAI_PROTO(int, comedi_command, (void *dev, comedi_cmd *cmd))
{
	if (cmd) {
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		int retval;
		comedi_cmd lcmd;
	        struct { void *dev; comedi_cmd *cmd; } arg = { dev, &lcmd };
		lcmd = cmd[0];
        	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMMAND, &arg).i[LOW];
		if (!retval) {
			cmd[0] = lcmd;
		}
        	return retval;
#else
	        struct { void *dev; comedi_cmd *cmd; } arg = { dev, cmd };
        	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMMAND, &arg).i[LOW];
#endif
	}
	return -1;
}

RTAI_PROTO(int, comedi_command_test, (void *dev, comedi_cmd *cmd))
{
	if (cmd) {
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		comedi_cmd lcmd;
		int retval;
	        struct { void *dev; comedi_cmd *cmd; } arg = { dev, &lcmd };
		lcmd = cmd[0];
	        retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMMAND_TEST, &arg).i[LOW];
		if (!retval) {
			cmd[0] = lcmd;
		}
        	return retval;
#else
	        struct { void *dev; comedi_cmd *cmd; } arg = { dev, cmd };
        	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMMAND_TEST, &arg).i[LOW];
#endif
	}
	return -1;
}

RTAI_PROTO(long, rt_comedi_wait, (unsigned int *cbmask))
{
       	struct { unsigned int *cbmask; } arg = { cbmask };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_WAIT, &arg).i[LOW];
}

RTAI_PROTO(long, rt_comedi_wait_if, (unsigned int *cbmask))
{
       	struct { unsigned int *cbmask; } arg = { cbmask };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_WAIT_IF, &arg).i[LOW];
}

RTAI_PROTO(long, rt_comedi_wait_until, (RTIME until, unsigned int *cbmask))
{
       	struct { unsigned int *cbmask; RTIME until; } arg = { cbmask, until };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_WAIT_UNTIL, &arg).i[LOW];
}

RTAI_PROTO(long, rt_comedi_wait_timed, (RTIME delay, unsigned int *cbmask))
{
       	struct { unsigned int *cbmask; RTIME delay; } arg = { cbmask, delay };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_WAIT_TIMED, &arg).i[LOW];
}

RTAI_PROTO(long, rt_comedi_command_data_read, (void *dev, unsigned int subdev, long nchans, lsampl_t *data))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	int retval;
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; } arg = { dev, subdev, nchans, ldata };
	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_READ, &arg).i[LOW];
	memcpy(data, &ldata, sizeof(ldata));
	return retval;
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; } arg = { dev, subdev, nchans, data };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_READ, &arg).i[LOW];
#endif
}

RTAI_PROTO(long, rt_comedi_command_data_wread, (void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	int retval;
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; } arg = { dev, subdev, nchans, ldata, cbmask };
	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD, &arg).i[LOW];
	memcpy(data, &ldata, sizeof(ldata));
	return retval;
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; } arg = { dev, subdev, nchans, data, cbmask };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD, &arg).i[LOW];
#endif
}

RTAI_PROTO(long, rt_comedi_command_data_wread_if, (void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	int retval;
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; } arg = { dev, subdev, nchans, ldata, cbmask };
	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_IF, &arg).i[LOW];
	memcpy(data, &ldata, sizeof(ldata));
	return retval;
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; } arg = { dev, subdev, nchans, data, cbmask };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_IF, &arg).i[LOW];
#endif
}

RTAI_PROTO(long, rt_comedi_command_data_wread_until, (void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME until, unsigned int *cbmask))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	int retval;
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; RTIME until; } arg = { dev, subdev, nchans, ldata, cbmask, until };
	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_UNTIL, &arg).i[LOW];
	memcpy(data, &ldata, sizeof(ldata));
	return retval;
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; RTIME until; } arg = { dev, subdev, nchans, data, cbmask, until };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_UNTIL, &arg).i[LOW];
#endif
}

RTAI_PROTO(long, rt_comedi_command_data_wread_timed, (void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME delay, unsigned int *cbmask))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	int retval;
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; RTIME delay; } arg = { dev, subdev, nchans, ldata, cbmask, delay };
	retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_TIMED, &arg).i[LOW];
	memcpy(data, &ldata, sizeof(ldata));
	return retval;
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; RTIME delay; } arg = { dev, subdev, nchans, data, cbmask, delay };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WREAD_TIMED, &arg).i[LOW];
#endif
}

RTAI_PROTO(long, rt_comedi_command_data_write, (void *dev, unsigned int subdev, long nchans, lsampl_t *data))
{
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	lsampl_t ldata[nchans];
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; } arg = { dev, subdev, nchans, ldata };
	memcpy(ldata, data, sizeof(ldata));
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WRITE, &arg).i[LOW];
#else
	struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; } arg = { dev, subdev, nchans, data };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_COMD_DATA_WRITE, &arg).i[LOW];
#endif
}

RTAI_PROTO(int, comedi_data_write, (void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t data))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; unsigned long data; } arg = { dev, subdev, chan, range, aref, data };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DATA_WRITE, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_data_read, (void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; lsampl_t *data; } arg = { dev, subdev, chan, range, aref, data };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DATA_READ, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_data_read_delayed, (void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data, unsigned int nanosec))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; lsampl_t *data; unsigned long nanosec;} arg = { dev, subdev, chan, range, aref, data, nanosec };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DATA_READ_DELAYED, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_data_read_hint, (void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref;} arg = { dev, subdev, chan, range, aref};
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DATA_READ_HINT, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_dio_config, (void *dev, unsigned int subdev, unsigned int chan, unsigned int io))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long io; } arg = { dev, subdev, chan, io };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DIO_CONFIG, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_dio_read, (void *dev, unsigned int subdev, unsigned int chan, unsigned int *val))
{
        struct { void *dev; unsigned long subdev; unsigned long chan; unsigned int *val; } arg = { dev, subdev, chan, val };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DIO_READ, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_dio_write, (void *dev, unsigned int subdev, unsigned int chan, unsigned int val))
{
        struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long val; } arg = { dev, subdev, chan, val };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DIO_WRITE, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_dio_bitfield, (void *dev, unsigned int subdev, unsigned int write_mask, unsigned int *bits))
{
        struct { void *dev; unsigned long subdev; unsigned long write_mask; unsigned int *bits; } arg = { dev, subdev, write_mask, bits };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DIO_BITFIELD, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_get_n_subdevices, (void *dev))
{
	struct { void *dev; } arg = { dev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_N_SUBDEVICES, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_get_version_code, (void *dev))
{
	struct { void *dev;} arg = { dev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_VERSION_CODE, &arg).i[LOW];
}

RTAI_PROTO(char *, rt_comedi_get_driver_name, (void *dev, char *name))
{
        struct { void *dev; char *name; } arg = { dev, name };
	if ((char *)rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_DRIVER_NAME, &arg).v[LOW] == name) {
		return name;
	}
	return NULL;
}

RTAI_PROTO(char *, rt_comedi_get_board_name, (void *dev, char *name))
{
        struct { void *dev; char *name; } arg = { dev, name };
	if ((char *)rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_BOARD_NAME, &arg).v[LOW] == name) {
		return name;
	}
	return NULL;
}

RTAI_PROTO(int, comedi_get_subdevice_type, (void *dev, unsigned int subdev))
{
        struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_SUBDEVICE_TYPE, &arg).i[LOW];
}

RTAI_PROTO(unsigned int, comedi_get_subdevice_flags, (void *dev, unsigned int subdev))
{
	struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_SUBDEVICE_FLAGS, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_find_subdevice_by_type,(void *dev, int type, unsigned int start_subdev))
{
	struct { void *dev; long type; unsigned long start_subdev; } arg = { dev, type, start_subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_FIND_SUBDEVICE_TYPE, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_get_n_channels,(void *dev, unsigned int subdev))
{
        struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_N_CHANNELS, &arg).i[LOW];
}

RTAI_PROTO(lsampl_t, comedi_get_maxdata,(void *dev, unsigned int subdev, unsigned int chan))
{
        struct { void *dev; unsigned long subdev; unsigned long chan;} arg = { dev, subdev, chan };
        return (lsampl_t)rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_MAXDATA, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_get_n_ranges,(void *dev, unsigned int subdev, unsigned int chan))
{
        struct { void *dev; unsigned long subdev; unsigned long chan;} arg = { dev, subdev, chan };
        return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_N_RANGES, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_do_insn, (void *dev, comedi_insn *insn))
{
	if (insn) {
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		comedi_insn linsn = insn[0];
		lsampl_t ldata[linsn.n];
	        struct { void *dev; comedi_insn *insn; } arg = { dev, &linsn };
		int retval;
		memcpy(ldata, linsn.data, sizeof(ldata));
		linsn.data = ldata;
		retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DO_INSN, &arg).i[LOW];
		memcpy(insn[0].data, ldata, sizeof(ldata));
        	return retval;
#else
	        struct { void *dev; comedi_insn *insn; } arg = { dev, insn };
		return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DO_INSN, &arg).i[LOW];
#endif
	}
	return -1;
}

RTAI_PROTO(int, rt_comedi_do_insnlist, (void *dev, comedi_insnlist *ilist))
{
	if (ilist) {
#ifdef CONFIG_RTAI_USE_STACK_ARGS
		comedi_insnlist lilist = ilist[0];
		comedi_insn insns[lilist.n_insns];
	        struct { void *dev; comedi_insnlist *ilist; } arg = { dev, &lilist };
		unsigned int i, maxdata;
		int retval;

		maxdata = 0;
		for (i = 0; i < lilist.n_insns; i++) { 
			if (lilist.insns[i].n > maxdata) {
				maxdata = lilist.insns[i].n;
			}
		} {
		lsampl_t ldata[lilist.n_insns][maxdata];
		for (i = 0; i < lilist.n_insns; i++) { 
			memcpy(ldata[i], lilist.insns[i].data, lilist.insns[i].n*sizeof(lsampl_t));
			insns[i] = lilist.insns[i];
			insns[i].data = ldata[i];
		}
		lilist.insns = insns;
		retval = rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DO_INSN_LIST, &arg).i[LOW];
		for (i = 0; i < (unsigned int)retval; i++) { 
			memcpy(ilist->insns[i].data, ldata[i], insns[i].n*sizeof(lsampl_t));
		} }
        	return retval;
#else
	        struct { void *dev; comedi_insnlist *ilist; } arg = { dev, ilist };
		return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_DO_INSN_LIST, &arg).i[LOW];
#endif
	}
	return -1;
}

RTAI_PROTO(int, rt_comedi_trigger, (void *dev, unsigned int subdev))
{
	struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_TRIGGER, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_poll,(void *dev, unsigned int subdev))
{
	struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_POLL, &arg).i[LOW];
}

RTAI_PROTO(int, comedi_get_krange,(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, comedi_krange *krange))
{
	struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; comedi_krange *krange; } arg = { dev, subdev, chan, range, krange };
	return rtai_lxrt(FUN_COMEDI_LXRT_INDX, COMEDI_LXRT_SIZARG, _KCOMEDI_GET_KRANGE, &arg).i[LOW];
}

#ifdef CONFIG_RTAI_NETRPC

static inline void *RT_comedi_open(unsigned long node, int port, const char *filename)
{
	if (node) {
		struct { const char *filname; unsigned long filnamelen; } arg = { filename, strnlen(filename, COMEDI_NAMELEN) + 1 };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_OPEN, 0), UR1(1, 2), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(SINT, UINT) };
                return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).v[LOW];
	}
	return comedi_open(filename);
}

static inline int RT_comedi_close(unsigned long node, int port, void *dev)
{
	if (node) {
        	struct { void *dev; } arg = { dev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_CLOSE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES1(VADR) };
                return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_close(dev);
}

static inline int RT_comedi_lock(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
      		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_LOCK, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
                return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_lock(dev, subdev);
}

static inline int RT_comedi_unlock(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
      		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_UNLOCK, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
                return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_unlock(dev, subdev);
}

static inline int RT_comedi_cancel(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
      		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_CANCEL, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
                return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_cancel(dev, subdev);
}

static inline int RT_comedi_register_callback(unsigned long node, int port, void *dev, unsigned int subdevice, unsigned int mask, int (*cb)(unsigned int, void *), void *task)
{
	if (node) {
		struct { void *dev; unsigned long subdevice; unsigned long mask; void *cb; void *task; } arg = { dev, subdevice, mask, NULL, NULL };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_REGISTER_CALLBACK, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES5(VADR, UINT, UINT, SINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_register_callback(dev, subdevice, mask, cb, task);
}

static inline long RT_comedi_wait(unsigned long node, int port, unsigned int *cbmask)
{
	if (node) {
		struct { unsigned int *cbmask; } arg = { cbmask };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_WAIT, 0), UW1(1, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES1(SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_wait(cbmask);
}

static inline long RT_comedi_wait_if(unsigned long node, int port, unsigned int *cbmask)
{
	if (node) {
		struct { unsigned int *cbmask; } arg = { cbmask };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_WAIT_IF, 0), UW1(1, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES1(SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_wait_if(cbmask);
}

static inline long RT_comedi_wait_until(unsigned long node, int port, RTIME until, unsigned int *cbmask)
{
	if (node) {
		struct waits { unsigned int *cbmask; RTIME until; } arg = { cbmask, until };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_WAIT_UNTIL, 2), UW1(1, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(SINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_wait_until(nano2count(until), cbmask);
}

static inline long RT_comedi_wait_timed(unsigned long node, int port, RTIME delay, unsigned int *cbmask)
{
	if (node) {
		struct waits { unsigned int *cbmask; RTIME delay; } arg = { cbmask, delay };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_WAIT_TIMED, 2), UW1(1, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(SINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_wait_timed(nano2count(delay), cbmask);
}

#define set_cmd_offsets_lens() \
do { \
	lcmd.cmd = *cmd; \
	memcpy(lcmd.cmd.chanlist, cmd->chanlist, cmd->chanlist_len); \
	memcpy(lcmd.cmd.data, cmd->data, cmd->data_len); \
	lcmd.ofstlens.cmd_ofst      = offsetof(struct rpced_cmd, cmd); \
	lcmd.ofstlens.cmd_len       = sizeof(comedi_cmd); \
	lcmd.ofstlens.chanlist_ofst = offsetof(struct rpced_cmd, chanlist); \
	lcmd.ofstlens.chanlist_len  = cmd->chanlist_len; \
	lcmd.ofstlens.data_ofst     = offsetof(struct rpced_cmd, data); \
	lcmd.ofstlens.data_len      = cmd->data_len;  \
} while (0)

#define MAX_CHANLIST_LEN 64
#define MAX_DATA_LEN     64

static inline int _RT_comedi_command(unsigned long node, int port, void *dev, comedi_cmd *cmd, long test)
{
#ifdef __cplusplus
	if (cmd->chanlist_len > MAX_CHANLIST_LEN || cmd->data_len > MAX_DATA_LEN) {
		return -1;
	}
#endif
	if (node) {
#ifdef __cplusplus
		struct rpced_cmd { struct cmds_ofstlens ofstlens; comedi_cmd cmd; unsigned int chanlist[MAX_CHANLIST_LEN]; short data[MAX_DATA_LEN]; } lcmd;
#else
		struct rpced_cmd { struct cmds_ofstlens ofstlens; comedi_cmd cmd; unsigned int chanlist[cmd->chanlist_len]; short data[cmd->data_len]; } lcmd;
#endif
        	struct { void *dev; void *lcmd; long test; long cmdlen; } arg = { dev, &lcmd, test, sizeof(struct rpced_cmd) };
		set_cmd_offsets_lens();
		memcpy(lcmd.chanlist, cmd->chanlist, lcmd.ofstlens.chanlist_len*sizeof(unsigned int)); 
		memcpy(lcmd.data, cmd->data, lcmd.ofstlens.data_len*sizeof(short)); 
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_COMMAND, 0), UR1(2, 4), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES4(VADR, SINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_command(dev, cmd);
}

static inline int RT_comedi_command(unsigned long node, int port, void *dev, comedi_cmd *cmd)
{
	return _RT_comedi_command(node, port, dev, cmd, 0);
}

static inline int RT_comedi_command_test(unsigned long node, int port, void *dev, comedi_cmd *cmd)
{
	return _RT_comedi_command(node, port, dev, cmd, 1);
}

static inline long RT_comedi_command_data_read(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data)
{
	if (node) {
		struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned long datalen; } arg = { dev, subdev, nchans, data, nchans*sizeof(lsampl_t) };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_COMD_DATA_READ, 0), UW1(4, 5), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES5(VADR, UINT, UINT, SINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_read(dev, subdev, nchans, data);
}

static inline long RT_comedi_command_data_write(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data)
{
	if (node) {
		struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned long datalen; } arg = { dev, subdev, nchans, data, nchans*sizeof(lsampl_t) };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_COMD_DATA_WRITE, 0), UR1(4, 5), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES5(VADR, UINT, UINT, SINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_write(dev, subdev, nchans, data);
}

static inline int RT_comedi_data_write(unsigned long node, long port, void *dev, unsigned long subdev, unsigned long chan, unsigned long range, unsigned long aref, long data)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; long data; } arg = { dev, subdev, chan, range, aref, data };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DATA_WRITE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES6(VADR, UINT, UINT, UINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_data_write(dev, subdev, chan, range, aref, data);
}

static inline int RT_comedi_data_read(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; lsampl_t *data; } arg = { dev, subdev, chan, range, aref, data };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DATA_READ, 0), UW1(6, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES6(VADR, UINT, UINT, UINT, UINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_data_read(dev, subdev, chan, range, aref, data);
}

static inline int RT_comedi_data_read_delayed(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data, unsigned int nanosec)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; lsampl_t *data; unsigned long nanosec; } arg = { dev, subdev, chan, range, aref, data, nanosec };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DATA_READ_DELAYED, 0), UW1(6, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES7(VADR, UINT, UINT, UINT, UINT, SINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_data_read_delayed(dev, subdev, chan, range, aref, data, nanosec);
}

static inline int RT_comedi_data_read_hint(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; unsigned long aref; } arg = { dev, subdev, chan, range, aref };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DATA_READ_HINT, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES5(VADR, UINT, UINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_data_read_hint(dev, subdev, chan, range, aref);
}

static inline int RT_comedi_dio_config(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int io)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long io; } arg = { dev, subdev, chan, io };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DIO_CONFIG, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES4(VADR, UINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_dio_config(dev, subdev, chan, io);
}

static inline int RT_comedi_dio_read(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int *val)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned int *val; } arg = { dev, subdev, chan, val };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DIO_READ, 0), UW1(4, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES5(VADR, UINT, UINT, SINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_dio_read(dev, subdev, chan, val);
}

static inline int RT_comedi_dio_write(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int val)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long val; } arg = { dev, subdev, chan, val };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DIO_WRITE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES4(VADR, UINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_dio_write(dev, subdev, chan, val);
}

static inline int RT_comedi_dio_bitfield(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int write_mask, unsigned int *bits)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long write_mask; unsigned int *bits; } arg = { dev, subdev, write_mask, bits };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_DIO_BITFIELD, 0), UR1(4, 0) | UW1(4, 0), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES4(VADR, UINT, UINT, SINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_dio_bitfield(dev, subdev, write_mask, bits);
}

static inline int RT_comedi_get_n_subdevices(unsigned long node, int port, void *dev)
{
	if (node) {
		struct { void *dev; } arg = { dev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_N_SUBDEVICES, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_n_subdevices(dev);
}

static inline int RT_comedi_get_version_code(unsigned long node, int port, void *dev)
{
	if (node) {
		struct { void *dev; } arg = { dev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_VERSION_CODE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES1(VADR) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_version_code(dev);
}

static inline char *RT_comedi_get_driver_name(unsigned long node, int port, void *dev, char *name)
{
	if (node) {
		struct { void *dev; char *name; long namelen; } arg = { dev, name, COMEDI_NAMELEN };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_DRIVER_NAME, 0), UW1(2, 3), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES3(VADR, SINT, UINT) };
		return (char *)rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).v[LOW];
	}
	return rt_comedi_get_driver_name(dev, name);
}

static inline char *RT_comedi_get_board_name(unsigned long node, int port, void *dev, char *name)
{
	if (node) {
		struct { void *dev; char *name; long namelen; } arg = { dev, name, COMEDI_NAMELEN };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_BOARD_NAME, 0), UW1(2, 3), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES3(VADR, SINT, UINT) };
		return (char *)rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).v[LOW];
	}
	return rt_comedi_get_board_name(dev, name);
}

static inline int RT_comedi_get_subdevice_type(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_SUBDEVICE_TYPE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_subdevice_type(dev, subdev);
}

static inline int RT_comedi_get_subdevice_flags(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_SUBDEVICE_FLAGS, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_subdevice_flags(dev, subdev);
}

static inline int RT_comedi_find_subdevice_by_type(unsigned long node, int port, void *dev, int type, unsigned int start_subdev)
{
	if (node) {
		struct { void *dev; long type; unsigned long start_subdev; } arg = { dev, type, start_subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_FIND_SUBDEVICE_TYPE, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_find_subdevice_by_type(dev, type, start_subdev);
}

static inline int RT_comedi_get_n_channels(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_N_CHANNELS, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_n_channels(dev, subdev);
}

static inline lsampl_t RT_comedi_get_maxdata(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; } arg = { dev, subdev, chan };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_MAXDATA, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return (lsampl_t)rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_maxdata(dev, subdev, chan);
}

static inline int RT_comedi_get_n_ranges(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; } arg = { dev, subdev, chan };
                struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_N_RANGES, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES3(VADR, UINT, UINT) };
		return (lsampl_t)rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_maxdata(dev, subdev, chan);
}

#define set_insn_offsets_lens() \
do { \
	linsns.ofstlens.n_ofst      = offsetof(comedi_insn, n); \
	linsns.ofstlens.subdev_ofst = offsetof(comedi_insn, subdev); \
	linsns.ofstlens.chanspec_ofst = offsetof(comedi_insn, chanspec);\
/*	linsns.ofstlens.insn_len    = (void *)&insns[1] - (void *)&insns[0]; */\
	linsns.ofstlens.insn_len    = (char *)&insns[1] - (char *)&insns[0]; \
	linsns.ofstlens.insns_ofst  = offsetof(struct rpced_insns, insns); \
	linsns.ofstlens.data_ofsts  = offsetof(struct rpced_insns, data_ofsts);\
	linsns.ofstlens.data_ofst   = offsetof(struct rpced_insns, data); \
} while (0)

#define MAX_N_INSNS 64
#define MAX_MAXDAT  64

static inline int _RT_comedi_do_insnlist(unsigned long node, int port, void *dev, long n_insns, comedi_insn *insns)
{
	if (n_insns > 0 && insns) {
		int i, maxdata;
		maxdata = 0;
		for (maxdata = i = 0; i < n_insns; i++) { 
			maxdata += insns[i].n;
		} 
#ifdef __cplusplus
		if (n_insns > MAX_N_INSNS || maxdata > MAX_MAXDAT) {
			return -1;
		} 
#endif
		do {
#ifdef __cplusplus
			struct rpced_insns { struct insns_ofstlens ofstlens; comedi_insn insns[MAX_N_INSNS]; unsigned int data_ofsts[MAX_N_INSNS]; lsampl_t data[MAX_MAXDAT]; } linsns;
#else	
			struct rpced_insns { struct insns_ofstlens ofstlens; comedi_insn insns[n_insns]; unsigned int data_ofsts[n_insns]; lsampl_t data[maxdata]; } linsns;
#endif	
			struct { void *dev; long n_insns; void *linsns; unsigned long linsns_len; } arg = { dev, n_insns, &linsns, sizeof(linsns) };
			struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_DO_INSN_LIST, 0), UR1(3, 4) | UW1(3, 4), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES4(VADR, UINT, SINT, UINT) };
			int retval, offset;
			for (offset = i = 0; i < n_insns; i++) { 
				linsns.insns[i] = insns[i];
				memcpy(&linsns.data[offset], insns[i].data, insns[i].n*sizeof(lsampl_t));
				linsns.data_ofsts[i] = offset;
				offset += insns[i].n;
			}
			set_insn_offsets_lens();
			retval = rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
			for (offset = i = 0; i < retval; i++) { 
				memcpy(insns[i].data, &linsns.data[offset], insns[i].n*sizeof(lsampl_t));
				offset += insns[i].n;
				insns[i].n = linsns.insns[i].n;
			} 
	       		return retval;
		} while(0);
	}
       	return -1;
}

static inline int RT_comedi_do_insn(unsigned long node, int port, void *dev, comedi_insn *insn)
{
	if (node) {
		return _RT_comedi_do_insnlist(node, port, dev, 1, insn);
	}
	return comedi_do_insn(dev, insn);
}

static inline int RT_comedi_do_insnlist(unsigned long node, int port, void *dev, comedi_insnlist *ilist)
{
	if (node) {
		return _RT_comedi_do_insnlist(node, port, dev, ilist->n_insns, ilist->insns);
	}
	return rt_comedi_do_insnlist(dev, ilist);
}

static inline int RT_comedi_trigger(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_TRIGGER, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_trigger(dev, subdev);
}

static inline int RT_comedi_poll(unsigned long node, int port, void *dev, unsigned int subdev)
{
	if (node) {
		struct { void *dev; unsigned long subdev; } arg = { dev, subdev };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_POLL, 0), 0, &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES2(VADR, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_poll(dev, subdev);
}

static inline int RT_comedi_get_krange(unsigned long node, int port, void *dev, unsigned int subdev, unsigned int chan, unsigned int range, comedi_krange *krange)
{
	if (node) {
		struct { void *dev; unsigned long subdev; unsigned long chan; unsigned long range; comedi_krange *krange; long krangelen;} arg = { dev, subdev, chan, range, krange, sizeof(comedi_krange) };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _KCOMEDI_GET_KRANGE, 0), UW1(5, 6), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES6(VADR, UINT, UINT, UINT, UINT, UINT) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return comedi_get_krange(dev, subdev, chan, range, krange);
}

static inline long RT_comedi_command_data_wread(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask)
{
	if (node) {
		struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; unsigned long datalen; RTIME time; } arg = { dev, subdev, nchans << 2, data, cbmask, nchans*sizeof(lsampl_t), 0LL };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_COMD_DATA_WREAD, 0), UR1(5, 0) | UW1(5, 0) | UW2(4, 6), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES7(VADR, UINT, UINT, SINT, SINT, UINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_wread(dev, subdev, nchans, data, cbmask);
}

static inline long RT_comedi_command_data_wread_if(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask)
{
	if (node) {
		struct { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; unsigned long datalen; RTIME time; } arg = { dev, subdev, nchans << 2 | 1, data, cbmask, nchans*sizeof(lsampl_t), 0LL };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_COMD_DATA_WREAD, 0), UR1(5, 0) | UW1(5, 0) | UW2(4, 6), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES7(VADR, UINT, UINT, SINT, SINT, UINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_wread_if(dev, subdev, nchans, data, cbmask);
}

static inline long RT_comedi_command_data_wread_until(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME until, unsigned int *cbmask)
{
	if (node) {
		struct reads { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; unsigned long datalen; RTIME until; } arg = { dev, subdev, nchans << 2 | 2, data, cbmask, nchans*sizeof(lsampl_t), until };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_COMD_DATA_WREAD, 7), UR1(5, 0) | UW1(5, 0) | UW2(4, 6), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES7(VADR, UINT, UINT, SINT, SINT, UINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_wread_until(dev, subdev, nchans, data, nano2count(until), cbmask);
}

static inline long RT_comedi_command_data_wread_timed(unsigned long node, int port, void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME delay, unsigned int *cbmask)
{
	if (node) {
		struct reads { void *dev; unsigned long subdev; long nchans; lsampl_t *data; unsigned int *cbmask; unsigned long datalen; RTIME delay; } arg = { dev, subdev, nchans << 2 | 3, data, cbmask, nchans*sizeof(lsampl_t), delay };
		struct { unsigned long fun; long type; void *args; long argsize; long space; unsigned long partypes; } args = { PACKPORT(port, FUN_COMEDI_LXRT_INDX, _RT_KCOMEDI_COMD_DATA_WREAD, 7), UR1(5, 0) | UW1(5, 0) | UW2(4, 6), &arg, COMEDI_LXRT_SIZARG, 0, PARTYPES7(VADR, UINT, UINT, SINT, SINT, UINT, RTIM) };
		return rtai_lxrt(NET_RPC_IDX, SIZARGS, NETRPC, &args).i[LOW];
	}
	return rt_comedi_command_data_wread_timed(dev, subdev, nchans, data, nano2count(delay), cbmask);
}

#endif

#endif /* #ifdef __KERNEL__ */

/* help macros, for the case one does not (want to) remember insn fields */

#define _BUILD_INSN(icode, instr, subdevice, datap, nd, chan, arange, aref) \
	do { \
		(instr).insn     = icode; \
		(instr).subdev   = subdevice; \
		(instr).n        = nd; \
		(instr).data     = datap; \
		(instr).chanspec = CR_PACK((chan), (arange), (aref)); \
	} while (0)

#define BUILD_AREAD_INSN(insn, subdev, data, nd, chan, arange, aref) \
	_BUILD_INSN(INSN_READ, (insn), subdev, &(data), nd, chan, arange, aref)

#define BUILD_AWRITE_INSN(insn, subdev, data, nd, chan, arange, aref) \
	_BUILD_INSN(INSN_WRITE, (insn), subdev, &(data), nd, chan, arange, aref)

#define BUILD_DIO_INSN(insn, subdev, data) \
	_BUILD_INSN(INSN_BITS, (insn), subdev, &(data), 2, 0, 0, 0)

#define BUILD_TRIG_INSN(insn, subdev, data) \
	_BUILD_INSN(INSN_INTTRIG, (insn), subdev, &(data), 1, 0, 0, 0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef _RTAI_COMEDI_H_ */
