/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
 * Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
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

#ifndef _RTAI_PROC_FS_H
#define _RTAI_PROC_FS_H

extern struct proc_dir_entry *rtai_proc_root;

#include <linux/seq_file.h>

#define PROC_READ_FUN(read_fun_name) \
	read_fun_name(struct seq_file *pf, void *v)

#define PROC_READ_OPEN_OPS(rtai_proc_fops, read_fun_name) \
\
static int rtai_proc_open(struct inode *inode, struct file *file) { \
	return single_open(file, read_fun_name, NULL); \
} \
\
static const struct file_operations rtai_proc_fops = { \
	.owner = THIS_MODULE, \
	.open = rtai_proc_open, \
	.read = seq_read, \
	.llseek = seq_lseek, \
	.release = single_release \
};

static inline void *CREATE_PROC_ENTRY(const char *name, umode_t mode, void *parent, const struct file_operations *proc_fops)
{
	return !parent ? proc_mkdir(name, NULL) : proc_create(name, mode, parent, proc_fops);
}

#define SET_PROC_READ_ENTRY(entry, read_fun)  do { } while(0)

#define PROC_PRINT_VARS 

#define PROC_PRINT(fmt, args...)  \
	do { seq_printf(pf, fmt, ##args); } while(0)

#define PROC_PRINT_RETURN do { goto done; } while(0)

#define PROC_PRINT_DONE do { return 0; } while(0)

// End of proc print macros

#endif  /* !_RTAI_PROC_FS_H */
