# Copyright (C) 2005-2017 The RTAI project
# Copyright (C) 2019 Alec Ari <neotheuser@ymail.com>
# This [file] is free software; the RTAI project
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.69)

AC_INIT([RTAI], [5.2.1], [neotheuser@ymail.com])
AC_CONFIG_AUX_DIR(scripts/autoconf)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS(rtai_config.h)

CONFIG_RTAI_VERSION_MAJOR=5
CONFIG_RTAI_VERSION_MINOR=2
CONFIG_RTAI_REVISION_LEVEL=1

AC_PROG_INSTALL
LT_INIT
AC_PROG_CC
CC="gcc"
CFLAGS=""

dnl Only GCC is supported.
AC_MSG_CHECKING([whether gcc version is sufficient])
GCC_VERSION=`gcc -dumpversion`
if test $? -eq 0; then
	GCC_VERSION_MAJOR=`echo $GCC_VERSION | cut -d. -f1`
	GCC_VERSION_MINOR=`echo $GCC_VERSION | cut -d. -f2`
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([GCC is required.])
fi
if test $GCC_VERSION_MAJOR -lt 4 -o $GCC_VERSION_MAJOR -eq 4 -a $GCC_VERSION_MINOR -lt 6 ; then
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([GCC version 4.6.0 or later is required.])
else
	AC_MSG_RESULT([yes])
fi

AC_DEFINE_UNQUOTED(CONFIG_RTAI_BUILD_STRING,"$build",[Build system alias])
RTAI_BUILD_STRING="$build"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_HOST_STRING,"$host",[Host system alias])
RTAI_HOST_STRING="$host"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_COMPILER,"`gcc -v 2>&1 | tail -n 1`",[Compiler])

dnl Only support X86 and AMD64
case "$host" in
	i*86*-*)
		RTAI_TARGET_SUBARCH="32-bit"
		;;
	x86_64*-*)
		RTAI_TARGET_SUBARCH="64-bit"
		;;
	*)
		echo "Only x86 and amd64 platforms are supported."
		exit 1;
esac

dnl Make it more difficult to do something stupid
RTAI_DEFCONFIG=$srcdir/src/arch/x86/defconfig
RTAI_USER_CONFIG=$srcdir/.rtai_config
AC_MSG_CHECKING([for RTAI Kconfig file])
if test \! -r $RTAI_DEFCONFIG; then
	AC_MSG_ERROR([Kconfig-file $RTAI_DEFCONFIG not found])
elif test -r $RTAI_USER_CONFIG; then
	KCONFIG_RESULT=$RTAI_USER_CONFIG
	sed -i '/TRUE/d' $RTAI_USER_CONFIG
	sed -i '/FALSE/d' $RTAI_USER_CONFIG
	source $RTAI_USER_CONFIG
else
	KCONFIG_RESULT=$RTAI_DEFCONFIG
	source $RTAI_DEFCONFIG
fi
AC_MSG_RESULT([$KCONFIG_RESULT])

dnl Make it more difficult to do something stupid here too
RTAI_MODULE_DIR='${prefix}/modules'

dnl
dnl Process the Linux kernel configuration
dnl

AC_MSG_CHECKING([for Linux source tree])

dnl safe default
if test x$CONFIG_RTAI_LINUXDIR != x ; then
	RTAI_LINUX_DIR=$CONFIG_RTAI_LINUXDIR
else
	RTAI_LINUX_DIR="/usr/src/linux"
fi

AC_ARG_WITH(linux-dir,
	[ --with-linux-dir=<linux-dir>	RTAI-enabled Linux kernel tree],
	[
	case "$withval" in
	"" | y | ye | yes | n | no)
		AC_MSG_ERROR([You must supply an argument to --with-linux-dir.])
		;;
	esac
	RTAI_LINUX_DIR="$withval"
	])

if test x${RTAI_LINUX_DIR} = x; then
	echo
	AC_MSG_ERROR([Linux kernel tree not specified -- use --with-linux-dir])
fi

if test \! -r ${RTAI_LINUX_DIR}/Makefile; then
	echo
	AC_MSG_ERROR([No Linux kernel tree in ${RTAI_LINUX_DIR}])
fi

if grep KERNELSRC $RTAI_LINUX_DIR/Makefile > /dev/null; then
eval LINUX_`grep '^KERNELSRC' $RTAI_LINUX_DIR/Makefile | sed -e 's, ,,g'|sed -e 's,:,,g'`
eval LINUX_`grep '^KERNELOUTPUT' $RTAI_LINUX_DIR/Makefile | sed -e 's, ,,g'|sed -e 's,:,,g'`
elif grep '^MAKEARGS *:= *-C' $RTAI_LINUX_DIR/Makefile > /dev/null; then
LINUX_KERNELSRC=`sed -n -e 's,^MAKEARGS *:= *-C *\(@<:@^@<:@:space:@:>@@:>@*\).*,\1,p' $RTAI_LINUX_DIR/Makefile`
else
LINUX_KERNELSRC=$RTAI_LINUX_DIR
fi

