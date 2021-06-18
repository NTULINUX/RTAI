/*
 * Copyright (C) 2006 Thomas Leibner (leibner@t-online.de) (first complete writeup)
 * Copyright (C) 2002 David Schleef (ds@schleef.org) (COMEDI master)
 * Copyright (C) 2002 Lorenzo Dozio (dozio@aero.polimi.it) (made it all work)
 * Copyright (C) 2006 Roberto Bucher <roberto.bucher@supsi.ch> (upgrade)
 * Copyright (C) 2002-2017 Paolo Mantegazza (mantegazza@aero.polimi.it) (continued maintenance)
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
 * This is the module to support using COMEDI in hard real time within RTAI.
 */


#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include <rtai_schedcore.h>
#include <rtai_sched.h>
#include <rtai_lxrt.h>
#include <rtai_shm.h>
#include <rtai_msg.h>
#include <rtai_comedi.h>

#define MODULE_NAME "rtai_comedi.o"
MODULE_DESCRIPTION("RTAI LXRT binding for COMEDI kcomedilib");
MODULE_AUTHOR("The Comedi Players");
MODULE_LICENSE("GPL");

#ifdef CONFIG_RTAI_USE_COMEDI_LOCK

#define RTAI_COMEDI_LOCK(dev, subdev) \
	do { comedi_lock(dev, subdev); } while (0)
#define RTAI_COMEDI_UNLOCK(dev, subdev) \
	do { comedi_unlock(dev, subdev); } while (0)

#else

#define RTAI_COMEDI_LOCK(dev, subdev)    do { } while (0)
#define RTAI_COMEDI_UNLOCK(dev, subdev)  do { } while (0)

#endif

#ifdef CONFIG_RTAI_USE_STACK_ARGS
#define RTAI_USE_STACK_ARGS 1
#else
#define RTAI_USE_STACK_ARGS 0
#endif

#define KSPACE(adr)  ((unsigned long)adr > PAGE_OFFSET)

static int rtai_comedi_callback(unsigned int, RT_TASK *) __attribute__ ((__unused__));
static int rtai_comedi_callback(unsigned int val, RT_TASK *task)
{
        if (task->magic == RT_TASK_MAGIC) {
		task->resumsg = val;
		rt_task_resume(task);
	}
	return 0;
}

static RTAI_SYSCALL_MODE void *_comedi_open(const char *filename)
{
	char lfilename[COMEDI_NAMELEN];
	rt_strncpy_from_user(lfilename, filename, COMEDI_NAMELEN - 1);
	return comedi_open(lfilename);
}

static RTAI_SYSCALL_MODE int _comedi_close(void *dev)
{
	return comedi_close(dev);
}

static RTAI_SYSCALL_MODE int _comedi_lock(void *dev, unsigned int subdev)
{
	return comedi_lock(dev, subdev);
}

static RTAI_SYSCALL_MODE int _comedi_unlock(void *dev, unsigned int subdev)
{
	return comedi_unlock(dev, subdev);
}

static RTAI_SYSCALL_MODE int _comedi_cancel(void *dev, unsigned int subdev)
{
	RTAI_COMEDI_LOCK(dev, subdev);
	return comedi_cancel(dev, subdev);
	RTAI_COMEDI_UNLOCK(dev, subdev);
}

