/*
@chapter Command Line Argument Parser
@comment mmh@analog.com

General command line parse utility that modifies pointers in application
to reflect the state of the switches.  It provides help and version
information in a cannonical format suitable for usage in software tools.

This utility also provides vendor, copyright_year,major.minor information
which is a compile time paramater.  The version information conveyed by
major.minor is not the version of a particular module but of the entire 
release.  This information is displayed as a banner with the AOP_VERSION
switch.

@section SYNTAX
   parse_args (control, argc, argv)
   print_args (control)

 COMMAND LINE SWITCH SYNTAX

@example
   /a5
   -a1
   -a 1

   --absolute 1

   --toggle
   --no-toggle

   --no-name      turn bool switch off

   -l+            turn bool switch on
   -l-            turn bool switch off
@end example


@section DEFAULT COLLECTION
   any argument passed that is not a switch is collected into the
   dargv array.  This array is sized by parse_args if dargv is non-zero
   if the value of maxdargc is MAXDEFARGS then the initial value becomes
   argc.  Which, is the maximum default collection count.

   If dargv is over run then a diagnostic is produced.


@section SYNOPSIS
   The argument parser for command line executable objects
   the implementation is isolated to the invokation of
   parse_args which takes a cmdline_p object.

@example
@cartouche
   +-----------+
   | progname  |
   +-----------+
   | version   |
   +-----------+
   | doc       |
   +-----------+
   | defcollect|
   +-----------+
   | maxdargc  |
   +-----------+
   | dargc     |
   +-----------+
   | dargv     |
   +-----------+       +---------------------------------------+
   | ptr2arg   |------>| sw | fmt | name | doc | place | func  |
   +-----------+       +---------------------------------------+
                       | sw | fmt | name | doc | place | func  |
                       +---------------------------------------+
                       | sw | fmt | name | doc | place | func  |
                       +---------------------------------------+
                       /                                       /
                       +---------------------------------------+
                       | sw | fmt | name | doc | place | func  |
                       +---------------------------------------+
                       | 0  |                                  |
                       +---------------------------------------+
                         
@end cartouche
@end example
@center The Argument Parsing Structure

The @code{sw} entry is a single character which if is a space is ignored by
the search algorithims bellow to allow only long arg names.

   Although not obvious the direction of this module is to create
   dialog box entries that can accept user input from a gui based
   entry method.  This  method is fairly trivial, but it makes 
   integration into windows trivial.

   Switches can begin with -, I have included / for DOS not because
   I'm fond of this style but; "when in rome do as the romans".

   Included for readability are long switchnames these are prefixed
   by --.

@section OPERATION
   Argument operations field describes how to interperate the switch.
   Every switch has an operation, the operation can be one of the 
   following:

@table @code
@item	AOP_NONE      
ignore no action
@item	AOP_HELP      
format arg controls as help, and exit
@item	AOP_VERSION   
print version and exit
@item   AOP_SYNTAX    
print command line syntax
@item	AOP_CHAR      
switch takes char arg, value is copied to place
@item	AOP_SHORT     
switch takes short arg, value is copied to place
@item	AOP_INT       
switch takes int arg, value is copied to place
@item	AOP_LONG      
switch takes long arg, value is copied to place
@item	AOP_FLOAT     
switch takes float arg, value is copied to place
@item	AOP_DOUBLE    
switch takes double arg, value is copied to place
@item	AOP_STRING    
switch takes string arg, value is copied to place
@item	AOP_BOOL      
switch is a flag 1 is stored in place as int
@item	AOP_INVBOOL   
switch is a flag 0 is stored in place as int
@item	AOP_FUNCALL   
switch envokes function
@end table

@subsection AOP_FUNCALL
   This is a special switch that causes an application specific function
   to be envoked.  Special attention must be made when using this operation.
   Firstly, be aware that the return value of your function causes the
   advancement of the position pointer of the argument parser.  A return
   value of zero cause no advancement, accept for the default switch skip.
   A negative value will cause a reverse motion and should never ever be
   used.  A positive value will cause a forward motion.  Positive values are
   usefull if the switch takes an argument structure that is not supported
   by the default parsing mechanism.  Like complex numbers ranges etc..

   The function is passed the argument control entry and the remaining
   argc and argv pointers.

@subsection AOP_BOOL
   This operation is a toggle which can be turned on or off via long name
   switch.

@example
     --name        1
     --no-name     0
@end example

@subsection AOP_INVBOOL
   This operation is a toggle which can be turned on or off via long name
   switch.

@example
     --name       0
     --no-name    1
@end example

@section DEFCOLLECT
   if defcollect is non zero then it is envoked per argument.  Where an
   argument is not a switch.

@section BUGS
   I'm sure there are plenty, but as of this writing their are none.

@section EXAMPLE

@example
  int collect_for_cpp (a, argc, argv)
    struct arg *a;
    int argc;
    char **argv;
  {
    struct obstack *o;
    o = a->place;
    obstack_grow (o, argv[0], strlen (argv[0]));
    obstack_grow (o, " ", 1);
    return 0;
  }
  
  struct arg args[] = {
    { 'V', "version",   AOP_VERSION, "print version number",   0, 0 },
    { 'h', "help",      AOP_HELP,    "print help information", 0, 0 },
    { 'I', "include-path", AOP_FUNCALL, "add include search path", &obstack, 
        collect_for_cpp },
    { 'l', "listing",   AOP_BOOL,    "generate listings",      &listing, 0 },
     0
  };
  
  cmdline_t cargs = {
    "asm",
    "1.0",
    "ADSP-fixpoint assembler program",
    0,
    MAXDEFARGS,
    0,0,
    args
  };
  

  main(argc, argv)
    int argc;
    char **argv;
  {
    int i;
    char *p;
  
    obstack_init (&obstack);
    p = obstack_alloc (&obstack, 0);
    parse_args (&cargs, argc, argv);
    print_args (&cargs);
  }  
@end example

@section FUTURE

@itemize
@item need .ini file initialization
@end itemize
   

*/
 