eval linux_`grep '^EXTRAVERSION =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^PATCHLEVEL =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^SUBLEVEL =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^VERSION =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`

dnl Load Linux kernel configuration
if test \! -r ${RTAI_LINUX_DIR}/.config; then
	AC_MSG_RESULT([invalid])
	AC_MSG_ERROR([$RTAI_LINUX_DIR is not configured -- --with-linux-dir sets linux-dir])
fi
source $RTAI_LINUX_DIR/.config

RTAI_LINUX_VERSION="$linux_VERSION.$linux_PATCHLEVEL.$linux_SUBLEVEL"
AC_MSG_RESULT([$RTAI_LINUX_DIR (kernel ${RTAI_LINUX_VERSION}${CONFIG_SMP:+-SMP})])
if test x${CONFIG_RTAI_LINUXDIR} = x; then
	CONFIG_RTAI_LINUXDIR=$RTAI_LINUX_DIR
fi

dnl
dnl Perform a few sanity checks.
dnl

AC_MSG_CHECKING([for kernel architecture])
KERNEL_ARCH="`uname -m`"
AC_MSG_RESULT([$KERNEL_ARCH])

if test x${CONFIG_MODULE_UNLOAD} = x ; then
	AC_MSG_ERROR([Please set CONFIG_MODULE_UNLOAD for $RTAI_LINUX_DIR and rebuild your kernel])
fi

if test \! -r $LINUX_KERNELSRC/include/linux/ipipe.h; then
	AC_MSG_ERROR([HAL patch not applied/enabled in $LINUX_KERNELSRC])
fi

dnl
dnl Process the build options, possibly overriding the loaded
dnl configuration.
dnl

RTAI_BUILTIN_MODLIST=""

AC_MSG_CHECKING(for real-time fifos support)
AC_ARG_ENABLE(fifos,
	[ --enable-fifos			Enable fifo IPC support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_FIFOS=y ;;
	m | mod | module) CONFIG_RTAI_FIFOS=m ;;
	*) unset CONFIG_RTAI_FIFOS ;;
	esac])
if test x$CONFIG_RTAI_FIFOS = x ; then
	# Enable by default for LinuxCNC
	CONFIG_RTAI_FIFOS=y
fi
AC_MSG_RESULT(${CONFIG_RTAI_FIFOS:-no})

if test x$CONFIG_RTAI_FIFOS = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/fifos"
fi

AC_MSG_CHECKING(for semaphore support)
AC_ARG_ENABLE(sem,
	[ --enable-sem			Enable semaphore support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_SEM=y ;;
	m | mod | module) CONFIG_RTAI_SEM=m ;;
	*) unset CONFIG_RTAI_SEM ;;
	esac])
if test x$CONFIG_RTAI_SEM = x ; then
	# Enable by default for LinuxCNC
	CONFIG_RTAI_SEM=y
fi
AC_MSG_RESULT(${CONFIG_RTAI_SEM:-no})

if test x$CONFIG_RTAI_SEM = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/sem"
fi

AC_MSG_CHECKING(for RT POLL support)
AC_ARG_ENABLE(rt-poll,
	[ --enable-rt-poll		Enable polling support of RTAI IPCs],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_RT_POLL=y ;;
	*) unset CONFIG_RTAI_RT_POLL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_RT_POLL:-no})

AC_MSG_CHECKING(for RT POLL on stack)
AC_ARG_ENABLE(rt-poll-on-stack,
	[ --enable-rt-poll-on-stack	Use the stack for rt_poll dynamic arrays],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_RT_POLL_ON_STACK=y ;;
	*) unset CONFIG_RTAI_RT_POLL_ON_STACK ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_RT_POLL_ON_STACK:-no})

AC_MSG_CHECKING(for messaging support)
AC_ARG_ENABLE(msg,
	[ --enable-msg			Enable messaging support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MSG=y ;;
	m | mod | module) CONFIG_RTAI_MSG=m ;;
	*) unset CONFIG_RTAI_MSG ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MSG:-no})

if test x$CONFIG_RTAI_MSG = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/msg"
fi

AC_MSG_CHECKING(for mailbox support)
AC_ARG_ENABLE(mbx,
	[ --enable-mbx			Enable mailbox support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MBX=y ;;
	m | mod | module) CONFIG_RTAI_MBX=m ;;
	*) unset CONFIG_RTAI_MBX ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MBX:-no})

if test x$CONFIG_RTAI_MBX = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/mbx"
fi