RTAI_SYSCALL_MODE int rt_comedi_register_callback(void *dev, unsigned int subdev, unsigned int mask, int (*callback)(unsigned int, void *), void *task)
{
	int retval = 0;
	if (task == NULL) {
		task = rt_whoami();
	}
	((RT_TASK *)task)->resumsg = 0;
	RTAI_COMEDI_LOCK(dev, subdev);
	if (KSPACE(callback)) {
		retval = comedi_register_callback(dev, subdev, mask, (void *)callback, task);
	} else if (callback == NULL) {
		retval = comedi_register_callback(dev, subdev, mask, (void *)rtai_comedi_callback, task);
	}
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_command(void *dev, comedi_cmd *cmd)
{
	int retval;
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	RTAI_COMEDI_LOCK(dev, cmd->subdev);
	retval = comedi_command(dev, (void *)cmd);
	RTAI_COMEDI_UNLOCK(dev, cmd->subdev);
#else
	do {
		comedi_cmd lcmd;
		rt_copy_from_user(&lcmd, cmd, sizeof(comedi_cmd));
		do {
			unsigned int lchanlist[lcmd.chanlist_len];
			sampl_t ldata[lcmd.data_len], *cmd_data = lcmd.data;
			lcmd.chanlist = lchanlist;
			lcmd.data = ldata;
			rt_copy_from_user(lchanlist, lcmd.chanlist, sizeof(lchanlist));
			rt_copy_from_user(ldata, lcmd.data, sizeof(ldata));
			RTAI_COMEDI_LOCK(dev, lcmd.subdev);
			retval = comedi_command(dev, &lcmd);
			RTAI_COMEDI_UNLOCK(dev, lcmd.subdev);
			if (!retval) {
				rt_copy_to_user(cmd_data, ldata, sizeof(ldata));
			} 
		} while (0);
	} while (0);
#endif
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_command_test(void *dev, comedi_cmd *cmd)
{
	int retval;
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	RTAI_COMEDI_LOCK(dev, cmd->subdev);
	retval = comedi_command_test(dev, (void *)cmd);
	RTAI_COMEDI_UNLOCK(dev, cmd->subdev);
#else
	do {
		comedi_cmd lcmd;
		rt_copy_from_user(&lcmd, cmd, sizeof(comedi_cmd));
		do {
			unsigned int lchanlist[lcmd.chanlist_len];
			sampl_t ldata[lcmd.data_len], *cmd_data = lcmd.data;
			lcmd.chanlist = lchanlist;
			lcmd.data = ldata;
			rt_copy_from_user(lchanlist, lcmd.chanlist, sizeof(lchanlist));
			rt_copy_from_user(ldata, lcmd.data, sizeof(ldata));
			RTAI_COMEDI_LOCK(dev, lcmd.subdev);
			retval = comedi_command_test(dev, &lcmd);
			RTAI_COMEDI_UNLOCK(dev, lcmd.subdev);
			if (!retval) {
				rt_copy_to_user(cmd_data, ldata, sizeof(ldata));
			} 
		} while (0);
	} while (0);
#endif
	return retval;
}

static RTAI_SYSCALL_MODE int RT_comedi_command(void *dev, struct cmds_ofstlens *ofstlens, long test)
{
	int retval;
	comedi_cmd cmd[2]; // 2 instances, a safe room for 64 getting into 32
	if (KSPACE(ofstlens) || RTAI_USE_STACK_ARGS) {
		memcpy(cmd, (void *)ofstlens + ofstlens->cmd_ofst, ofstlens->cmd_len);
	} else {
		rt_copy_from_user(cmd, (void *)ofstlens + ofstlens->cmd_ofst, ofstlens->cmd_len);
	}
	cmd[0].chanlist     = (void *)ofstlens + ofstlens->chanlist_ofst;
	cmd[0].chanlist_len = ofstlens->chanlist_len;
	cmd[0].data         = (void *)ofstlens + ofstlens->data_ofst;
	cmd[0].data_len     = ofstlens->data_len;
	retval = test ? _comedi_command_test(dev, cmd) : _comedi_command(dev, cmd);
	return retval;
}

#define WAIT       0
#define WAITIF     1
#define WAITUNTIL  2

static inline int __rt_comedi_wait(RTIME until, unsigned int *cbmask, int waitmode)
{ 
	if (cbmask) {
		long retval;
		RT_TASK *task = _rt_whoami();
		switch (waitmode) {
			case WAIT: 
				retval = rt_task_suspend(task);
				break;
			case WAITIF: 
				retval = rt_task_suspend_if(task);
				break;
			case WAITUNTIL: 
				retval = rt_task_suspend_until(task, until);
				break;
			default: // useless, just to avoid compiler warnings
				return RTE_PERM;
		}
		if (KSPACE(cbmask) || RTAI_USE_STACK_ARGS) {
			cbmask[0] = (unsigned int)task->resumsg;
		} else {
			rt_put_user((unsigned int)task->resumsg, cbmask);
		}
		task->resumsg = 0;
		return retval;
	}
	return RTE_PERM;
}

RTAI_SYSCALL_MODE long rt_comedi_wait(unsigned int *cbmask)
{ 
	return __rt_comedi_wait((RTIME)0, cbmask, WAIT);
}

RTAI_SYSCALL_MODE long rt_comedi_wait_if(unsigned int *cbmask)
{ 
	return __rt_comedi_wait((RTIME)0, cbmask, WAITIF);
}

RTAI_SYSCALL_MODE long _rt_comedi_wait_until(unsigned int *cbmask, RTIME until)
{
	return __rt_comedi_wait(until, cbmask, WAITUNTIL);
}

RTAI_SYSCALL_MODE long _rt_comedi_wait_timed(unsigned int *cbmask, RTIME delay)
{
	return _rt_comedi_wait_until(cbmask, rt_get_time() + delay);
}

RTAI_SYSCALL_MODE long rt_comedi_command_data_read(void *dev, unsigned int subdev, long nchans, lsampl_t *data)
{
	lsampl_t *aibuf;
	int i, ofsti, ofstf, size, kspace = KSPACE(data);
#if 1
	if (comedi_map(dev, subdev, &aibuf)) {
		return RTE_OBJINV;
	}
	if ((i = comedi_get_buffer_contents(dev, subdev)) < nchans) {
		return i;
	}
	size = comedi_get_buffer_size(dev, subdev);
	RTAI_COMEDI_LOCK(dev, subdev);
	ofstf = ofsti = comedi_get_buffer_offset(dev, subdev);
	for (i = 0; i < nchans; i++) {
		if (kspace || RTAI_USE_STACK_ARGS) {
			data[i] = *(aibuf + ofstf % size);
		} else {
			rt_put_user(*(aibuf + ofstf % size), &data[i]);
		}
		ofstf += sizeof(sampl_t);
	}
	comedi_mark_buffer_read(dev, subdev, ofstf - ofsti);
	RTAI_COMEDI_UNLOCK(dev, subdev);
#else
        comedi_device *cdev = (comedi_device *)dev;
	comedi_async *async;

	RTAI_COMEDI_LOCK(dev, subdev);
        if (subdev >= cdev->n_subdevices) {
		return RTE_OBJINV;
        }
        if ((async = (cdev->subdevices + cdev->n_subdevices)->async) == NULL) {
                return RTE_OBJINV;
	}
	aibuf = (sampl_t *)async->prealloc_buf;
	if ((i = comedi_buf_read_n_available(async)) < nchans) {
		return i;
	}
	ofstf = ofsti = async->buf_read_ptr;
	size = async->prealloc_bufsz;
	for (i = 0; i < nchans; i++) {
		data[i] = *(sampl_t *)(aibuf + ofstf % size);
		ofstf += sizeof(sampl_t);
	}
	comedi_buf_read_alloc(async, ofstf - ofsti);
	comedi_buf_read_free(async, ofstf - ofsti);
	RTAI_COMEDI_UNLOCK(dev, subdev);
#endif
	return nchans;
}

static inline int __rt_comedi_command_data_wread(void *dev, unsigned int subdev, long nchans, lsampl_t *data, RTIME until, unsigned int *cbmaskarg, int waitmode)
{
	unsigned int cbmask, mask;
	long retval, kspace;
	if ((kspace = KSPACE(cbmaskarg))) {
		mask = cbmaskarg[0];
	} else {
		rt_get_user(mask, cbmaskarg);
	}
	switch (waitmode) {
		case WAIT: 
			retval = rt_comedi_wait(&cbmask);
			break;
		case WAITIF: 
			retval = rt_comedi_wait_if(&cbmask);
			break;
		case WAITUNTIL: 
			retval = _rt_comedi_wait_until(&cbmask, until);
			break;
		default: // useless, just to avoid compiler warnings
			return RTE_PERM;
	}
	if (!retval && (mask & cbmask)) {
		if (kspace || RTAI_USE_STACK_ARGS) {
			cbmaskarg[0] = cbmask;
		} else {
			rt_put_user(cbmask, cbmaskarg);
		}
		return rt_comedi_command_data_read(dev, subdev, nchans, data);
	}
	return retval;
}

RTAI_SYSCALL_MODE long rt_comedi_command_data_wread(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask)
{
	return __rt_comedi_command_data_wread(dev, subdev, nchans, data, (RTIME)0, cbmask, WAIT);
}

RTAI_SYSCALL_MODE long rt_comedi_command_data_wread_if(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask)
{
	return __rt_comedi_command_data_wread(dev, subdev, nchans, data, (RTIME)0, cbmask, WAITIF);
}

RTAI_SYSCALL_MODE long _rt_comedi_command_data_wread_until(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask, RTIME until)
{
	return __rt_comedi_command_data_wread(dev, subdev, nchans, data, until, cbmask, WAITUNTIL);
}

RTAI_SYSCALL_MODE long _rt_comedi_command_data_wread_timed(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask, RTIME delay)
{
	return _rt_comedi_command_data_wread_until(dev, subdev, nchans, data, cbmask, rt_get_time() + delay);
}

RTAI_SYSCALL_MODE long RT_comedi_command_data_wread(void *dev, unsigned int subdev, long nchans, lsampl_t *data, unsigned int *cbmask, unsigned int datalen, RTIME time)
{
	switch (nchans & 0x3) {
		case 0: 
			return __rt_comedi_command_data_wread(dev, subdev, nchans >> 2, data, (RTIME)0, cbmask, WAIT);
		case 1: 
			return __rt_comedi_command_data_wread(dev, subdev, nchans >> 2, data, (RTIME)0, cbmask, WAITIF);
		case 2: 
			return __rt_comedi_command_data_wread(dev, subdev, nchans >> 2, data, time, cbmask, WAITUNTIL);
		case 3: 
			return _rt_comedi_command_data_wread_until(dev, subdev, nchans >> 2, data, cbmask, rt_get_time() + time);
	}
	return 0;
}

RTAI_SYSCALL_MODE long rt_comedi_command_data_write(void *dev, unsigned int subdev, long nchans, lsampl_t *data)
{
	lsampl_t *aobuf;
	int i, ofsti, ofstf, size, avbs, kspace = KSPACE(data);
	if (comedi_map(dev, subdev, &aobuf)) {
		return RTE_OBJINV;
	}
	size = comedi_get_buffer_size(dev, subdev);
	avbs = comedi_get_buffer_contents(dev, subdev);
	if ((size - avbs) < nchans) {
		return (size - avbs);
	}
	RTAI_COMEDI_LOCK(dev, subdev);
	ofstf = ofsti = (comedi_get_buffer_offset(dev, subdev) + avbs) % size;
	for (i = 0; i < nchans; i++) {
		if (kspace || RTAI_USE_STACK_ARGS) {
			*(aobuf + ofstf % size) = data[i];
		} else {
			rt_get_user(*(aobuf + ofstf % size), &data[i]);
		}
		ofstf += sizeof(sampl_t);
	}
	comedi_mark_buffer_written(dev, subdev, ofstf - ofsti);
#if 0
	if (!avbs) {
		int retval;
		if ((retval = _rt_comedi_trigger(dev, subdev)) < 0) {
			return retval;
		}
	}
#endif
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return nchans;
}

static RTAI_SYSCALL_MODE int _comedi_data_write(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t data)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_data_write(dev, subdev, chan, range, aref, data);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_data_read(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data)
{
	lsampl_t ldata;
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_data_read(dev, subdev, chan, range, aref, &ldata);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	rt_put_user(ldata, data);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_data_read_delayed(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref, lsampl_t *data, unsigned int nanosec)
{
	lsampl_t ldata;
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_data_read_delayed(dev, subdev, chan, range, aref, &ldata, nanosec);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	rt_put_user(ldata, data);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_data_read_hint(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, unsigned int aref)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_data_read_hint(dev, subdev, chan, range, aref);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_dio_config(void *dev, unsigned int subdev, unsigned int chan, unsigned int io)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_dio_config(dev, subdev, chan, io);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_dio_read(void *dev, unsigned int subdev, unsigned int chan, unsigned int *val)
{
	unsigned int lval;
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_dio_read(dev, subdev, chan, &lval);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	rt_put_user(lval, val);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_dio_write(void *dev, unsigned int subdev, unsigned int chan, unsigned int val)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_dio_write(dev, subdev, chan, val);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_dio_bitfield(void *dev, unsigned int subdev, unsigned int write_mask, unsigned int *bits)
{
	unsigned int lbits;
	int retval;
	rt_get_user(lbits, bits);
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_dio_bitfield(dev, subdev, write_mask, &lbits);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	rt_put_user(lbits, bits);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_get_n_subdevices(void *dev)
{
	return comedi_get_n_subdevices(dev);
}

static RTAI_SYSCALL_MODE int _comedi_get_version_code(void *dev)
{
	return comedi_get_version_code(dev);
}

RTAI_SYSCALL_MODE char *rt_comedi_get_driver_name(void *dev, char *name)
{
	const char *kname;
	if ((kname = comedi_get_driver_name((void *)dev)) != 0) {
		if (KSPACE(name)  || RTAI_USE_STACK_ARGS) {
			strncpy(name, kname, COMEDI_NAMELEN);
		} else {
			rt_copy_to_user(name, kname, strnlen(kname, COMEDI_NAMELEN));
		}
		return name;
	};
	return NULL;
}

RTAI_SYSCALL_MODE char *rt_comedi_get_board_name(void *dev, char *name)
{
	const char *kname;
	if ((kname = comedi_get_board_name((void *)dev)) != 0) {
		if (KSPACE(name) || RTAI_USE_STACK_ARGS) {
			strncpy(name, kname, COMEDI_NAMELEN);
		} else {
			rt_copy_to_user(name, kname, strnlen(kname, COMEDI_NAMELEN));
		}
		return name;
	}
	return NULL;
}

static RTAI_SYSCALL_MODE int _comedi_get_subdevice_type(void *dev, unsigned int subdev)
{
	return comedi_get_subdevice_type(dev, subdev);
}

static RTAI_SYSCALL_MODE unsigned int _comedi_get_subdevice_flags(void *dev, unsigned int subdev)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_get_subdevice_flags(dev, subdev);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_find_subdevice_by_type(void *dev, int type, unsigned int start_subdevice)
{
	return comedi_find_subdevice_by_type(dev, type, start_subdevice);
}

static RTAI_SYSCALL_MODE int _comedi_get_n_channels(void *dev, unsigned int subdev)
{
	return comedi_get_n_channels(dev, subdev);
}

static RTAI_SYSCALL_MODE lsampl_t _comedi_get_maxdata(void *dev, unsigned int subdev, unsigned int chan)
{
	return comedi_get_maxdata(dev, subdev, chan);
}

static RTAI_SYSCALL_MODE int _comedi_get_n_ranges(void *dev, unsigned int subdev, unsigned int chan)
{
	return comedi_get_n_ranges(dev, subdev, chan);
}

static RTAI_SYSCALL_MODE int _comedi_do_insn(void *dev, comedi_insn *insn)
{
	int retval;
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	RTAI_COMEDI_LOCK(dev, insn->subdev);
	retval = comedi_do_insn(dev, insn);
	RTAI_COMEDI_UNLOCK(dev, insn->subdev);
#else
	do {
		comedi_insn linsn;
		rt_copy_from_user(&linsn, insn, sizeof(comedi_insn));
		do {
			lsampl_t ldata[linsn.n], *insn_data = linsn.data;
			rt_copy_from_user(ldata, insn_data, sizeof(ldata));
			linsn.data = ldata;
			RTAI_COMEDI_LOCK(dev, linsn.subdev);
			retval = comedi_do_insn(dev, &linsn);
			RTAI_COMEDI_UNLOCK(dev, linsn.subdev);
			rt_copy_to_user(insn_data, ldata, sizeof(ldata));
		} while (0);
	} while (0);
#endif
	return retval;
}

RTAI_SYSCALL_MODE int rt_comedi_do_insnlist(void *dev, comedi_insnlist *ilist)
{
	int i, n;
#ifdef CONFIG_RTAI_USE_STACK_ARGS
	for (i = 0; i < ilist->n_insns; i ++) {
		RTAI_COMEDI_LOCK(dev, ilist->insns[i].subdev);
		n = comedi_do_insn(dev, &ilist->insns[i]);
		RTAI_COMEDI_UNLOCK(dev, ilist->insns[i].subdev);
		if (n < 0) break;
	}
#else
	comedi_insnlist lilist = { 0, NULL };
	rt_copy_from_user(&lilist, ilist, sizeof(comedi_insnlist));
	do {
		comedi_insn linsns[lilist.n_insns];
		rt_copy_from_user(linsns, lilist.insns, sizeof(linsns));
		for (i = 0; i < lilist.n_insns; i ++) {
			lsampl_t ldata[linsns[i].n], *insns_data = linsns[i].data;
			rt_copy_from_user(ldata, insns_data, sizeof(ldata));
			linsns[i].data = ldata;
			RTAI_COMEDI_LOCK(dev, linsns[i].subdev);
			n = comedi_do_insn(dev, &linsns[i]);
			RTAI_COMEDI_UNLOCK(dev, linsns[i].subdev);
			if (n < 0) break;
			rt_copy_to_user(insns_data, ldata, sizeof(ldata));
		}
	} while (0);
#endif
	return i;
}

RTAI_SYSCALL_MODE int RT_comedi_do_insnlist(void *dev, long n_insns, struct insns_ofstlens *ofstlens)
{
#define RT_INSN(offset) (*((unsigned int *)(insns + ofstlens->offset)))
#define RT_INSN_ADR(offset) ((void *)ofstlens + ofstlens->offset)
	int i;
	void *insns              = RT_INSN_ADR(insns_ofst);
	unsigned int *data_ofsts = RT_INSN_ADR(data_ofsts);
	lsampl_t *data           = RT_INSN_ADR(data_ofst);
	comedi_insn insn[2]; // 2 instances, a safe room for 64 getting into 32
	for (i = 0; i < n_insns; i ++) {

		if (KSPACE(ofstlens) || RTAI_USE_STACK_ARGS) {
			memcpy(insn, insns, ofstlens->insn_len);	
		} else {
			rt_copy_from_user(insn, insns, ofstlens->insn_len);	
		}
		insn[0].data     = data + data_ofsts[i];
		insn[0].subdev   = RT_INSN(subdev_ofst);
		insn[0].chanspec = RT_INSN(chanspec_ofst);
		if ((RT_INSN(n_ofst) = comedi_do_insn(dev, insn)) < 0) {
			break;
		}
		insns += ofstlens->insn_len;
	}
	return i;
}

static inline int _rt_comedi_trigger(void *dev, unsigned int subdev)
{
        comedi_insn insn;
        lsampl_t data = 0;
        insn.insn   = INSN_INTTRIG;
        insn.subdev = subdev;
        insn.n      = 1;
        insn.data   = &data;
        return comedi_do_insn(dev, &insn);
}

RTAI_SYSCALL_MODE int rt_comedi_trigger(void *dev, unsigned int subdev)
{
        return _rt_comedi_trigger(dev, subdev);
}

static RTAI_SYSCALL_MODE int _comedi_poll(void *dev, unsigned int subdev)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_poll(dev, subdev);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

/* DEPRECATED FUNCTION
static RTAI_SYSCALL_MODE int _comedi_get_rangetype(unsigned int minor, unsigned int subdevice, unsigned int chan)
{
	return comedi_get_rangetype(minor, subdevice, chan);
}
*/

static RTAI_SYSCALL_MODE int _comedi_get_krange(void *dev, unsigned int subdev, unsigned int chan, unsigned int range, comedi_krange *krange)
{
        int retval;
        comedi_krange lkrange;
	retval = comedi_get_krange(dev, subdev, chan, range, &lkrange);
	rt_copy_from_user(krange, &lkrange, sizeof(lkrange));
        return retval;
}

static RTAI_SYSCALL_MODE int _comedi_get_buf_head_pos(void *dev, unsigned int subdev)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_get_buf_head_pos(dev, subdev);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_set_user_int_count(void *dev, unsigned int subdev, unsigned int buf_user_count)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_set_user_int_count(dev, subdev, buf_user_count);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_map(void *dev, unsigned int subdev, void *ptr)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_map(dev, subdev, ptr);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static RTAI_SYSCALL_MODE int _comedi_unmap(void *dev, unsigned int subdev)
{
	int retval;
	RTAI_COMEDI_LOCK(dev, subdev);
	retval = comedi_unmap(dev, subdev);
	RTAI_COMEDI_UNLOCK(dev, subdev);
	return retval;
}

static struct rt_fun_entry rtai_comedi_fun[] = {
  [_KCOMEDI_OPEN]                  = { 0, _comedi_open }
 ,[_KCOMEDI_CLOSE]                 = { 0, _comedi_close }
 ,[_KCOMEDI_LOCK]                  = { 0, _comedi_lock }
 ,[_KCOMEDI_UNLOCK]                = { 0, _comedi_unlock }
 ,[_KCOMEDI_CANCEL]                = { 0, _comedi_cancel }
 ,[_KCOMEDI_REGISTER_CALLBACK]     = { 0, rt_comedi_register_callback }
 ,[_KCOMEDI_COMMAND]               = { 0, _comedi_command }
 ,[_KCOMEDI_COMMAND_TEST]          = { 0, _comedi_command_test }
 ,[_KCOMEDI_TRIGGER    ]           = { 0, rt_comedi_trigger }
 ,[_KCOMEDI_DATA_WRITE]            = { 0, _comedi_data_write}
 ,[_KCOMEDI_DATA_READ]             = { 0, _comedi_data_read }
 ,[_KCOMEDI_DATA_READ_DELAYED]     = { 0, _comedi_data_read_delayed }       
 ,[_KCOMEDI_DATA_READ_HINT]        = { 0, _comedi_data_read_hint }          
 ,[_KCOMEDI_DIO_CONFIG]            = { 0, _comedi_dio_config }
 ,[_KCOMEDI_DIO_READ]              = { 0, _comedi_dio_read }
 ,[_KCOMEDI_DIO_WRITE]             = { 0, _comedi_dio_write }
 ,[_KCOMEDI_DIO_BITFIELD]          = { 0, _comedi_dio_bitfield }
 ,[_KCOMEDI_GET_N_SUBDEVICES]      = { 0, _comedi_get_n_subdevices }
 ,[_KCOMEDI_GET_VERSION_CODE]      = { 0, _comedi_get_version_code }
 ,[_KCOMEDI_GET_DRIVER_NAME]       = { 0, rt_comedi_get_driver_name }
 ,[_KCOMEDI_GET_BOARD_NAME]        = { 0, rt_comedi_get_board_name }
 ,[_KCOMEDI_GET_SUBDEVICE_TYPE]    = { 0, _comedi_get_subdevice_type }
 ,[_KCOMEDI_FIND_SUBDEVICE_TYPE]   = { 0, _comedi_find_subdevice_by_type }
 ,[_KCOMEDI_GET_N_CHANNELS]        = { 0, _comedi_get_n_channels }
 ,[_KCOMEDI_GET_MAXDATA]           = { 0, _comedi_get_maxdata }
 ,[_KCOMEDI_GET_N_RANGES]          = { 0, _comedi_get_n_ranges }
 ,[_KCOMEDI_DO_INSN]               = { 0, _comedi_do_insn }
 ,[_KCOMEDI_DO_INSN_LIST]          = { 0, rt_comedi_do_insnlist }
 ,[_KCOMEDI_POLL]                  = { 0, _comedi_poll }
/*
 ,[_KCOMEDI_GET_RANGETYPE]         = { 0, _comedi_get_rangetype }
*/
 ,[_KCOMEDI_GET_SUBDEVICE_FLAGS]   = { 0, _comedi_get_subdevice_flags }
 ,[_KCOMEDI_GET_KRANGE]            = { 0, _comedi_get_krange }
 ,[_KCOMEDI_GET_BUF_HEAD_POS]      = { 0, _comedi_get_buf_head_pos }
 ,[_KCOMEDI_SET_USER_INT_COUNT]    = { 0, _comedi_set_user_int_count }
 ,[_KCOMEDI_MAP]                   = { 0, _comedi_map }
 ,[_KCOMEDI_UNMAP]                 = { 0, _comedi_unmap }
 ,[_KCOMEDI_WAIT]                  = { 1, rt_comedi_wait }
 ,[_KCOMEDI_WAIT_IF]               = { 0, rt_comedi_wait_if }
 ,[_KCOMEDI_WAIT_UNTIL]            = { 1, _rt_comedi_wait_until }
 ,[_KCOMEDI_WAIT_TIMED]            = { 1, _rt_comedi_wait_timed }
 ,[_KCOMEDI_COMD_DATA_READ]        = { 0, rt_comedi_command_data_read }
 ,[_KCOMEDI_COMD_DATA_WREAD]       = { 1, rt_comedi_command_data_wread }
 ,[_KCOMEDI_COMD_DATA_WREAD_IF]    = { 0, rt_comedi_command_data_wread_if }
 ,[_KCOMEDI_COMD_DATA_WREAD_UNTIL] = { 1, _rt_comedi_command_data_wread_until }
 ,[_KCOMEDI_COMD_DATA_WREAD_TIMED] = { 1, _rt_comedi_command_data_wread_timed }
 ,[_KCOMEDI_COMD_DATA_WRITE]       = { 0, rt_comedi_command_data_write }
 ,[_RT_KCOMEDI_COMMAND]            = { 0, RT_comedi_command }
 ,[_RT_KCOMEDI_DO_INSN_LIST]       = { 0, RT_comedi_do_insnlist }
 ,[_RT_KCOMEDI_COMD_DATA_WREAD]    = { 0, RT_comedi_command_data_wread }
};

#ifdef CONFIG_RTAI_USE_LINUX_COMEDI

extern void *rt_comedi_request_irq;
extern void *rt_comedi_release_irq;
extern void *rt_comedi_busy_sleep;

#define RTAI_NR_IRQS  IPIPE_NR_XIRQS
static int (*comedi_irq_handler_p[RTAI_NR_IRQS])(unsigned int, void *);

static int comedi_irq_handler(unsigned int irq, void *dev_id)
{
	comedi_irq_handler_p[irq](irq, dev_id);
	rt_enable_irq(irq);
	return IRQ_HANDLED;
}

static int comedi_request_irq(unsigned int irq, int (*handler)(unsigned int irq, void *dev_id), unsigned long flags, const char *name, void *dev)
{
	int retval;
	if (comedi_irq_handler_p[irq]) {
		return -EBUSY;
	}
	if ((retval = rt_request_irq(irq, comedi_irq_handler, dev, 0))) {
		return retval;
	}
	comedi_irq_handler_p[irq] = handler;
	rt_startup_irq(irq);
	return 0;
}

static void comedi_release_irq(unsigned int irq, void *dev)
{
	rt_shutdown_irq(irq);
	rt_release_irq(irq);
}

static int us2tsc;
void comedi_busy_sleep(int us)
{
	RTIME break_time;
	break_time = rtai_rdtsc() + us*us2tsc;
	while (rtai_rdtsc() < break_time);
}

#endif /* CONFIG_RTAI_USE_LINUX_COMEDI) */

int __rtai_comedi_init(void)
{
	if( set_rt_fun_ext_index(rtai_comedi_fun, FUN_COMEDI_LXRT_INDX) ) {
		printk("Recompile your module with a different index\n");
		return -EACCES;
	}
#ifdef CONFIG_RTAI_USE_LINUX_COMEDI
	us2tsc = tuned.cpu_freq/1000000;
	rt_comedi_request_irq = comedi_request_irq;
	rt_comedi_release_irq = comedi_release_irq;
	rt_comedi_busy_sleep  = comedi_busy_sleep;
#endif
	return 0;
}

void __rtai_comedi_exit(void)
{
#ifdef CONFIG_RTAI_USE_LINUX_COMEDI
	int irq;
	for (irq = 0; irq < RTAI_NR_IRQS; irq++) {
		if (comedi_irq_handler_p[irq]) {
			comedi_release_irq(irq, NULL);
		}
	}
	rt_comedi_request_irq = rt_request_irq;
	rt_comedi_release_irq = rt_release_irq;
	rt_comedi_busy_sleep  = __udelay;
#endif
	reset_rt_fun_ext_index(rtai_comedi_fun, FUN_COMEDI_LXRT_INDX);
}

module_init(__rtai_comedi_init);
module_exit(__rtai_comedi_exit);

EXPORT_SYMBOL(rt_comedi_wait);
EXPORT_SYMBOL(rt_comedi_wait_if);
EXPORT_SYMBOL(_rt_comedi_wait_until);
EXPORT_SYMBOL(_rt_comedi_wait_timed);
EXPORT_SYMBOL(rt_comedi_get_driver_name);
EXPORT_SYMBOL(rt_comedi_get_board_name);
EXPORT_SYMBOL(rt_comedi_register_callback);
EXPORT_SYMBOL(rt_comedi_command_data_read);
EXPORT_SYMBOL(rt_comedi_command_data_wread);
EXPORT_SYMBOL(rt_comedi_command_data_wread_if);
EXPORT_SYMBOL(_rt_comedi_command_data_wread_until);
EXPORT_SYMBOL(_rt_comedi_command_data_wread_timed);
EXPORT_SYMBOL(rt_comedi_do_insnlist);
EXPORT_SYMBOL(rt_comedi_trigger);
EXPORT_SYMBOL(rt_comedi_command_data_write);
