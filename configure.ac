# Copyright (C) 2005-2017 The RTAI project
# This [file] is free software; the RTAI project
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)

AC_INIT(rtai,5.2,rtai@rtai.org)

CONFIG_RTAI_VERSION_MAJOR=5
CONFIG_RTAI_VERSION_MINOR=0
CONFIG_RTAI_REVISION_LEVEL=2

AC_CONFIG_HEADERS(rtai_config.h)
AC_CONFIG_AUX_DIR(base/config/autoconf)
AC_CONFIG_SRCDIR(base/sched/sched.c)
AC_PREFIX_DEFAULT(/usr/realtime)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_PROG_INSTALL

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

dnl set only if undefined
if test x"$CROSS_COMPILE" = x ; then
	CROSS_COMPILE=
	using_crosscompile=n
else
	using_crosscompile=y
fi
case "$host" in
 m68knommu-*)
	enable_shared=no
	if test "$using_crosscompile" = y; then
		LDFLAGS="-Wl,-elf2flt"
		CROSSCOMPILE_TEST="tools/ucfront-gcc "$CROSS_COMPILE"gcc"
	fi
	;;
esac
AC_MSG_CHECKING([for CROSS_COMPILE])
AC_MSG_RESULT([$CROSS_COMPILE])

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

AC_DEFINE_UNQUOTED(CONFIG_RTAI_BUILD_STRING,"$build",[Build system alias])
RTAI_BUILD_STRING="$build"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_HOST_STRING,"$host",[Host system alias])
RTAI_HOST_STRING="$host"
AC_DEFINE_UNQUOTED(CONFIG_RTAI_COMPILER,"`$CC -v 2>&1 | tail -n 1`",[Compiler])

AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE
AC_PROG_LIBTOOL
AM_PROG_AS
dnl Find arch-specific requirements and supports. Features which
dnl are not supported by all archs must be listed here, so that a
dnl an Autoconf flag can later be defined/undefined in rtai_config.h.

arch_supports_lxrt=
arch_supports_leds=
arch_supports_sim=
arch_supports_rtailab=
arch_has_mmu=y

case "$host" in
 i*86*-*)
	RTAI_TARGET_ARCH=x86
	RTAI_TARGET_SUBARCH=32
	arch_supports_lxrt=y
	arch_supports_leds=y
	arch_supports_sim=y
	arch_supports_rtailab=y
	wanted_kernel_arch=CONFIG_X86_32
        ;;
 x86_64*-*)
        RTAI_TARGET_ARCH=x86
	RTAI_TARGET_SUBARCH=64
        arch_supports_lxrt=y
        arch_supports_rtailab=n
        wanted_kernel_arch=CONFIG_X86_64
        ;;
 arm-*)
	RTAI_TARGET_ARCH=arm
	wanted_kernel_arch=CONFIG_ARM
        ;;
 powerpc-*|ppc-*)
	RTAI_TARGET_ARCH=ppc
	wanted_kernel_arch=CONFIG_PPC
        ;;
 m68knommu-*)
 	RTAI_TARGET_ARCH=m68k
	wanted_kernel_arch=CONFIG_M68K
	arch_has_mmu=
	;;
 m68k-*)
	RTAI_TARGET_ARCH=m68k
	wanted_kernel_arch=CONFIG_M68K
	;;
 *) echo ""
   echo "*******************************************"
   echo "Still unsupported host: $host -- Sorry."
   echo "*******************************************"
   echo ""
   exit 1;;
esac

RTAI_INSTALL_DIR=

dnl
dnl Load the RTAI configuration.
dnl

RTAI_KCONFIG_FILE=$srcdir/base/arch/$RTAI_TARGET_ARCH/defconfig
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

dnl Are we building into the source tree?
if test "$srcdir" = `pwd`; then
   CONFIG_RTAI_OLD_FASHIONED_BUILD=y
else
   CONFIG_RTAI_OLD_FASHIONED_BUILD=
fi

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
	"" )
	    if test x${CONFIG_RTAI_LINUXDIR} = x; then
		AC_MSG_ERROR([You must supply an argument to --with-linux-dir.])
	    else
		withval=${CONFIG_RTAI_LINUXDIR}
	    fi
	  ;;
	y | ye | yes | n | no)
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
dnl define the name of the target arch (only matters for powerpc so far)
dnl
case $RTAI_TARGET_ARCH in
ppc|powerpc)
	dnl $linux_VERSION.$linux_PATCHLEVEL.$linux_SUBLEVEL
	if test $linux_VERSION -gt 2 ; then
		ARCH_SUBDIR=powerpc
	elif test $linux_VERSION -eq 2 -a $linux_PATCHLEVEL -lt 6 ; then
		ARCH_SUBDIR=ppc
	elif test "$linux_VERSION.$linux_PATCHLEVEL" = "2.6" \
			-a $linux_SUBLEVEL -gt 28 ; then
		ARCH_SUBDIR=powerpc
	else
		ARCH_SUBDIR=ppc
	fi
	RTAI_TARGET_ARCH_NAME=$ARCH_SUBDIR
	;;
m68k)
	if test x$arch_has_mmu = x; then
		RTAI_TARGET_ARCH_NAME=m68knommu
	else
		RTAI_TARGET_ARCH_NAME=$RTAI_TARGET_ARCH
	fi
	;;
*)
	RTAI_TARGET_ARCH_NAME=$RTAI_TARGET_ARCH
	;;
esac

dnl
dnl Identify the base technology used.
dnl

unset CONFIG_RTAI_ADEOS
unset CONFIG_RTAI_IPIPE

AC_MSG_CHECKING([for RTAI base technology])

dnl we simply check for {adeos,ipipe}.h; whatever is available
dnl is used, regardless of the version of the Linux sources.
if test -r $LINUX_KERNELSRC/include/linux/adeos.h; then
   AC_DEFINE(CONFIG_RTAI_ADEOS,1,[Use RTAI Adeos-oldgen HAL])
   AC_MSG_RESULT([RTAI Adeos-oldgen HAL])
   CONFIG_RTAI_ADEOS=y
elif test -r $LINUX_KERNELSRC/include/linux/ipipe.h; then
   AC_DEFINE(CONFIG_RTAI_IPIPE,1,[Use RTAI Adeos-newgen HAL])
   AC_MSG_RESULT([RTAI Adeos-newgen HAL])
   CONFIG_RTAI_IPIPE=y
else
    echo
    AC_MSG_ERROR([HAL patch not applied/enabled in $LINUX_KERNELSRC])
fi

case "$RTAI_LINUX_VERSION" in
dnl  2.4.*)
dnl	dnl by design, only RTAI Adeos-oldgen HAL is available with 2.4
dnl	unset CONFIG_KBUILD
dnl    ;;
dnl  2.6.*)
dnl	dnl by design, RTAI Adeos-oldgen HAL is supported up to 2.6.12 included,
dnl	dnl and RTAI Adeos-newgen HAL is supported since 2.6.13 included
dnl	CONFIG_KBUILD=y
dnl    ;;
  3.*)
	CONFIG_KBUILD=y
    ;;
  4.*)
	CONFIG_KBUILD=y
    ;;
  *) 
    AC_MSG_ERROR([Unsupported kernel version $RTAI_LINUX_VERSION])
esac

dnl
dnl Perform a few sanity checks.
dnl

AC_MSG_CHECKING([for kernel architecture])
arch_select=`eval echo \\$$wanted_kernel_arch`
if test x$arch_select = x; then
   AC_MSG_RESULT([FAIL])
   echo
   echo "************************************************************"
   echo " The target RTAI architecture ($RTAI_TARGET_ARCH) does not match"
   echo " the kernel architecture selected in $RTAI_LINUX_DIR"
   echo " (i.e. $wanted_kernel_arch is undefined)"
   echo "************************************************************"
   exit 1
fi
AC_MSG_RESULT([$wanted_kernel_arch])

case "${RTAI_LINUX_VERSION}" in
2.4.*)
	dnl MMU not supported in 2.4
	;;
*)
	AC_MSG_CHECKING([for MMU support])
	mmu_select=`eval echo \\$CONFIG_MMU`
	if test x$mmu_select != x$arch_has_mmu; then
		echo
		echo "************************************************************"
		echo " The target RTAI architecture MMU check failed:"
		if test x$mmu_select = x; then
			echo " MMU is unsupported by Linux architecure, but it must."
		else
			echo " MMU is supported by Linux architecure, but it must not."
		fi
		echo "************************************************************"
		exit 1
	fi
	if test x$mmu_select != x; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	;;
esac

if test x${CONFIG_MODULES} = x ; then
  AC_MSG_ERROR([Please set CONFIG_MODULES for $RTAI_LINUX_DIR and rebuild your kernel])
fi

dnl No longer used.
dnl if test x${CONFIG_REGPARM} = xy ; then
dnl   AC_MSG_ERROR([Please unset CONFIG_REGPARM for $RTAI_LINUX_DIR and 
dnl rebuild your kernel])
dnl fi

if test x${CONFIG_MODVERSIONS} = xy ; then
	case "${RTAI_LINUX_VERSION}" in
	2.4.*)
  AC_MSG_ERROR([Please unset CONFIG_MODVERSIONS for $RTAI_LINUX_DIR and rebuild your kernel])
		;;
	*)
		dnl reported to be harmless in 2.6
		;;
	esac
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

if test x${CONFIG_RTAI_MATH} = xy; then
   dnl Override Kconfig settings for now
   CONFIG_RTAI_MATH=m
fi

AC_MSG_CHECKING(for math support)
AC_ARG_ENABLE(math,
	[ --enable-math		Enable math support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_MATH=y ;;
	m | mod | module) CONFIG_RTAI_MATH=m ;;
	*) unset CONFIG_RTAI_MATH ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MATH:-no})

RTAI_MATH_LIBM=

if test x$CONFIG_RTAI_MATH = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST math"
fi