#include <stdio.h>
#include <config.h>
#include <ctype.h>
#include <adi/clargpar.h>

#undef ALLOW_SLASH_SWITCHES

/* @section Release Info

This module defines major release information.  Compile this module with
special paramaters by -D C preprocessor defines.  The defines are:

@table @code
@item RELEASE_VENDOR
This is the name of the organization which maintains the software.
The default value is @emph{"Analog Devices Inc."}.
*/
#ifndef RELEASE_VENDOR
#define RELEASE_VENDOR "Analog Devices Inc."
#endif

/*
@item RELEASE_MAJOR
This is the major revision number of the release.
The default value is @emph{1}.
*/
#ifndef RELEASE_MAJOR
#define RELEASE_MAJOR 1
#endif

/*
@item RELEASE_MINOR
This is the minor revision number of the release.
The default value is @emph{0}.
*/
#ifndef RELEASE_MINOR
#define RELEASE_MINOR 0
#endif

/*
@item RELEASE_DATE
This is the date of the release
The default value is @emph{"1996"}.
*/
#ifndef RELEASE_DATE
#define RELEASE_DATE "1991-1996"
#endif
/* @end table */

static char *aopname[] = {
  "NONE",
  "HELP",
  "VERSION",
  "SYNTAX",
  "CHAR",
  "SHORT",
  "INT",
  "LONG",
  "FLOAT",
  "DOUBLE",
  "STRING",
  "BOOL",
  "INVBOOL",
  "FUNCALL",
  0
};

#ifdef MSC
#define index strchr
#endif
/* @section Implementation Details */

/* @function clargparse_int

The format of integral argument are:

@table @code
@item [1-9][0-9]*        
decimal
@item 0[xX][0-9a-fA-F]*  
hexidecimal
@item 0[bB][01]*
binary
@item 0[0-7]*
octal
@item 0.[0-9]*
fixpoint 1.15
@item [dD]#?[0-9]+
decimal
@item [hH]#?[0-9a-fA-F]+ 
hexidecimal
@item [oO]#?[0-7]+
octal
@item [bB]#?[01]+
binary
@item [fF]#?[0-9]+ 
fixpoint 1.15
@end table
*/

