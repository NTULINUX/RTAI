/**
 * @ingroup fifos
 * @ingroup fifos_ipc
 * @ingroup fifos_sem
 * @file
 *
 * Interface of the @ref fifos "RTAI FIFO module".
 *
 * @note Copyright &copy; 1999-2003 Paolo Mantegazza <mantegazza@aero.polimi.it>
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

#ifndef _RTAI_FIFOS_H
#define _RTAI_FIFOS_H

#include <rtai_types.h>

#define MAX_FIFOS 64

#define RTAI_FIFOS_MAJOR 150

#define RESET		 	1
#define RESIZE		 	2
#define RTF_SUSPEND_TIMED	3
#define OPEN_SIZED	 	4
#define READ_ALL_AT_ONCE 	5
#define READ_TIMED	 	6
#define WRITE_TIMED	 	7
#define RTF_SEM_INIT	 	8
#define RTF_SEM_WAIT	 	9
#define RTF_SEM_TRYWAIT		10
#define RTF_SEM_TIMED_WAIT	11
#define RTF_SEM_POST		12
#define RTF_SEM_DESTROY		13
#define SET_ASYNC_SIG		14
#define EAVESDROP		19
#define OVRWRITE		20
#define READ_IF         	21
#define WRITE_IF        	22
#define RTF_NAMED_CREATE    	23

#define RTF_GET_N_FIFOS		15
#define RTF_GET_FIFO_INFO	16
#define RTF_CREATE_NAMED	17
#define RTF_NAME_LOOKUP		18

#define RTF_NAMELEN 15

struct rt_fifo_info_struct{
    	unsigned int fifo_number;
	unsigned int size;
	unsigned int opncnt;
	int avbs, frbs;
	char name[RTF_NAMELEN+1];
};

struct rt_fifo_get_info_struct{
    	unsigned int fifo;
	unsigned int n;
	struct rt_fifo_info_struct *ptr;
};

#define  FUN_FIFOS_LXRT_INDX 10

#define _CREATE         0
#define _DESTROY        1
#define _PUT            2
#define _GET            3
#define _RESET          4
#define _RESIZE         5
#define _SEM_INIT       6
#define _SEM_DESTRY     7
#define _SEM_POST       8
#define _SEM_TRY        9
#define _CREATE_NAMED  10
#define _GETBY_NAME    11
#define _OVERWRITE     12
#define _PUT_IF        13
#define _GET_IF        14
#define _NAMED_CREATE  15
#define _AVBS          16
#define _FRBS          17

#ifdef __KERNEL__

#include <rtai.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int __rtai_fifos_init(void);

void __rtai_fifos_exit(void);

int rtf_init(void);

typedef int (*rtf_handler_t)(unsigned int fifo, int rw);

/* Attach a handler to an RT-FIFO.
 *
 * Allow function handler to be called when a user process reads or writes to 
 * the FIFO. When the function is called, it is passed the fifo number as the 
 * argument.
 */

int rtf_create_handler(unsigned int fifo,	/* RT-FIFO */
		       void *handler		/* function to be called */);


/**
 * @ingroup fifos_ipc
 * Extended fifo handler.
 *
 * The usage of X_FIFO_HANDLER(handler) allows to install an extended handler,
 * i.e. one prototyped as:
 * @code
 * int (*handler)(unsigned int fifo, int rw);
 * @endcode
 * to allow the user to easily understand if the handler was called at fifo
 * read (@a rw is 'r') or write (rw is 'w').
 */
#define X_FIFO_HANDLER(handler) ((int (*)(unsigned int, int rw))(handler))

/* Create an RT-FIFO.
 * 
 * An RT-FIFO fifo is created with initial size of size.
 * Return value: On success, 0 is returned. On error, -1 is returned.
 */
#undef rtf_create
RTAI_SYSCALL_MODE int rtf_create(unsigned int fifo, int size);

/* Create an RT-FIFO with a name and size.
 *
 * An RT-FIFO is created with a name of name, it will be allocated
 * the first unused minor number and will have a user assigned size.
 * Return value: On success, the allocated minor number is returned.
 *               On error, -errno is returned.
 */

int rtf_named_create(const char *name, int size);

/* Create an RT-FIFO with a name.
 *
 * An RT-FIFO is created with a name of name, it will be allocated
 * the first unused minor number and will have a default size.
 * Return value: On success, the allocated minor number is returned. 
 *               On error, -errno is returned.
 */

RTAI_SYSCALL_MODE int rtf_create_named(const char *name);

/* Look up a named RT-FIFO.
 *
 * Find the RT-FIFO with the name name.
 * Return value: On success, the minor number is returned. 
 *               On error, -errno is returned.
 */