if test x$CONFIG_RTAI_MATH != x ; then
  AC_MSG_CHECKING(for libm dir/file for math support)
  AC_ARG_WITH(math-libm-dir,
	      [ --with-math-libm-dir	libm dir/file for math support])
  if test x"$CONFIG_RTAI_MATH_LIBM_DIR" = x ; then
    dnl "safe" default?
    RTAI_MATH_LIBM="/usr/lib/libm.a"
  elif test -f "$CONFIG_RTAI_MATH_LIBM_DIR" ; then
    RTAI_MATH_LIBM="$CONFIG_RTAI_MATH_LIBM_DIR"
  elif test -d "$CONFIG_RTAI_MATH_LIBM_DIR" ; then
    RTAI_MATH_LIBM="$CONFIG_RTAI_MATH_LIBM_DIR/libm.a"
  else
    AC_MSG_ERROR([Bad argument $withvar to --with-math-libm-dir])
  fi
  AC_MSG_RESULT([libm: $RTAI_MATH_LIBM])
fi

AC_MSG_CHECKING(for bits IPC support)
AC_ARG_ENABLE(bits,
	[ --enable-bits		Enable bits IPC support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_BITS=y ;;
	m | mod | module) CONFIG_RTAI_BITS=m ;;
	*) unset CONFIG_RTAI_BITS ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_BITS:-no})

if test x$CONFIG_RTAI_BITS = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/bits"
fi

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

AC_MSG_CHECKING(for netrpc support)
AC_ARG_ENABLE(netrpc,
	[ --enable-netrpc	Enable netrpc support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_NETRPC=y ;;
	m | mod | module) CONFIG_RTAI_NETRPC=m ;;
	*) unset CONFIG_RTAI_NETRPC ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_NETRPC:-no})

if test x$CONFIG_RTAI_NETRPC = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/netrpc"
fi

AC_MSG_CHECKING(for netrpc emulation of RTNet)
AC_ARG_ENABLE(netrpc-rtnet,
	[ --enable-netrpc-rtnet	Enable netrpc emulation of RTNet],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_NETRPC_RTNET=y ;;
	*) unset CONFIG_RTAI_NETRPC_RTNET ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_NETRPC_RTNET:-no})

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

AC_MSG_CHECKING(for typed mailbox support)
AC_ARG_ENABLE(tbx,
	[ --enable-tbx		Enable typed mailbox support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_TBX=y ;;
	m | mod | module) CONFIG_RTAI_TBX=m ;;
	*) unset CONFIG_RTAI_TBX ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TBX:-no})

if test x$CONFIG_RTAI_TBX = xy ; then
  RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST ipc/tbx"
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

AC_MSG_CHECKING(for user-space interrupt support)
AC_ARG_ENABLE(usi,
	[ --enable-usi		Enable user-space interrupt support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_USI=y ;;
	*) unset CONFIG_RTAI_USI ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USI:-no})

AC_MSG_CHECKING(for watchdog support)
AC_ARG_ENABLE(watchdog,
	[ --enable-watchdog	Enable watchdog support],
	[case "$enableval" in
	y | yes | m | mod | module) CONFIG_RTAI_WD=m ;;
	*) unset CONFIG_RTAI_WD ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_WD:-no})

dnl AC_MSG_CHECKING(for leds-based debugging support)
dnl AC_ARG_ENABLE(leds,
dnl 	[ --enable-leds		Enable leds-based debugging support],
dnl 	[case "$enableval" in
dnl 	y | yes | b | builtin | built-in) CONFIG_RTAI_LEDS=y ;;
dnl 	m | mod | module) CONFIG_RTAI_LEDS=m ;;
dnl 	*) unset CONFIG_RTAI_LEDS ;;
dnl 	esac])
dnl AC_MSG_RESULT(${CONFIG_RTAI_LEDS:-no})
dnl 
dnl if test x$CONFIG_RTAI_LEDS = xy ; then
dnl   RTAI_BUILTIN_MODLIST="$RTAI_BUILTIN_MODLIST leds"
dnl fi

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

AC_MSG_CHECKING(for using RTAI way for user-kernel space on stack args exchange)
AC_ARG_ENABLE(use-stack-args,
	[ --enable-use-stack-args       Keep using RTAI way for user-kernel space on stack args exchange],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_USE_STACK_ARGS=y ;;
	*) unset CONFIG_RTAI_USE_STACK_ARGS;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USE_STACK_ARGS:-no})

AC_MSG_CHECKING(for latency self calibration metrics)
AC_ARG_ENABLE(latency-self-calibration-metrics,
	[ --enable-latency-self-calibration-metrics	Set latency self calibration metrics],
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
AC_ARG_ENABLE(latency-self-calibration-freq,
	[ --enable-latency-self-calibration-freq	Set latency self calibration freq],
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
AC_ARG_ENABLE(latency-self-calibration-cycles,
	[ --enable-latency-self-calibration-cycles	Set latency self calibration cycles],
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
  CONFIG_RTAI_SCHED_LXRT_NUMSLOTS=150
fi
AC_MSG_RESULT($CONFIG_RTAI_SCHED_LXRT_NUMSLOTS)

AC_MSG_CHECKING(for RTDM support)
AC_ARG_ENABLE(rtdm,
	[ --enable-rtdm		Enable RTDM support],
	[case "$enableval" in
	y | yes | b | builtin | built-in) CONFIG_RTAI_RTDM=y ;;
	m | mod | module) CONFIG_RTAI_RTDM=m ;;
	*) unset CONFIG_RTAI_RTDM ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_RTDM:-no})

AC_MSG_CHECKING(for maximum file descriptors in RTDM)
AC_ARG_ENABLE(rtdm-fd-max,
	      [ --enable-rtdm-fd-max        	Set maximum number of RTDM file descriptor slots],
	[case "$enableval" in
 changequote(,)dnl
	[1-9][0-9]*) CONFIG_RTAI_RTDM_FD_MAX=$enableval ;;
 changequote([,])dnl
	*) AC_MSG_ERROR([Bad argument to option: --enable-rtdm-fd-max=<number>]) ;;
	esac])
if test x$CONFIG_RTAI_RTDM_FD_MAX = x ; then
  # Use a reasonable default if unset
  CONFIG_RTAI_RTDM_FD_MAX=128
else
	case $CONFIG_RTAI_RTDM_FD_MAX in
	changequote(,)dnl
	[1-9][0-9]*) ;;
	changequote([,])dnl
	*) AC_MSG_ERROR([Bad value $CONFIG_RTAI_RTDM_FD_MAX for RTAI_RTDM_FD_MAX]) ;;
	esac
fi
AC_MSG_RESULT($CONFIG_RTAI_RTDM_FD_MAX)

AC_MSG_CHECKING(for shared interrupts in RTDM)
AC_ARG_ENABLE(rtdm-shirq,
	      [ --enable-rtdm-shirq	Enable shared interrupts in RTDM],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_RTDM_SHIRQ=y ;;
	*) unset CONFIG_RTAI_RTDM_SHIRQ ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_RTDM_SHIRQ:-no})

AC_MSG_CHECKING(for select(2) support in RTDM)
AC_ARG_ENABLE(rtdm-select,
	      [ --enable-rtdm-select	Enable select support in RTDM],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_RTDM_SELECT=y ;;
	*) unset CONFIG_RTAI_RTDM_SELECT ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_RTDM_SELECT:-no})

AC_MSG_CHECKING(for RTDM debug)
AC_ARG_ENABLE(debug-rtdm,
	      [ --enable-debug-rtdm		Enable RTDM debug],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_DEBUG_RTDM=y ;;
	*) unset CONFIG_RTAI_DEBUG_RTDM ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_DEBUG_RTDM:-no})

AC_MSG_CHECKING(for task switch signal)
AC_ARG_ENABLE(task-switch-signal,
	      [ --enable-task-switch-signal		Enable RTAI task switch signal],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TASK_SWITCH_SIGNAL=y ;;
	*) unset CONFIG_RTAI_TASK_SWITCH_SIGNAL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TASK_SWITCH_SIGNAL:-no})

dnl AC_MSG_CHECKING(for hard-soft toggler)
dnl AC_ARG_ENABLE(hard-soft-toggler,
dnl 	      [ --enable-hard-soft-toggler		Enable RTAI hard-soft toggler],
dnl 	[case "$enableval" in
dnl 	y | yes) CONFIG_RTAI_HARD_SOFT_TOGGLER=y ;;
dnl 	*) unset CONFIG_RTAI_HARD_SOFT_TOGGLER ;;
dnl 	esac])
dnl AC_MSG_RESULT(${CONFIG_RTAI_HARD_SOFT_TOGGLER:-no})

AC_MSG_CHECKING(for Linux task priority alignment to RTAI)
AC_ARG_ENABLE(align-priority,
	      [ --enable-align-priority		Keep Linux task priority aligned to RTAI],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_ALIGN_LINUX_PRIORITY=y ;;
	*) unset CONFIG_RTAI_ALIGN_LINUX_PRIORITY ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_ALIGN_LINUX_PRIORITY:-no})

AC_MSG_CHECKING(for in-kernel C++ support)
AC_ARG_ENABLE(cplusplus,
	[ --enable-cplusplus	Build in-kernel C++ support],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_CPLUSPLUS=y ;;
	*) unset CONFIG_RTAI_CPLUSPLUS ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_CPLUSPLUS:-no})

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

AC_MSG_CHECKING(for comedi support over LXRT)
AC_ARG_ENABLE(comedi-lxrt,
	[ --enable-comedi-lxrt	Enable comedi/LXRT support],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_COMEDI_LXRT=y ;;
	*) unset CONFIG_RTAI_COMEDI_LXRT ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_COMEDI_LXRT:-no})

CONFIG_RTAI_USE_LINUX_COMEDI=n
if test x$CONFIG_RTAI_COMEDI_LXRT = xy; then
	SC_PATH_COMEDI($CONFIG_RTAI_COMEDI_DIR)
	if test "${RTAI_LINUX_DIR}" = "${CONFIG_RTAI_COMEDI_DIR:0:${#RTAI_LINUX_DIR}}" ; then
		CONFIG_RTAI_USE_LINUX_COMEDI=y
	fi
fi

