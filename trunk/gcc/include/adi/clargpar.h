#ifndef _CLARGPAR_H
#define _CLARGPAR_H

#define MAXDEFARGS -1

#define AOP_NONE     0
#define AOP_HELP     1
#define AOP_VERSION  2
#define AOP_SYNTAX   3
#define AOP_CHAR     4
#define AOP_SHORT    5
#define AOP_INT      6
#define AOP_LONG     7
#define AOP_FLOAT    8
#define AOP_DOUBLE   9
#define AOP_STRING   10
#define AOP_BOOL     11
#define AOP_INVBOOL  12
#define AOP_FUNCALL  13

typedef struct CmdLineArgs {
  char *progname;
  char *version;
  char *doc;
  int (* defcollect)();
  int  maxdargc;
  int  dargc;
  char **dargv;
  struct arg {
    char sw;
    char *name;
    char fmt;
    char *doc;
    void *place;
    int (* func)();
  } *args;
} cmdline_t, *cmdline_p;

extern print_args ();
extern int parse_args ();
/*
 parse_args (cargs, argc, argv)
  cmdline_p cargs;
  int argc;
  char **argv;
*/
#endif