RTAI_SYSCALL_MODE int rtf_getfifobyname(const char *name);

/* Reset an RT-FIFO.
 * 
 * An RT-FIFO fifo is reset by setting its buffer pointers to zero, so
 * that any existing data is discarded and the fifo started anew like at its
 * creation.
 */

RTAI_SYSCALL_MODE int rtf_reset(unsigned int fifo);

/* destroy an RT-FIFO.
 * 
 * Return value: On success, 0 is returned.
 */

RTAI_SYSCALL_MODE int rtf_destroy(unsigned int fifo);


/* Resize an RT-FIFO.
 * 
 * Return value: size is returned on success. On error, a negative value
 * is returned.
 */

RTAI_SYSCALL_MODE int rtf_resize(unsigned int minor, int size);


/* Write to an RT-FIFO.
 *
 * Try to write count bytes to an FIFO. Returns the number of bytes written.
 */

RTAI_SYSCALL_MODE int rtf_put(unsigned int fifo,	/* RT-FIFO */
	    void * buf,		/* buffer address */
	    int count		/* number of bytes to write */);



/* Write to an RT-FIFO, over writing if there is not enough space.
 *
 * Try to write count bytes to an FIFO. Returns 0.
 */

RTAI_SYSCALL_MODE int rtf_ovrwr_put(unsigned int fifo,	/* RT-FIFO */
		  void * buf,		/* buffer address */
		  int count		/* number of bytes to write */);



/* Write atomically to an RT-FIFO.
 *
 * Try to write count bytes in block to an FIFO. Returns the number of bytes
 * written.
 */

extern RTAI_SYSCALL_MODE int rtf_put_if (unsigned int fifo,	/* RT-FIFO */
		void * buf,			/* buffer address */
	       	int count			/* number of bytes to write */);

/* Read from an RT-FIFO.
 *
 * Try to read count bytes from a FIFO. Returns the number of bytes read.
 */

RTAI_SYSCALL_MODE int rtf_get(unsigned int fifo,	/* RT-FIFO */
	    void * buf, 		/* buffer address */
	    int count		/* number of bytes to read */);


/* Atomically read from an RT-FIFO.
 *
 * Try to read count bytes in a block from an FIFO. Returns the number of bytes read.
 */

RTAI_SYSCALL_MODE int rtf_get_if(unsigned int fifo,	/* RT-FIFO */
	    void * buf, 		/* buffer address */
	    int count		/* number of bytes to read */);


/* 
 * Preview the an RT-FIFO content.
 */

int rtf_evdrp(unsigned int fifo,	/* RT-FIFO */
	      void * buf, 		/* buffer address */
	      int count		/* number of bytes to read */);

/* Open an RT-FIFO semaphore.
 *
 */

RTAI_SYSCALL_MODE int rtf_sem_init(unsigned int fifo,	/* RT-FIFO */
		 int value			/* initial semaphore value */);


/* Post to an RT-FIFO semaphore.
 *
 */

RTAI_SYSCALL_MODE int rtf_sem_post(unsigned int fifo	/* RT-FIFO */);


/* Try to acquire an RT-FIFO semaphore.
 *
 */

RTAI_SYSCALL_MODE int rtf_sem_trywait(unsigned int fifo	/* RT-FIFO */);


/* Destroy an RT-FIFO semaphore.
 *
 */

RTAI_SYSCALL_MODE int rtf_sem_destroy(unsigned int fifo	/* RT-FIFO */);

#define rtf_sem_delete rtf_sem_destroy


/* Get an RT-FIFO free bytes in buffer.
 *
 */

RTAI_SYSCALL_MODE int rtf_get_frbs(unsigned int fifo /* RT-FIFO */);

/* Just for compatibility with earlier rtai_fifos releases. No more bh and user
buffers. Fifos are now awakened immediately and buffers > 128K are vmalloced */

