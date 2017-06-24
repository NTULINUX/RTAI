dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.69)

AC_INIT([4.1.0], [4.1.0], [neotheuser@ymail.com])
AC_CONFIG_AUX_DIR(base/scripts/autoconf)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS(rtai_config.h)

CONFIG_RTAI_VERSION_MAJOR=4
CONFIG_RTAI_VERSION_MINOR=1
CONFIG_RTAI_REVISION_LEVEL=0

AC_PROG_INSTALL
LT_INIT
AM_PROG_AS

AC_ARG_WITH(CC,
    [  --with-cc=compiler      use specific C compiler],
    [
	case "$withval" in
	"" | y | ye | yes | n | no)
	    AC_MSG_ERROR([You must supply an argument to --with-cc.])
	  ;;
	esac
	CC="$withval"
    ])
AC_PROG_CC
CFLAGS=""

AC_ARG_WITH(CXX,
    [  --with-cxx=compiler      use specific C++ compiler],
    [ case "$withval" in
	"" | y | ye | yes | n | no)
	    AC_MSG_ERROR([You must supply an argument to --with-cxx.])
	  ;;
	esac
	CXX="$withval" ])
AC_PROG_CXX
CXXFLAGS=""

dnl If we're using GCC, make sure that it is at least version 3.3.0.  Older
dnl versions are explictly not supported.
AC_MSG_CHECKING([whether gcc version is sufficient])
major=0
minor=0
GCC_VERSION=`$CC -dumpversion`
if test $? -eq 0; then
	GCC_VERSION_MAJOR=`echo $GCC_VERSION | cut -d. -f1`
	GCC_VERSION_MINOR=`echo $GCC_VERSION | cut -d. -f2`
fi
if test $GCC_VERSION_MAJOR -lt 3 -o $GCC_VERSION_MAJOR -eq 3 -a $GCC_VERSION_MINOR -lt 3 ; then
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([If using GCC, version 3.3.0 or later is required.])
else
	AC_MSG_RESULT([yes])
fi

AC_DEFINE_UNQUOTED(CONFIG_RTAI_BUILD_STRING,"$build",[Build system alias])
RTAI_BUILD_STRING="$build"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_HOST_STRING,"$host",[Host system alias])
RTAI_HOST_STRING="$host"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_COMPILER,"`$CC -v 2>&1 | tail -n 1`",[Compiler])

dnl Find arch-specific requirements and supports. Features which
dnl are not supported by all archs must be listed here, so that a
dnl an Autoconf flag can later be defined/undefined in rtai_config.h.

arch_supports_lxrt=y
arch_has_mmu=y

case "$host" in
 i*86*-*)
	RTAI_TARGET_ARCH=x86
	RTAI_TARGET_SUBARCH=32
	wanted_kernel_arch=CONFIG_X86_32
	;;
 x86_64*-*)
	RTAI_TARGET_ARCH=x86
	RTAI_TARGET_SUBARCH=64
	wanted_kernel_arch=CONFIG_X86_64
	;;
esac

dnl
dnl Load the RTAI configuration.
dnl

RTAI_KCONFIG_FILE=$srcdir/base/arch/x86/defconfig
kconfig_type=default

AC_ARG_WITH(kconfig-file,
    [  --with-kconfig-file=<kconfig-file>      KConfig parameter file],
    [
	case "$withval" in
	"" | y | ye | yes | n | no)
	    AC_MSG_ERROR([You must supply an argument to --with-kconfig-file.])
	  ;;
	esac
	RTAI_KCONFIG_FILE="$withval"
	kconfig_type=supplied
    ])

AC_MSG_CHECKING([for RTAI Kconfig file])
if test \! -f $RTAI_KCONFIG_FILE; then
    AC_MSG_ERROR([Kconfig-file $RTAI_KCONFIG_FILE not found -- --with-kconfig-file sets Kconfig file])
fi
. $RTAI_KCONFIG_FILE
AC_MSG_RESULT([$RTAI_KCONFIG_FILE ($kconfig_type)])

dnl
dnl Installation directory for modules
dnl

AC_MSG_CHECKING([for module installation directory])

AC_ARG_WITH(module-dir,
    [  --with-module-dir=<module-dir>      Installation directory for modules],
    [
	case "$withval" in
	"" | y | ye | yes | n | no)
	    AC_MSG_ERROR([You must supply an argument to --with-module-dir.])
	  ;;
	esac
	RTAI_MODULE_DIR="$withval"
    ])

CONFIG_RTAI_OLD_FASHIONED_BUILD=y

