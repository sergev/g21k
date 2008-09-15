#
#  mem21k
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

all: mem21k

SRC = main.c ach_sprt.c error.c process_args.c \
 output_data.c standalone.c creat_list.c clean_list.c

include ../make.def

OBJ2 = util.o coff_io.o release.o achparse.o global.o

mem21k: ${OBJ} ${OBJ2}
	${CC} ${CFLAGS} ${OBJ} ${OBJ2} -o mem21k