#define rtf_create_using_bh(fifo, size, bh_list) rtf_create(fifo, size)
#define rtf_create_using_bh_and_usr_buf(fifo, buf, size, bh_list) rtf_create(fifo, size)
#define rtf_destroy_using_usr_buf(fifo) rtf_destroy(fifo)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#else  /* !__KERNEL__ */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <rtai_lxrt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RTAI_PROTO(int, rtf_create,(unsigned int fifo, int size))
{
	struct { unsigned long fifo, size; } arg = { fifo, size }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _CREATE, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_destroy,(unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _DESTROY, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_put,(unsigned int fifo, const void *buf, int count))
{
	struct { unsigned long fifo; const void *buf; long count; } arg = { fifo, buf, count };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _PUT, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_put_if,(unsigned int fifo, const void *buf, int count))
{
	struct { unsigned long fifo; const void *buf; long count; } arg = { fifo, buf, count };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _PUT_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_get,(unsigned int fifo, void *buf, int count))
{
	struct { unsigned long fifo; void *buf; long count; } arg = { fifo, buf, count };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _GET, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_get_if,(unsigned int fifo, void *buf, int count))
{
	struct { unsigned long fifo; void *buf; long count; } arg = { fifo, buf, count };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _GET_IF, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_get_avbs, (unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _AVBS, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_get_frbs, (unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _FRBS, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_reset_lxrt,(unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _RESET, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_resize_lxrt,(unsigned int fifo, int size))
{
	struct { unsigned long fifo, size; } arg = { fifo, size }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _RESIZE, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_sem_init_lxrt,(unsigned int fifo, int value))
{
	struct { unsigned long fifo, value; } arg = { fifo, value }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _SEM_INIT, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_sem_post_lxrt,(unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _SEM_POST, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_sem_trywait_lxrt,(unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _SEM_TRY, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_sem_destroy_lxrt,(unsigned int fifo))
{
	struct { unsigned long fifo; } arg = { fifo }; 
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _SEM_DESTRY, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_named_create_lxrt,(const char *name, int size))
{
	int len;
	char lname[len = strlen(name)];
	struct { char * name; long size; } arg = { lname, size };
	strncpy(lname, name, len);
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _NAMED_CREATE, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_create_named_lxrt,(const char *name))
{
	int len;
	char lname[len = strlen(name)];
	struct { char * name; } arg = { lname };
	strncpy(lname, name, len);
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _CREATE_NAMED, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_getfifobyname_lxrt,(const char *name))
{
	int len;
	char lname[len = strlen(name)];
	struct { char * name; } arg = { lname };
	strncpy(lname, name, len);
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _GETBY_NAME, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_ovrwr_put,(unsigned int fifo, const void *buf, int count))
{
	struct { unsigned long fifo; const void *buf; long count; } arg = { fifo, buf, count };
	return rtai_lxrt(FUN_FIFOS_LXRT_INDX, SIZARG, _OVERWRITE, &arg).i[LOW];
}

RTAI_PROTO(int, rtf_reset,(int fd))
{
	int ret = ioctl(fd, RESET);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_resize,(int fd, int size))
{
	int ret = ioctl(fd, RESIZE, size);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Suspend a process for some time
 *
 * rtf_suspend_timed suspends a Linux process according to @a delay.
 *
 * @param fd is the file descriptor returned at fifo open, rtf_suspend_timed
 * needs a fifo support.
 * @param ms_delay is the timeout time in milliseconds.
 *
 * @note The standard, clumsy, way to achieve the same result is to use select
 * with null file arguments, for long sleeps, with seconds resolution, sleep is
 * also available.
 */
RTAI_PROTO(int, rtf_suspend_timed,(int fd, int ms_delay))
{
	int ret = ioctl(fd, RTF_SUSPEND_TIMED, ms_delay);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Create a real-time FIFO
 *
 * rtf_open_sized is the equivalent of rtf_create() in user space; it creates a
 * real-time fifo (RT-FIFO) of initial size @a size.
 *
 * @param size is the requested size for the fifo. 
 * 
 * The RT-FIFO is a character based mechanism to communicate among real-time
 * tasks and ordinary Linux processes. The rtf_* functions are used by the
 * real-time tasks; Linux processes use standard character device access
 * functions such as read, write, and select.
 *
 * If this function finds an existing fifo of lower size it resizes it to the
 * larger new size. Note that the same condition apply to the standard Linux
 * device open, except that when it does not find any already existing fifo it
 * creates it with a default size of 1K bytes.
 *
 * It must be remarked that practically any fifo size can be asked for. In
 * fact if @a size is within the constraint allowed by kmalloc such a function
 * is used, otherwise vmalloc is called, thus allowing any size that can fit
 * into the available core memory.
 *
 * Multiple calls of this function are allowed, a counter is kept internally to
 * track their number, and avoid destroying/closing a fifo that is still used.
 *
 * @return the usual Unix file descriptor on succes, to be used in standard reads
 * and writes. 
 * @retval -ENOMEM if the necessary size could not be allocated for the RT-FIFO.
 *
 * @note In user space, the standard UNIX open acts like rtf_open_sized with a
 * default 1K size.
 */
RTAI_PROTO(int, rtf_open_sized,(const char *dev, int perm, int size))
{
	int fd;

	if ((fd = open(dev, perm)) < 0) { 
		return -errno;
	}
	if (ioctl(fd, RESIZE, size) < 0) {
	    	close(fd);
		return -errno;
	}
	return fd; 
}

RTAI_PROTO(int, rtf_evdrp,(int fd, void *buf, int count))
{
	struct { void *buf; long count; } args = { buf, count };
	int ret = ioctl(fd, EAVESDROP, &args);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Read data from FIFO in user space, waiting for all of them
 *
 * rtf_read_all_at_once reads a block of data from a real-time fifo identified
 * by the file descriptor @a fd blocking till all waiting at most @a count
 * bytes are available, whichever option was used at the related device
 * opening.
 *
 * @param fd is the file descriptor returned at fifo open.
 * @param buf points the block of data to be written.
 * @param count is the size in bytes of the buffer.
 *
 * @return the number of bytes read on success.
 * @retval -EINVAL if @a fd refers to a not opened fifo.
 */
RTAI_PROTO(int, rtf_read_all_at_once,(int fd, void *buf, int count))
{
	struct { void *buf; long count; } args = { buf, count };
	int ret = ioctl(fd, READ_ALL_AT_ONCE, &args);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Read data from FIFO in user space, with timeout.
 *
 * rtf_read_timed reads a block of data from a real-time fifo identified by the
 * file descriptor @a fd waiting at most @a delay milliseconds to complete the
 * operation.
 *
 * @param fd is the file descriptor returned at fifo open.
 * @param buf points the block of data to be written.
 * @param count is the size of the block in bytes.
 * @param ms_delay is the timeout time in milliseconds.
 *
 * @return the number of bytes read is returned on success or timeout. Note that
 * this value may be less than @a count if @a count bytes of free space is not
 * available in the fifo or a timeout occured.
 * @retval -EINVAL if @a fd refers to a not opened fifo.
 *
 * @note The standard, clumsy, Unix way to achieve the same result is to use
 * select.
 */
RTAI_PROTO(int, rtf_read_timed,(int fd, void *buf, int count, int ms_delay))
{
	struct { void *buf; long count, delay; } args = { buf, count, ms_delay };
	int ret = ioctl(fd, READ_TIMED, &args);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_read_if,(int fd, void *buf, int count))
{
	struct { void *buf; long count; } args = { buf, count };
	int ret = ioctl(fd, READ_IF, &args);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Write data to FIFO in user space, with timeout.
 *
 * rtf_write_timed writes a block of data to a real-time fifo identified by the
 * file descriptor @a fd waiting at most @æ delay milliseconds to complete the
 * operation.
 *
 * @param fd is the file descriptor returned at fifo open.
 * @param buf points the block of data to be written.
 * @param count is the size of the block in bytes.
 * @param ms_delay is the timeout time in milliseconds.
 *
 * @return the number of bytes written on succes. Note that this value may
 * be less than @a count if @a count bytes of free space is not available in the
 * fifo.
 * @retval -EINVAL if @a fd refers to a not opened fifo.
 *
 * @note The standard, clumsy, Unix way to achieve the same result is to use
 * select.
 */
RTAI_PROTO(int, rtf_write_timed,(int fd, void *buf, int count, int ms_delay))
{
	struct { void *buf; long count, delay; } args = { buf, count, ms_delay };
	int ret = ioctl(fd, WRITE_TIMED, &args);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_overwrite,(int fd, void *buf, int count))
{
	struct { void *buf; long count; } args = { buf, count };
	int ret = ioctl(fd, OVRWRITE, &args);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_write_if,(int fd, void *buf, int count))
{
	struct { void *buf; long count; } args = { buf, count };
	int ret = ioctl(fd, WRITE_IF, &args);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_sem_init,(int fd, int value))
{
	int ret = ioctl(fd, RTF_SEM_INIT, value);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_sem
 * Take a semaphore.
 *
 * rtf_sem_wait waits for a event to be posted (signaled) to a semaphore. The
 * semaphore value is set to tested and set to zero.   If it was one
 * rtf_sem_wait returns immediately. Otherwise the caller process is blocked and
 * queued up in a priority order based on is POSIX real time priority.
 *
 * A process blocked on a semaphore returns when:
 * - the caller task is in the first place of the waiting queue and somebody
 *   issues a rtf_sem_post; 
 * - an error occurs (e.g. the semaphore is destroyed).
 *
 * @param fd is the file descriptor returned by standard UNIX open in user space
 * 
 * Since it is blocking rtf_sem_waitcannot be used both in kernel and user
 * space.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @a fd_fifo  refers to an invalid file descriptor or fifo.
 */
RTAI_PROTO(int, rtf_sem_wait,(int fd))
{
	int ret = ioctl(fd, RTF_SEM_WAIT);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_sem_trywait,(int fd))
{
	int ret = ioctl(fd, RTF_SEM_TRYWAIT);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_sem
 * Wait a semaphore with timeout
 *
 * rtf_sem_timed_wait is a timed version of the standard semaphore wait
 * call. The semaphore value is tested and set to zero. If it was one
 * rtf_sem_timed_wait returns immediately. Otherwise the caller process is
 * blocked and queued up in a priority order based on is POSIX real time
 * priority.
 *
 * A process blocked on a semaphore returns when:
 * - the caller task is in the first place of the waiting queue and somebody
 *   issues a rtf_sem_post;
 * - timeout occurs;
 * - an error occurs (e.g. the semaphore is destroyed).
 *
 * @param fd is the file descriptor returned by standard UNIX open in user
 * space. In case of timeout the semaphore value is set to one before return.
 * @param ms_delay is in milliseconds and is relative to the Linux current time.
 *
 * Since it is blocking rtf_sem_timed_wait cannot be used both in kernel and
 * user space.
 *
 * @retval 0 on success.
 * @retval -EINVAL if fd_fifo refers to an invalid file descriptor or fifo.
 */
RTAI_PROTO(int, rtf_sem_timed_wait,(int fd, int ms_delay))
{
	int ret = ioctl(fd, RTF_SEM_TIMED_WAIT, ms_delay);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_sem_post,(int fd))
{
	int ret = ioctl(fd, RTF_SEM_POST);
	return ret < 0 ? -errno : ret;
}

RTAI_PROTO(int, rtf_sem_destroy,(int fd))
{
	int ret = ioctl(fd, RTF_SEM_DESTROY);
	return ret < 0 ? -errno : ret;
}

/**
 * @ingroup fifos_ipc
 * Activate asynchronous notification of data availability
 *
 * rtf_set_async_sig activate an asynchronous signals to notify data
 * availability by catching a user set signal signum.
 *
 * @param signum is a user chosen signal number to be used, default is SIGIO.
 *
 * @retval -EINVAL if fd refers to a not opened fifo.
 */
RTAI_PROTO(int, rtf_set_async_sig,(int fd, int signum))
{
	int ret = ioctl(fd, SET_ASYNC_SIG, signum);
	return ret < 0 ? -errno : ret;
}

/*
 * Support for named FIFOS : Ian Soanes (ians@zentropix.com)
 * Based on ideas from Stuart Hughes and David Schleef
 */

RTAI_PROTO_ALWAYS_INLINE(char *, rtf_getfifobyminor,(int minor, char *buf, int len))
{
    snprintf(buf,len,CONFIG_RTAI_FIFOS_TEMPLATE,minor);
    return buf;
}

RTAI_PROTO(int, rtf_getfifobyname,(const char *name))
{
    	int fd, minor;
	char nm[RTF_NAMELEN+1];

	if (strlen(name) > RTF_NAMELEN) {
	    	return -1;
	}
	if ((fd = open(rtf_getfifobyminor(0,nm,sizeof(nm)), O_RDONLY)) < 0) {
	    	return -errno;
	}
	strncpy(nm, name, RTF_NAMELEN+1);
	minor = ioctl(fd, RTF_NAME_LOOKUP, nm);
	close(fd);
	return minor < 0 ? -errno : minor;
}

RTAI_PROTO(int, rtf_named_create,(const char *name, int size))
{
	int fd, minor;
	char nm[RTF_NAMELEN+1];

	if (strlen(name) > RTF_NAMELEN) {
		return -1;
	}
	if ((fd = open(rtf_getfifobyminor(0,nm,sizeof(nm)), O_RDONLY)) < 0) {
		return -errno;
	}
	strncpy(nm, name, RTF_NAMELEN+1);
	minor = ioctl(fd, RTF_NAMED_CREATE, nm, size);
	close(fd);
	return minor < 0 ? -errno : minor;
}

RTAI_PROTO(int, rtf_create_named,(const char *name))
{
	int fd, minor;
	char nm[RTF_NAMELEN+1];

	if (strlen(name) > RTF_NAMELEN) {
	    	return -1;
	}
	if ((fd = open(rtf_getfifobyminor(0,nm,sizeof(nm)), O_RDONLY)) < 0) { 
		return -errno;
	}
	strncpy(nm, name, RTF_NAMELEN+1);
	minor = ioctl(fd, RTF_CREATE_NAMED, nm);
	close(fd);
	return minor < 0 ? -errno : minor;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

#endif /* !_RTAI_FIFOS_H */