dnl Set a default module dir if unspecified
if test "x" = "x$RTAI_MODULE_DIR"
then
   RTAI_MODULE_DIR='${exec_prefix}/modules'
fi

AC_MSG_RESULT([$RTAI_MODULE_DIR])
CONFIG_RTAI_MODULE_DIR=$RTAI_MODULE_DIR

dnl
dnl Process the Linux kernel configuration
dnl

AC_MSG_CHECKING([for Linux source tree])

dnl CONFIG_MODULES has been forcibly set in RTAI's configuration
dnl file, therefore we must unset it before later checking if the kernel
dnl actually defined it as required too.
unset CONFIG_MODULES

dnl safe default
if test x$CONFIG_RTAI_LINUXDIR != x ; then
	RTAI_LINUX_DIR=$CONFIG_RTAI_LINUXDIR
else
	RTAI_LINUX_DIR="/usr/src/linux"
fi

AC_ARG_WITH(linux-dir,
    [  --with-linux-dir=<linux-dir>      RTAI-enabled Linux kernel tree],
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
LINUX_KERNELOUTPUT=$RTAI_LINUX_DIR
else
LINUX_KERNELOUTPUT=$RTAI_LINUX_DIR
LINUX_KERNELSRC=$RTAI_LINUX_DIR
fi

eval linux_`grep '^EXTRAVERSION =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^PATCHLEVEL =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^SUBLEVEL =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`
eval linux_`grep '^VERSION =' $LINUX_KERNELSRC/Makefile | sed -e 's, ,,g'`

dnl Load Linux kernel configuration now, before config symbols are tested.
if test \! -r ${LINUX_KERNELOUTPUT}/.config; then
   AC_MSG_RESULT([invalid])
   AC_MSG_ERROR([$LINUX_KERNELOUTPUT is not configured -- --with-linux-dir sets linux-dir])
fi

. $LINUX_KERNELOUTPUT/.config

RTAI_LINUX_VERSION="$linux_VERSION.$linux_PATCHLEVEL.$linux_SUBLEVEL"
AC_MSG_RESULT([$RTAI_LINUX_DIR (kernel ${RTAI_LINUX_VERSION}${CONFIG_SMP:+-SMP})])
if test x${CONFIG_RTAI_LINUXDIR} = x; then
   CONFIG_RTAI_LINUXDIR=$RTAI_LINUX_DIR
fi

dnl
dnl Identify the base technology used.
dnl

AC_MSG_CHECKING([for RTAI base technology])
if test -r $LINUX_KERNELSRC/include/linux/ipipe.h; then
   AC_DEFINE(CONFIG_RTAI_IPIPE,1,[Use RTAI Adeos-newgen HAL])
   AC_MSG_RESULT([RTAI Adeos-newgen HAL])
   CONFIG_RTAI_IPIPE=y
else
    echo
    AC_MSG_ERROR([HAL patch not applied/enabled in $LINUX_KERNELSRC])
fi

CONFIG_KBUILD=y

dnl
dnl Perform a few sanity checks.
dnl

AC_MSG_CHECKING([for kernel architecture])
KERNEL_ARCH="`uname -m`"
AC_MSG_RESULT([$KERNEL_ARCH])

if test x${CONFIG_MODULES} = x ; then
  AC_MSG_ERROR([Please set CONFIG_MODULES for $RTAI_LINUX_DIR and rebuild your kernel])
fi

if test x${CONFIG_RTAI_ADEOS} = xy -a x${CONFIG_ADEOS_CORE} = x; then
   AC_MSG_ERROR([Please activate the Adeos support in $RTAI_LINUX_DIR and rebuild your kernel])
fi

dnl
dnl Process the build options, possibly overriding the loaded
dnl configuration.
dnl

RTAI_BUILTIN_MODLIST=""

AC_MSG_CHECKING(for trace support)
AC_ARG_ENABLE(trace,
	[ --enable-trace		Enable trace support],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TRACE=y ;;
	*) unset CONFIG_RTAI_TRACE ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TRACE:-no})

CONFIG_RTAI_MATH=m

AC_MSG_CHECKING(for real-time fifos support)
AC_ARG_ENABLE(fifos,
	[ --enable-fifos		Enable fifo IPC support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_FIFOS=y ;;
	m | mod | module) CONFIG_RTAI_FIFOS=m ;;
	*) unset CONFIG_RTAI_FIFOS ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_FIFOS:-no})

if test x$CONFIG_RTAI_FIFOS = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/fifos"
fi

