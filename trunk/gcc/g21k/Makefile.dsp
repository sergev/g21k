		target	=	21k
21xx	:= 	target	=	21xx

21k  	: cc1
g21k    : gcc21k
21xx 	: cc1

21k	:=	AO	=	21k/aux-output.c
21xx	:=	AO	=	21xx/aux-output.c

21k	:=	MD	=	21k/md
21xx	:=	MD	=	21xx/md

21k	:=	DT	=	21k/debug-trace.c
21xx	:=	DT	=	21xx/debug-trace.c

21k	:=	SM	=	21k/stat-md.c
21xx	:=	SM	=	21xx/stat-md.c

21k	:=	DP	=	21k/debug-prints.c
21xx	:=	DP	=	21xx/debug-prints.c

21k	:=	DR	=	21k/debug-tree.c
21xx	:=	DR	=	21xx/debug-tree.c


g21k	:=	TM	=	21k/tm.h
21k	:=	TM	=	21k/tm.h
21xx	:=	TM	= 	21xx/tm.h

aux-output.c:
	-/usr/bin/rm $@
	ln -s $(AO) $@

md:
	-/usr/bin/rm $@
	ln -s $(MD) $@

debug-trace.c:
	-/usr/bin/rm $@
	ln -s $(DT) $@

debug-tree.c:
	-/usr/bin/rm $@
	ln -s $(DR) $@

stat-md.c:
	-/usr/bin/rm $@
	ln -s $(SM) $@

debug-prints.c:
	-/usr/bin/rm $@
	ln -s $(DP) $@

tm.h:
	-/usr/bin/rm $@
	ln -s $(TM) $@

EXTRA_OBJS =  debug-trace.o stat-md.o debug-prints.o debug-tree.o /usr/lib/debug/malloc.o

LIB21K_SUPPORT=/usr/gcc/xsoft/21k/lib

LIB21K_INC=/usr/gcc/xsoft/21k/include

EXTRA_GCCOBJS = gcc_ach.o achparse.o util.o atfile.o

loop.o:	loop.c doloop.c doloop.h


### This probably won't work since this file is included at the
### very beginning of the main Makefile, before C_OBJS is defined.

CC1_SOURCES = $(C_OBJS:.o=.c) $(OBJS:.o=.c) \
	$(LIBDEPS:.o=.c) $(EXTRA_OBJECTS:.o=.c)

Cflow:
	cflow 	$(CC1_SOURCES) > Cflow

list-sources:
	@echo $(CC1_SOURCES)

dump-stat:	dump-stat.o stat-md.o insn-output.o
		$(CC) $(ALL_CFLAGS) $(LDFLAGS) -o dump-stat \
		dump-stat.o stat-md.o  insn-output.o

GCCDIR = /usr/gcc
	
gcc21k.o: gcc.c  tm.h gvarargs.h obstack.h
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(INCLUDES) -I/usr/include \
  -o gcc21k.o	\
  -DSTANDARD_STARTFILE_PREFIX=\"$(GCCDIR)/lib/\" \
  -DSTANDARD_EXEC_PREFIX=\"$(GCCDIR)/lib/g21k-\" \
  -DDEFAULT_TARGET_MACHINE=\"dsp21k\" \
  -c gcc.c

gcc21k: gcc21k.o version.o $(LIBDEPS) $(EXTRA_GCCOBJS)
	$(CC) $(ALL_CFLAGS) $(LDFLAGS) -o gccnew gcc21k.o version.o $(EXTRA_GCCOBJS) $(LIBS)
# Go via `gccnew' to avoid `file busy' if $(CC) is `gcc'.
	mv -f gccnew gcc21k
# Dump a specs file to make -B./ read these specs over installed ones
	./gcc21k -dumpspecs > specs


##
## SPARC VERSION 
g21k.o: gcc.c $(CONFIG_H) gvarargs.h obstack.h
	$(CC) $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(INCLUDES) -c gcc.c -o g21k.o
g21k-sparc: g21k.o version.o $(LIBDEPS)
	$(CC) $(ALL_CFLAGS) $(LDFLAGS) -o g21k g21k.o version.o $(LIBS)
	./g21k -dumpspecs > specs

dump-macros:
	@echo "LIBS = $(LIBS)"
	@echo "CC = $(CC)"
	@echo "OBSTACK = $(OBSTACK)"
	@echo "USE_ALLOCA = $(USE_ALLOCA)"
	@echo "MALLOC = $(MALLOC)"
	@echo "CLIB = $(CLIB)"
	@echo "X_CFLAGS = $(X_CFLAGS)"
	@echo "T_CFLAGS = $(T_CFLAGS)"
	@echo "XCFLAGS = $(XCFLAGS)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "ALL_CFLAGS = $(ALL_CFLAGS)"
	@echo "OBJS = $(OBJS)"
	@echo "C_OBJS = $(C_OBJS)"
	@echo "HOST_RTL = $(HOST_RTL)"
	@echo "HOST_LIBDEPS = $(HOST_LIBDEPS)"


########## Interpretting cc1 in saber:


GNULIB=/usr/tools/lib/$(TARGET_ARCH:-%=%)/gcc-gnulib
saber_cc1:
	#setopt long_not_int
	#setopt ccargs $(CFLAGS) 
	#setopt load_flags $(CFLAGS) 
	#load $(ALL_CFLAGS) $(C_OBJS) $(OBJS) 
	#load $(LDFLAGS) $(LIBS) $(GNULIB)


C_SRCS = $(C_OBJS:.o=.c)

SRCS = $(OBJS:.o=.c)

interpret_cc1:
	#setopt long_not_int
	#setopt ccargs $(CFLAGS) 
	#setopt load_flags $(CFLAGS) 
	#load $(ALL_CFLAGS) $(C_SRCS) $(SRCS) 
	#load $(LDFLAGS) $(LIBS) $(GNULIB)

GREPPAT = "Some strange pattern for grep"

grep_cc1:
	grep $(GREPPAT) $(C_SRCS) $(SRCS)

cc1.cflow:
	cflow $(INCLUDES) $(C_OBJS:.o=.c) $(OBJS:.o=.c) | expand -2 > cc1.cflow

.SUFFIXES:  .i

.c.i:
	$(COMPILE.c) -E $(INCLUDES) $< > $@

%.o : $(LIB21K_SUPPORT)/%.c
	$(CC) -c $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(INCLUDES) -I$(LIB21K_INC) $< -o $@


# ADI common files - from [...]/xsoft/21k/lib

gcc_ach.o : tm.h $(LIB21K_SUPPORT)/gcc_ach.c $(LIB21K_INC)/app.h \
	$(LIB21K_INC)/achparse.h $(LIB21K_INC)/gcc_ach.h \
	$(LIB21K_INC)/atfile.h $(LIB21K_INC)/util.h 

achparse.o : $(LIB21K_SUPPORT)/achparse.c $(LIB21K_INC)/app.h \
	$(LIB21K_INC)/util.h $(LIB21K_INC)/achparse.h

util.o : $(LIB21K_SUPPORT)/util.c $(LIB21K_INC)/app.h $(LIB21K_INC)/util.h

atfile.o : $(LIB21K_SUPPORT)/atfile.c $(LIB21K_INC)/atfile.h