AC_MSG_CHECKING(for shared memory support)
AC_ARG_ENABLE(shm,
	[ --enable-shm			Enable shared memory support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_SHM=y ;;
	m | mod | module) CONFIG_RTAI_SHM=m ;;
	*) unset CONFIG_RTAI_SHM ;;
	esac])
if test x$CONFIG_RTAI_SHM = x ; then
	# Enable by default for LinuxCNC
	CONFIG_RTAI_SHM=y
fi
AC_MSG_RESULT(${CONFIG_RTAI_SHM:-no})

if test x$CONFIG_RTAI_SHM = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/shm"
fi

AC_MSG_CHECKING(for real-time malloc support)
AC_ARG_ENABLE(malloc,
	[ --enable-malloc		Enable real-time malloc support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MALLOC=y ;;
	m | mod | module) CONFIG_RTAI_MALLOC=m ;;
	*) unset CONFIG_RTAI_MALLOC ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MALLOC:-no})

if test x$CONFIG_RTAI_MALLOC = xy ; then
	RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST malloc"
fi

AC_MSG_CHECKING(whether LXRT wraps service calls through Linux syscalls)
AC_ARG_ENABLE(lxrt-use-linux-syscall,
	[ --enable-lxrt-linux-syscall	Wrap LXRT service calls through Linux syscall mechanism],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL=y ;;
	*) unset CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL:-no})

AC_MSG_CHECKING(for scheduler lock in ISRs)
AC_ARG_ENABLE(sched-lock-isr,
	[ --enable-sched-lock-isr	Enable scheduler lock in ISRs],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_SCHED_ISR_LOCK=y ;;
	*) unset CONFIG_RTAI_SCHED_ISR_LOCK ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_SCHED_ISR_LOCK:-no})

AC_MSG_CHECKING(for RTC freq)
AC_ARG_ENABLE(rtc-freq,
	[ --enable-rtc-freq		Enable RTC freq],
	[case "$enableval" in
 changequote(,)dnl add a test for 0 or power of 2, <2-8192>
	[0-9]*) CONFIG_RTAI_RTC_FREQ=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-rtc-freq=<power of 2>]) ;;
	esac])
if test x$CONFIG_RTAI_RTC_FREQ = x ; then
	# Use a reasonable default if unset
	CONFIG_RTAI_RTC_FREQ=0
fi
AC_MSG_RESULT($CONFIG_RTAI_RTC_FREQ)

AC_MSG_CHECKING(for long timed lists)
AC_ARG_ENABLE(long-timed-lists,
	[ --enable-long-timed-lists	Enable long timed lists],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_LONG_TIMED_LIST=y ;;
	*) unset CONFIG_RTAI_LONG_TIMED_LIST;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_LONG_TIMED_LIST:-no})

AC_MSG_CHECKING(for using RTAI way for user-kernel space on stack args exchange)
AC_ARG_ENABLE(use-stack-args,
	[ --enable-use-stack-args	Keep using RTAI way for user-kernel space on stack args exchange],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_USE_STACK_ARGS=y ;;
	*) unset CONFIG_RTAI_USE_STACK_ARGS;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USE_STACK_ARGS:-no})

AC_MSG_CHECKING(for latency self calibration metrics)
AC_ARG_ENABLE(self-calibration-metrics,
	[ --enable-self-calibration-metrics	Set latency self calibration metrics],
	[case "$enableval" in
 changequote(,)dnl
	[1-3]*) CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-latency-self-calibration-metrics=[1-3]]) ;;
	esac])
if test x$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS=1
fi
AC_MSG_RESULT($CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS)

AC_MSG_CHECKING(for latency self calibration freq)
AC_ARG_ENABLE(self-calibration-freq,
	[ --enable-self-calibration-freq		Set latency self calibration freq],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-latency-self-calibration-freq=<Hz>]) ;;
	esac])
if test x$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ=10000
fi
AC_MSG_RESULT($CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ)

AC_MSG_CHECKING(for latency self calibration cycles)
AC_ARG_ENABLE(self-calibration-cycles,
	[ --enable-self-calibration-cycles	Set latency self calibration cycles],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-latency-self-calibration-cycles=<numcycles>]) ;;
	esac])
if test x$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES=$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ
fi
AC_MSG_RESULT($CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES)

AC_MSG_CHECKING(for user busy align ret delay)
AC_ARG_ENABLE(user-ret-delay,
	[ --enable-user-ret-delay	Set user busy align ret delay],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-user-ret-delay=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY = x ; then
	# Use a reasonable default if unset
	CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY=0
fi
AC_MSG_RESULT($CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY)

AC_MSG_CHECKING(for kernel busy align ret delay)
AC_ARG_ENABLE(kernel-ret-delay,
	[ --enable-kernel-ret-delay	Set kernel busy align ret delay],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-kernel-ret-delay=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY = x ; then
	# Use a reasonable default if unset
	CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY=0
fi
AC_MSG_RESULT($CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY)

AC_MSG_CHECKING(for maximum number of LXRT slots)
AC_ARG_ENABLE(lxrt-numslots,
	[ --enable-lxrt-numslots		Set maximum number of LXRT slots],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_SCHED_LXRT_NUMSLOTS=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-lxrt-numslots=<number>]) ;;
	esac])