AC_MSG_CHECKING(for semaphore support)
AC_ARG_ENABLE(sem,
	[ --enable-sem		Enable semaphore support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_SEM=y ;;
	m | mod | module) CONFIG_RTAI_SEM=m ;;
	*) unset CONFIG_RTAI_SEM ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_SEM:-no})

if test x$CONFIG_RTAI_SEM = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/sem"
fi

AC_MSG_CHECKING(for RT POLL support)
AC_ARG_ENABLE(rt-poll,
	[ --enable-rt-poll		enable polling support of RTAI IPCs],
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
	[ --enable-msg		Enable messaging support],
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
	[ --enable-mbx		Enable mailbox support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MBX=y ;;
	m | mod | module) CONFIG_RTAI_MBX=m ;;
	*) unset CONFIG_RTAI_MBX ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MBX:-no})

if test x$CONFIG_RTAI_MBX = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/mbx"
fi

AC_MSG_CHECKING(for POSIX-like message queue support)
AC_ARG_ENABLE(mq,
	[ --enable-mq		Enable POSIX-like message queue support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MQ=y ;;
	m | mod | module) CONFIG_RTAI_MQ=m ;;
	*) unset CONFIG_RTAI_MQ ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MQ:-no})

if test x$CONFIG_RTAI_MQ = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/mq"
fi

AC_MSG_CHECKING(for shared memory support)
AC_ARG_ENABLE(shm,
	[ --enable-shm		Enable shared memory support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_SHM=y ;;
	m | mod | module) CONFIG_RTAI_SHM=m ;;
	*) unset CONFIG_RTAI_SHM ;;
	esac])
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

AC_MSG_CHECKING(for tasklets support)
AC_ARG_ENABLE(tasklets,
	[ --enable-tasklets	Enable tasklets support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_TASKLETS=y ;;
	m | mod | module) CONFIG_RTAI_TASKLETS=m ;;
	*) unset CONFIG_RTAI_TASKLETS ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TASKLETS:-no})

if test x$CONFIG_RTAI_TASKLETS = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST tasklets"
fi

AC_MSG_CHECKING(whether LXRT wraps service calls through Linux syscalls)
AC_ARG_ENABLE(lxrt-use-linux-syscall,
	[ --enable-lxrt-use-linux-syscall	Wrap LXRT service calls throug Linux syscall mechanism],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL=y ;;
	*) unset CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL:-no})

dnl CONFIG_RTAI_KTASKS_SCHED_LXRT has to be forced
AC_ARG_ENABLE(ktasks-sched-lxrt,
	[ --enable-ktasks-sched-lxrt	Enable LXRT scheduler with kernel tasks support],
	[case "$enableval" in
	y | yes | m | mod | module) CONFIG_RTAI_KTASKS_SCHED_LXRT_=y ;;
	*) echo "--enable-ktasks-sched-lxrt forced to \"yes\"" ; CONFIG_RTAI_KTASKS_SCHED_LXRT_=y ;;
	esac])

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
	[ --enable-rtc-freq	Enable RTC freq],
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

AC_MSG_CHECKING(for 8254 tuning latency)
AC_ARG_ENABLE(sched-8254-latency,
	[ --enable-sched-8254-latency	Set 8254 tuning latency],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_SCHED_8254_LATENCY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-8254-latency=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_SCHED_8254_LATENCY = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_SCHED_8254_LATENCY=4700
fi
AC_MSG_RESULT($CONFIG_RTAI_SCHED_8254_LATENCY)

AC_MSG_CHECKING(for APIC tuning latency)
AC_ARG_ENABLE(sched-apic-latency,
	[ --enable-sched-apic-latency	Set APIC tuning latency],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_SCHED_APIC_LATENCY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-sched-apic-latency=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_SCHED_APIC_LATENCY = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_SCHED_APIC_LATENCY=3944
fi
AC_MSG_RESULT($CONFIG_RTAI_SCHED_APIC_LATENCY)

AC_MSG_CHECKING(for user busy align ret delay)
AC_ARG_ENABLE(user-busy-align-ret-delay,
	[ --user-busy-align-ret-delay	Set user busy align ret delay],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --user-busy-align-ret-delay=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY=0
fi
AC_MSG_RESULT($CONFIG_RTAI_USER_BUSY_ALIGN_RET_DELAY)

AC_MSG_CHECKING(for kernel busy align ret delay)
AC_ARG_ENABLE(kernel-busy-align-ret-delay,
	[ --kernel-busy-align-ret-delay	Set kernel busy align ret delay],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --kernel-busy-align-ret-delay=<nanoseconds>]) ;;
	esac])
if test x$CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY=0
fi
AC_MSG_RESULT($CONFIG_RTAI_KERN_BUSY_ALIGN_RET_DELAY)