long clargparse_int (arg)
  char *arg;
{
  char fmt;
  int not_done = 1;
  int shiftvalue = 0;
  char * char_bag;
  long value = 0;
  char c;

  switch (*arg) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      fmt = 'd';
      break;

    case '0':  /* Accept diffrent formated integers hex octal and binary. */
      {
	c = *++arg; arg++;
	if (c == 'x' || c == 'X') /* hex input */
	  fmt = 'h';
	else if (c == 'b' || c == 'B')
	  fmt = 'b';
	else if (c == '.')
	  fmt = 'f';
	else {             /* octal */
	  arg--;
	  fmt = 'o';
	}
	break;
      }

    case 'd':
    case 'D':
    case 'h':
    case 'H':
    case 'o':
    case 'O':
    case 'b':
    case 'B':
    case 'f':
    case 'F':
      {
	fmt = *arg++;
	if (*arg == '#')
	  arg++;
      }
  }

  switch (fmt) {
  case 'h':
  case 'H':
    shiftvalue = 4;
    char_bag = "0123456789ABCDEFabcdef";
    break;

  case 'o':
  case 'O':
    shiftvalue = 3;
    char_bag = "01234567";
    break;

  case 'b':
  case 'B':
    shiftvalue = 1;
    char_bag = "01";
    break;

/* @subsection Fractional constants.
The assembler allows for fractional constants to be created
by either the 0.xxxx or the f#xxxx format 

i.e.   0.5 would result in 0x4000

note .5 would result in the identifier .5.

The assembler converts to fractional format 1.15
by the simple rule.

@example
  value = (short)(finput*(1<<15))
@end example
*/
  case 'f':
  case 'F':
  {
    double fval = 0.0;
    double pos = 10.0;
    while (1) {
      int c;
      c = *arg++;

      if (isdigit(c)) {
        double digit = (c - '0') / pos;
        fval = fval + digit;
        pos = pos * 10.0;
      }
      else
      {
	*--arg = c;
        value = (short) (fval * (1 << 15));
        break;
      }
    }
    return value;
  }

  case 'd':
  case 'D':
  default:
  {
    while (1) {
      int c;
      c = *arg++;
      if (isdigit(c))
        value = (value * 10) + (c - '0');
      else
      {
/* @subsection the k-suffix
Constants that are suffixed with k|K are multiplied by 1024

@table @code
@item 1k 
1024
@item 2k
2048
@item nk
etc.
@end table
This suffix is only allowed on decimal constants. */
        if (c == 'k' || c == 'K')
          value *= 1024;
        else
          *--arg = c;
        break;
      }
    }
    return value;
  }
  }

  while (not_done) {
    char c;
    c = *arg++;
    if (c == 0 || !index (char_bag, c)) {
      not_done = 0;
      *--arg = c;
    }
    else
    {
      if (c >= 'a' && c <= 'z')
        c = c - ('a' - '9') + 1;
      else if (c >= 'A' && c <= 'Z')
        c = c - ('A' - '9') + 1;

      c -= '0';

      value = (value << shiftvalue) + c;
    }
  }
  return value;
}

/* @function internal default_arg_value

   Print the default argument value for help 

   if note is non zero then we format the
      value as if it were part of a sentence 
     ; note (value)
*/
static
print_arg_value (note,arg)
  char *note;
  struct arg *arg;
{
  switch (arg->fmt) {
  case AOP_VERSION:
  case AOP_HELP:
  case AOP_SYNTAX:
  case AOP_FUNCALL:
   return;
  }

  if (note)
    printf ("; %s (", note);
  if (arg->place) {
    switch (arg->fmt) {
    case AOP_INVBOOL:
      if (!*(int *)(arg->place))
	printf ("true");
      else
	printf ("false");
      break;
    case AOP_BOOL:
      if (*(int *)(arg->place))
	printf ("true");
      else
	printf ("false");
      break;
      
    case AOP_CHAR:
      printf ("\'%c\'",*(char *)(arg->place));
      break;
    case AOP_SHORT:
      printf ("%d",*(short *)(arg->place));
      break;
    case AOP_INT:
      printf ("%d",*(int *)(arg->place));
      break;
    case AOP_LONG:
      printf ("%ld",*(long *)(arg->place));
      break;
    case AOP_FLOAT:
      printf ("%f",*(float *)(arg->place));
      break;
    case AOP_DOUBLE:
      printf ("%lf",*(double *)(arg->place));
      break;
    case AOP_STRING:
      if (*(char**)(arg->place))
	printf ("%s",*(char **)(arg->place));
      break;
    }
  }
  if (note)
    printf (")");
}