AC_MSG_CHECKING(for comedi lock)
AC_ARG_ENABLE(comedi-lock,
	[ --enable-comedi-lock	Enable comedi lock ],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_USE_COMEDI_LOCK=y ;;
	*) unset CONFIG_RTAI_USE_COMEDI_LOCK ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USE_COMEDI_LOCK:-no})

AC_MSG_CHECKING(for real-time serial driver)
AC_ARG_ENABLE(serial,
	[ --enable-serial	Build real-time serial driver],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_DRIVERS_SERIAL=y ;;
	*) unset CONFIG_RTAI_DRIVERS_SERIAL ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_DRIVERS_SERIAL:-no})

AC_MSG_CHECKING(for real-time serial driver)
AC_ARG_ENABLE(16550a,
	[ --enable-16550a Build real-time serial driver],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_DRIVERS_16550A=y ;;
	*) unset CONFIG_RTAI_DRIVERS_16550A;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_DRIVERS_16550A:-no})

dnl Set default inline mode for LXRT as specified from
dnl Kconfig. Defaults to conditional inlining.

if test x$CONFIG_RTAI_16550A_PIO = xy; then
   CONFIG_RTAI_16550A_HAM=1
elif test x$CONFIG_RTAI_16550A_MMIO = xy; then
   CONFIG_RTAI_16550A_HAM=2
elif test x$CONFIG_RTAI_16550A_ANY = xy; then
   CONFIG_RTAI_16550A_HAM=3
fi

AC_MSG_CHECKING(for real-time serial driver hardware access mode)
AC_ARG_WITH(16550a-ham,
	[ --with-16550a-ham	Set real-time serial driver hardware access mode (pio|mmio|any)],
	[case "$withval" in
	pio) CONFIG_RTAI_16550A_HAM=1 ;;
	mmio) CONFIG_RTAI_16550A_HAM=2 ;;
	any) CONFIG_RTAI_16550A_HAM=3 ;;
	*) AC_MSG_ERROR([Bad argument to option: --with-16550a-ham=<pio|mmio|any>]) ;;
	esac])
case "$CONFIG_RTAI_" in
  1) ham_type=pio ;;
  2) ham_type=mmio ;;
  3) ham_type=any ;;
esac
AC_MSG_RESULT([$ham_type])

AC_MSG_CHECKING(for testsuite)
AC_ARG_ENABLE(testsuite,
	[ --enable-testsuite	Build RTAI testsuite],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_TESTSUITE=y ;;
	*) unset CONFIG_RTAI_TESTSUITE ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_TESTSUITE:-no})

AC_MSG_CHECKING(for RTAI-Lab)
AC_ARG_ENABLE(rtailab,
	[ --enable-rtailab	Build RTAI-Lab],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_LAB=y ;;
	*) unset CONFIG_RTAI_LAB ;;
	esac])
if test x$arch_supports_rtailab = x; then
  unset CONFIG_RTAI_LAB
fi
AC_MSG_RESULT(${CONFIG_RTAI_LAB:-no})

if test x$CONFIG_RTAI_LAB = xy; then
AC_MSG_CHECKING(for gcc version)
AC_CACHE_VAL(ac_cv_mingcc,
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
[AC_TRY_COMPILE(
[#if !( __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
#error GCC too old
#endif],
[return 0;],
[ac_cv_mingcc=yes],
[ac_cv_mingcc=no])]
AC_LANG_RESTORE)
if [[ $ac_cv_mingcc = no ]]; then
  AC_MSG_ERROR([Building RTAI-Lab requires GCC version >= 3.1])
else
  AC_MSG_RESULT([ok])
fi
MDL_HAVE_OPENGL
SC_PATH_EFLTK($CONFIG_RTAI_EFLTK_DIR)
fi

AC_MSG_CHECKING(for FPU support)
AC_ARG_ENABLE(fpu,
	[ --enable-fpu		Enable FPU support],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_FPU_SUPPORT=y ;;
	*) unset CONFIG_RTAI_FPU_SUPPORT ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_FPU_SUPPORT:-no})

AC_MSG_CHECKING(for math C99 support)
AC_ARG_ENABLE(math-c99,
	[ --enable-math-c99	Enable math C99 support],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_MATH_C99=y ;;
	*) unset CONFIG_RTAI_MATH_C99 ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_MATH_C99:-no})

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

AC_MSG_CHECKING(for number of CPUs (SMP-only))
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

dnl
dnl Check whether compile calibration/smispv
dnl
CONFIG_BUILD_SMISPV=n
SENSORS_LIBS=
if test $RTAI_TARGET_ARCH = x86 ; then
	dnl check for libsensors
	AC_MSG_CHECKING(for libsensors)
	save_LIBS="$LIBS"
	LIBS="$LIBS -lsensors"
	link_sensors=n
	AC_TRY_LINK([
#include <sensors/sensors.h>
		],[
	(void)sensors_init(0);
		],[link_sensors=y])
	LIBS="$save_LIBS"

	if test $link_sensors = y ; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(CONFIG_RTAI_HAVE_SENSORS,1,[have libsensors])
		SENSORS_LIBS="-lsensors"
	else
		AC_MSG_RESULT(no)
	fi

	dnl check for libpci
	AC_MSG_CHECKING(for libpci)
	save_LIBS="$LIBS"
	LIBS="$LIBS -lpci"
	link_pci=n
	AC_TRY_LINK([
#include <pci/pci.h>
		],[
	(void)pci_init(0);
		],[link_pci=y])
	LIBS="$save_LIBS"

	if test $link_pci = y ; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_PCI,1,[have libpci])

		CONFIG_BUILD_SMISPV=y
	else
		AC_MSG_RESULT(no)
	fi

	if test $CONFIG_BUILD_SMISPV = y ; then
		if test $link_sensors = n ; then
			AC_MSG_RESULT(    ... smispv will be built WITHOUT temperature monitoring)
		else
			AC_MSG_RESULT(    ... smispv will be built)
		fi
	else
		AC_MSG_RESULT(    ... smispv will NOT be built)
	fi
fi

AC_SUBST(SENSORS_LIBS)
AM_CONDITIONAL(CONFIG_BUILD_SMISPV,[test "$CONFIG_BUILD_SMISPV" = y])

dnl
dnl Check for additional build options.
dnl

if test \! -d $srcdir/doc; then 
    unset RTAI_MAYBE_DOCDIR
else
    RTAI_MAYBE_DOCDIR=doc
fi
AC_SUBST(RTAI_MAYBE_DOCDIR)

AC_MSG_CHECKING(for Doxygen documentation)
AC_ARG_ENABLE(doc,
	[ --enable-dox-doc	Build Doxygen documentation],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_DOX_DOC=y ;;
	*) unset CONFIG_RTAI_DOX_DOC ;;
	esac])

if test \! -d $srcdir/doc; then
    if test x$CONFIG_RTAI_DOX_DOC = xy ; 
    then
        AC_MSG_ERROR([documentation tree is missing.])
    fi
    AC_MSG_RESULT([not present])
else
    AC_MSG_RESULT(${CONFIG_RTAI_DOX_DOC:-no})
fi

AC_CHECK_PROG(DOXYGEN, doxygen, doxygen)

if test x${CONFIG_RTAI_DOX_DOC} = xy -a x"$DOXYGEN" = x; then
   AC_MSG_ERROR([Missing the Doxygen tools to generate the RTAI documentation.])
fi

AC_MSG_CHECKING(doxygen version)
if test x"$DOXYGEN" != x; then
   dox_version=`"$DOXYGEN" --version`
fi
AC_MSG_RESULT(${dox_version:-not installed})

if test x${CONFIG_RTAI_DOX_DOC} = xy
then
   dox_version_number=`echo $dox_version | sed 'y,., ,' | {
        read dox_major dox_minor dox_rev dox_patch
        expr $dox_major \* 10000 + $dox_minor \* 100 + $dox_rev
   }`
   if test $dox_version_number -lt 10304
   then
        AC_MSG_ERROR([Doxygen version 1.3.4 or higher is needed.])
   fi
fi

AC_CHECK_PROG(DOXYGEN_HAVE_DOT, dot, YES, NO)
if test x"$DOXYGEN_HAVE_DOT" = xYES; then
   DOXYGEN_SHOW_INCLUDE_FILES=NO
else
   DOXYGEN_SHOW_INCLUDE_FILES=YES
fi

#case "$CONFIG_RTAI_DBX_DOC" in
#y | ye | yes)
#    DBX_DOC=yes;;
#*)
#    DBX_DOC="";;
#esac
#DBX_NET="$CONFIG_RTAI_DBX_NET"
#DBX_ROOT="$CONFIG_RTAI_DBX_ROOT"
#DBX_XSL_ROOT="$CONFIG_RTAI_DBX_XSL_ROOT"
#DBX_DOC_INIT(doc/docbook, doc/generated, 4.2)

AC_MSG_CHECKING(whether using LaTeX non-stop mode)
AC_ARG_ENABLE(verbose-latex,
        [ --enable-verbose-latex Uses LaTeX non-stop mode],
        [case "$enableval" in
        y | yes) CONFIG_RTAI_DOC_LATEX_NONSTOP=y ;;
        *) unset CONFIG_RTAI_DOC_LATEX_NONSTOP ;;
        esac])
AC_MSG_RESULT(${CONFIG_RTAI_DOC_LATEX_NONSTOP:-no})

if test x${CONFIG_RTAI_DOC_LATEX_NONSTOP} = xy; then
   LATEX_BATCHMODE=NO
   LATEX_MODE=nonstopmode
else
   LATEX_BATCHMODE=YES
   LATEX_MODE=batchmode
fi

dnl Set default inline mode for LXRT as specified from
dnl Kconfig. Defaults to conditional inlining.

if test x$CONFIG_RTAI_LXRT_STATIC_INLINE = xy; then
   CONFIG_RTAI_LXRT_INLINE=1
elif test x$CONFIG_RTAI_LXRT_NO_INLINE = xy; then
   CONFIG_RTAI_LXRT_INLINE=3
else
   CONFIG_RTAI_LXRT_INLINE=2
fi