if test x$CONFIG_RTAI_SCHED_LXRT_NUMSLOTS = x ; then
	# Use a reasonable default if unset
	CONFIG_RTAI_SCHED_LXRT_NUMSLOTS=256
fi
AC_MSG_RESULT($CONFIG_RTAI_SCHED_LXRT_NUMSLOTS)

AC_MSG_CHECKING(for task switch signal)
AC_ARG_ENABLE(task-switch-signal,
	[ --enable-task-switch-signal	Enable RTAI task switch signal],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TASK_SWITCH_SIGNAL=y ;;
	*) unset CONFIG_RTAI_TASK_SWITCH_SIGNAL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TASK_SWITCH_SIGNAL:-no})

AC_MSG_CHECKING(for hard-soft toggler)
AC_ARG_ENABLE(hard-soft-toggler,
	[ --enable-hard-soft-toggler	Enable RTAI hard-soft toggler],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_HARD_SOFT_TOGGLER=y ;;
	*) unset CONFIG_RTAI_HARD_SOFT_TOGGLER ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_HARD_SOFT_TOGGLER:-no})

AC_MSG_CHECKING(for Linux task priority alignment to RTAI)
AC_ARG_ENABLE(align-priority,
		[ --enable-align-priority	Keep Linux task priority aligned to RTAI],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_ALIGN_LINUX_PRIORITY=y ;;
	*) unset CONFIG_RTAI_ALIGN_LINUX_PRIORITY ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_ALIGN_LINUX_PRIORITY:-no})

AC_MSG_CHECKING(for calibration frequency factor)
AC_ARG_ENABLE(cal-freq-fact,
	[ --enable-cal-freq-fact		Set calibration frequency factor],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_CAL_FREQS_FACT=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-cal-freq-fact=<number>]) ;;
	esac])
if test x$CONFIG_RTAI_CAL_FREQS_FACT = x ; then
	# Use a reasonable default if unset
	CONFIG_RTAI_CAL_FREQS_FACT=0
fi
AC_MSG_RESULT($CONFIG_RTAI_CAL_FREQS_FACT)

AC_MSG_CHECKING(for tlsf use in real-time malloc support)
AC_ARG_ENABLE(malloc-tlsf,
	[ --enable-malloc-tlsf		Enable tlsf support in malloc],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_USE_TLSF=y ;;
	*) unset CONFIG_RTAI_USE_TLSF;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USE_TLSF:-no})

AC_MSG_CHECKING(for vmalloc use in real-time malloc support)
AC_ARG_ENABLE(malloc-vmalloc,
	[ --enable-malloc-vmalloc	Enable vmalloc support in malloc],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_MALLOC_VMALLOC=y ;;
	*) unset CONFIG_RTAI_MALLOC_VMALLOC ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MALLOC_VMALLOC:-no})

AC_MSG_CHECKING(for size of real-time malloc heap)
AC_ARG_ENABLE(malloc-heap-size,
	[ --enable-malloc-heap-size	Set size of real-time malloc heap],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_MALLOC_HEAPSZ=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-malloc-heap-size=<Kbytes>]) ;;
	esac])
if test x$CONFIG_RTAI_MALLOC_HEAPSZ = x ; then
	# Use a reasonable default if unset (in Kbytes)
	CONFIG_RTAI_MALLOC_HEAPSZ=128
fi
AC_MSG_RESULT($CONFIG_RTAI_MALLOC_HEAPSZ)

AC_MSG_CHECKING(for size of real-time kernel task stack heap)
AC_ARG_ENABLE(kstack-heap-size,
	[ --enable-kstack-heap-size	Set size of real-time kernel task stack heap],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_KSTACK_HEAPSZ=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-kstack-heap-size=<Kbytes>]) ;;
	esac])
if test x$CONFIG_RTAI_KSTACK_HEAPSZ = x ; then
	# Use a reasonable default if unset (in Kbytes)
	CONFIG_RTAI_KSTACK_HEAPSZ=512
fi
AC_MSG_RESULT($CONFIG_RTAI_KSTACK_HEAPSZ)

dnl Allow building for 16 cores, hopefully making this tree somewhat ready
dnl for the upcoming Ryzen 9 3950X (kernel support will be needed)
AC_MSG_CHECKING(for number of CPUs)
AC_ARG_ENABLE(cpus,
	[ --enable-cpus			Number of physical CPU cores],
	[case "$enableval" in
 changequote(,)dnl
	[1-16]*)
 changequote([,])dnl
		if test $enableval -lt 1 -o $enableval -gt 16 ; then
			AC_MSG_ERROR([Invalid value to option: --enable-cpus=<number-of-CPUs> (must be between 1 and 16)])
		fi
		CONFIG_RTAI_CPUS=$enableval ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_CPUS:-1})

