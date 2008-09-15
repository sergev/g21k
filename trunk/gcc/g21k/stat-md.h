static char *__stat_md__h_ident__ = "@(#) stat-md.h 2.1@(#)";

#include <fcntl.h>
#include <stdio.h>
#include "insn-codes.h"

extern int rule_use[MAX_INSN_CODE][2];

extern void read_stat_file ();

extern void write_stat_file();

extern void mark_as_used();