/* @function print_args
Foreach argument structure print the values of each arg */
print_args (cargs)
  cmdline_p cargs;
{
  int i;
  struct arg *arg;
  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    switch (arg->fmt) {
    case AOP_VERSION:
    case AOP_SYNTAX:
    case AOP_HELP:
    case AOP_FUNCALL:
      break;
    default:
      printf ("%s: ", arg->name);
      print_arg_value (0,arg);
      printf ("\n");
    } 
  }
}

/* @function clargparse_syntax
Command Line Syntax

Generate a brief help on how to use the command.  The arguments
are pretty printed and bracketed.

@example
asm: ADSP-fixpoint assembler program, version 1.0
  [-V] [-h] [-l] [-v] [-K] [-N] [-g] [-c] [-_] [-S] [-M] 
  [-I FUNCALL] [-D FUNCALL] [-o STRING] [-A STRING] [-t STRING] 
  [-d FUNCALL] [-O STRING] [--version] [--help] [--listing] 
  [--verbose] [--keep] [--name-mangle] [--debuginfo] 
  [--case-sensitive] [--strip-underscore] [--input-stdin] 
  [--dump-map] [--include-path FUNCALL] [--define FUNCALL] 
  [--output STRING] [--adidsp STRING] [--target-mach STRING] 
  [--debug-dump FUNCALL] [--output-format STRING] 
@end example

*/
clargparse_syntax (cargs)
  cmdline_p cargs;
{
  int pt;
  int i;
  struct arg *arg;
  pt = 0;
  printf ("  ");
  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    if (arg->sw != ' '
        && (arg->fmt == AOP_BOOL 
            || arg->fmt == AOP_INVBOOL 
            || arg->fmt == AOP_HELP 
            || arg->fmt == AOP_VERSION
            || arg->fmt == AOP_SYNTAX))
       printf ("[-%c] ", arg->sw), pt += 5;
    if (pt > 50) {
       printf ("\n  ");
       pt = 0;
    }
  }

  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    if (arg->sw != ' '
        && arg->fmt != AOP_BOOL 
        && arg->fmt != AOP_INVBOOL 
        && arg->fmt != AOP_HELP 
        && arg->fmt != AOP_VERSION
        && arg->fmt != AOP_SYNTAX)
       printf ("[-%c %s] ", arg->sw, aopname[arg->fmt]),
         pt += 6 + strlen (aopname[arg->fmt]);
    if (pt > 50) {
       printf ("\n  ");
       pt = 0;
    }
  }
    
  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    if (arg->fmt == AOP_BOOL 
        || arg->fmt == AOP_INVBOOL 
        || arg->fmt == AOP_HELP 
        || arg->fmt == AOP_VERSION
        || arg->fmt == AOP_SYNTAX)
       printf ("[--%s] ", arg->name),
         pt += 5 + strlen (arg->name);
    if (pt > 50) {
       printf ("\n  ");
       pt = 0;
    }
  }

  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    if (arg->fmt != AOP_BOOL 
        && arg->fmt != AOP_INVBOOL 
        && arg->fmt != AOP_HELP 
        && arg->fmt != AOP_VERSION
        && arg->fmt != AOP_SYNTAX)
       printf ("[--%s %s] ", arg->name, aopname[arg->fmt]),
         pt += 6 + strlen (arg->name) + strlen (aopname[arg->fmt]);
    if (pt > 50) {
       printf ("\n  ");
       pt = 0;
    }
  }
  printf ("\n");
}

/* @function clargparse_version
print version banner */
clargparse_version (cargs)
  cmdline_p cargs;
{
  printf ("%s %s\n", RELEASE_VENDOR, cargs->doc);
  printf ("Copyright (c) %s, %s\n", RELEASE_DATE, RELEASE_VENDOR);
  printf ("Software Release Version %d.%d, module version %s\n", RELEASE_MAJOR, RELEASE_MINOR, cargs->version);
  printf ("\n%s:\n", cargs->progname);
}