dnl
dnl Check for TSC options
dnl
AC_MSG_CHECKING(for diagnose out of sync MP-TSCs)
AC_ARG_ENABLE(diag-tsc-sync,
	[ --enable-diag-tsc-sync		Enable diagnose out of sync MP-TSCs],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_DIAG_TSC_SYNC=y ;;
	*) unset CONFIG_RTAI_DIAG_TSC_SYNC ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_DIAG_TSC_SYNC:-no})

AC_MSG_CHECKING(for master CPU for aligning MP-TSCs)
AC_ARG_ENABLE(master-tsc-cpu,
	[ --enable-master-tsc-cpu	Enable master TSC CPU],
	[case "$enableval" in
 changequote(,)dnl
	[0-15]*)
 changequote([,])dnl
		if test x$CONFIG_RTAI_CPUS = x ; then
			AC_MSG_ERROR([Option --enable-cpus must be defined before --enable-master-tsc-cpu])
		fi
		if test $enableval -lt 0 -o $enableval -ge $CONFIG_RTAI_CPUS ; then
			AC_MSG_ERROR([Invalid value to option: --enable-master-tsc-cpu=<master-CPU-number> (must be between 0 and CONFIG_RTAI_CPUS)])
		fi
		CONFIG_RTAI_MASTER_TSC_CPU=$enableval ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MASTER_TSC_CPU:0})

AC_MSG_CHECKING(for tune out of sync MP-TSCs)
AC_ARG_ENABLE(tune-tsc-sync,
		[ --enable-tune-tsc-sync		Enable tune out of sync MP-TSCs],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TUNE_TSC_SYNC=y ;;
	*) unset CONFIG_RTAI_TUNE_TSC_SYNC ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TUNE_TSC_SYNC:-no})

dnl Test for TSC if 32-bit is forced and too old of CPU type
AC_MSG_CHECKING(if kernel has TSC enabled)
if test x$CONFIG_X86_TSC = x ; then
	AC_MSG_ERROR([Missing TSC support: Please select a newer CPU type in kernel options.])
fi
AC_MSG_RESULT([yes])

dnl
dnl Check for invalid combinations
dnl

if test x$CONFIG_RTAI_RT_POLL = xy ; then
	if test x$CONFIG_RTAI_SEM = x ; then
	AC_MSG_ERROR([--enable-rt-poll requires --enable-sem])
	fi
fi

if test x$CONFIG_RTAI_RT_POLL_ON_STACK = xy ; then
	if test x$CONFIG_RTAI_RT_POLL = x ; then
	AC_MSG_ERROR([--enable-rt-poll-on-stack requires --enable-rt-poll])
	fi
fi

if test x$CONFIG_RTAI_MBX = xy -o x$CONFIG_RTAI_MBX = xm ; then
	if test x$CONFIG_RTAI_SEM = x ; then
	AC_MSG_ERROR([--enable-mbx requires --enable-sem])
	fi
fi

if test x$CONFIG_RTAI_MBX = xy ; then
	if test x$CONFIG_RTAI_SEM = xm ; then
	AC_MSG_ERROR([--enable-mbx=y requires --enable-sem=y])
	fi
fi

dnl
dnl Build a Kconfig file corresponding to the current configuration
dnl if none exists.
dnl

if test \! -r .rtai_config; then
	set | grep -e '^CONFIG_RTAI' -e '^CONFIG_MODULES' | \
	sed -e 's,\(.*\)=\(.*\)$,\1=\2,' > .rtai_config
fi

dnl
dnl Produce the info needed to build rtai_config.h
dnl

AC_DEFINE_UNQUOTED(CONFIG_RTAI_VERSION_MAJOR,$CONFIG_RTAI_VERSION_MAJOR,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_VERSION_MINOR,$CONFIG_RTAI_VERSION_MINOR,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_REVISION_LEVEL,$CONFIG_RTAI_REVISION_LEVEL,[Kconfig])
test \! x$CONFIG_RTAI_LINUXDIR = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_LINUXDIR,$CONFIG_RTAI_LINUXDIR,[Kconfig])