AC_MSG_CHECKING(for LXRT inlining)
AC_ARG_WITH(lxrt-inline,
	[ --with-lxrt-inline	Set LXRT inlining mode (static|extern|none)],
	[case "$withval" in
	static) CONFIG_RTAI_LXRT_INLINE=1 ;;
	extern) CONFIG_RTAI_LXRT_INLINE=2 ;;
	none) CONFIG_RTAI_LXRT_INLINE=3 ;;
	*) AC_MSG_ERROR([Bad argument to option: --with-lxrt-inline=<static|extern|none>]) ;;
	esac])
case "$CONFIG_RTAI_LXRT_INLINE" in
  1) inline_type=static ;;
  2) inline_type=extern ;;
  3) inline_type=none ;;
esac
AC_MSG_RESULT([$inline_type])

AC_MSG_CHECKING(for debug symbols in kernel modules)
AC_ARG_ENABLE(module-debug,
	[ --enable-module-debug	Enable debug information in kernel modules],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_KMOD_DEBUG=y ;;
	*) unset CONFIG_RTAI_KMOD_DEBUG ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_KMOD_DEBUG:-no})

AC_MSG_CHECKING(for debug symbols in user-space programs)
AC_ARG_ENABLE(user-debug,
	[ --enable-user-debug	Enable debug information in user-space programs],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_USER_DEBUG=y ;;
	*) unset CONFIG_RTAI_USER_DEBUG ;;
	esac])
AC_MSG_RESULT(${CONFIG_RTAI_USER_DEBUG:-no})

dnl Catch sub-package flags

AC_MSG_CHECKING(for RTAI simulator)
AC_ARG_ENABLE(sim,
	[ --enable-sim		Build RTAI simulator (MVM) ],
	[case "$enableval" in
	y | yes) CONFIG_RTAI_MVM=y ;;
	*) unset CONFIG_RTAI_MVM ;;
	esac])

unset RTAI_MAYBE_SIMDIR
if test \! -d $srcdir/rtai-sim; then
    if test x$CONFIG_RTAI_MVM = xy; then
        AC_MSG_ERROR([simulator is missing.])
    fi
    AC_MSG_RESULT([not present])
else
    if test x$CONFIG_RTAI_MVM = xy; then
	export CONFIG_RTAI_MVM_DEBUG
    	export CONFIG_RTAI_MVM_GCC_TARBALL
    else
	RTAI_MAYBE_SIMDIR=rtai-sim
    fi
    AC_MSG_RESULT(${CONFIG_RTAI_MVM:-no})
fi
if test x$arch_supports_sim = x -a x$CONFIG_RTAI_MVM = xy; then
   AC_MSG_ERROR([simulator is not available on this architecture.])
fi
AC_SUBST(RTAI_MAYBE_SIMDIR)

dnl We need to know this before building the APIC support
dnl into the SMP scheduler for the x86 platform. Autoconf imposes
dnl us to define all Automake conditionals...unconditionally. e.g.
dnl regardless of the current architecture setup.
AM_CONDITIONAL(CONFIG_X86_LOCAL_APIC,[test -n "$CONFIG_X86_LOCAL_APIC"])

if test -n "$CONFIG_RTAI_TRACE" -a x$CONFIG_TRACE = x; then
   AC_MSG_ERROR([Missing LTT support in kernel to enable trace support.])
fi

dnl Check for invalid combinations

if test x$CONFIG_RTAI_NETRPC = xy -o x$CONFIG_RTAI_NETRPC = xm ; then
   if test x$CONFIG_RTAI_MSG = x ; then
      AC_MSG_ERROR([--enable-netrpc requires --enable-msg])
   fi
fi

if test x$CONFIG_RTAI_NETRPC = xy ; then
   if test x$CONFIG_RTAI_MSG = xm ; then
      AC_MSG_ERROR([--enable-netrpc=y requires --enable-msg=y])
   fi
fi

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

if test x$CONFIG_RTAI_TBX = xy -o x$CONFIG_RTAI_TBX = xm ; then
   if test x$CONFIG_RTAI_SEM = x ; then
      AC_MSG_ERROR([--enable-tbx requires --enable-sem])
   fi
fi

if test x$CONFIG_RTAI_TBX = xy ; then
   if test x$CONFIG_RTAI_SEM = xm ; then
      AC_MSG_ERROR([--enable-tbx=y requires --enable-sem=y])
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

if test x$CONFIG_RTAI_RTDM_SELECT = xy ; then
   if test x$CONFIG_RTAI_RTDM = x ; then
      AC_MSG_ERROR([--enable-rtdm-select=y requires --enable-rtdm=y])
   fi
fi

if test x$CONFIG_RTAI_RTDM_SHIRQ = xy ; then
   if test x$CONFIG_RTAI_RTDM = x ; then
      AC_MSG_ERROR([--enable-rtdm-shirq=y requires --enable-rtdm=y])
   fi
fi

dnl Built-in math support is not currently available with Kbuild-built
dnl kernels due to bad dependencies on user-space headers we really
dnl need to fix first. So we leave in place all the config machinery
dnl for having built-in/modular math support, but will complain here
dnl until those dependencies are properly cleared.
if test x$CONFIG_KBUILD = xy -a x$CONFIG_RTAI_MATH = xy ; then
   AC_MSG_ERROR([Built-in math support is not currently available with Linux >= 2.6.x])
fi

dnl Kbuild is not quite C++ friendly these days.
if test x$CONFIG_KBUILD = xy -a x$CONFIG_RTAI_CPLUSPLUS = xy ; then
   AC_MSG_ERROR([In-kernel C++ support is not available with Linux >= 2.6.x])
fi

dnl
dnl Build a Kconfig file corresponding to the current configuration
dnl if none exists.
dnl

if test \! -r .rtai_config; then
   set | grep -e '^CONFIG_RTAI' -e '^CONFIG_MODULES' | \
   sed -e 's,\(.*\)=\(.*\)$,\1=\"\2\",' > .rtai_config
fi

dnl
dnl Produce the info needed to build rtai_config.h
dnl

AC_DEFINE_UNQUOTED(CONFIG_RTAI_VERSION_MAJOR,$CONFIG_RTAI_VERSION_MAJOR,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_VERSION_MINOR,$CONFIG_RTAI_VERSION_MINOR,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_REVISION_LEVEL,$CONFIG_RTAI_REVISION_LEVEL,[Kconfig])

AC_DEFINE(CONFIG_RTAI,1,[Legacy])
AC_DEFINE(CONFIG_RTAI_MOUNT_ON_LOAD,1,[Legacy])
test \! x$CONFIG_KBUILD = x && AC_DEFINE(CONFIG_KBUILD,1,[Kconfig])
test \! x$CONFIG_RTAI_ADEOS = x && AC_DEFINE(CONFIG_RTAI_ADEOS,1,[Kconfig])
test \! x$CONFIG_RTAI_HAL_PREEMPT = x && AC_DEFINE(CONFIG_RTAI_HAL_PREEMPT,1,[Kconfig])
test \! x$CONFIG_RTAI_CPUS = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_CPUS,$CONFIG_RTAI_CPUS,[Kconfig])
test x$CONFIG_RTAI_DIAG_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_DIAG_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_MASTER_TSC_CPU = x && AC_DEFINE_UNQUOTED(CONFIG_RTAI_MASTER_TSC_CPU,$CONFIG_RTAI_MASTER_TSC_CPU,[Kconfig])
test x$CONFIG_RTAI_TUNE_TSC_SYNC = xy && AC_DEFINE(CONFIG_RTAI_TUNE_TSC_SYNC,1,[Kconfig])
test \! x$CONFIG_RTAI_FPU_SUPPORT = x && AC_DEFINE(CONFIG_RTAI_FPU_SUPPORT,1,[Kconfig])
test \! x$CONFIG_RTAI_MATH_C99 = x && AC_DEFINE(CONFIG_RTAI_MATH_C99,1,[Kconfig])
test \! x$CONFIG_RTAI_TRACE = x && AC_DEFINE(CONFIG_RTAI_TRACE,1,[Kconfig])
test \! x$CONFIG_RTAI_MATH = x && AC_DEFINE(CONFIG_RTAI_MATH,1,[Kconfig])
test \! x$CONFIG_RTAI_BITS = x && AC_DEFINE(CONFIG_RTAI_BITS,1,[Kconfig])
test \! x$CONFIG_RTAI_FIFOS = x && AC_DEFINE(CONFIG_RTAI_FIFOS,1,[Kconfig])
test \! x$CONFIG_RTAI_NETRPC = x && AC_DEFINE(CONFIG_RTAI_NETRPC,1,[Kconfig])
test x$CONFIG_RTAI_NETRPC_RTNET = xy && AC_DEFINE(CONFIG_RTAI_NETRPC_RTNET,1,[Kconfig])
test \! x$CONFIG_RTAI_SEM = x && AC_DEFINE(CONFIG_RTAI_SEM,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL = x && AC_DEFINE(CONFIG_RTAI_RT_POLL,1,[Kconfig])
test \! x$CONFIG_RTAI_RT_POLL_ON_STACK = x && AC_DEFINE(CONFIG_RTAI_RT_POLL_ON_STACK,1,[Kconfig])
test \! x$CONFIG_RTAI_MSG = x && AC_DEFINE(CONFIG_RTAI_MSG,1,[Kconfig])
test \! x$CONFIG_RTAI_MBX = x && AC_DEFINE(CONFIG_RTAI_MBX,1,[Kconfig])
test \! x$CONFIG_RTAI_TBX = x && AC_DEFINE(CONFIG_RTAI_TBX,1,[Kconfig])
test \! x$CONFIG_RTAI_MQ = x && AC_DEFINE(CONFIG_RTAI_MQ,1,[Kconfig])
test \! x$CONFIG_RTAI_SHM = x && AC_DEFINE(CONFIG_RTAI_SHM,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_USE_TLSF = x && AC_DEFINE(CONFIG_RTAI_USE_TLSF,1,[Kconfig])
test \! x$CONFIG_RTAI_MALLOC_VMALLOC = x && AC_DEFINE(CONFIG_RTAI_MALLOC_VMALLOC,1,[Kconfig])
test \! x$CONFIG_RTAI_TASKLETS = x && AC_DEFINE(CONFIG_RTAI_TASKLETS,1,[Kconfig])
test \! x$CONFIG_RTAI_USI = x && AC_DEFINE(CONFIG_RTAI_USI,1,[Kconfig])
test \! x$CONFIG_RTAI_WD = x && AC_DEFINE(CONFIG_RTAI_WD,1,[Kconfig])
dnl test \! x$CONFIG_RTAI_LEDS = x && AC_DEFINE(CONFIG_RTAI_LEDS,1,[Kconfig])
test \! x$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL = x && AC_DEFINE(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,1,[Kconfig])
test \! x$CONFIG_RTAI_DRIVERS_SERIAL = x && AC_DEFINE(CONFIG_RTAI_DRIVERS_SERIAL,1,[Kconfig])
test \! x$CONFIG_RTAI_DRIVERS_16550A = x && AC_DEFINE(CONFIG_RTAI_DRIVERS_16550A,1,[Kconfig])
case $CONFIG_RTAI_16550A_HAM in
	1) AC_DEFINE(CONFIG_RTAI_16550A_PIO,1,[Kconfig]) ;;
	2) AC_DEFINE(CONFIG_RTAI_16550A_MMIO,1,[Kconfig]) ;;
	3) AC_DEFINE(CONFIG_RTAI_16550A_ANY,1,[Kconfig]) ;;
	*) if test \! x$CONFIG_RTAI_DRIVERS_16550A = x ; then
		AC_MSG_ERROR([Missing 16550A hardware access mode])
	fi ;;
