/*
 * Copyright (C) 1999-2017 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,9,0)

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

#else /* LINUX_VERSION_CODE <= KERNEL_VERSION(2,9,0) */

#define PROC_READ_FUN \
	static int rtai_read_proc (char *page, char **start, off_t off, int count, int *eof, void *data)

static inline void *CREATE_PROC_ENTRY(const char *name, umode_t mode, void *parent, const struct file_operations *proc_fops)
{
	return create_proc_entry(name, mode, parent);
}

#define SET_PROC_READ_ENTRY(entry, read_fun)  \
	do { entry->read_proc = read_fun; } while(0)

#define LIMIT (PAGE_SIZE - 80)

// proc print macros - Contributed by: Erwin Rol (erwin@muffin.org)

// macro that holds the local variables that
// we use in the PROC_PRINT_* macros. We have
// this macro so we can add variables with out
// changing the users of this macro, of course
// only when the names don't colide!
#define PROC_PRINT_VARS                                 \
    off_t pos = 0;                                      \
    off_t begin = 0;                                    \
    int len = 0 /* no ";" */            

// macro that prints in the procfs read buffer.
// this macro expects the function arguments to be 
// named as follows.
// static int FOO(char *page, char **start, 
//                off_t off, int count, int *eof, void *data)

#define PROC_PRINT(fmt,args...) \
do {	\
    len += sprintf(page + len , fmt, ##args);           \
    pos += len;                                         \
    if(pos < off) {                                     \
        len = 0;                                        \
        begin = pos;                                    \
    }                                                   \
    if(pos > off + count)                               \
        goto done; \
} while(0)

// macro to leave the read function for a other
// place than at the end. 
#define PROC_PRINT_RETURN                              \
do {	\
    *eof = 1;                                          \
    goto done; \
} while(0)

// macro that should only used ones at the end of the
// read function, to return from a other place in the 
// read function use the PROC_PRINT_RETURN macro. 
#define PROC_PRINT_DONE                                 \
do {	\
        *eof = 1;                                       \
    done:                                               \
        *start = page + (off - begin);                  \
        len -= (off - begin);                           \
        if(len > count)                                 \
            len = count;                                \
        if(len < 0)                                     \
            len = 0;                                    \
        return len; \
} while(0)

#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,9,0) */

// End of proc print macros

#endif  /* !_RTAI_PROC_FS_H */