test \! x$CONFIG_RTAI_CPUS = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_CPUS,$CONFIG_RTAI_CPUS,[Kconfig])
test x$CONFIG_RTAI_DIAG_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_DIAG_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_MASTER_TSC_CPU = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_MASTER_TSC_CPU,$CONFIG_RTAI_MASTER_TSC_CPU,[Kconfig])
test x$CONFIG_RTAI_TUNE_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_TUNE_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_FIFOS = x && AC_DEFINE(CONFIG_RTAI_FIFOS,1,[Kconfig])
test \! x$CONFIG_RTAI_SEM = x && AC_DEFINE(CONFIG_RTAI_SEM,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL = x && AC_DEFINE(CONFIG_RTAI_RT_POLL,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL_ON_STACK = x && AC_DEFINE(CONFIG_RTAI_RT_POLL_ON_STACK,1,[Kconfig])
test \! x$CONFIG_RTAI_MSG = x && AC_DEFINE(CONFIG_RTAI_MSG,1,[Kconfig])
test \! x$CONFIG_RTAI_MBX = x && AC_DEFINE(CONFIG_RTAI_MBX,1,[Kconfig])
test \! x$CONFIG_RTAI_SHM = x && AC_DEFINE(CONFIG_RTAI_SHM,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_USE_TLSF = x && AC_DEFINE(CONFIG_RTAI_USE_TLSF,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC_VMALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC_VMALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL = x && AC_DEFINE(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,1,[Kconfig])
test \! x$CONFIG_RTAI_ALIGN_LINUX_PRIORITY = x && AC_DEFINE(CONFIG_RTAI_ALIGN_LINUX_PRIORITY,1,[Kconfig])
test \! x$CONFIG_RTAI_TASK_SWITCH_SIGNAL = x && AC_DEFINE(CONFIG_RTAI_TASK_SWITCH_SIGNAL,1,[Kconfig])
test \! x$CONFIG_RTAI_HARD_SOFT_TOGGLER = x && AC_DEFINE(CONFIG_RTAI_HARD_SOFT_TOGGLER,1,[Kconfig])

test x$CONFIG_RTAI_FIFOS = xy && AC_DEFINE(CONFIG_RTAI_FIFOS_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SEM = xy && AC_DEFINE(CONFIG_RTAI_SEM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MSG = xy && AC_DEFINE(CONFIG_RTAI_MSG_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MBX = xy && AC_DEFINE(CONFIG_RTAI_MBX_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SHM = xy && AC_DEFINE(CONFIG_RTAI_SHM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MALLOC = xy && AC_DEFINE(CONFIG_RTAI_MALLOC_BUILTIN,1,[Kconfig])

test x$CONFIG_RTAI_SCHED_ISR_LOCK = xy && AC_DEFINE(CONFIG_RTAI_SCHED_ISR_LOCK,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_RTC_FREQ,$CONFIG_RTAI_RTC_FREQ,[Kconfig])
test x$CONFIG_RTAI_LONG_TIMED_LIST = xy && AC_DEFINE(CONFIG_RTAI_LONG_TIMED_LIST,1,[Kconfig])
test x$CONFIG_RTAI_USE_STACK_ARGS = xy && AC_DEFINE(CONFIG_RTAI_USE_STACK_ARGS,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_SCHED_LXRT_NUMSLOTS,$CONFIG_RTAI_SCHED_LXRT_NUMSLOTS,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_CAL_FREQS_FACT,$CONFIG_RTAI_CAL_FREQS_FACT,[Kconfig])

case $CONFIG_RTAI_MONITOR_EXECTIME in
	y) CONFIG_RTAI_MONITOR_EXECTIME=1 ;;
	n|*) CONFIG_RTAI_MONITOR_EXECTIME=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_MONITOR_EXECTIME,$CONFIG_RTAI_MONITOR_EXECTIME,[Kconfig])

case $CONFIG_RTAI_ALLOW_RR in
	y) CONFIG_RTAI_ALLOW_RR=1 ;;
	n|*) CONFIG_RTAI_ALLOW_RR=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_ALLOW_RR,$CONFIG_RTAI_ALLOW_RR,[Kconfig])

test x$CONFIG_RTAI_FULL_PRINHER = xy && AC_DEFINE(CONFIG_RTAI_FULL_PRINHER,1,[Kconfig])
case $CONFIG_RTAI_USE_NEWERR in
	y) CONFIG_RTAI_USE_NEWERR=1 ;;
	n|*) CONFIG_RTAI_USE_NEWERR=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_USE_NEWERR,$CONFIG_RTAI_USE_NEWERR,[Kconfig])

test x$CONFIG_RTAI_CLOCK_REALTIME = xy && AC_DEFINE(CONFIG_RTAI_CLOCK_REALTIME,1,[Kconfig])

case $CONFIG_RTAI_IMMEDIATE_LINUX_SYSCALL in
	y) CONFIG_RTAI_IMMEDIATE_LINUX_SYSCALL=1 ;;
	n|*) CONFIG_RTAI_IMMEDIATE_LINUX_SYSCALL=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_IMMEDIATE_LINUX_SYSCALL,$CONFIG_RTAI_IMMEDIATE_LINUX_SYSCALL,[Kconfig])

