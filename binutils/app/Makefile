#
#  generate asm21k
#  asm21k is a front end to a21000 providing macros and stuff
#  cc is a front end to g21k, presumably
#
#  mp       Analog Devices ADSP-210x0 Assembler Pre-Processor
#  shell    Analog Devices ADSP-210x0 Assembler
#
#  Copyright (c) 1991-1997 Analog Devices, Inc.
#  Copyright (c) 1999 Egil Kvaleberg
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

all: asm21k cc21k

SRC = mp.c mpdata.c mpexpr.c mpsym.c mputil.c

include ../make.def

OBJS = shell.o
OBJ2 = util.o release.o

asm21k: ${OBJ} ${OBJ2}
	${CC} ${CFLAGS} ${OBJ} ${OBJ2} -o asm21k

cc21k:  ${OBJS} ${OBJ2}
	${CC} ${CFLAGS} ${OBJS} ${OBJ2} -o cc21k