esac
test \! x$CONFIG_RTAI_RTDM = x && AC_DEFINE(CONFIG_RTAI_RTDM,1,[Kconfig])
test \! x$CONFIG_RTAI_ALIGN_LINUX_PRIORITY = x && AC_DEFINE(CONFIG_RTAI_ALIGN_LINUX_PRIORITY,1,[Kconfig])
test \! x$CONFIG_RTAI_TASK_SWITCH_SIGNAL = x && AC_DEFINE(CONFIG_RTAI_TASK_SWITCH_SIGNAL,1,[Kconfig])
dnl test \! x$CONFIG_RTAI_HARD_SOFT_TOGGLER = x && AC_DEFINE(CONFIG_RTAI_HARD_SOFT_TOGGLER,1,[Kconfig])

test x$CONFIG_RTAI_MATH = xy && AC_DEFINE(CONFIG_RTAI_MATH_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_BITS = xy && AC_DEFINE(CONFIG_RTAI_BITS_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_FIFOS = xy && AC_DEFINE(CONFIG_RTAI_FIFOS_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_NETRPC = xy && AC_DEFINE(CONFIG_RTAI_NETRPC_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SEM = xy && AC_DEFINE(CONFIG_RTAI_SEM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MSG = xy && AC_DEFINE(CONFIG_RTAI_MSG_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MBX = xy && AC_DEFINE(CONFIG_RTAI_MBX_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_TBX = xy && AC_DEFINE(CONFIG_RTAI_TBX_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MQ = xy && AC_DEFINE(CONFIG_RTAI_MQ_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_SHM = xy && AC_DEFINE(CONFIG_RTAI_SHM_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_MALLOC = xy && AC_DEFINE(CONFIG_RTAI_MALLOC_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_TASKLETS = xy && AC_DEFINE(CONFIG_RTAI_TASKLETS_BUILTIN,1,[Kconfig])
test x$CONFIG_RTAI_USI = xy && AC_DEFINE(CONFIG_RTAI_USI_BUILTIN,1,[Kconfig])
dnl test x$CONFIG_RTAI_LEDS = xy && AC_DEFINE(CONFIG_RTAI_LEDS_BUILTIN,1,[Kconfig])

test x$CONFIG_RTAI_SCHED_ISR_LOCK = xy && AC_DEFINE(CONFIG_RTAI_SCHED_ISR_LOCK,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_RTC_FREQ,$CONFIG_RTAI_RTC_FREQ,[Kconfig])
test x$CONFIG_RTAI_LONG_TIMED_LIST = xy && AC_DEFINE(CONFIG_RTAI_LONG_TIMED_LIST,1,[Kconfig])
test x$CONFIG_RTAI_USE_STACK_ARGS = xy && AC_DEFINE(CONFIG_RTAI_USE_STACK_ARGS,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_SCHED_8254_LATENCY,$CONFIG_RTAI_SCHED_8254_LATENCY,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_METRICS,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_FREQ,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES,$CONFIG_RTAI_LATENCY_SELF_CALIBRATION_CYCLES,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_SCHED_LXRT_NUMSLOTS,$CONFIG_RTAI_SCHED_LXRT_NUMSLOTS,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_RTDM_FD_MAX,$CONFIG_RTAI_RTDM_FD_MAX,[Kconfig])
test x$CONFIG_RTAI_RTDM_SHIRQ = xy && AC_DEFINE(CONFIG_RTAI_RTDM_SHIRQ,1,[Kconfig])
test x$CONFIG_RTAI_RTDM_SELECT = xy && AC_DEFINE(CONFIG_RTAI_RTDM_SELECT,1,[Kconfig])
test x$CONFIG_RTAI_DEBUG_RTDM = xy && AC_DEFINE(CONFIG_RTAI_DEBUG_RTDM,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_CAL_FREQS_FACT,$CONFIG_RTAI_CAL_FREQS_FACT,[Kconfig])
test x$CONFIG_RTAI_USE_LINUX_COMEDI = xy && AC_DEFINE(CONFIG_RTAI_USE_LINUX_COMEDI,1,[Kconfig])
test x$CONFIG_RTAI_USE_COMEDI_LOCK = xy && AC_DEFINE(CONFIG_RTAI_USE_COMEDI_LOCK,1,[Kconfig])

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

AC_DEFINE_UNQUOTED(CONFIG_RTAI_MATH_LIBM_TO_USE,$CONFIG_RTAI_MATH_LIBM_TO_USE,[Kconfig])
test x$CONFIG_RTAI_MATH_KCOMPLEX = xy && AC_DEFINE(CONFIG_RTAI_MATH_KCOMPLEX,1,[Kconfig])

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

test x$CONFIG_RTAI_MAINTAINER = xy && AC_DEFINE(CONFIG_RTAI_MAINTAINER,1,[Kconfig])
test x$CONFIG_RTAI_MAINTAINER_PMA = xy && AC_DEFINE(CONFIG_RTAI_MAINTAINER_PMA,1,[Kconfig])
test x$CONFIG_RTAI_MAINTAINER_PGM = xy && AC_DEFINE(CONFIG_RTAI_MAINTAINER_PGM,1,[Kconfig])

if test x$arch_supports_lxrt = xy;  then
    AC_DEFINE(CONFIG_RTAI_INTERNAL_LXRT_SUPPORT,1,[Kconfig])
else
    AC_DEFINE(CONFIG_RTAI_INTERNAL_LXRT_SUPPORT,0,[Kconfig])
fi

dnl test x$arch_supports_leds = xy && AC_DEFINE(CONFIG_RTAI_INTERNAL_LEDS_SUPPORT,1,[Kconfig])
AC_DEFINE_UNQUOTED(CONFIG_RTAI_INTERNAL_LXRT_INLINE,$CONFIG_RTAI_LXRT_INLINE,[Kconfig])

AH_VERBATIM(CONFIG_RTAI_LXRT_INLINE,[
#ifndef CONFIG_RTAI_LXRT_INLINE
#define CONFIG_RTAI_LXRT_INLINE CONFIG_RTAI_INTERNAL_LXRT_INLINE
#endif /* !CONFIG_RTAI_LXRT_INLINE */
#define __hidefun(name) name ## _hidden_inline
#if CONFIG_RTAI_LXRT_INLINE == 1
#define RTAI_PROTO(type,name,arglist) static inline type name arglist
#elif CONFIG_RTAI_LXRT_INLINE == 2
#define RTAI_PROTO(type,name,arglist) extern inline type name arglist
#elif CONFIG_RTAI_LXRT_INLINE == 3
#define RTAI_PROTO(type,name,arglist) type name arglist; static inline type __hidefun(name) arglist
#else
#define RTAI_PROTO(type,name,arglist) type name arglist
#endif /* CONFIG_RTAI_LXRT_INLINE */
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

dnl No longer used after 2.6.17
dnl AH_BOTTOM([#include <linux/config.h>])

RTAI_INSTALL_DIR='${exec_prefix}'

dnl
dnl Produce automake conditionals.
dnl

dnl Whether Linux is using the Kbuild make system.
AM_CONDITIONAL(CONFIG_KBUILD,[test "$CONFIG_KBUILD" = y])

dnl CPU architecture
dnl FIXME_X86
AM_CONDITIONAL(CONFIG_X86_64,[test "$CONFIG_X86_64" = y])
AM_CONDITIONAL(CONFIG_I386,[test "$CONFIG_X86_32" = y])
AM_CONDITIONAL(CONFIG_X86,[test "$CONFIG_X86" = y])
AM_CONDITIONAL(CONFIG_PPC,[test "$CONFIG_PPC" = y])
AM_CONDITIONAL(CONFIG_ARM,[test "$CONFIG_ARM" = y])
AM_CONDITIONAL(CONFIG_ARCH_EP9301,[test "$CONFIG_ARCH_EP9301" = y])
AM_CONDITIONAL(CONFIG_ARCH_PXA,[test "$CONFIG_ARCH_PXA" = y])
AM_CONDITIONAL(CONFIG_ARCH_AT91,[test "$CONFIG_ARCH_AT91" = y])
AM_CONDITIONAL(CONFIG_ARCH_IMX,[test "$CONFIG_ARCH_IMX" = y])
AM_CONDITIONAL(CONFIG_UCLINUX,[test "$CONFIG_UCLINUX" = y])
AM_CONDITIONAL(CONFIG_SMP,[test "$CONFIG_SMP" = y])
AM_CONDITIONAL(CONFIG_M68KNOMMU,[test "$CONFIG_M68K" = y -a "x$CONFIG_MMU" != xy])
AM_CONDITIONAL(CONFIG_M68K,[test "$CONFIG_M68K" = y -a "$CONFIG_MMU" = y])
AM_CONDITIONAL(CONFIG_M5407,[test "$CONFIG_M5407" = y])
AM_CONDITIONAL(CONFIG_M5329,[test "$CONFIG_M5329" = y])
AM_CONDITIONAL(CONFIG_M54455,[test "$CONFIG_M54455" = y])

AM_CONDITIONAL(CONFIG_RTAI_TRACE,[test -n "$CONFIG_RTAI_TRACE"])
AM_CONDITIONAL(CONFIG_RTAI_MATH,[test -n "$CONFIG_RTAI_MATH"])
AM_CONDITIONAL(CONFIG_RTAI_BITS,[test -n "$CONFIG_RTAI_BITS"])
AM_CONDITIONAL(CONFIG_RTAI_FIFOS,[test -n "$CONFIG_RTAI_FIFOS"])
AM_CONDITIONAL(CONFIG_RTAI_NETRPC,[test -n "$CONFIG_RTAI_NETRPC"])
AM_CONDITIONAL(CONFIG_RTAI_SEM,[test -n "$CONFIG_RTAI_SEM"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL,[test -n "$CONFIG_RTAI_RT_POLL"])
AM_CONDITIONAL(CONFIG_RTAI_RT_POLL_ON_STACK,[test -n "$CONFIG_RTAI_RT_POLL_ON_STACK"])
AM_CONDITIONAL(CONFIG_RTAI_MSG,[test -n "$CONFIG_RTAI_MSG"])
AM_CONDITIONAL(CONFIG_RTAI_MBX,[test -n "$CONFIG_RTAI_MBX"])
AM_CONDITIONAL(CONFIG_RTAI_TBX,[test -n "$CONFIG_RTAI_TBX"])
AM_CONDITIONAL(CONFIG_RTAI_MQ,[test -n "$CONFIG_RTAI_MQ"])
AM_CONDITIONAL(CONFIG_RTAI_SHM,[test -n "$CONFIG_RTAI_SHM"])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC,[test -n "$CONFIG_RTAI_MALLOC"])
AM_CONDITIONAL(CONFIG_RTAI_TASKLETS,[test -n "$CONFIG_RTAI_TASKLETS"])
AM_CONDITIONAL(CONFIG_RTAI_USI,[test -n "$CONFIG_RTAI_USI"])
AM_CONDITIONAL(CONFIG_RTAI_WD,[test -n "$CONFIG_RTAI_WD"])
dnl AM_CONDITIONAL(CONFIG_RTAI_LEDS,[test -n "$CONFIG_RTAI_LEDS"])
AM_CONDITIONAL(CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL,[test -n "$CONFIG_RTAI_LXRT_USE_LINUX_SYSCALL"])

AM_CONDITIONAL(CONFIG_RTAI_MATH_BUILTIN,[test x$CONFIG_RTAI_MATH = xy])
AM_CONDITIONAL(CONFIG_RTAI_BITS_BUILTIN,[test x$CONFIG_RTAI_BITS = xy])
AM_CONDITIONAL(CONFIG_RTAI_FIFOS_BUILTIN,[test x$CONFIG_RTAI_FIFOS = xy])
AM_CONDITIONAL(CONFIG_RTAI_NETRPC_BUILTIN,[test x$CONFIG_RTAI_NETRPC = xy])
AM_CONDITIONAL(CONFIG_RTAI_SEM_BUILTIN,[test x$CONFIG_RTAI_SEM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MSG_BUILTIN,[test x$CONFIG_RTAI_MSG = xy])
AM_CONDITIONAL(CONFIG_RTAI_MBX_BUILTIN,[test x$CONFIG_RTAI_MBX = xy])
AM_CONDITIONAL(CONFIG_RTAI_TBX_BUILTIN,[test x$CONFIG_RTAI_TBX = xy])
AM_CONDITIONAL(CONFIG_RTAI_MQ_BUILTIN,[test x$CONFIG_RTAI_MQ = xy])
AM_CONDITIONAL(CONFIG_RTAI_SHM_BUILTIN,[test x$CONFIG_RTAI_SHM = xy])
AM_CONDITIONAL(CONFIG_RTAI_MALLOC_BUILTIN,[test x$CONFIG_RTAI_MALLOC = xy])
AM_CONDITIONAL(CONFIG_RTAI_TASKLETS_BUILTIN,[test x$CONFIG_RTAI_TASKLETS = xy])
AM_CONDITIONAL(CONFIG_RTAI_USI_BUILTIN,[test x$CONFIG_RTAI_USI = xy])
dnl AM_CONDITIONAL(CONFIG_RTAI_LEDS_BUILTIN,[test x$CONFIG_RTAI_LEDS = xy])

dnl Non-core packages.
AM_CONDITIONAL(CONFIG_RTAI_COMEDI_LXRT,[test x$CONFIG_RTAI_COMEDI_LXRT = xy])
AM_CONDITIONAL(CONFIG_RTAI_USE_LINUX_COMEDI,[test x$CONFIG_RTAI_USE_LINUX_COMEDI = xy])
dnl AM_CONDITIONAL(CONFIG_RTAI_USE_COMEDI_LOCK,[test x$CONFIG_RTAI_USE_COMEDI_LOCK = xy])
AM_CONDITIONAL(CONFIG_RTAI_CPLUSPLUS,[test x$CONFIG_RTAI_CPLUSPLUS = xy])
dnl Supported skins
AM_CONDITIONAL(CONFIG_RTAI_RTDM,[test x$CONFIG_RTAI_RTDM = xy])
dnl Supported drivers
AM_CONDITIONAL(CONFIG_RTAI_DRIVERS_SERIAL,[test x$CONFIG_RTAI_DRIVERS_SERIAL = xy])
AM_CONDITIONAL(CONFIG_RTAI_DRIVERS_16550A,[test x$CONFIG_RTAI_DRIVERS_16550A = xy])

AM_CONDITIONAL(CONFIG_RTAI_LAB,[test x$CONFIG_RTAI_LAB = xy])
AM_CONDITIONAL(CONFIG_RTAI_MVM,[test x$CONFIG_RTAI_MVM = xy])

AM_CONDITIONAL(CONFIG_RTAI_TESTSUITE,[test x$CONFIG_RTAI_TESTSUITE = xy])

dnl Misc compilation conditionals
AM_CONDITIONAL(CONFIG_RTAI_ADEOS,[test "$CONFIG_RTAI_ADEOS" = y])
AM_CONDITIONAL(CONFIG_RTAI_MATH_C99,[test "$CONFIG_RTAI_MATH_C99" = y])

dnl Documentation
AM_CONDITIONAL(CONFIG_RTAI_DOX_DOC,[test "$CONFIG_RTAI_DOX_DOC" = y])
# Automake variables related to DocBook documentation are handled in docbook.m4.

dnl Maintenance symbols and profiles
AM_CONDITIONAL(CONFIG_RTAI_MAINTAINER,[test "$CONFIG_RTAI_MAINTAINER" = y])
AM_CONDITIONAL(CONFIG_RTAI_MAINTAINER_PMA,[test "$CONFIG_RTAI_MAINTAINER_PMA" = y])
AM_CONDITIONAL(CONFIG_RTAI_MAINTAINER_PGM,[test "$CONFIG_RTAI_MAINTAINER_PGM" = y])
AM_CONDITIONAL(CONFIG_RTAI_OLD_FASHIONED_BUILD,[test "$CONFIG_RTAI_OLD_FASHIONED_BUILD" = y])

dnl
dnl Define arch-specific CFLAGS and links
dnl

if test x$CONFIG_KBUILD = x; then
   dnl The following sets RTAI_KMOD_CFLAGS to the value used by the
   dnl specified Linux tree for building kernel modules. This hack is
   dnl not needed for 2.6 and above since out-of-tree builds of modules
   dnl are fully controlled by the kbuild system, so that we don't even
   dnl need to know of the compilation flags which will be used.
   AC_MSG_CHECKING([for module compilation flags])
   kmod_cflags=`cd $srcdir/base/config/modules && make -s RTAI_LINUX_DIR=$RTAI_LINUX_DIR ARCH=$RTAI_TARGET_ARCH_NAME CC=$CC | grep '^RTAI_KMOD_CFLAGS='`
   eval $kmod_cflags
   if test "$RTAI_KMOD_CFLAGS" = ""; then
      AC_MSG_ERROR([Unable to retrieve compilation flags for kernel modules out of $RTAI_LINUX_DIR/Makefile])
   fi
   AC_MSG_RESULT([ok])
   RTAI_MODULE_EXT=.o
   RTAI_KBUILD_ENV=
   RTAI_KBUILD_CMD=
   RTAI_KBUILD_TOP=
   RTAI_KBUILD_BOTTOM=
   RTAI_KBUILD_CLEAN=
   RTAI_KBUILD_DISTCLEAN=
else
   RTAI_MODULE_EXT=.ko
   RTAI_KBUILD_ENV='src2obj = $(patsubst %.$(1),%.o,$(filter %.$(1),$(call notdir,$(2))))'
   RTAI_KBUILD_TOP="@if test \\! -r Module.symvers ; then \$(LN_S) -f \"\`cd \$(top_builddir) && pwd\`/Module.symvers\" . ; fi;"
   RTAI_KBUILD_BOTTOM=
   RTAI_KBUILD_CMD="for src in \$^; do if test \\! -r \`basename \$\$src\`; then \$(LN_S) \$\$src; fi; done; \
		    rm -f $RTAI_LINUX_DIR/.tmp_versions/rtai_*.mod $RTAI_LINUX_DIR/.tmp_versions/*_rt.mod; \
                    \$(MAKE) -C $RTAI_LINUX_DIR CROSS_COMPILE=\$(CROSS_COMPILE) CC=$CC ARCH=$RTAI_TARGET_ARCH_NAME \
		    SUBDIRS=\$\$PWD M=\$\$PWD rtai_srctree=\`cd \$(top_srcdir) && pwd\` rtai_srcdir=\`cd \$(srcdir) && pwd\` V=\$(V) modules \
		    rtai_objs='\$(call src2obj,c,\$^) \$(call src2obj,S,\$^) \$(call src2obj,cc,\$^) \$(call src2obj,cpp,\$^)' rtai_target=\$(basename \$@)"
   RTAI_KBUILD_CLEAN='rm -f *.ko *.mod.c .*.cmd *.o Module.symvers Module.markers modules.order && rm -Rf .tmp_versions'
   RTAI_KBUILD_DISTCLEAN=""
fi

AC_MSG_CHECKING([for kernel module extension])
AC_MSG_RESULT([$RTAI_MODULE_EXT])

dnl gcc 3.3 and above needs -D_FORTIFY_SOURCE=0 in math
dnl (suggested by Paul <paul_c AT tuxcnc DOT org>);
dnl not sure about the implications, in general I'd prefer
dnl that code causing warnings gets fixed...
RTAI_FP_CFLAGS=
AC_MSG_CHECKING(for gcc version >= 3.3)
AC_CACHE_VAL(ac_cv_gcc33,
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
[AC_TRY_COMPILE(
[#if !( __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#error GCC does not need _FORTIFY_SOURCE
#endif],
[return 0;],
[ac_cv_gcc33=yes],
[ac_cv_gcc33=no])]
AC_LANG_RESTORE)
if [[ $ac_cv_gcc33 = no ]]; then
  AC_MSG_RESULT([no])
else
  if test x"$CONFIG_RTAI_FORTIFY_SOURCE" != x ; then
  case "$CONFIG_RTAI_FORTIFY_SOURCE" in
    0|1|2)
      dnl undef first, in case it is already defined
      RTAI_FP_CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=$CONFIG_RTAI_FORTIFY_SOURCE"
      ;;
    *)
      AC_MSG_ERROR([invalid _FORTIFY_SOURCE value $RTAI_FORTIFY_SOURCE])
      ;;
  esac
  dnl else use value set for the kernel
  fi
  AC_MSG_RESULT([ok])
fi

dnl FIXME: used in rtai-config.in but never defined
RTAI_COMPAT_CPPFLAGS=

AC_MSG_CHECKING([for RTAI target architecture])

RTAI_REAL_USER_CFLAGS=""

if test x"$LINUX_KERNELOUTPUT" != x"$LINUX_KERNELSRC"; then
  RTAI_REAL_USER_CFLAGS="-I$LINUX_KERNELOUTPUT/include $RTAI_REAL_USER_CFLAGS"
fi

dnl extra arch-dependent opts to be passed to the compiler
RTAI_TARGET_ARCH_OPTS=

case $RTAI_TARGET_ARCH in
 x86)
 case $RTAI_TARGET_SUBARCH in
   32)
	RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CFLAGS -fno-rtti -fno-exceptions -fno-strength-reduce -pipe"
	RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -Wall -Wstrict-prototypes -pipe"
	if test x$CONFIG_RTAI_FPU_SUPPORT = xy; then
	   RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -ffast-math -mhard-float"
	else
	   RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -ffast-math -msoft-float"
	fi
        ;;
   64)
        RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CFLAGS -fno-rtti -fno-exceptions -fno-strength-reduce -pipe"
        RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -Wall -Wstrict-prototypes -pipe"
        if test x$CONFIG_RTAI_FPU_SUPPORT = xy; then
           RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -ffast-math -mhard-float"
        else
           RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -ffast-math -msoft-float"
        fi
	# 2009-03-23
	# `-msse2' temporarily removed by Pierangelo Masarati
	# as suggested by Paul <paul_c AT tuxcnc DOT org>
	# RTAI_TARGET_ARCH_OPTS="$RTAI_TARGET_ARCH_OPTS -msse"
	RTAI_TARGET_ARCH_OPTS="$RTAI_TARGET_ARCH_OPTS -msse -mpreferred-stack-boundary=4"
        ;;
   esac
   ;;
 arm)
	if test "$CONFIG_ARCH_EP9301" = y; then
		RTAI_TARGET_SUBARCH=ep9301
	elif test "$CONFIG_ARCH_PXA" = y; then
		RTAI_TARGET_SUBARCH=pxa
	elif test "$CONFIG_ARCH_AT91" = y; then
		RTAI_TARGET_SUBARCH=at91
	elif test "$CONFIG_ARCH_IMX" = y; then
		RTAI_TARGET_SUBARCH=imx
	else
	   echo
	   echo "******************************************"
	   echo "Unsupported ARM sub-architecture -- Sorry."
	   echo "******************************************"
	   exit 1
	fi
	RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CFLAGS -fno-rtti -fno-exceptions -fno-strength-reduce -pipe"
	RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -Wall -Wstrict-prototypes -pipe"
	RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -ffast-math -msoft-float"
	AC_CONFIG_LINKS(base/include/asm-arm/arch:$srcdir/base/include/asm-arm/arch-$RTAI_TARGET_SUBARCH)
        ;;
 ppc)
	RTAI_TARGET_SUBARCH=
	dnl apparently, powerpc needs specific include/asm path to find errno.h
	RTAI_KMOD_CFLAGS="$RTAI_KMOD_CFLAGS -I${RTAI_LINUX_DIR}/arch/$RTAI_TARGET_ARCH_NAME -I${RTAI_LINUX_DIR}/arch/$RTAI_TARGET_ARCH_NAME/include/ -I${RTAI_LINUX_DIR}/arch/$RTAI_TARGET_ARCH_NAME/include/asm"
	RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CFLAGS -fno-rtti -fno-exceptions -fno-strength-reduce -pipe"
	RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -I${RTAI_LINUX_DIR}/arch/$RTAI_TARGET_ARCH_NAME/include/ -Wall -Wstrict-prototypes -pipe"
	if test x$CONFIG_RTAI_FPU_SUPPORT = xy; then
	   RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -mhard-float"
	else
	   RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -msoft-float"
	fi
	;;
 m68k)
	if test "$CONFIG_M5407" = y; then
		RTAI_TARGET_SUBARCH=MCF5407
		CFLAGS="-m5407 -Wa,-m5407"
	elif test "$CONFIG_M532x" = y; then
		RTAI_TARGET_SUBARCH=MCF5329
		if test "$using_crosscompile" = y; then
			CFLAGS="-m5307 -Wa,-m5307"
		fi
	elif test "$CONFIG_M54455" = y; then
		RTAI_TARGET_SUBARCH=MCF54455
	else
		echo
		echo "************************************************"
		echo "Unsupported M68K sub-architecture -- Sorry."
		echo "************************************************"
		exit 1
	fi
	#RTAI_KMOD_CFLAGS="$RTAI_KMOD_CFLAGS -I${RTAI_LINUX_DIR}/arch/m68k"
	RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CFLAGS -fno-rtti -fno-exceptions -fno-strength-reduce -pipe"
	RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -Wall -Wstrict-prototypes -pipe -DCONFIG_COLDFIRE"
	RTAI_FP_CFLAGS="$RTAI_FP_CFLAGS -msoft-float"
	;;


esac

if test x$CONFIG_RTAI_KMOD_DEBUG = xy; then
   RTAI_KMOD_CFLAGS="-g $RTAI_KMOD_CFLAGS"
   RTAI_KMOD_CXXFLAGS="-g $RTAI_KMOD_CXXFLAGS"
fi

if [[ $ac_cv_accepts_cxa_atexit = yes ]]; then
   RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CXXFLAGS -fno-use-cxa-atexit"
fi

dnl Ok, the following isn't pretty, but required to process the
dnl various inlining modes correctly. This boils down to:
dnl - if mode == "none", then we will always need -llxrt to be
dnl   tackled on the library deps for user-space programs.
dnl - if mode == "eager/static", then we will never need -llxrt
dnl - if mode == "conditional/extern", then we will not need -llxrt
dnl   unless the optimizer has been switched off, i.e. when the
dnl   debugging mode has been switched on.

if test $CONFIG_RTAI_LXRT_INLINE = 3; then
   RTAI_LXRT_LDADD=-llxrt
else
   RTAI_LXRT_LDADD=
   if test x$CONFIG_RTAI_USER_DEBUG = xy; then
      RTAI_REAL_USER_CFLAGS="-g $RTAI_REAL_USER_CFLAGS"
      if test $CONFIG_RTAI_LXRT_INLINE = 2; then
         RTAI_LXRT_LDADD="-llxrt"
      fi
   else
      RTAI_REAL_USER_CFLAGS="-O2 $RTAI_REAL_USER_CFLAGS"
   fi
fi

RTAI_KMOD_APP_CFLAGS=$RTAI_KMOD_CFLAGS
RTAI_KMOD_APP_CXXFLAGS=$RTAI_KMOD_CXXFLAGS
RTAI_USER_APP_CFLAGS=$RTAI_REAL_USER_CFLAGS
RTAI_KMOD_CFLAGS="$RTAI_KMOD_CFLAGS -D__IN_RTAI__"
RTAI_KMOD_CXXFLAGS="$RTAI_KMOD_CXXFLAGS -D__IN_RTAI__"
RTAI_REAL_USER_CFLAGS="$RTAI_REAL_USER_CFLAGS -D__IN_RTAI__"

AC_CONFIG_LINKS(base/include/asm:$srcdir/base/include/asm-$RTAI_TARGET_ARCH)

if test x$CONFIG_RTAI_OLD_FASHIONED_BUILD = xy; then
   AC_CONFIG_LINKS(base/include/rtai_config.h:rtai_config.h)
fi

if test x$RTAI_TARGET_SUBARCH = x; then
   AC_MSG_RESULT([$RTAI_TARGET_ARCH])
else
   AC_MSG_RESULT([$RTAI_TARGET_ARCH ($RTAI_TARGET_SUBARCH)])
fi

if test x$CONFIG_KBUILD = xy; then
   AC_CONFIG_LINKS(base/arch/$RTAI_TARGET_ARCH/hal/Makefile:base/arch/$RTAI_TARGET_ARCH/hal/Makefile.kbuild)
   if test -d $srcdir/base/arch/$RTAI_TARGET_ARCH/calibration; then
      AC_CONFIG_LINKS(base/arch/$RTAI_TARGET_ARCH/calibration/Makefile:base/arch/$RTAI_TARGET_ARCH/calibration/Makefile.kbuild)
   fi
   AC_CONFIG_LINKS(base/ipc/bits/Makefile:base/ipc/bits/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/fifos/Makefile:base/ipc/fifos/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/mbx/Makefile:base/ipc/mbx/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/mq/Makefile:base/ipc/mq/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/msg/Makefile:base/ipc/msg/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/sem/Makefile:base/ipc/sem/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/tbx/Makefile:base/ipc/tbx/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/netrpc/Makefile:base/ipc/netrpc/Makefile.kbuild)
   AC_CONFIG_LINKS(base/ipc/shm/Makefile:base/ipc/shm/Makefile.kbuild)
dnl   AC_CONFIG_LINKS(base/leds/Makefile:base/leds/Makefile.kbuild)
   AC_CONFIG_LINKS(base/posix/Makefile:base/posix/Makefile.kbuild)
   AC_CONFIG_LINKS(base/malloc/Makefile:base/malloc/Makefile.kbuild)
   AC_CONFIG_LINKS(base/tasklets/Makefile:base/tasklets/Makefile.kbuild)
   AC_CONFIG_LINKS(base/wd/Makefile:base/wd/Makefile.kbuild)
   AC_CONFIG_LINKS(base/usi/Makefile:base/usi/Makefile.kbuild)
   AC_CONFIG_LINKS(base/math/Makefile:base/math/Makefile.kbuild)
   AC_CONFIG_LINKS(base/sched/Makefile:base/sched/Makefile.kbuild)
   AC_CONFIG_LINKS(addons/drivers/serial/Makefile:addons/drivers/serial/Makefile.kbuild)
   AC_CONFIG_LINKS(addons/comedi/Makefile:addons/comedi/Makefile.kbuild)
   AC_CONFIG_LINKS(addons/drivers/16550A/Makefile:addons/drivers/16550A/Makefile.kbuild)
   AC_CONFIG_LINKS(addons/rtdm/Makefile:addons/rtdm/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kern/latency/Makefile:testsuite/kern/latency/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kern/preempt/Makefile:testsuite/kern/preempt/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kern/switches/Makefile:testsuite/kern/switches/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kthreads/latency/Makefile:testsuite/kthreads/latency/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kthreads/preempt/Makefile:testsuite/kthreads/preempt/Makefile.kbuild)
   AC_CONFIG_LINKS(testsuite/kthreads/switches/Makefile:testsuite/kthreads/switches/Makefile.kbuild)
fi

dnl
dnl POSIX wraps
dnl
RTAI_POSIX_WRAP=`for f in \`cat "${srcdir}/base/posix/POSIX_WRAPS" | grep -v "#"\`; do echo -n " -Wl,--wrap -Wl,$f" ; done`

RTAI_USER_CFLAGS="-I$LINUX_KERNELSRC/include $RTAI_REAL_USER_CFLAGS"

dnl
dnl Build the Makefiles
dnl

AC_SUBST(CROSS_COMPILE)
AC_SUBST(DOXYGEN_SHOW_INCLUDE_FILES)
AC_SUBST(DOXYGEN_HAVE_DOT)
AC_SUBST(DOXYGEN)
AC_SUBST(LATEX_BATCHMODE)
AC_SUBST(LATEX_MODE)
AC_SUBST(EFLTK_DIR)
AC_SUBST(COMEDI_DIR)

AC_SUBST(RTAI_BUILD_STRING)
AC_SUBST(RTAI_HOST_STRING)
AC_SUBST(RTAI_TARGET_ARCH)
AC_SUBST(RTAI_TARGET_ARCH_OPTS)
AC_SUBST(RTAI_TARGET_SUBARCH)
AC_SUBST(RTAI_BUILTIN_MODLIST)
AC_SUBST(RTAI_KMOD_CFLAGS)
AC_SUBST(RTAI_KMOD_CXXFLAGS)
AC_SUBST(RTAI_COMPAT_CPPFLAGS)
AC_SUBST(RTAI_USER_CFLAGS)
AC_SUBST(RTAI_REAL_USER_CFLAGS)
AC_SUBST(RTAI_KMOD_APP_CFLAGS)
AC_SUBST(RTAI_KMOD_APP_CXXFLAGS)
AC_SUBST(RTAI_USER_APP_CFLAGS)
AC_SUBST(RTAI_LXRT_LDADD)
AC_SUBST(RTAI_FP_CFLAGS)
AC_SUBST(RTAI_LINUX_DIR)
AC_SUBST(RTAI_LINUX_VERSION)
AC_SUBST(RTAI_MATH_LIBM)
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
AC_SUBST(CROSSCOMPILE_TEST)

AC_SUBST(RTAI_INSTALL_DIR)

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
	base/arch/x86/calibration/smicnt \
        base/include/asm-x86/GNUmakefile \
        base/arch/arm/GNUmakefile \
	base/arch/arm/hal/GNUmakefile \
	base/include/asm-arm/GNUmakefile \
   	base/ipc/bits/GNUmakefile \
	base/ipc/fifos/GNUmakefile \
	base/ipc/netrpc/GNUmakefile \
	base/ipc/shm/GNUmakefile \
	base/ipc/sem/GNUmakefile \
	base/ipc/msg/GNUmakefile \
	base/ipc/mbx/GNUmakefile \
	base/ipc/tbx/GNUmakefile \
	base/ipc/mq/GNUmakefile \
	base/math/GNUmakefile \
	base/malloc/GNUmakefile \
	base/tasklets/GNUmakefile \
	base/trace/GNUmakefile \
	base/wd/GNUmakefile \
	base/usi/GNUmakefile \
	base/posix/GNUmakefile \
	])

dnl base/leds/GNUmakefile \
dnl base/arch/ppc/GNUmakefile \
dnl base/arch/ppc/hal/GNUmakefile \
dnl base/include/asm-ppc/GNUmakefile \
dnl base/arch/m68k/GNUmakefile \
dnl base/arch/m68k/hal/GNUmakefile \
dnl base/include/asm-m68k/GNUmakefile \

build_drivers=$CONFIG_RTAI_DRIVERS_SERIAL$CONFIG_RTAI_DRIVERS_16550A
build_addons=$CONFIG_RTAI_CPLUSPLUS$CONFIG_RTAI_COMEDI_LXRT$build_drivers$CONFIG_RTAI_RTDM

if test -d $srcdir/addons; then
  AC_CONFIG_FILES([ \
       addons/GNUmakefile \
       addons/cpp/GNUmakefile \
       addons/comedi/GNUmakefile \
       addons/drivers/GNUmakefile \
       addons/drivers/serial/GNUmakefile \
       addons/drivers/16550A/GNUmakefile \
       addons/rtdm/GNUmakefile \
       addons/rtdm/lib/GNUmakefile \
       ])
elif test \! x$build_addons = x; then
     AC_MSG_ERROR([addons package is missing])
fi

AM_CONDITIONAL(CONFIG_RTAI_ADDONS,[test \! x$build_addons = x])

if test -d $srcdir/rtai-lab; then
   AC_CONFIG_FILES([ \
        rtai-lab/GNUmakefile \
        rtai-lab/matlab/GNUmakefile \
        rtai-lab/scilab/GNUmakefile \
        rtai-lab/scicoslab/GNUmakefile \
        rtai-lab/scilab/utility/GNUmakefile \
   ])
elif test \! x$CONFIG_RTAI_LAB = x; then
     AC_MSG_ERROR([rtai-lab package is missing])
fi

if test -d $srcdir/testsuite; then
   AC_CONFIG_FILES([ \
        testsuite/GNUmakefile \
	testsuite/kern/GNUmakefile \
	testsuite/kern/latency/GNUmakefile \
	testsuite/kern/preempt/GNUmakefile \
	testsuite/kern/switches/GNUmakefile \
	testsuite/kthreads/GNUmakefile \
	testsuite/kthreads/latency/GNUmakefile \
	testsuite/kthreads/preempt/GNUmakefile \
	testsuite/kthreads/switches/GNUmakefile \
	testsuite/user/GNUmakefile \
	testsuite/user/latency/GNUmakefile \
	testsuite/user/preempt/GNUmakefile \
	testsuite/user/switches/GNUmakefile \
        ])
elif test \! x$CONFIG_RTAI_TESTSUITE = x; then
     AC_MSG_ERROR([testsuite package is missing])
fi

dnl FIXME: configurable?
CONFIG_RTAI_PYTHON=y
if test -d $srcdir/rtai-py; then
   AC_CONFIG_FILES([ \
        rtai-py/GNUmakefile \
        ])
elif test \! x$CONFIG_RTAI_PYTHON = x; then
     AC_MSG_ERROR([python package is missing])
fi

if test \! x$RTAI_MAYBE_DOCDIR = x; then
#	   doc/docbook/GNUmakefile \
#	   doc/docbook/catalog \
#	   doc/docbook/custom-stylesheets/GNUmakefile \
#	   doc/docbook/custom-stylesheets/xsl/GNUmakefile \
#	   doc/docbook/custom-stylesheets/xsl/common/GNUmakefile \
#	   doc/docbook/custom-stylesheets/xsl/fo/GNUmakefile \
#	   doc/docbook/custom-stylesheets/xsl/html/GNUmakefile \
#	   doc/docbook/custom-stylesheets/xsl/html/chunk.xsl \
#	   doc/docbook/docbook-test/GNUmakefile \
      AC_CONFIG_FILES([ \
	   doc/GNUmakefile \
	   doc/doxygen/GNUmakefile \
	   doc/doxygen/Doxyfile \
      ])
fi

if test x$CONFIG_RTAI_MVM = xy; then
   AC_CONFIG_SUBDIRS(rtai-sim)
fi

AC_OUTPUT()