case $CONFIG_RTAI_BUSY_TIME_ALIGN in
	y) CONFIG_RTAI_BUSY_TIME_ALIGN=1 ;;
	n|*) CONFIG_RTAI_BUSY_TIME_ALIGN=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_BUSY_TIME_ALIGN,$CONFIG_RTAI_BUSY_TIME_ALIGN,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY,$CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY,$CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY,[Kconfig])

case $CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS in
	y) CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS=1 ;;
	n|*) CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS,$CONFIG_RTAI_DONT_DISPATCH_CORE_IRQS,[Kconfig])

AC_DEFINE_UNQUOTED(CONFIG_RTAI_MALLOC_HEAPSZ,$CONFIG_RTAI_MALLOC_HEAPSZ,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_KSTACK_HEAPSZ,$CONFIG_RTAI_KSTACK_HEAPSZ,[Kconfig])

fifos_template="/dev/rtf%d"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_FIFOS_TEMPLATE,"$fifos_template",[Kconfig])

AH_VERBATIM(,[
#define __hidefun(name) name ## _hidden_inline
#define RTAI_PROTO(type,name,arglist) static inline type name arglist
#define RTAI_PROTO_ALWAYS_INLINE(type,name,arglist) static inline type name arglist
#define RTAI_PROTO_NEVER_INLINE(type,name,arglist) type name arglist; static inline type __hidefun(name) arglist
])

dnl CAUTION: We need to have the CONFIG_RTAI_XX symbols always defined when
dnl the configuration header is read, but we want the Autoconf-produced
dnl symbols to be defined only when compiling RTAI. This way, we won't
dnl pollute the namespace with the latter when our configuration header is
dnl indirectly included by a client application. To achieve this, we ask
dnl autoheader to produce the following header structure:
dnl #define CONFIG_XX
dnl #define CONFIG_XX ...
dnl #ifdef __IN_RTAI__
dnl <Autoconf-defined symbols>
dnl #endif /* __IN_RTAI__ */
dnl This is quite a hack since we have to rely on the fact that
dnl all Autoconf-generated symbols are lexicographically sorted
dnl after CONFIG_RTAI_XX ones, but, well...

