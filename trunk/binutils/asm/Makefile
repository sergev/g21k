#
#  generate a21000
#  BUG: y_tab.c is auto-generated, but I think we don't have the source
#       for it anywhere
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

all: a21000

CDEFS = -DASM

SRC = action.c action_table.c addressing.c assemble.c codegen.c \
 error.c expression.c fileio.c ieee.c lex_yy.c list.c listing.c \
 main.c make_ilnode.c obj_sym_misc.c object.c pass1.c pass2.c reloc.c \
 section.c str.c symbol.c tempio.c token.c token_init.c update.c y_tab.c

include ../make.def

OBJ2 = util.o coff_io.o release.o atfile.o global.o

a21000: ${OBJ} ${OBJ2}
	${CC} ${CFLAGS} ${OBJ} ${OBJ2} -o a21000