AC_MSG_CHECKING(for maximum number of LXRT slots)
AC_ARG_ENABLE(sched-lxrt-numslots,
	[ --enable-sched-lxrt-numslots	Set maximum number of LXRT slots],
	[case "$enableval" in
 changequote(,)dnl
	[0-9]*) CONFIG_RTAI_SCHED_LXRT_NUMSLOTS=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-sched-lxrt-numslots=<number>]) ;;
	esac])
if test x$CONFIG_RTAI_SCHED_LXRT_NUMSLOTS = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_SCHED_LXRT_NUMSLOTS=1024
fi
AC_MSG_RESULT($CONFIG_RTAI_SCHED_LXRT_NUMSLOTS)

AC_MSG_CHECKING(for task switch signal)
AC_ARG_ENABLE(task-switch-signal,
	      [ --enable-task-switch-signal		Enable RTAI task switch signal],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TASK_SWITCH_SIGNAL=y ;;
	*) unset CONFIG_RTAI_TASK_SWITCH_SIGNAL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TASK_SWITCH_SIGNAL:-no})

AC_MSG_CHECKING(for hard-soft toggler)
AC_ARG_ENABLE(hard-soft-toggler,
	      [ --enable-hard-soft-toggler		Enable RTAI hard-soft toggler],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_HARD_SOFT_TOGGLER=y ;;
	*) unset CONFIG_RTAI_HARD_SOFT_TOGGLER ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_HARD_SOFT_TOGGLER:-no})

AC_MSG_CHECKING(for Linux task priority alignment to RTAI)
AC_ARG_ENABLE(align-priority,
	      [ --enable-align-priority		Keep Linux task priority aligned to RTAI],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_ALIGN_LINUX_PRIORITY=y ;;
	*) unset CONFIG_RTAI_ALIGN_LINUX_PRIORITY ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_ALIGN_LINUX_PRIORITY:-no})