dnl Use a key which will cause the verbatim string to be put after
dnl all CONFIG_RTAI_XX symbols, but still before any Autoconf-generated
dnl symbol, hence CONFIG_RTAI___.
AH_VERBATIM(CONFIG_RTAI___,[#ifdef __IN_RTAI__])

dnl Now we can close the conditional section, right after all
dnl Autoconf-generated symbols have been listed.
AH_BOTTOM([#endif /* __IN_RTAI__ */])

dnl
dnl Produce automake conditionals.
dnl

dnl CPU architecture
AM_CONDITIONAL(CONFIG_X86_64,[test "$CONFIG_X86_64" = y])
AM_CONDITIONAL(CONFIG_X86,[test "$CONFIG_X86" = y])
AM_CONDITIONAL(CONFIG_SMP,[test "$CONFIG_SMP" = y])

AM_CONDITIONAL(CONFIG_RTAI_FIFOS,[test -n "$CONFIG_RTAI_FIFOS"])
AM_CONDITIONAL(CONFIG_RTAI_SEM,[test -n "$CONFIG_RTAI_SEM"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL,[test -n "$CONFIG_RTAI_RT_POLL"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL_ON_STACK,[test -n "$CONFIG_RTAI_RT_POLL_ON_STACK"])
AM_CONDITIONAL(CONFIG_RTAI_MSG,[test -n "$CONFIG_RTAI_MSG"])
AM_CONDITIONAL(CONFIG_RTAI_MBX,[test -n "$CONFIG_RTAI_MBX"])
AM_CONDITIONAL(CONFIG_RTAI_SHM,[test -n "$CONFIG_RTAI_SHM"])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC,[test -n "$CONFIG_RTAI_MALLOC"])
AM_CONDITIONAL(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,[test -n "$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL"])

AM_CONDITIONAL(CONFIG_RTAI_FIFOS_BUILTIN,[test x$CONFIG_RTAI_FIFOS = xy])
AM_CONDITIONAL(CONFIG_RTAI_SEM_BUILTIN,[test x$CONFIG_RTAI_SEM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MSG_BUILTIN,[test x$CONFIG_RTAI_MSG = xy])
AM_CONDITIONAL(CONFIG_RTAI_MBX_BUILTIN,[test x$CONFIG_RTAI_MBX = xy])
AM_CONDITIONAL(CONFIG_RTAI_SHM_BUILTIN,[test x$CONFIG_RTAI_SHM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC_BUILTIN,[test x$CONFIG_RTAI_MALLOC = xy])

dnl
dnl Define arch-specific CFLAGS and links
dnl

RTAI_KBUILD_ENV='src2obj = $(patsubst %.$(1),%.o,$(filter %.$(1),$(call notdir,$(2))))'
RTAI_KBUILD_TOP="@if test \\! -r Module.symvers ; then \$(LN_S) -f \"\`cd \$(top_builddir) && pwd\`/Module.symvers\" . ; fi;"
RTAI_KBUILD_BOTTOM=
RTAI_KBUILD_CMD="for src in \$^; do if test \\! -r \`basename \$\$src\`; then \$(LN_S) \$\$src; fi; done; \
			rm -f $RTAI_LINUX_DIR/.tmp_versions/rtai_*.mod $RTAI_LINUX_DIR/.tmp_versions/*_rt.mod; \
			\$(MAKE) --no-print-directory -C $RTAI_LINUX_DIR CC=gcc ARCH=x86 \
			SUBDIRS=\$\$PWD M=\$\$PWD rtai_srctree=\`cd \$(top_srcdir) && pwd\` rtai_srcdir=\`cd \$(srcdir) && pwd\` V=\$(V) modules \
			rtai_objs='\$(call src2obj,c,\$^) \$(call src2obj,S,\$^) \$(call src2obj,cc,\$^) \$(call src2obj,cpp,\$^)' rtai_target=\$(basename \$@)"
RTAI_KBUILD_CLEAN='rm -f *.ko *.mod.c .*.cmd *.o Module.symvers Module.markers modules.order && rm -Rf .tmp_versions'
RTAI_KBUILD_DISTCLEAN=""

dnl extra arch-dependent opts to be passed to the compiler
RTAI_USER_CFLAGS="-O2 -fomit-frame-pointer -pipe"
RTAI_FP_CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -O2 -ffreestanding -frounding-math -fsignaling-nans -mieee-fp -mhard-float -mpreferred-stack-boundary=4 -mfpmath=sse -msse -msse2 -Wno-undef"

RTAI_KMOD_CFLAGS="$RTAI_KMOD_CFLAGS -D__IN_RTAI__"
RTAI_USER_CFLAGS="$RTAI_USER_CFLAGS -D__IN_RTAI__"

AC_CONFIG_LINKS(src/include/asm:$srcdir/src/include/asm-x86)
AC_CONFIG_LINKS(src/ipc/fifos/Makefile:src/ipc/fifos/Makefile.kbuild)
AC_CONFIG_LINKS(src/ipc/mbx/Makefile:src/ipc/mbx/Makefile.kbuild)
AC_CONFIG_LINKS(src/ipc/msg/Makefile:src/ipc/msg/Makefile.kbuild)
AC_CONFIG_LINKS(src/ipc/sem/Makefile:src/ipc/sem/Makefile.kbuild)
AC_CONFIG_LINKS(src/ipc/shm/Makefile:src/ipc/shm/Makefile.kbuild)
AC_CONFIG_LINKS(src/malloc/Makefile:src/malloc/Makefile.kbuild)
AC_CONFIG_LINKS(src/math/Makefile:src/math/Makefile.kbuild)
AC_CONFIG_LINKS(src/sched/Makefile:src/sched/Makefile.kbuild)

dnl
dnl Build the Makefiles
dnl

AC_SUBST(RTAI_BUILD_STRING)
AC_SUBST(RTAI_HOST_STRING)
AC_SUBST(RTAI_BUILTIN_MODLIST)
AC_SUBST(RTAI_KMOD_CFLAGS)
AC_SUBST(RTAI_USER_CFLAGS)
AC_SUBST(RTAI_FP_CFLAGS)
AC_SUBST(RTAI_LINUX_DIR)
AC_SUBST(RTAI_LINUX_VERSION)
AC_SUBST(RTAI_MODULE_DIR)
AC_SUBST(RTAI_KBUILD_ENV)
AC_SUBST(RTAI_KBUILD_TOP)
AC_SUBST(RTAI_KBUILD_BOTTOM)
AC_SUBST(RTAI_KBUILD_CMD)
AC_SUBST(RTAI_KBUILD_CLEAN)
AC_SUBST(RTAI_KBUILD_DISTCLEAN)

AC_CONFIG_FILES([ \
	GNUmakefile \
	src/GNUmakefile \
	src/arch/GNUmakefile \
	src/include/GNUmakefile \
	src/ipc/GNUmakefile \
	src/sched/GNUmakefile \
	src/include/rtai_defs.h \
	src/include/asm-x86/GNUmakefile \
	src/ipc/fifos/GNUmakefile \
	src/ipc/shm/GNUmakefile \
	src/ipc/sem/GNUmakefile \
	src/ipc/msg/GNUmakefile \
	src/ipc/mbx/GNUmakefile \
	src/malloc/GNUmakefile \
	src/math/GNUmakefile \
	scripts/GNUmakefile \
	scripts/rtai-config \
	scripts/rtai-load \
	testsuite/GNUmakefile
	])

AC_OUTPUT