/* @function clargparse_help
Print verbose help */
clargparse_help (cargs)
  cmdline_p cargs;
{
  int i;
  struct arg *arg;
  printf ("%s: %s, version %s\n", 
          cargs->progname, cargs->doc, cargs->version);
  for (i=0;cargs->args[i].sw;i++) {
    arg = &cargs->args[i];
    if (arg->sw != ' ')
      printf ("-%c --%s, %s",arg->sw, arg->name, arg->doc);
    else
      printf ("--%s, %s", arg->name, arg->doc);
    print_arg_value ("default",arg);
    printf ("\n");
  } 
}

/* @function internal eval_arg
   This routine is given the arg entry from the args database
   contained in cargs->arg the entry is 'a'.
   we evaluate with respect to the fmt of the arg see clargpar.h 

   this routine returns a 1 if it needs to advance over the next
   argument.  */
static eval_arg (cargs, a, argc, argv, longargname, invert)
  cmdline_p cargs;
  int a;
  int argc;
  char **argv;
  int longargname;
{
  int inc = 0;
  struct arg *arg;
  char *ap;

  arg = &cargs->args[a];
  ap = argv[0];

  switch (arg->fmt) {
  case AOP_VERSION:
  case AOP_SYNTAX:
  case AOP_HELP:
  case AOP_FUNCALL:
  case AOP_BOOL:
  case AOP_INVBOOL:
   break;

  case AOP_CHAR:
  case AOP_SHORT:
  case AOP_INT:
  case AOP_LONG:
  case AOP_FLOAT:
  case AOP_DOUBLE:
  case AOP_STRING:
   if (longargname) {
     ap = argv[1];
     inc = 1;
   } else {
     if (ap[2] != 0) {
       ap = ap+2;
       inc = 0;
     } else {
       ap = argv[1];
       inc = 1;
     }
   }
   break;
  }

  switch (cargs->args[a].fmt) {
  case AOP_VERSION:
    clargparse_version (cargs);
    clargparse_syntax (cargs);
    exit (0);
    break;

  case AOP_SYNTAX: 
      printf ("%s\n", cargs->progname);
      clargparse_syntax (cargs);
      exit (0);
      break;
  case AOP_HELP: {
      clargparse_help (cargs);
      exit (0);
      break;
    } 
  case AOP_FUNCALL:
    if (arg->func)
      inc = (* arg->func)(arg,argc,argv);
    break;
  case AOP_CHAR:
    *(char *)(arg->place) = *ap;
    break;
  case AOP_SHORT:
    *(short *)arg->place = clargparse_int (ap);
    break;
  case AOP_INT:
    *(int *)arg->place = clargparse_int (ap);
    break;
  case AOP_LONG:
    *(long *)arg->place = clargparse_int (ap);
    break;
  case AOP_FLOAT:
    sscanf (ap, "%f", arg->place);
    break;
  case AOP_DOUBLE:
    sscanf (ap, "%lf", arg->place);
    break;
  case AOP_STRING:
    *(char **)(arg->place) = ap;
    break;
  case AOP_BOOL:
    if (invert)
      *(int *)(arg->place) = 0;  
    else
      *(int *)(arg->place) = 1;  
    break;

  case AOP_INVBOOL:
    if (invert)
      *(int *)(arg->place) = 1;  
    else
      *(int *)(arg->place) = 0;  
    break;
  }
  return inc;
}

/* @function clargparse_set_arg_value
Given the name of an argument and a value
if found in cargs modify the value. 

Boolean and inverses use yes,true,on

strings and funcallable args replicate via malloc the arg
*/

clargparse_set_arg_value (cargs, name, val)
  cmdline_p cargs;
  char *name;
  char *val;
{ 
  int i;
  int fmt;
  int argc;
  char *argv[2];
  int swindex = 0;
  int invert = 0;
  argv[0] = name;
  argv[1] = val;
  argc = 2;
  
  for (i=0;cargs->args[i].sw;i++) {
    if (strcmp (&argv[0][swindex],cargs->args[i].name) == 0) {
      fmt = cargs->args[i].fmt;
      if (fmt == AOP_BOOL
	  || fmt == AOP_INVBOOL) {
        if (strncmp (val, "on", 2) == 0
            || strncmp (val, "true", 4) == 0
            || strncmp (val, "yes", 3) == 0)
          invert = 0;
        else
          invert = 1;
        if (fmt == AOP_INVBOOL)
          invert = !invert;
      }
      if (fmt == AOP_STRING) {
         argv[1] = malloc (strlen (val) + 1);
         strcpy (argv[1], val);
      }
      return eval_arg (cargs, i, argc, argv, 1, invert);
    }
  }
}