AC_MSG_CHECKING(for calibration frequency factor)
AC_ARG_ENABLE(cal-freq-fact,
	[ --enable-cal-freq-fact      	Set calibration frequency factor],
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

AC_MSG_CHECKING([if gcc accepts -fno-use-cxa-atexit])
save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="-fno-use-cxa-atexit -Werror"
AC_CACHE_VAL(ac_cv_accepts_cxa_atexit,
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
[AC_TRY_COMPILE([],
[int i = 0; return i; ],
[ac_cv_accepts_cxa_atexit=yes],
[ac_cv_accepts_cxa_atexit=no])]
AC_LANG_RESTORE)
AC_MSG_RESULT($ac_cv_accepts_cxa_atexit)
CXXFLAGS="$save_CXXFLAGS"

AC_MSG_CHECKING(for tlsf use in real-time malloc support)
AC_ARG_ENABLE(malloc-tlsf,
	[ --enable-malloc-tlsf	Enable tlsf support in malloc],
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

AC_MSG_CHECKING(for number of CPUs)
AC_ARG_ENABLE(cpus,
	[ --enable-cpus		Enable CPUs],
	[case "$enableval" in
 changequote(,)dnl
	[1-8]) CONFIG_RTAI_CPUS=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-cpus=<number-of-CPUs>]) ;;
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

dnl FIXME: is 0-7 correct?
dnl what about larger CPU numbers (if at all possible)?
dnl should we test for valid CPU number?
AC_MSG_CHECKING(for master CPU for aligning MP-TSCs)
AC_ARG_ENABLE(master-tsc-cpu,
	[ --enable-master-tsc-cpu	Enable master TSC CPU],
	[case "$enableval" in
 changequote(,)dnl
	[0-7])
 changequote([,])dnl
		if test x$CONFIG_RTAI_CPUS = x ; then
			AC_MSG_ERROR([Option --enable-cpus must be defined before --enable-master-tsc-cpu])
		fi
		if test $enableval -lt 0 -o $enableval -ge $CONFIG_RTAI_CPUS ; then
			AC_MSG_ERROR([Invalid value to option: --enable-master-tsc-cpu=<master-CPU-number> (must be between 0 and CONFIG_RTAI_CPUS)])
		fi
		CONFIG_RTAI_MASTER_TSC_CPU=$enableval ;;
	*) AC_MSG_ERROR([Bad argument to option: --enable-master-tsc-cpu=<master-CPU-number>]) ;;
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

dnl add arch specific tests
if test x$CONFIG_X86_TSC = xy ; then
	AC_DEFINE(CONFIG_RTAI_TSC,1,[Use TSC in user space])
fi

dnl Catch sub-package flags

dnl We need to know this before building the APIC support
dnl into the SMP scheduler for the x86 platform. Autoconf imposes
dnl us to define all Automake conditionals...unconditionally. e.g.
dnl regardless of the current architecture setup.
AM_CONDITIONAL(CONFIG_X86_LOCAL_APIC,[test -n "$CONFIG_X86_LOCAL_APIC"])

if test -n "$CONFIG_RTAI_TRACE" -a x$CONFIG_TRACE = x; then
   AC_MSG_ERROR([Missing LTT support in kernel to enable trace support.])
fi

dnl Check for invalid combinations

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

if test x$CONFIG_RTAI_MQ = xy -o x$CONFIG_RTAI_MQ = xm ; then
   if test x$CONFIG_RTAI_SEM = x ; then
      AC_MSG_ERROR([--enable-mq requires --enable-sem])
   fi
fi

if test x$CONFIG_RTAI_MQ = xy ; then
   if test x$CONFIG_RTAI_SEM = xm ; then
      AC_MSG_ERROR([--enable-mq=y requires --enable-sem=y])
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

AC_DEFINE(CONFIG_RTAI,1,[Legacy])
AC_DEFINE(CONFIG_RTAI_MOUNT_ON_LOAD,1,[Legacy])
AC_DEFINE(CONFIG_RTAI_MATH,1,[Kconfig])
test \! x$CONFIG_KBUILD = x && AC_DEFINE(CONFIG_KBUILD,1,[Kconfig])
test \! x$CONFIG_RTAI_ADEOS = x && AC_DEFINE(CONFIG_RTAI_ADEOS,1,[Kconfig])
test \! x$CONFIG_RTAI_HAL_PREEMPT = x && AC_DEFINE(CONFIG_RTAI_HAL_PREEMPT,1,[Kconfig])
test \! x$CONFIG_RTAI_CPUS = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_CPUS,$CONFIG_RTAI_CPUS,[Kconfig])
test x$CONFIG_RTAI_DIAG_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_DIAG_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_MASTER_TSC_CPU = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_MASTER_TSC_CPU,$CONFIG_RTAI_MASTER_TSC_CPU,[Kconfig])
test x$CONFIG_RTAI_TUNE_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_TUNE_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_TRACE = x && AC_DEFINE(CONFIG_RTAI_TRACE,1,[Kconfig])
test \! x$CONFIG_RTAI_FIFOS = x && AC_DEFINE(CONFIG_RTAI_FIFOS,1,[Kconfig])
test \! x$CONFIG_RTAI_SEM = x && AC_DEFINE(CONFIG_RTAI_SEM,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL = x && AC_DEFINE(CONFIG_RTAI_RT_POLL,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL_ON_STACK = x && AC_DEFINE(CONFIG_RTAI_RT_POLL_ON_STACK,1,[Kconfig])
test \! x$CONFIG_RTAI_MSG = x && AC_DEFINE(CONFIG_RTAI_MSG,1,[Kconfig])
test \! x$CONFIG_RTAI_MBX = x && AC_DEFINE(CONFIG_RTAI_MBX,1,[Kconfig])
test \! x$CONFIG_RTAI_MQ = x && AC_DEFINE(CONFIG_RTAI_MQ,1,[Kconfig])
test \! x$CONFIG_RTAI_SHM = x && AC_DEFINE(CONFIG_RTAI_SHM,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_USE_TLSF = x && AC_DEFINE(CONFIG_RTAI_USE_TLSF,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC_VMALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC_VMALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_TASKLETS = x && AC_DEFINE(CONFIG_RTAI_TASKLETS,1,[Kconfig])
test \! x$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL = x && AC_DEFINE(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,1,[Kconfig])
test \! x$CONFIG_RTAI_ALIGN_LINUX_PRIORITY = x && AC_DEFINE(CONFIG_RTAI_ALIGN_LINUX_PRIORITY,1,[Kconfig])
test \! x$CONFIG_RTAI_TASK_SWITCH_SIGNAL = x && AC_DEFINE(CONFIG_RTAI_TASK_SWITCH_SIGNAL,1,[Kconfig])
test \! x$CONFIG_RTAI_HARD_SOFT_TOGGLER = x && AC_DEFINE(CONFIG_RTAI_HARD_SOFT_TOGGLER,1,[Kconfig])

test x$CONFIG_RTAI_FIFOS = xy && AC_DEFINE(CONFIG_RTAI_FIFOS_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SEM = xy && AC_DEFINE(CONFIG_RTAI_SEM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MSG = xy && AC_DEFINE(CONFIG_RTAI_MSG_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MBX = xy && AC_DEFINE(CONFIG_RTAI_MBX_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MQ = xy && AC_DEFINE(CONFIG_RTAI_MQ_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SHM = xy && AC_DEFINE(CONFIG_RTAI_SHM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MALLOC = xy && AC_DEFINE(CONFIG_RTAI_MALLOC_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_TASKLETS = xy && AC_DEFINE(CONFIG_RTAI_TASKLETS_BUILTIN,1,[Kconfig])

test x$CONFIG_RTAI_SCHED_ISR_LOCK = xy && AC_DEFINE(CONFIG_RTAI_SCHED_ISR_LOCK,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_RTC_FREQ,$CONFIG_RTAI_RTC_FREQ,[Kconfig])
test x$CONFIG_RTAI_LONG_TIMED_LIST = xy && AC_DEFINE(CONFIG_RTAI_LONG_TIMED_LIST,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_SCHED_8254_LATENCY,$CONFIG_RTAI_SCHED_8254_LATENCY,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_SCHED_APIC_LATENCY,$CONFIG_RTAI_SCHED_APIC_LATENCY,[Kconfig])
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

case $CONFIG_RTAI_ONE_SHOT in
	y) CONFIG_RTAI_ONE_SHOT=1 ;;
	n|*) CONFIG_RTAI_ONE_SHOT=0 ;;
esac
AC_DEFINE_UNQUOTED(CONFIG_RTAI_ONE_SHOT,$CONFIG_RTAI_ONE_SHOT,[Kconfig])

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
if test x$CONFIG_DEVFS_FS = xy; then
   fifos_template="/dev/rtf/%d"
else
   fifos_template="/dev/rtf%d"
fi
AC_DEFINE_UNQUOTED(CONFIG_RTAI_FIFOS_TEMPLATE,"$fifos_template",[Kconfig])

AC_DEFINE(CONFIG_RTAI_INTERNAL_LXRT_SUPPORT,1,[Kconfig])

dnl Set default inline mode for LXRT as specified from
dnl Kconfig. Defaults to static / eager inlining.

CONFIG_RTAI_LXRT_INLINE=1

AC_DEFINE_UNQUOTED(CONFIG_RTAI_INTERNAL_LXRT_INLINE,1,[Kconfig])

AH_VERBATIM(CONFIG_RTAI_LXRT_INLINE,[
#ifndef CONFIG_RTAI_LXRT_INLINE
#define CONFIG_RTAI_LXRT_INLINE CONFIG_RTAI_INTERNAL_LXRT_INLINE
#endif /* !CONFIG_RTAI_LXRT_INLINE */
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

dnl Whether Linux is using the Kbuild make system.
AM_CONDITIONAL(CONFIG_KBUILD,[test "$CONFIG_KBUILD" = y])

dnl CPU architecture
AM_CONDITIONAL(CONFIG_X86_64,[test "$CONFIG_X86_64" = y])
AM_CONDITIONAL(CONFIG_X86,[test "$CONFIG_X86" = y])
AM_CONDITIONAL(CONFIG_SMP,[test "$CONFIG_SMP" = y])

AM_CONDITIONAL(CONFIG_RTAI_TRACE,[test -n "$CONFIG_RTAI_TRACE"])
AM_CONDITIONAL(CONFIG_RTAI_MATH,[test -n "$CONFIG_RTAI_MATH"])
AM_CONDITIONAL(CONFIG_RTAI_FIFOS,[test -n "$CONFIG_RTAI_FIFOS"])
AM_CONDITIONAL(CONFIG_RTAI_SEM,[test -n "$CONFIG_RTAI_SEM"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL,[test -n "$CONFIG_RTAI_RT_POLL"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL_ON_STACK,[test -n "$CONFIG_RTAI_RT_POLL_ON_STACK"])
AM_CONDITIONAL(CONFIG_RTAI_MSG,[test -n "$CONFIG_RTAI_MSG"])
AM_CONDITIONAL(CONFIG_RTAI_MBX,[test -n "$CONFIG_RTAI_MBX"])
AM_CONDITIONAL(CONFIG_RTAI_MQ,[test -n "$CONFIG_RTAI_MQ"])
AM_CONDITIONAL(CONFIG_RTAI_SHM,[test -n "$CONFIG_RTAI_SHM"])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC,[test -n "$CONFIG_RTAI_MALLOC"])
AM_CONDITIONAL(CONFIG_RTAI_TASKLETS,[test -n "$CONFIG_RTAI_TASKLETS"])
AM_CONDITIONAL(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,[test -n "$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL"])

AM_CONDITIONAL(CONFIG_RTAI_FIFOS_BUILTIN,[test x$CONFIG_RTAI_FIFOS = xy])
AM_CONDITIONAL(CONFIG_RTAI_SEM_BUILTIN,[test x$CONFIG_RTAI_SEM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MSG_BUILTIN,[test x$CONFIG_RTAI_MSG = xy])
AM_CONDITIONAL(CONFIG_RTAI_MBX_BUILTIN,[test x$CONFIG_RTAI_MBX = xy])
AM_CONDITIONAL(CONFIG_RTAI_MQ_BUILTIN,[test x$CONFIG_RTAI_MQ = xy])
AM_CONDITIONAL(CONFIG_RTAI_SHM_BUILTIN,[test x$CONFIG_RTAI_SHM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC_BUILTIN,[test x$CONFIG_RTAI_MALLOC = xy])
AM_CONDITIONAL(CONFIG_RTAI_TASKLETS_BUILTIN,[test x$CONFIG_RTAI_TASKLETS = xy])

# OLD_FASHIONED_BUILD will be removed
AM_CONDITIONAL(CONFIG_RTAI_OLD_FASHIONED_BUILD,[test "$CONFIG_RTAI_OLD_FASHIONED_BUILD" = y])

dnl
dnl Define arch-specific CFLAGS and links
dnl

RTAI_MODULE_EXT=.ko
RTAI_KBUILD_ENV='src2obj = $(patsubst %.$(1),%.o,$(filter %.$(1),$(call notdir,$(2))))'
RTAI_KBUILD_TOP="@if test \\! -r Module.symvers ; then \$(LN_S) -f \"\`cd \$(top_builddir) && pwd\`/Module.symvers\" . ; fi;"
RTAI_KBUILD_BOTTOM=
RTAI_KBUILD_CMD="for src in \$^; do if test \\! -r \`basename \$\$src\`; then \$(LN_S) \$\$src; fi; done; \
			rm -f $RTAI_LINUX_DIR/.tmp_versions/rtai_*.mod $RTAI_LINUX_DIR/.tmp_versions/*_rt.mod; \
			\$(MAKE) -C $RTAI_LINUX_DIR CC=$CC ARCH=x86 \
			SUBDIRS=\$\$PWD M=\$\$PWD rtai_srctree=\`cd \$(top_srcdir) && pwd\` rtai_srcdir=\`cd \$(srcdir) && pwd\` V=\$(V) modules \
			rtai_objs='\$(call src2obj,c,\$^) \$(call src2obj,S,\$^) \$(call src2obj,cc,\$^) \$(call src2obj,cpp,\$^)' rtai_target=\$(basename \$@)"
RTAI_KBUILD_CLEAN='rm -f *.ko *.mod.c .*.cmd *.o Module.symvers Module.markers modules.order && rm -Rf .tmp_versions'
RTAI_KBUILD_DISTCLEAN=""

AC_MSG_CHECKING([for kernel module extension])
AC_MSG_RESULT([$RTAI_MODULE_EXT])

dnl extra arch-dependent opts to be passed to the compiler
RTAI_KMOD_CXXFLAGS="-fno-rtti -fno-exceptions -fno-strength-reduce"
RTAI_USER_CFLAGS="-O2 -Wall -Wstrict-prototypes -fomit-frame-pointer"
RTAI_FP_CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -O2 -ffreestanding -frounding-math -fsignaling-nans -mieee-fp -mhard-float -mpreferred-stack-boundary=4 -mfpmath=sse -msse -msse2 -Wno-undef"

if [[ $ac_cv_accepts_cxa_atexit = yes ]]; then
   RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CXXFLAGS -fno-use-cxa-atexit"
fi

RTAI_LXRT_LDADD=

RTAI_KMOD_APP_CFLAGS=$RTAI_KMOD_CFLAGS
RTAI_KMOD_APP_CXXFLAGS=$RTAI_KMOD_CXXFLAGS
RTAI_USER_APP_CFLAGS=$RTAI_USER_CFLAGS
RTAI_KMOD_CFLAGS="$RTAI_KMOD_CFLAGS -D__IN_RTAI__"
RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CXXFLAGS -D__IN_RTAI__"
RTAI_USER_CFLAGS="$RTAI_USER_CFLAGS -D__IN_RTAI__"

AC_CONFIG_LINKS(base/include/asm:$srcdir/base/include/asm-x86)

if test x$CONFIG_RTAI_OLD_FASHIONED_BUILD = xy; then
   AC_CONFIG_LINKS(base/include/rtai_config.h:rtai_config.h)
fi

AC_MSG_CHECKING([for RTAI target architecture])
if test x$RTAI_TARGET_SUBARCH = x; then
   AC_MSG_RESULT([x86])
else
   AC_MSG_RESULT([x86 ($RTAI_TARGET_SUBARCH)])
fi

AC_CONFIG_LINKS(base/arch/x86/hal/Makefile:base/arch/x86/hal/Makefile.kbuild)
AC_CONFIG_LINKS(base/arch/x86/calibration/Makefile:base/arch/x86/calibration/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/fifos/Makefile:base/ipc/fifos/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/mbx/Makefile:base/ipc/mbx/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/mq/Makefile:base/ipc/mq/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/msg/Makefile:base/ipc/msg/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/sem/Makefile:base/ipc/sem/Makefile.kbuild)
AC_CONFIG_LINKS(base/ipc/shm/Makefile:base/ipc/shm/Makefile.kbuild)
AC_CONFIG_LINKS(base/posix/Makefile:base/posix/Makefile.kbuild)
AC_CONFIG_LINKS(base/malloc/Makefile:base/malloc/Makefile.kbuild)
AC_CONFIG_LINKS(base/tasklets/Makefile:base/tasklets/Makefile.kbuild)
AC_CONFIG_LINKS(base/math/Makefile:base/math/Makefile.kbuild)
AC_CONFIG_LINKS(base/sched/Makefile:base/sched/Makefile.kbuild)

dnl
dnl POSIX wraps
dnl
RTAI_POSIX_WRAP=`for f in \`cat "${srcdir}/base/posix/POSIX_WRAPS"\`; do echo -n " -Wl,--wrap -Wl,$f" ; done`

dnl
dnl Build the Makefiles
dnl

AC_SUBST(RTAI_BUILD_STRING)
AC_SUBST(RTAI_HOST_STRING)
AC_SUBST(RTAI_TARGET_SUBARCH)
AC_SUBST(RTAI_BUILTIN_MODLIST)
AC_SUBST(RTAI_KMOD_CFLAGS)
AC_SUBST(RTAI_KMOD_CXXFLAGS)
AC_SUBST(RTAI_COMPAT_CPPFLAGS)
AC_SUBST(RTAI_USER_CFLAGS)
AC_SUBST(RTAI_KMOD_APP_CFLAGS)
AC_SUBST(RTAI_KMOD_APP_CXXFLAGS)
AC_SUBST(RTAI_USER_APP_CFLAGS)
AC_SUBST(RTAI_LXRT_LDADD)
AC_SUBST(RTAI_FP_CFLAGS)
AC_SUBST(RTAI_LINUX_DIR)
AC_SUBST(RTAI_LINUX_VERSION)
AC_SUBST(RTAI_MODULE_DIR)
AC_SUBST(RTAI_MODULE_EXT)
AC_SUBST(RTAI_KBUILD_ENV)
AC_SUBST(RTAI_KBUILD_TOP)
AC_SUBST(RTAI_KBUILD_BOTTOM)
AC_SUBST(RTAI_KBUILD_CMD)
AC_SUBST(RTAI_KBUILD_CLEAN)
AC_SUBST(RTAI_KBUILD_DISTCLEAN)
AC_SUBST(RTAI_POSIX_WRAP)

AC_SUBST(CONFIG_RTAI_LINUXDIR)

AC_CONFIG_FILES([ \
	GNUmakefile \
	base/GNUmakefile \
	base/arch/GNUmakefile \
	base/include/GNUmakefile \
	base/ipc/GNUmakefile \
	base/sched/GNUmakefile \
	base/sched/liblxrt/GNUmakefile \
	base/scripts/GNUmakefile \
	base/scripts/rtai-config \
	base/scripts/rtai-load \
	base/arch/x86/GNUmakefile \
	base/arch/x86/hal/GNUmakefile \
	base/arch/x86/calibration/GNUmakefile \
	base/arch/x86/calibration/setsmi \
	base/include/asm-x86/GNUmakefile \
	base/ipc/fifos/GNUmakefile \
	base/ipc/shm/GNUmakefile \
	base/ipc/sem/GNUmakefile \
	base/ipc/msg/GNUmakefile \
	base/ipc/mbx/GNUmakefile \
	base/ipc/mq/GNUmakefile \
	base/math/GNUmakefile \
	base/malloc/GNUmakefile \
	base/tasklets/GNUmakefile \
	base/trace/GNUmakefile \
	base/posix/GNUmakefile
	])

AC_OUTPUT
