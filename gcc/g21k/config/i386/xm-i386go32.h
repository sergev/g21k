/* Configuration for GNU C-compiler for Intel 80386.
   Copyright (C) 1988 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "i386/xm-dos.h"

#define HAVE_VPRINTF
#define NEED_MODF
#define NEED_FREXP
#define NEED_LDEXP
#define NEED_ATOF

#ifdef __MSDOS__
#define __MSDOS__ 1
#endif

#define BROKEN_DJGCC_DOUBLE_UNSIGNED

#ifdef __WINNT__
#undef alloca  /* Is this necessary? */
#endif