/* @function internal search_arg
   search args data base, 
   if we have a match evaluate the arg passing argv and argc
   
   if the argv has -- then we assume long argnames and
     hence we use the 'name' field of the arg entry. */
static search_args (cargs, argc, argv)
  cmdline_p cargs;
  int argc;
  char **argv;
{
  int i;
  int longargname,invert;
  int swindex;
  int inc;
  longargname = 0;
  invert = 0;
  swindex = 1;
  inc = 0;

  if (argv[0][1] == '-') {
    longargname = 1;
    swindex = 2;
    if ((argv[0][2] == 'n' 
         && argv[0][3] == 'o')
        || (argv[0][2] == 'N' 
            && argv[0][3] == 'O')
        && argv[0][4] == '-') {
       invert = 1;
       swindex = 5;
    }
  }

nextswitch:
  for (i=0;cargs->args[i].sw;i++) {
    if (longargname) {
      if (strcmp (&argv[0][swindex],cargs->args[i].name) == 0) {
         if (invert && (cargs->args[i].fmt != AOP_BOOL
			||cargs->args[i].fmt != AOP_INVBOOL)) {
           fprintf (stderr,
                   "%s, switch inversion is illegal on non boolean switches\n", 
                   cargs->args[i].name);
           exit(0);
         }
         return eval_arg (cargs, i, argc, argv, longargname, invert);
      }
    } else if (argv[0][swindex] == cargs->args[i].sw) {
       if (argv[0][swindex+1] && (cargs->args[i].fmt == AOP_BOOL
				  ||cargs->args[i].fmt == AOP_INVBOOL)) {
         if (argv[0][swindex+1] == '-')
            invert = 1;
         else if (argv[0][swindex+1] == '+')
            invert = 0;
       }
       inc += eval_arg (cargs, i, argc, argv, longargname, invert);

       /* allow single switches to be passed as -l -x or -lx */
       if ((cargs->args[i].fmt == AOP_BOOL 
	    || cargs->args[i].fmt == AOP_INVBOOL)
           && argv[0][++swindex])
          goto nextswitch;

       return inc;
    }
  }
  fprintf (stderr, "%s: unknown switch specified %s, try -h or --help\n",
                   cargs->progname,
                   argv[0]);
  exit(0);
}

/* @function parse_args
Given the argc,argv vector and the @code{cmdline_p}, parse each argument
in argv via cargs block. */
int parse_args (cargs, argc, argv)
  cmdline_p cargs;
  int argc;
  char **argv;
{
  int i,inc;

  process_atfiles (&argc, &argv);

  read_inifile (cargs, NULL);

  if (cargs->defcollect == NULL) {
    if (cargs->dargv == NULL) {
      if (cargs->maxdargc == MAXDEFARGS)
        cargs->maxdargc = argc;
      cargs->dargc = 0;
      cargs->dargv = (char **)malloc (sizeof(char **)*cargs->maxdargc);
    }
  }
     
  for (i=1;i<argc;i++) {
    if (argv[i][0] == '-'
#ifdef ALLOW_SLASH_SWITCHES
        || argv[i][0] == '/'  /* This causes problems with unix file names */
#endif
) {
      inc = search_args (cargs, argc-i, argv+i);
      if (inc) {
        argc-=inc;
        argv+=inc;
      } 
    }
    else {
      /* perform default collections of command line arguments */
      if (cargs->defcollect) {
        (* cargs->defcollect)(argv[i]);
      } else if (cargs->dargc < cargs->maxdargc)
        cargs->dargv[cargs->dargc++] = argv[i];
      else
        fprintf (stderr, "default argument over run collection %s\n", argv[i]);
    }
  }
}


