# Copyright (C) 2005-2017 The RTAI project
# This [file] is free software; the RTAI project
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

KCONFIG_DIR=./base/config/kconfig
CC ?= gcc
#CC ?= gcc -Wall -pedantic

ifneq ($(MAKECMDGOALS),help)
ifeq ($(srctree),)
ifneq ($(MAKEFILE_LIST),)
# Since 3.80, we can find out which Makefile is currently processed,
# and infere the location of the source tree using MAKEFILE_LIST.
srctree := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
else
ifeq ($(srctree),)
srctree := $(shell test -d base/config && pwd)
ifeq ($(srctree),)
$(error Please specify the location of your source tree: make srctree=...)
endif
endif
endif
endif
build_alias := $(shell $(srctree)/base/config/autoconf/config.guess)
host_alias := $(shell  $(srctree)/base/config/autoconf/arch2host.sh $(ARCH) $(build_alias))
override ARCH := $(shell echo $(host_alias)|cut -f1 -d-|sed -e s/^i.86/x86/ -e s/^x86_64/x86/ -e s/^arm.*/arm/ -e s/^sa110/arm/ -e s/^powerpc/ppc/)
endif

ifeq ($(MAKECMDGOALS),)
confgui = $(shell test $(TERM) = emacs && echo xconfig || echo menuconfig)
forceconf = $(shell test -r .rtai_config -a -r .cfok || echo $(confgui))
endif

override srctree := $(shell cd $(srctree) && pwd)

all:: $(forceconf)
	@/bin/true

xconfig: qconf config-script

gconfig: gconf config-script

mconfig menuconfig: mconf config-script

config: conf config-script

oldconfig: oldconf config-script

config-script: config.status

reconfig::
	@touch .rtai_config

reconfig:: config-script

config.status: .rtai_config
	@test -r config.status && recf=yes || recf=no ; \
	eval `grep ^CONFIG_RTAI_INSTALLDIR $<`; \
	eval `grep ^CONFIG_RTAI_MAINTAINER $<`; \
	test x$$CONFIG_RTAI_MAINTAINER_AUTOTOOLS = xy && confopts=--enable-maintainer-mode; \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	CC="$(CROSS_COMPILE)$(CC)" \
	CXX="$(CROSS_COMPILE)$(CXX)" \
	LD="$(CROSS_COMPILE)$(LD)" \
	AR="$(CROSS_COMPILE)$(AR)" \
	RANLIB=$(CROSS_COMPILE)ranlib \
	STRIP=$(CROSS_COMPILE)strip \
	NM=$(CROSS_COMPILE)nm \
	$(srctree)/configure \
	--build=$(build_alias) \
	--host=$(host_alias) \
	--with-kconfig-file=./$< \
	--with-linux-dir=$(RTAI_LINUX_DIR) \
	--prefix=$$CONFIG_RTAI_INSTALLDIR \
	$$confopts ; \
	if test $$? = 0; then \
	   touch .cfok ; \
	   if test x$$recf = xyes ; then \
	      touch .cfchanged ; \
	   fi ; \
	else \
	   rm -f .cfok ; false; \
	fi

qconf: $(KCONFIG_DIR)
	@$(MAKE) -C $(KCONFIG_DIR) \
	-f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig xconfig \
	srctree=$(srctree) ARCH=$(ARCH)

gconf: $(KCONFIG_DIR)
	@$(MAKE) -C $(KCONFIG_DIR) \
	-f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig gconfig \
	srctree=$(srctree) ARCH=$(ARCH)

mconf: $(KCONFIG_DIR)
	@$(MAKE) -C $(KCONFIG_DIR) \
	-f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig menuconfig \
	srctree=$(srctree) ARCH=$(ARCH)

conf: $(KCONFIG_DIR)
	@$(MAKE) -C $(KCONFIG_DIR) \
	-f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig config \
	srctree=$(srctree) ARCH=$(ARCH)

oldconf: $(KCONFIG_DIR)
	@$(MAKE) -C $(KCONFIG_DIR) \
	-f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig oldconfig \
	srctree=$(srctree) ARCH=$(ARCH)

$(KCONFIG_DIR):
	@test -d $@ || mkdir -p $@

help:
	@echo ; echo "This is RTAI's build bootstrapping Makefile. In order to build RTAI," ; \
	echo 'you first need to configure it. Proceed as follows:' ; \
	echo ; echo '$$ cd $$your_builddir' ; echo ; \
	echo '# Configuration using a KDE-based, GTK-based or Dialog-based GUI' ; \
	echo '$$ make -f $$rtai_srcdir/makefile srctree=$$rtai_srcdir {xconfig,gconfig,menuconfig}' ; \
	echo '                            OR,' ; \
	echo '# Configuration using a text-based interface' ; \
	echo '$$ make -f $$rtai_srcdir/makefile srctree=$$rtai_srcdir {config,oldconfig}' ; \
	echo ; echo 'In case a configuration error occurs, re-run the command above to fix the' ; \
	echo 'faulty parameter. Once the configuration is successful, type:' ; echo ; \
	echo '$$ make [all]' ; \
	echo ; echo "To change the configuration from now on, simply run:"; echo ; \
	echo '$$ make {xconfig,gconfig,menuconfig,config}' ; echo

clean distclean:
	if test -r $(KCONFIG_DIR)/Makefile.kconfig ; then \
	$(MAKE) -C $(KCONFIG_DIR) -f Makefile.kconfig clean ; fi
	if test -r GNUmakefile ; then \
	$(MAKE) -f GNUmakefile $@ ; else \
	$(MAKE) -C $(KCONFIG_DIR) -f $(srctree)/$(KCONFIG_DIR)/Makefile.kconfig clean ; \
	fi
	@find . -name autom4te.cache | xargs rm -fr

all ::
	@if test -r GNUmakefile ; then \
	$(MAKE) -f GNUmakefile $@ ; else \
	echo "*** Please configure RTAI first (running 'make help' in RTAI's toplevel dir might help)." ; \
	exit 1; fi

.PHONY: config-script reconfig xconfig gconfig mconfig menuconfig config oldconfig qconf mconf conf clean distclean help
