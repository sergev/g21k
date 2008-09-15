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

#define NEED_MODF
#define NEED_FREXP
#define NEED_LDEXP

#ifndef __MSDOS__
#define __MSDOS__ 1
#endif

#define index(x,y) strchr(x,y)
#define rindex(x,y) strrchr(x,y)

#ifdef MSDOS
#define bzero(x,l) memset(x,0,l)
#define bcopy(s,d,l) memcpy(d,s,l)
#endif

#ifdef __WATCOMC__
#include <string.h>
#include <malloc.h>
#define bcopy(s,d,l) memcpy(d,s,l)
#define bcmp(x,y,l) memcmp(x,y,l)
#define bzero(x,l) memset(x,0,l)

#define ONLY_INT_FIELDS 1
#define USE_PROTOTYPES 1
#define __USE_STDARGS__ 1
#define HAVE_VPRINTF
#endif

#ifdef MSC
#define HAVE_VPRINTF
#endif
