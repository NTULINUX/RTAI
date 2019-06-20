# Copyright (C) 2005-2017 The RTAI project
# This [file] is free software; the RTAI project
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# Makefile generate from template rtai.mak
# ========================================

all: ../$$MODEL$$

RTAIDIR = $(shell rtai-config --prefix)
C_FLAGS = $(shell rtai-config --lxrt-cflags)
COMEDIDIR = $(shell rtai-config --comedi-dir)
ifneq ($(strip $(COMEDIDIR)),)
COMEDILIB = -lcomedi
endif 

SCIDIR = $$SCILAB_DIR$$
RM = rm -f
FILES_TO_CLEAN = *.o ../$$MODEL$$ $$MODEL$$

CC = gcc
CC_OPTIONS = -O -DNDEBUG -Dlinux -DNARROWPROTO -D_GNU_SOURCE

MODEL = $$MODEL$$
OBJSSTAN = rtmain.o common.o $$MODEL$$.o $$OBJ$$

LIBDIRS = -L$(SCIDIR)/../../lib/scilab\
        -L$(SCIDIR)/../../lib/thirdparty\
        -L${JAVA_HOME}/jre/lib/i386 \
         -L${JAVA_HOME}/jre/lib/i386/native_threads \
        -L${JAVA_HOME}/jre/lib/i386/server \
        -L${JAVA_HOME}/lib/i386 \
         -L${JAVA_HOME}/lib/i386/native_threads \
        -L${JAVA_HOME}/lib/i386/server

SCILIBS =\
          -lsciscicos \
          -lsciscicos_blocks \
          -lsciapi_scilab \
          -lscipvm \
         -lscishell \
         -lscicompletion        \
         -lsciscinotes\
         -lscifunctions\
         -lscicore\
         -lsciconsole \
        -lscifftw \
        -lscijvm \
         -lscisparse \
        -lscigraphic_export \
        -lsciboolean \
     -lscispecial_functions\
      -lscilocalization \
        -lsciwindows_tools \
        -lscidouble \
        -lscistring \
        -lmat \
        -lmx \
        -lsciintersci \
        -lscigraphics \
        -lscielementary_functions\
        -lscitime\
        -lscidynamic_link\
        -lscisundials\
	-lmex\
        -lscimalloc\
        -lscioutput_stream\
        -lsciarnoldi\
        -lscicacsd\
        -lscirenderer\
        -lscipolynomials\
        -lscifileio\
        -lscilinear_algebra\
        -lsciaction_binding\
        -lscihistory_manager\
        -lscihistory_browser \
        -lscidynamiclibrary\
        -lsciio\
        -lscistatistics\
        -lpcreposix\
        -lpcre\
        -lscidata_structures\
        -lscilibst\
        -lsciinteger\
        -lscigui\
        -lscihashtable\
        -lscitclsci\
        -lscidoublylinkedlist\
        -lscidifferential_equations\
	-lscicall_scilab

JAVALIBS =  -ljava -lverify  -ljvm  -lhpi
ifeq ($(strip $(COMEDIDIR)),)
OTHERLIBS = 
else
OTHERLIBS = -lcomedi
endif
ULIBRARY = $(RTAIDIR)/lib/libsciblk.so $(RTAIDIR)/lib/liblxrt.so

CFLAGS = $(CC_OPTIONS) -O2 -I$(SCIDIR)/../../include/scilab/core -I$(SCIDIR)/../../include/scilab/scicos -I$(SCIDIR)/../../include/scilab/scicos_blocks  $(C_FLAGS) -DMODEL=$(MODEL) -DMODELN=$(MODEL).c

rtmain.c: $(RTAIDIR)/share/rtai/scicos/rtmain.c $(MODEL).c
	cp $< .

$$MODEL$$: $(OBJSSTAN) $(ULIBRARY)
	gcc  -o $@  $(OBJSSTAN) $(LIBDIRS) $(SCILIBS) $(ULIBRARY) $(OTHERLIBS) -lpthread -lstdc++ -lrt  $(COMEDILIB) -lm
	@echo "### Created executable: $(MODEL) ###"

../$$MODEL$$: $$MODEL$$
	echo  'LD_LIBRARY_PATH="${LD_LIBRARY_PATH};$(SCIDIR)/../../lib/scilab;$(SCIDIR)/../../lib/thirdparty;${JAVA_HOME}/jre/lib/i386;${JAVA_HOME}/jre/lib/i386/native_threads;${JAVA_HOME}/jre/lib/i386/server;$(RTAIDIR)/lib"' > ../$$MODEL$$
	echo  'export LD_LIBRARY_PATH' >> ../$$MODEL$$
	echo  '$(PWD)/$$MODEL$$ \0044*' >> ../$$MODEL$$
	echo '\n' >> ../$$MODEL$$
	chmod a+x ../$$MODEL$$


clean::
	@$(RM) $(FILES_TO_CLEAN)
