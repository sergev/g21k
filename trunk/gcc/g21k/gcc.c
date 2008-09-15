/* Compiler driver program that can handle many languages.
   Copyright (C) 1987, 89, 92, 93, 94, 1995 Free Software Foundation, Inc.

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
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

This paragraph is here to try to keep Sun CC from dying.
The number of chars here seems crucial!!!!  */

/* This program is the user interface to the C compiler and possibly to
other compilers.  It is used because compilation is a complicated procedure
which involves running several programs and passing temporary files between
them, forwarding the users switches to those programs selectively,
and deleting the temporary files at the end.

CC recognizes how to compile each input file by suffixes in the file names.
Once it knows which kind of compilation to perform, the procedure for
compilation is specified by a string called a "spec".  */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h> /*EK*/

#if (defined __WATCOMC__) || (defined MSC)
#define DOSENV /* Building for PC on a PCish compiler */
#endif

#ifndef _WIN32
#ifdef DOSENV
#include <io.h>
#else
#include <sys/file.h>   /* May get R_OK, etc. on some systems.  */
#include <unistd.h>
#include <memory.h>     /* stupid SunOS */
#include <strings.h>
#ifndef __GO32__
#if !defined(SVR4) && !defined(sun) && !defined(POSIX) /*EK* added POSIX!? */
#include <vfork.h>
#endif
#include <sys/wait.h>
#endif /* not __GO32__ */
#endif /* not DOSENV */
#endif /* not _WIN32 */

#include "config.h"
#include "obstack.h"
#ifdef __GNUC__
#define __USE_STDARGS__
#include "stdarg.h"
#else
/* #include "stdarg.h"  Use this for WATCOM */
#include "gvarargs.h"
#endif

#include "../patchlevel.h" /*EK*/

#include "../adiach/atfile.h"

#ifndef R_OK
#define R_OK 4
#define W_OK 2
#define X_OK 1
#endif

#ifndef bcopy
#define bcopy(s,d,l) memcpy(d,s,l)
#endif
#ifndef bzero
#define bzero(x,l) memset(x,0,l)
#endif

/* Define a generic NULL if one hasn't already been defined.  */

#ifndef NULL
#define NULL 0
#endif

#ifndef GENERIC_PTR
#if defined (USE_PROTOTYPES) ? USE_PROTOTYPES : defined (__STDC__)
#define GENERIC_PTR void *
#else
#define GENERIC_PTR char *
#endif
#endif

#ifndef NULL_PTR
#define NULL_PTR ((GENERIC_PTR)0)
#endif

#ifdef USG
#define vfork fork
#endif /* USG */

/* On MSDOS, write temp files in current dir
   because there's no place else we can expect to use.  */
#ifdef __MSDOS__
#ifndef P_tmpdir
#ifdef DOSENV
#define P_tmpdir ".\\"
#else
#define P_tmpdir "./"
#endif
#endif
#endif

/* Test if something is a normal file.  */
#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

/* Test if something is a directory.  */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

/* By default there is no special suffix for executables.  */
#ifndef EXECUTABLE_SUFFIX
#define EXECUTABLE_SUFFIX ""
#endif

#if defined(DSP21K) && !defined(PATH_SEPARATOR)
	/** this is actually a bug fix, but we mark it anyway **/
	/** to make merges easier **/
#if __MSDOS__
#define PATH_SEPARATOR ';'
#else
#define PATH_SEPARATOR ':'
#endif
#endif

#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR '/'
#endif

static char dir_separator_str[] = {DIR_SEPARATOR, 0};

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

extern void free (void *);
extern char *getenv (const char *);

#ifndef errno
extern int errno;
#endif

#ifndef sys_nerr
extern int sys_nerr;
#endif
#ifndef HAVE_STRERROR
#if defined(bsd4_4)
extern const char *const sys_errlist[];
#else
extern char *sys_errlist[]; /*EK* sys_errlist clashes with glibc */
#endif
#else
extern char *strerror();
#endif

#if !defined(sun) && !defined (SVR4) && !defined (__MSDOS__)
extern int execv (const char *a, const char **b), execvp (const char *a, const char **b);
#endif
/* If a stage of compilation returns an exit status >= 1,
   compilation of that file ceases.  */

#define MIN_FATAL_STATUS 1

/* Flag saying to print the full filename of libgcc.a
   as found through our usual search mechanism.  */

static int print_libgcc_file_name;

/* Flag indicating whether we should print the command and arguments */

static int verbose_flag;
static int debug_flag = 0;

/* Nonzero means write "temp" files in source directory
   and use the source file's name in them, and don't delete them.  */

static int save_temps_flag;

/* The compiler version specified with -V */

static char *spec_version;

/* The target machine specified with -b.  */

static char *spec_machine = DEFAULT_TARGET_MACHINE;

/* Nonzero if cross-compiling.
   When -b is used, the value comes from the `specs' file.  */

#ifdef CROSS_COMPILE
static int cross_compile = 1;
#else
static int cross_compile = 0;
#endif

/* This is the obstack which we use to allocate many strings.  */

static struct obstack obstack;

/* This is the obstack to build an environment variable to pass to
   collect2 that describes all of the relevant switches of what to
   pass the compiler in building the list of pointers to constructors
   and destructors.  */

static struct obstack collect_obstack;

extern char *version_string;


/* Specs are strings containing lines, each of which (if not blank)
is made up of a program name, and arguments separated by spaces.
The program name must be exact and start from root, since no path
is searched and it is unreliable to depend on the current working directory.
Redirection of input or output is not supported; the subprograms must
accept filenames saying what files to read and write.

In addition, the specs can contain %-sequences to substitute variable text
or for conditional text.  Here is a table of all defined %-sequences.
Note that spaces are not generated automatically around the results of
expanding these sequences; therefore, you can concatenate them together
or with constant text in a single argument.

 %%     substitute one % into the program name or argument.
 %i     substitute the name of the input file being processed.
 %b     substitute the basename of the input file being processed.
	This is the substring up to (and not including) the last period
	and not including the directory.
 %g     substitute the temporary-file-name-base.  This is a string chosen
	once per compilation.  Different temporary file names are made by
	concatenation of constant strings on the end, as in `%g.s'.
	%g also has the same effect of %d.
 %d     marks the argument containing or following the %d as a
	temporary file name, so that that file will be deleted if CC exits
	successfully.  Unlike %g, this contributes no text to the argument.
 %w     marks the argument containing or following the %w as the
	"output file" of this compilation.  This puts the argument
	into the sequence of arguments that %o will substitute later.
 %W{...}
	like %{...} but mark last argument supplied within
	as a file to be deleted on failure.
 %o     substitutes the names of all the output files, with spaces
	automatically placed around them.  You should write spaces
	around the %o as well or the results are undefined.
	%o is for use in the specs for running the linker.
	Input files whose names have no recognized suffix are not compiled
	at all, but they are included among the output files, so they will
	be linked.
 %p     substitutes the standard macro predefinitions for the
	current target machine.  Use this when running cpp.
 %P     like %p, but puts `__' before and after the name of each macro.
	(Except macros that already have __.)
	This is for ANSI C.
 %I     Substitute a -iprefix option made from GCC_EXEC_PREFIX.
 %s     current argument is the name of a library or startup file of some sort.
	Search for that file in a standard list of directories
	and substitute the full name found.
 %eSTR  Print STR as an error message.  STR is terminated by a newline.
	Use this when inconsistent options are detected.
 %!STR  Print STR as a warning message.  STR is terminated by a newline.
 %x{OPTION}     Accumulate an option for %X.
 %X     Output the accumulated linker options specified by compilations.
 %Y     Output the accumulated assembler options specified by compilations.
 %Z     Output the accumulated preprocessor options specified by compilations.
 %V     Output the accumulated compactor options specified by compilations.
 %a     process ASM_SPEC as a spec.
	This allows config.h to specify part of the spec for running as.
 %A     process ASM_FINAL_SPEC as a spec.  A capital A is actually
	used here.  This can be used to run a post-processor after the
	assembler has done it's job.
 %D     Dump out a -L option for each directory in library_prefixes,
	followed by a -L option for each directory in startfile_prefixes.
 %l     process LINK_SPEC as a spec.
 %L     process LIB_SPEC as a spec.
 %S     process STARTFILE_SPEC as a spec.  A capital S is actually used here.
 %E     process ENDFILE_SPEC as a spec.  A capital E is actually used here.
 %c     process SIGNED_CHAR_SPEC as a spec.
 %C     process CPP_SPEC as a spec.  A capital C is actually used here.
 %1     process CC1_SPEC as a spec.
 %2     process CC1PLUS_SPEC as a spec.
 %3     insert default include path
 %|     output "-" if the input for the current command is coming from a pipe.
 %*     substitute the variable part of a matched option.  (See below.)
	Note that each comma in the substituted string is replaced by
	a single space.

 %$     substitute the variable part of a matched option.  (See below.)
	Note that each comma in the substituted string is replaced by
	a single space.  Note that this subsitutes the basename.


 %{S}   substitutes the -S switch, if that switch was given to CC.
	If that switch was not specified, this substitutes nothing.
	Here S is a metasyntactic variable.
 %{S*}  substitutes all the switches specified to CC whose names start
	with -S.  This is used for -o, -D, -I, etc; switches that take
	arguments.  CC considers `-o foo' as being one switch whose
	name starts with `o'.  %{o*} would substitute this text,
	including the space; thus, two arguments would be generated.
 %{S*:X} substitutes X if one or more switches whose names start with -S are
	specified to CC.  Note that the tail part of the -S option
	(i.e. the part matched by the `*') will be substituted for each
	occurrence of %* within X.
 %{S:X} substitutes X, but only if the -S switch was given to CC.
 %{!S:X} substitutes X, but only if the -S switch was NOT given to CC.
 %{|S:X} like %{S:X}, but if no S switch, substitute `-'.
 %{|!S:X} like %{!S:X}, but if there is an S switch, substitute `-'.
 %{.S:X} substitutes X, but only if processing a file with suffix S.
 %{!.S:X} substitutes X, but only if NOT processing a file with suffix S.
 %(Spec) processes a specification defined in a specs file as *Spec:
 %[Spec] as above, but put __ around -D arguments

The conditional text X in a %{S:X} or %{!S:X} construct may contain
other nested % constructs or spaces, or even newlines.  They are
processed as usual, as described above.

The -O, -f, -m, and -W switches are handled specifically in these
constructs.  If another value of -O or the negated form of a -f, -m, or
-W switch is found later in the command line, the earlier switch
value is ignored, except with {S*} where S is just one letter; this
passes all matching options.

The character | is used to indicate that a command should be piped to
the following command, but only if -pipe is specified.

Note that it is built into CC which switches take arguments and which
do not.  You might think it would be useful to generalize this to
allow each compiler's spec to say which switches take arguments.  But
this cannot be done in a consistent fashion.  CC cannot even decide
which input files have been specified without knowing which switches
take arguments, and it must know which input files to compile in order
to tell which compilers to run.

CC also knows implicitly that arguments starting in `-l' are to be
treated as compiler output files, and passed to the linker in their
proper position among the other output files.  */

/* Define the macros used for specs %a, %l, %L, %S, %c, %C, %1.  */

/* config.h can define ASM_SPEC to provide extra args to the assembler
   or extra switch-translations.  */
#ifndef ASM_SPEC
#define ASM_SPEC ""
#endif

/* config.h can define ASM_FINAL_SPEC to run a post processor after
   the assembler has run.  */
#ifndef ASM_FINAL_SPEC
#define ASM_FINAL_SPEC ""
#endif

/* config.h can define CPP_SPEC to provide extra args to the C preprocessor
   or extra switch-translations.  */
#ifndef CPP_SPEC
#define CPP_SPEC ""
#endif

/* config.h can define CC1_SPEC to provide extra args to cc1 and cc1plus
   or extra switch-translations.  */
#ifndef CC1_SPEC
#define CC1_SPEC ""
#endif

/* config.h can define CC1PLUS_SPEC to provide extra args to cc1plus
   or extra switch-translations.  */
#ifndef CC1PLUS_SPEC
#define CC1PLUS_SPEC ""
#endif

/* config.h can define LINK_SPEC to provide extra args to the linker
   or extra switch-translations.  */
#ifndef LINK_SPEC
#define LINK_SPEC ""
#endif

/* config.h can define LIB_SPEC to override the default libraries.  */
#ifndef LIB_SPEC
#define LIB_SPEC "%{g*:-lg} %{!p:%{!pg:-lc}}%{p:-lc_p}%{pg:-lc_p}"
#endif

/* config.h can define STARTFILE_SPEC to override the default crt0 files.  */
#ifndef STARTFILE_SPEC
#define STARTFILE_SPEC  \
  "%{!nostdlib:\
   %{runhdr*:%*}}"

/*  "%{pg:gcrt0.o%s}%{!pg:%{p:mcrt0.o%s}%{!p:crt0.o%s}}" */
#endif

/* config.h can define SWITCHES_NEED_SPACES to control passing -o and -L.
   Make the string nonempty to require spaces there.  */
#ifndef SWITCHES_NEED_SPACES
#define SWITCHES_NEED_SPACES ""
#endif

/* config.h can define ENDFILE_SPEC to override the default crtn files.  */
#ifndef ENDFILE_SPEC
#define ENDFILE_SPEC ""
#endif

/* This spec is used for telling cpp whether char is signed or not.  */
#ifndef SIGNED_CHAR_SPEC
#define SIGNED_CHAR_SPEC  \
  (DEFAULT_SIGNED_CHAR ? "%{funsigned-char:-D__CHAR_UNSIGNED__}"        \
   : "%{!fsigned-char:-D__CHAR_UNSIGNED__}")
#endif

static char *cpp_spec = CPP_SPEC;
static char *cpp_predefines = CPP_PREDEFINES;
static char *cpp_default_include;
static char *cc1_spec = CC1_SPEC;
static char *cc1plus_spec = CC1PLUS_SPEC;
static char *signed_char_spec = SIGNED_CHAR_SPEC;
static char *asm_spec = ASM_SPEC;
static char *asm_final_spec = ASM_FINAL_SPEC;
static char *link_spec = LINK_SPEC;
static char *lib_spec = LIB_SPEC;
static char *endfile_spec = ENDFILE_SPEC;
static char *switches_need_spaces = SWITCHES_NEED_SPACES;

/* Need this as global for gcc_ach.c.  This was changed to allow the 
architecture file to dictate which runtime header to place in the linker
command line.  The assignment is done in gcc_ach.c - AS*/

char *startfile_spec=STARTFILE_SPEC;

/* This defines which switch letters take arguments.  */

#ifndef SWITCH_TAKES_ARG
#define SWITCH_TAKES_ARG(CHAR)      \
  ((CHAR) == 'D' || (CHAR) == 'U' || (CHAR) == 'o' \
   || (CHAR) == 'e' || (CHAR) == 'T' || (CHAR) == 'u' \
   || (CHAR) == 'I' || (CHAR) == 'm' \
   || (CHAR) == 'L' || (CHAR) == 'A')
#endif

/* This defines which multi-letter switches take arguments.  */

#ifndef WORD_SWITCH_TAKES_ARG
#define WORD_SWITCH_TAKES_ARG(STR)                      \
 (!strcmp (STR, "Tdata") || !strcmp (STR, "include")    \
  || !strcmp (STR, "imacros") || !strcmp (STR, "aux-info"))
#endif

/* Record the mapping from file suffixes for compilation specs.  */

struct compiler
{
  char *suffix;                 /* Use this compiler for input files
				   whose names end in this suffix.  */
  char *spec[4];                /* To use this compiler, pass this spec
				   to do_spec.  This was done especially
				   for MSC 5.1 compiler which had trouble
				   concatenating spec strings*/};

/* Pointer to a vector of `struct compiler' that gives the spec for
   compiling a file, based on its suffix.
   A file that does not end in any of these suffixes will be passed
   unchanged to the loader and nothing else will be done to it.

   An entry containing two 0s is used to terminate the vector.

   If multiple entries match a file, the last matching one is used.  */

static struct compiler *compilers;

/* Number of entries in `compilers', not counting the null terminator.  */

static int n_compilers;

#if defined (DSP21K)
#define ASM_CMD "a21000"
#elif defined (A21C0)
#define ASM_CMD "a21c"
#define ASM_NEEDS_DASH /* Apparently, needs an explicit '-' to read stdin. */
#elif defined (DSP21XX)     /*EK*/
#define ASM_CMD "asm2"      /*EK*/
#else
#define ASM_CMD "as"
#endif

#ifdef ASM_NEEDS_DASH
#define APIPE "|!pipe:"
#else
#define APIPE "!pipe:"
#endif

/* The default list of file name suffixes and their compilation specs.  */

static struct compiler default_compilers[] =
{
  {".c", "@c"},

#ifdef ADI
  {".C", "@c"},
#endif
#ifdef CRIPPLE
  {"@c",
   "cpp -lang-c %{C} %{v} %{I*} %{P} %I %{O2:}"
     "%{C:%{!E:%eGNU C does not support -C without using -E}}"
     "-undef -D__GNUC__=2 %p %P %c %{!g*:-D__OPTIMIZE__}%{g*} %C %{D*} %{U*} "
     "%i %{!E:%g.i}%{E:%W{o*}} \n",

     "%{!E:cc1 %g.i %1 -quiet -dumpbase %b.c %{g*}%{!g*:-O2} "
       "%{g*:%{O2:%!Optimized code (-O2) with debugging (-g) is not supported "
	 "in the lite version of the compiler.  Optimization will be "
	 "suppressed.}}"
       "%{v:-version} %{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %g.s} \n"
       "%{!S:cpp -lang-asm -P %{v} %I -undef -D__GNUC__=2 %C %g.s -o %g.is \n"
	 ASM_CMD " %a "
	 "%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %g.is \n }}"},
#else
  {"@c",
#ifdef DSP21K
   "%{pipe:%e-pipe is unsupported for g21k cross tools.} "
#endif
   "cpp -lang-c %{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I "
	"%{C:%{!E:%eGNU C does not support -C without using -E}} "
	"%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} "
	"-undef -D__GNUC__=2 %{ansi:-trigraphs -$ -D__STRICT_ANSI__} "
	"%{!undef:%{!ansi:%p} %P} %{trigraphs} "
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional} "
	"%{traditional-cpp:-traditional} "
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z ",
	"%i %{!M:%{!MM:%{!E:%{!pipe:%g.i}}}}%{E:%W{o*}}%{M:%W{o*}}%{MM:%W{o*}} |\n "
    "%{!M:%{!MM:%{!E:cc1 %{!pipe:%g.i} %1 "
		   "%{!Q:-quiet} -dumpbase %b.c %{d*} %{m*} %{a} "
		   "%{g*} %{O*} %{W*} %{w} %{pedantic*} %{ansi} "
		   "%{traditional} %{v:-version} %{pg:-p} %{p} %{f*} "
		   "%{aux-info*} "
		   "%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} "
		   "%{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n ",
#ifndef DSP21K
	      "%{!S:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
		      "%{c:%W{o*}%{!o*:-o %w%b.o}} %{!c:-o %d%w%b.o} "
		      "%{" APIPE "%g.s} %A\n }"
#else
       "%{!S:"
	 "%{!S1:cpp -lang-asm -P %{nostdinc*} %{v} %{i*} %I "
	   "-undef -D__GNUC__=2 %{H} %{d*} %C %{|!pipe:%g.s} "
	   "%{!CO:%{SO:%W{o*}%{!o*:%b.is}}%{!SO:%{!pipe:-o %g.is}}}"
	   "%{CO:%{g*:%{SO:%W{o*}%{!o*:%b.is}}%{!SO:%{!pipe:-o %g.is}}}"
	     "%{!g*:%{!pipe:%g.us}}} |\n }"
	 "%{S1:%{!CO:%{SO:%eThe -SO -S1 combination is only supported "
	   "with -CO.  To get an assembly listing, use -S instead.}}}",

	 "%{CO:%{!g*:gco -fo %{v:-verbose} %V -i %{|!pipe:%g.%{!S1:u}s} "
	   "%{SO:%W{o*}%{!o*:-o %b.is}}%{!SO:%{!pipe:-o %g.is}} |\n }"
	   "%{g*:%!Warning: The global code optimizer is incompatible "
	   "with -g and will be suppressed.  For full optimization, "
	   "turn off debugging.}}"
	
	 "%{!SO:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
	   "%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} "
	   "%{" APIPE "%g.%{CO:%{!g*:i}%{g*:%{!S1:i}}}%{!CO:%{!S1:i}}s} "
	   "%A\n }}"
#endif /* DSP21K */
    "}}}"},
#endif /* !CRIPPLE */


  {"-",
#ifdef CRIPPLE
   "%{E:cpp -lang-c %{C} %{v} %{I*} %{P} %I"
     "%{C:%{!E:%eGNU C does not support -C without using -E}}"
     "-undef -D__GNUC__=2 %p %P %c %{!g*:-D__OPTIMIZE__}%{g*} %C %{D*} %{U*} "
     "%i %W{o*}}"
#else
   "%{E:cpp -lang-c %{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I"
	"%{C:%{!E:%eGNU C does not support -C without using -E}}"
	"%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} "
	"-undef -D__GNUC__=2 %{ansi:-trigraphs -$ -D__STRICT_ANSI__}"
	"%{!undef:%{!ansi:%p} %P} %{trigraphs}"
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional}"
	"%{traditional-cpp:-traditional}"
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z"
	"%i %W{o*}}"
#endif /* !CRIPPLE */
    "%{!E:%e-E required when input is from standard input}"},


#ifndef ADI
  {".m", "@objective-c"},
  {"@objective-c",
   "cpp -lang-objc %{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I "
	"%{C:%{!E:%eGNU C does not support -C without using -E}} "
	"%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} "
	"-undef -D__OBJC__ -D__GNUC__=2 %{ansi:-trigraphs -$ -D__STRICT_ANSI__} "
	"%{!undef:%{!ansi:%p} %P} %{trigraphs} "
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional} "
	"%{traditional-cpp:-traditional} "
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z "
	"%i %{!M:%{!MM:%{!E:%{!pipe:%g.i}}}}%{E:%W{o*}}%{M:%W{o*}}%{MM:%W{o*}} |\n "
    "%{!M:%{!MM:%{!E:cc1obj %{!pipe:%g.i} %1 "
		   "%{!Q:-quiet} -dumpbase %b.m %{d*} %{m*} %{a} "
		   "%{g*} %{O*} %{W*} %{w} %{pedantic*} %{ansi} "
		   "%{traditional} %{v:-version} %{pg:-p} %{p} %{f*} "
		   "-lang-objc %{gen-decls} "
		   "%{aux-info*} "
		   "%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} "
		   "%{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n "
	      "%{!S:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
		      "%{c:%W{o*}%{!o*:-o %w%b.o}} %{!c:-o %d%w%b.o} "
		      "%{" APIPE "%g.s} %A\n }}}}"},
#endif /* !ADI */


#ifndef CRIPPLE
  {".h", "@c-header"},
  {"@c-header",
   "%{!E:%eCompilation of header file requested} "
    "cpp %{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I"
	"%{C:%{!E:%eGNU C does not support -C without using -E}}"
	 "%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} "
	"-undef -D__GNUC__=2 %{ansi:-trigraphs -$ -D__STRICT_ANSI__}"
	"%{!undef:%{!ansi:%p} %P} %{trigraphs}"
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional}"
	"%{traditional-cpp:-traditional}"
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z "
	"%i %W{o*}"},
#endif /* !CRIPPLE */


#ifndef ADI
  {".cc", "@c++"},
  {".cxx", "@c++"},

  {".C", "@c++"},

  {"@c++",
   "cpp -lang-c++ %{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I "
	"%{C:%{!E:%eGNU C++ does not support -C without using -E}} "
	"%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} "
	"-undef -D__GNUC__=2 -D__GNUG__=2 -D__cplusplus "
	"%{ansi:-trigraphs -$ -D__STRICT_ANSI__} %{!undef:%{!ansi:%p} %P} "
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional} "
	"%{traditional-cpp:-traditional} %{trigraphs} "
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z "
	"%i %{!M:%{!MM:%{!E:%{!pipe:%g.i}}}}%{E:%W{o*}}%{M:%W{o*}}%{MM:%W{o*}} |\n "
    "%{!M:%{!MM:%{!E:cc1plus %{!pipe:%g.i} %1 %2 "
		   "%{!Q:-quiet} -dumpbase %b.cc %{d*} %{m*} %{a} "
		   "%{g*} %{O*} %{W*} %{w} %{pedantic*} %{ansi} %{traditional} "
		   "%{v:-version} %{pg:-p} %{p} %{f*} "
		   "%{aux-info*} "
		   "%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} "
		   "%{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n "
	      "%{!S:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
		      "%{c:%W{o*}%{!o*:-o %w%b.o}} %{!c:-o %d%w%b.o} "
		      "%{" APIPE "%g.s} %A\n }}}}"},
#endif /* !ADI */


  {".i", {"@cpp-output"}},
#ifdef CRIPPLE
  {"@cpp-output",
   "%{!E:cc1 %i %1 -quiet -dumpbase %b.c %{g*}%{!g*:-O2} "
     "%{g*:%{O2:%!Optimized code (-O2) with debugging (-g) is not supported "
       "in the lite version of the compiler.  Optimization will be "
       "suppressed.}}"
     "%{v:-version} %{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %g.s} \n"
     "%{!S:cpp -lang-asm -P %{v} %I -undef -D__GNUC__=2 %C %g.s -o %g.is \n"
       ASM_CMD " %a %{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %g.is \n }}"},
#else
  {"@cpp-output",
   "%{!M:%{!MM:%{!E:cc1 %i %1 %{!Q:-quiet} %{d*} %{m*} %{a}"
			"%{g*} %{O*} %{W*} %{w} %{pedantic*} %{ansi}"
			"%{traditional} %{v:-version} %{pg:-p} %{p} %{f*}"
			"%{aux-info*}"
			"%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}}"
			"%{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n"
#ifndef DSP21K
		     "%{!S:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y"
			     "%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o}"
			     "%{" APIPE "%g.s} %A\n }"
#else
    "%{!S:"
      "%{!S1:cpp -lang-asm -P "
	"%{nostdinc*} %{C} %{v} %{i*} %I ",
	"-undef -D__GNUC__=2 %{H} %{d*} %C %{|!pipe:%g.s} "
	"%{!CO:%{SO:%W{o*}%{!o*:%b.is}}%{!SO:%{!pipe:-o %g.is}}}"
	"%{CO:%{g*:%{SO:%W{o*}%{!o*:%b.is}}%{!SO:%{!pipe:-o %g.is}}}"
	  "%{!g*:%{!pipe:%g.us}}} |\n }"
      "%{S1:%{!CO:%{SO:%eThe -SO -S1 combination is only supported "
	"with -CO.  To get an assembly listing, use -S instead.}}}",

      "%{CO:%{!g*:gco -fo %{v:-verbose} %V -i %{|!pipe:%g.%{!S1:u}s} "
	"%{SO:%W{o*}%{!o*:-o %b.is}}%{!SO:%{!pipe:-o %g.is}} |\n }"
	"%{g*:%!Warning: The global code optimizer is incompatible "
	"with -g and will be suppressed.  For full optimization, "
	"turn off debugging.}}"

      "%{!SO:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
	"%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} "
	"%{" APIPE "%g.%{CO:%{!g*:i}%{g*:%{!S1:i}}}%{!CO:%{!S1:i}}s} %A\n }}"
#endif /* DSP21K */
   "}}}"},
#endif /* !CRIPPLE */


#ifndef ADI
  {".ii", "@c++-cpp-output"},
  {"@c++-cpp-output",
   "cc1plus %i %1 %2 %{!Q:-quiet} %{d*} %{m*} %{a} "
	    "%{g*} %{O*} %{W*} %{w} %{pedantic*} %{ansi} %{traditional} "
	    "%{v:-version} %{pg:-p} %{p} %{f*} "
	    "%{aux-info*} "
	    "%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} "
	    "%{S:%W{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n "
       "%{!S:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
	       "%{c:%W{o*}%{!o*:-o %w%b.o}} %{!c:-o %d%w%b.o} "
	       "%{" APIPE "%g.s} %A\n }"},
#endif /* !ADI */

  {".s",   "@assembler"},

  {".S",   "@assembler"},

  {".asm", "@assembler" },

  {".ASM", "@assembler" },

#ifdef CRIPPLE
  {"@assembler",
   "%{!S:cpp -lang-asm -P %{v} %{I*} %I -undef %p %P %c "
     "%{!g*:-D__OPTIMIZE__}%{g*} %C %{D*} %{U*} %i %{!E:%g.is}%{E:%W{o*}} \n",
    "%{!E:" ASM_CMD " %a %{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %g.is \n }}"},
#else
  {"@assembler",
   "%{!S:%{!S1:cpp -lang-asm -P "
	"%{nostdinc*} %{C} %{v} %{A*} %{I*} %{P} %I"
	"%{C:%{!E:%eGNU C does not support -C without using -E}}"
	"%{M} %{MM} %{MD:-MD %b.d} %{MMD:-MMD %b.d} %{trigraphs}"
	"-undef -$ %{!undef:%p %P}"
	"%c %{O*:%{!O0:-D__OPTIMIZE__}} %{traditional} %{ftraditional:-traditional}"
	"%{traditional-cpp:-traditional}"
	"%{g*} %{W*} %{w} %{pedantic*} %{H} %{d*} %C %{D*} %{U*} %{i*} %Z"
#ifdef DSP21K
	"%i %{!M:%{!MM:%{!E:%{!pipe:%g.%{CO:%{!g*:u}%{g*:i}}%{!CO:i}s}}}}"
#else
	"%i %{!M:%{!MM:%{!E:%{!pipe:%g.is}}}}"
#endif
	   "%{E:%W{o*}}%{M:%W{o*}}%{MM:%W{o*}} |\n }",
   "%{!M:%{!MM:%{!E:"
#ifdef DSP21K
	 "%{S1:%{!CO:%{SO:%eThe -SO -S1 combination is only supported "
	   "with -CO.}}}"
	 "%{CO:%{!g*:gco -fo %{v:-verbose} %V -i "
	   "%{S1:%i}%{!S1:%{|!pipe:%g.us}} "
	   "%{SO:%W{o*}%{!o*:-o %b.is}}%{!SO:%{!pipe:-o %g.is}} |\n }"
	   "%{g*:%!Warning: The global code optimizer is incompatible "
	   "with -g and will be suppressed.  For full optimization, "
	   "turn off debugging.}}",
	
	 "%{!SO:" ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
	   "%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} "
	   "%{" APIPE "%{CO:%{g*:%{S1:%i}%{!S1:%g.is}}%{!no-CO:%{!g*:%g.is}}}"
	     "%{!CO:%{S1:%i}%{!S1:%g.is}}} %A\n }"
#else
	ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
	       "%{c:%W{o*}%{!o*:-o %w%b.o}} %{!c:-o %d%w%b.o} "
	       "%{" APIPE "%g.is} %A\n "
#endif /* !DSP21K */
    "}}}}"},
#endif /* !CRIPPLE */


#ifdef DSP21K
  {".is",   "@preprocessed-assembler"},
  {".IS",   "@preprocessed-assembler"},
  {"@preprocessed-assembler",
   "%{!M:%{!MM:%{!E:%{!S:%{!SO:"
     ASM_CMD " %{R} %{j} %{J} %{h} %{d2} %a %Y "
     "%{c:%W{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %i %A\n }}}}}"},
#endif /* !DSP21K */

  /* Mark end of table */
  {0, 0}
};

/* Number of elements in default_compilers, not counting the terminator.  */

static int n_default_compilers
  = (sizeof default_compilers / sizeof (struct compiler)) - 1;

/* Here is the spec for running the linker, after compiling all files.  */

#ifdef DSP21K

#ifdef CRIPPLE
static char *link_command_spec =
"%{!E:%{!S:%{!c:ld21k %{map} %S %l %o %{L*} %{o*}%{!o*:-o 21k.exe} %L %X}}}";
#else
static char *link_command_spec =
"%{!M:%{!MM:%{!SO:%{!E:%{!S:%{!c:ld21k %{map} %S %l %o %{L*} "
		"-o %{nomem: %{o*: %*} %{!o*: 21k.exe}}"
		   "%{!nomem:"
		      "%{!save-temps:  %d%{o*:%$.lnk} %d%{!o*:21k.lnk}}"
		      "%{save-temps: %{o*: %$.lnk} %{!o*: 21k.lnk}}} "
		"%{arch*:-a %*} "
		"%{!nostdlib: %L} %X "
		"%{!nomem:|\n mem21k %{arch*:-a %*} "
		   "%{o*: %$.lnk} %{!o*: 21k.lnk} "
		   "-o %{o*: %*} %{!o*: 21k.exe }}}}}}}}";
#endif


#else /* DSP21K */
/*EK* added support for -arch, 4 places */
#ifdef LINK_LIBGCC_SPECIAL
/* Have gcc do the search for libgcc.a.  */
/* -u* was put back because both BSD and SysV seem to support it.  */
static char *link_command_spec =
"%{!c:%{!M:%{!MM:%{!E:%{!S:ld %l %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} "
			"%{r} %{s} %{T*} %{t} %{u*} %{x} %{z} "
			"%{!A:%{!nostdlib:%S}} "
			"%{arch*:-a %*} "
			"%{L*} %D %o %{!nostdlib:libgcc.a%s %L libgcc.a%s %{!A:%E}}\n }}}}}";
#else
/* Use -L and have the linker do the search for -lgcc.  */
#if defined (A21C0)
static char *link_command_spec = 
"%{!c:%{!M:%{!MM:%{!E:%{!S:ld21c "
		       "%l %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} "
		       "%{r} %{s} %{T*} %{t} %{u*} %{x} %{z} "
		       "%{!A:%{!nostdlib:%S}} "
		       "%{arch*:-a %*} "
		       "%{L*} %D %o %{!nostdlib: %L %{!A:%E}}\n }}}}}";
			
#elif defined (DSP21XX)     /*EK*/
static char *link_command_spec = 
"%{!c:%{!M:%{!MM:%{!E:%{!S:ld21 "
		       "%l %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} "
		       "%{r} %{s} %{T*} %{t} %{u*} %{x} %{z} "
		       "%{!A:%{!nostdlib:%S}} "
		       "%{arch*:-a %*} "
		       "%{L*} %D %o %{!nostdlib: %L %{!A:%E}}\n }}}}}";
#else
static char *link_command_spec = 
"%{!c:%{!M:%{!MM:%{!E:%{!S:ld %l %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} "
			"%{r} %{s} %{T*} %{t} %{u*} %{x} %{z} "
			"%{!A:%{!nostdlib:%S}} "
			"%{arch*:-a %*} "
			"%{L*} %D %o %{!nostdlib:-lgcc %L -lgcc %{!A:%E}}\n }}}}}";
#endif /* !A21C0 */
#endif /* !LINK_LIBGCC_SPECIAL */
#endif /* !DSP21K */

/* A vector of options to give to the linker.
   These options are accumulated by %x,
   and substituted into the linker command with %X.  */
static int n_linker_options;
static char **linker_options;

/* A vector of options to give to the assembler.
   These options are accumulated by -Wa,
   and substituted into the assembler command with %Y.  */
static int n_assembler_options;
static char **assembler_options;

/* A vector of options to give to the preprocessor.
   These options are accumulated by -Wp,
   and substituted into the preprocessor command with %Z.  */
static int n_preprocessor_options;
static char **preprocessor_options;

#ifdef DSP21K
/* A vector of options to give to the compactor.
   These options are accumulated by -Wo,
   and substituted into the compactor command with %V.  */
static int n_compactor_options;
static char **compactor_options;
#endif


/* Structures to keep track of prefixes to try when looking for files. */

struct prefix_list
{
  char *prefix;               /* String to prepend to the path. */
  struct prefix_list *next;   /* Next in linked list. */
  int require_machine_suffix; /* Don't use without machine_suffix.  */
  int *used_flag_ptr;         /* 1 if a file was found with this prefix.  */
};

struct path_prefix
{
  struct prefix_list *plist;  /* List of prefixes to try */
  int max_len;                /* Max length of a prefix in PLIST */
  char *name;                 /* Name of this list (used in config stuff) */
};


static void set_spec (char *name, char *spec);
static struct compiler *lookup_compiler (char *name, int length, char *language);
static char *find_a_file (struct path_prefix *pprefix, char *name, int mode);
static void add_prefix (struct path_prefix *pprefix, char *prefix, int first, int require_machine_suffix, int *warn);
static char *skip_whitespace (char *p);
static void record_temp_file (char *filename, int always_delete, int fail_delete);
static char *handle_braces (register char *p);
static char *save_string (char *s, int len);
static char *concat (char *s1, const char *s2, const char *s3);
static int do_spec (char *spec);
static int do_spec_1 (char *spec, int inswitch, char *soft_matched_part);
static char *find_file (char *name);
static int is_linker_dir (char *path1, char *path2);
static void validate_switches (char *start);
static void validate_all_switches (void);
static int check_live_switch (int switchnum, int prefix_length);
static void give_switch (int switchnum, int omit_first_word);
static void pfatal_with_name (char *name);
static void perror_with_name (char *name);
static void perror_exec (char *name);
static int is_bad_switch(char *p);
#ifndef CRIPPLE
#define is_suppressed_option(x) FALSE
#define is_default_option(x) FALSE
#else
static int is_suppressed_option (char *option);
static int is_default_option (char *option);
#endif
#ifdef __USE_STDARGS__
void fatal (char *,...);
void error (char *,...);
#else
void fatal ();
void error ();
#endif

void fancy_abort (void);
void *xmalloc (unsigned int size);
void *xrealloc (void *ptr, unsigned int size);
#ifdef __WATCOMC__
char *mktemp(char *template);
#endif

char *
my_strerror(e)
     int e;
{

#ifdef HAVE_STRERROR
  return strerror(e);

#else

  static char buffer[30];
  if (!e)
    return "";

  if (e > 0 && e < sys_nerr)
    return sys_errlist[e];

  sprintf (buffer, "Unknown error %d", e);
  return buffer;
#endif
}

/* Read compilation specs from a file named FILENAME,
   replacing the default ones.

   A suffix which starts with `*' is a definition for
   one of the machine-specific sub-specs.  The "suffix" should be
   *asm, *cc1, *cpp, *link, *startfile, *signed_char, etc.
   The corresponding spec is stored in asm_spec, etc.,
   rather than in the `compilers' vector.

   Anything invalid in the file is a fatal error.  */

static void
read_specs (char *filename)
{
  int desc;
  int readlen;
  struct stat statbuf;
  char *buffer;
  register char *p;

  if (verbose_flag)
    fprintf (stderr, "Reading specs from %s\n", filename);

  /* Open and stat the file.  */
  desc = open (filename, 0, 0);
  if (desc < 0)
    pfatal_with_name (filename);
  if (stat (filename, &statbuf) < 0)
    pfatal_with_name (filename);

  /* Read contents of file into BUFFER.  */
  buffer = xmalloc ((unsigned) statbuf.st_size + 1);
  readlen = read (desc, buffer, (unsigned) statbuf.st_size);
  if (readlen < 0)
    pfatal_with_name (filename);
  buffer[readlen] = 0;
  close (desc);

  /* Scan BUFFER for specs, putting them in the vector.  */
  p = buffer;
  while (1)
    {
      char *suffix;
      char *spec;
      char *in, *out, *p1, *p2;

      /* Advance P in BUFFER to the next nonblank nocomment line.  */
      p = skip_whitespace (p);
      if (*p == 0)
	break;

      /* Find the colon that should end the suffix.  */
      p1 = p;
      while (*p1 && *p1 != ':' && *p1 != '\n') p1++;
      /* The colon shouldn't be missing.  */
      if (*p1 != ':')
	fatal ("specs file malformed after %d characters", p1 - buffer);
      /* Skip back over trailing whitespace.  */
      p2 = p1;
      while (p2 > buffer && (p2[-1] == ' ' || p2[-1] == '\t')) p2--;
      /* Copy the suffix to a string.  */
      suffix = save_string (p, p2 - p);
      /* Find the next line.  */
      p = skip_whitespace (p1 + 1);
      if (p[1] == 0)
	fatal ("specs file malformed after %d characters", p - buffer);
      p1 = p;
      /* Find next blank line.  */
      while (*p1 && !(*p1 == '\n' && p1[1] == '\n')) p1++;
      /* Specs end at the blank line and do not include the newline.  */
      spec = save_string (p, p1 - p);
      p = p1;

      /* Delete backslash-newline sequences from the spec.  */
      in = spec;
      out = spec;
      while (*in != 0)
	{
	  if (in[0] == '\\' && in[1] == '\n')
	    in += 2;
	  else if (in[0] == '#')
	    {
	      while (*in && *in != '\n') in++;
	    }
	  else
	    *out++ = *in++;
	}
      *out = 0;

      if (suffix[0] == '*')
	{
	  if (! strcmp (suffix, "*link_command"))
	    link_command_spec = spec;
	  else
	    set_spec (suffix + 1, spec);
	}
      else
	{
	  /* Add this pair to the vector.  */
	  compilers
	    = ((struct compiler *)
	       xrealloc (compilers, (n_compilers + 2) * sizeof (struct compiler)));
	  compilers[n_compilers].suffix = suffix;
	  memset (compilers[n_compilers].spec, 0,
		 sizeof compilers[n_compilers].spec);
	  compilers[n_compilers].spec[0] = spec;
	  n_compilers++;
	  memset (&compilers[n_compilers], 0,
		 sizeof compilers[n_compilers]);
	}

      if (*suffix == 0)
	link_command_spec = spec;
    }

  if (link_command_spec == 0)
    fatal ("spec file has no spec for linking");
}

static char *
skip_whitespace (char *p)
{
  while (1)
    {
      /* A fully-blank line is a delimiter in the SPEC file and shouldn't
	 be considered whitespace.  */
      if (p[0] == '\n' && p[1] == '\n' && p[2] == '\n')
	return p + 1;
      else if (*p == '\n' || *p == ' ' || *p == '\t')
	p++;
      else if (*p == '#')
	{
	  while (*p != '\n') p++;
	  p++;
	}
      else
	break;
    }

  return p;
}

/* Structure to keep track of the specs that have been defined so far.  These
   are accessed using %(specname) or %[specname] in a compiler or link spec. */

struct spec_list
{
  char *name;                 /* Name of the spec. */
  char *spec;                 /* The spec itself. */
  struct spec_list *next;     /* Next spec in linked list. */
};

/* List of specs that have been defined so far. */

static struct spec_list *specs = (struct spec_list *) 0;

/* Change the value of spec NAME to SPEC.  If SPEC is empty, then the spec is
   removed; If the spec starts with a + then SPEC is added to the end of the
   current spec. */

static void
set_spec (char *name, char *spec)
{
  struct spec_list *sl;
  char *old_spec;

  /* See if the spec already exists */
  for (sl = specs; sl; sl = sl->next)
    if (strcmp (sl->name, name) == 0)
      break;

  if (!sl)
    {
      /* Not found - make it */
      sl = (struct spec_list *) xmalloc (sizeof (struct spec_list));
      sl->name = save_string (name, strlen (name));
      sl->spec = save_string ("", 0);
      sl->next = specs;
      specs = sl;
    }

  old_spec = sl->spec;
  if (name && spec[0] == '+' && isspace (spec[1]))
    sl->spec = concat (old_spec, spec + 1, "");
  else
    sl->spec = save_string (spec, strlen (spec));

  if (! strcmp (name, "asm"))
    asm_spec = sl->spec;
  else if (! strcmp (name, "asm_final"))
    asm_final_spec = sl->spec;
  else if (! strcmp (name, "cc1"))
    cc1_spec = sl->spec;
  else if (! strcmp (name, "cc1plus"))
    cc1plus_spec = sl->spec;
  else if (! strcmp (name, "cpp"))
    cpp_spec = sl->spec;
  else if (! strcmp (name, "endfile"))
    endfile_spec = sl->spec;
  else if (! strcmp (name, "lib"))
    lib_spec = sl->spec;
  else if (! strcmp (name, "link"))
    link_spec = sl->spec;
  else if (! strcmp (name, "predefines"))
    cpp_predefines = sl->spec;
  else if (! strcmp (name, "signed_char"))
    signed_char_spec = sl->spec;
  else if (! strcmp (name, "startfile"))
    startfile_spec = sl->spec;
  else if (! strcmp (name, "switches_need_spaces"))
    switches_need_spaces = sl->spec;
  else if (! strcmp (name, "cross_compile"))
    cross_compile = atoi (sl->spec);
  /* Free the old spec */
  if (old_spec)
    free (old_spec);
}

/* Accumulate a command (program name and args), and run it.  */

/* Vector of pointers to arguments in the current line of specifications.  */

static char **argbuf;

/* Number of elements allocated in argbuf.  */

static int argbuf_length;

/* Number of elements in argbuf currently in use (containing args).  */

static int argbuf_index;

/* Number of commands executed so far.  */

static int execution_count;

/* Name with which this program was invoked.  */

static char *programname;


/* List of prefixes to try when looking for executables. */

static struct path_prefix exec_prefixes = { 0, 0, "exec" };

/* List of prefixes to try when looking for startup (crt0) files. */

static struct path_prefix startfile_prefixes = { 0, 0, "startfile" };

/* List of prefixes to try when looking for libraries. */

static struct path_prefix library_prefixes = { 0, 0, "libraryfile" };

/* Suffix to attach to directories searched for commands.  */

static char *machine_suffix = 0;

/* Adjusted value of GCC_EXEC_PREFIX envvar.  */

static char *gcc_exec_prefix;

/* Default prefixes to attach to command names.  */

#ifdef CROSS_COMPILE  /* Don't use these prefixes for a cross compiler.  */
#undef MD_EXEC_PREFIX
#undef MD_STARTFILE_PREFIX
#undef MD_STARTFILE_PREFIX_1
#endif

#ifndef STANDARD_EXEC_PREFIX
#define STANDARD_EXEC_PREFIX "/usr/local/lib/gcc-lib/"
#endif /* !defined STANDARD_EXEC_PREFIX */

#ifndef STANDARD_EXEC_PREFIX_1
#define STANDARD_EXEC_PREFIX_1 "/usr/lib/gcc/"
#endif /* !defined STANDARD_EXEC_PREFIX */

static char *standard_exec_prefix = STANDARD_EXEC_PREFIX;
static char *standard_exec_prefix_1 = STANDARD_EXEC_PREFIX_1;
#ifdef MD_EXEC_PREFIX
static char *md_exec_prefix = MD_EXEC_PREFIX;
#endif

#ifndef STANDARD_STARTFILE_PREFIX
#define STANDARD_STARTFILE_PREFIX "/usr/local/lib/"
#endif /* !defined STANDARD_STARTFILE_PREFIX */

#ifndef STANDARD_STARTFILE_PREFIX_1
#define STANDARD_STARTFILE_PREFIX_1 "/lib/"
#endif /* !defined STANDARD_STARTFILE_PREFIX */

#ifndef STANDARD_STARTFILE_PREFIX_2
#define STANDARD_STARTFILE_PREFIX_2 "/usr/lib/"
#endif /* !defined STANDARD_STARTFILE_PREFIX */



#ifdef MD_STARTFILE_PREFIX
static char *md_startfile_prefix = MD_STARTFILE_PREFIX;
#endif
#ifdef MD_STARTFILE_PREFIX_1
static char *md_startfile_prefix_1 = MD_STARTFILE_PREFIX_1;
#endif
static char *standard_startfile_prefix   = STANDARD_STARTFILE_PREFIX;
static char *standard_startfile_prefix_1 = STANDARD_STARTFILE_PREFIX_1;
static char *standard_startfile_prefix_2 = STANDARD_STARTFILE_PREFIX_1;

/* Clear out the vector of arguments (after a command is executed).  */

static void
clear_args (void)
{
  argbuf_index = 0;
}

/* Add one argument to the vector at the end.
   This is done when a space is seen or at the end of the line.
   If DELETE_ALWAYS is nonzero, the arg is a filename
    and the file should be deleted eventually.
   If DELETE_FAILURE is nonzero, the arg is a filename
    and the file should be deleted if this compilation fails.  */

static void
store_arg (char *arg, int delete_always, int delete_failure)
{
  if (argbuf_index + 1 == argbuf_length)
    {
      argbuf = (char **) xrealloc (argbuf, (argbuf_length *= 2) * sizeof (char *));
    }

  argbuf[argbuf_index++] = arg;
  argbuf[argbuf_index] = 0;

  if (delete_always || delete_failure)
    record_temp_file (arg, delete_always, delete_failure);
}

/* Record the names of temporary files we tell compilers to write,
   and delete them at the end of the run.  */

/* This is the common prefix we use to make temp file names.
   It is chosen once for each run of this program.
   It is substituted into a spec by %g.
   Thus, all temp file names contain this prefix.
   In practice, all temp file names start with this prefix.

   This prefix comes from the envvar TMPDIR if it is defined;
   otherwise, from the P_tmpdir macro if that is defined;
   otherwise, in /usr/tmp or /tmp.  */

static char *temp_filename;

/* Length of the prefix.  */

static int temp_filename_length;

/* Define the list of temporary files to delete.  */

struct temp_file
{
  char *name;
  struct temp_file *next;
};

/* Queue of files to delete on success or failure of compilation.  */
static struct temp_file *always_delete_queue;
/* Queue of files to delete on failure of compilation.  */
static struct temp_file *failure_delete_queue;

/* Record FILENAME as a file to be deleted automatically.
   ALWAYS_DELETE nonzero means delete it if all compilation succeeds;
   otherwise delete it in any case.
   FAIL_DELETE nonzero means delete it if a compilation step fails;
   otherwise delete it in any case.  */

static void
record_temp_file (char *filename, int always_delete, int fail_delete)
{
  register char *name;
  name = xmalloc (strlen (filename) + 1);
  strcpy (name, filename);

  if (always_delete)
    {
      register struct temp_file *temp;
      for (temp = always_delete_queue; temp; temp = temp->next)
	if (! strcmp (name, temp->name))
	  goto already1;
      temp = (struct temp_file *) xmalloc (sizeof (struct temp_file));
      temp->next = always_delete_queue;
      temp->name = name;
      always_delete_queue = temp;
    already1:;
    }

  if (fail_delete)
    {
      register struct temp_file *temp;
      for (temp = failure_delete_queue; temp; temp = temp->next)
	if (! strcmp (name, temp->name))
	  goto already2;
      temp = (struct temp_file *) xmalloc (sizeof (struct temp_file));
      temp->next = failure_delete_queue;
      temp->name = name;
      failure_delete_queue = temp;
    already2:;
    }
}

/* Delete all the temporary files whose names we previously recorded.  */

static void
delete_temp_files (void)
{
  register struct temp_file *temp, *next;

  for (temp = always_delete_queue; temp; temp = next)
    {
#ifdef DEBUG
      int i;
      printf ("Delete %s? (y or n) ", temp->name);
      fflush (stdout);
      i = getchar ();
      if (i != '\n')
	while (getchar () != '\n') ;
      if (i == 'y' || i == 'Y')
#endif /* DEBUG */
	{
	  struct stat st;
	  if (stat (temp->name, &st) >= 0)
	    {
	      /* Delete only ordinary files.  */
	      if (S_ISREG (st.st_mode))
		if (unlink (temp->name) < 0)
		  if (verbose_flag)
		    perror_with_name (temp->name);
	    }
	}
      next = temp->next;
      free (temp->name);
      free (temp);
    }

  always_delete_queue = 0;
}

/* Delete all the files to be deleted on error.  */

static void
delete_failure_queue (void)
{
  register struct temp_file *temp;

  for (temp = failure_delete_queue; temp; temp = temp->next)
    {
#ifdef DEBUG
      int i;
      printf ("Delete %s? (y or n) ", temp->name);
      fflush (stdout);
      i = getchar ();
      if (i != '\n')
	while (getchar () != '\n') ;
      if (i == 'y' || i == 'Y')
#endif /* DEBUG */
	{
	  if (unlink (temp->name) < 0)
	    if (verbose_flag)
	      perror_with_name (temp->name);
	}
    }
}

static void
clear_failure_queue (void)
{
  failure_delete_queue = 0;
}

/* Compute a string to use as the base of all temporary file names.
   It is substituted for %g.  */

/* No trailing spaces allowed .. */
#if defined(MSDOS) || defined(_WIN32)
#ifdef DOSENV
#define TMPDIR_1 "C:\\TMP"
#else
#define TMPDIR_1 "C:/TMP"
#endif
#define TMPDIR_2 ""
#else
#define TMPDIR_1 "/usr/tmp"
#define TMPDIR_2 "/tmp"
#endif


static void
choose_temp_base (void)
{
  char *base = getenv ("TMP");

  int len;

  if (base == (char *)0)
    {
#if defined (P_tmpdir) && !defined (MSC)
      if (access (P_tmpdir, R_OK | W_OK) == 0)
	base = P_tmpdir;
#endif
      if (base == (char *)0)
	{
	  if (access (TMPDIR_1, R_OK | W_OK) == 0)
	    base = TMPDIR_1;
	  else
	    base = TMPDIR_2;
	}
    }
#if defined (MSDOS) && !defined (DOSENV)
  {
    char *p;
    for (p=base; *p; p++) 
      if (*p == '\\')
	*p = '/';
  }
#endif

  len = strlen (base);
  temp_filename = xmalloc (len + sizeof("/ccXXXXXX"));
  strcpy (temp_filename, base);

  if (len > 0
      && temp_filename[len-1] != '/' && temp_filename[len-1] != DIR_SEPARATOR)
    temp_filename[len++] = DIR_SEPARATOR;

  strcpy (temp_filename + len, "ccXXXXXX");

  mktemp (temp_filename);

  temp_filename_length = strlen (temp_filename);
  if (temp_filename_length == 0)
    {
#ifdef DOSENV
      fprintf(stderr,"Couldn't create temporary file\n");
#endif
      abort ();
    }
#ifdef DEBUG
  printf("temp_filename is \"%s\"\n", temp_filename);
#endif
}


/* Routine to add variables to the environment.  We do this to pass
   the pathname of the gcc driver, and the directories search to the
   collect2 program, which is being run as ld.  This way, we can be
   sure of executing the right compiler when collect2 wants to build
   constructors and destructors.  Since the environment variables we
   use come from an obstack, we don't have to worry about allocating
   space for them.  */

#ifndef HAVE_PUTENV

void
putenv (str)
     char *str;
{
#ifndef VMS                     /* nor about VMS */

  extern char **environ;
  char **old_environ = environ;
  char **envp;
  int num_envs = 0;
  int name_len = 1;
  int str_len = strlen (str);
  char *p = str;
  int ch;

  while ((ch = *p++) != '\0' && ch != '=')
    name_len++;

  if (!ch)
   {
#ifdef DOSENV
      fprintf(stderr,"Couldn't putenv\n");
#endif
    abort ();
    }

  /* Search for replacing an existing environment variable, and
     count the number of total environment variables.  */
  for (envp = old_environ; *envp; envp++)
    {
      num_envs++;
      if (!strncmp (str, *envp, name_len))
	{
	  *envp = str;
	  return;
	}
    }

  /* Add a new environment variable */
  environ = (char **) xmalloc (sizeof (char *) * (num_envs+2));
  *environ = str;
  bcopy (old_environ, environ+1, sizeof (char *) * (num_envs+1));

#endif  /* VMS */
}

#endif  /* HAVE_PUTENV */


/* Rebuild the COMPILER_PATH and LIBRARY_PATH environment variables for collect.  */

static void
putenv_from_prefixes (struct path_prefix *paths, char *env_var)
{
  int suffix_len = (machine_suffix) ? strlen (machine_suffix) : 0;
  int first_time = TRUE;
  struct prefix_list *pprefix;

  obstack_grow (&collect_obstack, env_var, strlen (env_var));

  for (pprefix = paths->plist; pprefix != 0; pprefix = pprefix->next)
    {
      int len = strlen (pprefix->prefix);

      if (machine_suffix)
	{
	  if (!first_time)
	    obstack_grow (&collect_obstack, ":", 1);
		
	  first_time = FALSE;
	  obstack_grow (&collect_obstack, pprefix->prefix, len);
	  obstack_grow (&collect_obstack, machine_suffix, suffix_len);
	}

      if (!pprefix->require_machine_suffix)
	{
	  if (!first_time)
	    obstack_grow (&collect_obstack, ":", 1);

	  first_time = FALSE;
	  obstack_grow (&collect_obstack, pprefix->prefix, len);
	}
    }
  obstack_grow (&collect_obstack, "\0", 1);
  putenv (obstack_finish (&collect_obstack));
}


/* Search for NAME using the prefix list PREFIXES.  MODE is passed to
   access to check permissions.
   Return 0 if not found, otherwise return its name, allocated with malloc. */

static char *
find_a_file (struct path_prefix *pprefix, char *name, int mode)
{
  char *temp;
  char *file_suffix = ((mode & X_OK) != 0 ? EXECUTABLE_SUFFIX : "");
  struct prefix_list *pl;
  int len = pprefix->max_len + strlen (name) + strlen (file_suffix) + 1;

  if (machine_suffix)
    len += strlen (machine_suffix);

  temp = xmalloc (len);

  /* Determine the filename to execute (special case for absolute paths).  */

  if (debug_flag)
    printf ("find_a_file: %s %s  name\n", name, file_suffix);

  if (*name == '/' || *name == DIR_SEPARATOR)
    {
      if (access (name, mode))
	{
	  strcpy (temp, name);
	  return temp;
	}
    }
  else
    for (pl = pprefix->plist; pl; pl = pl->next)
      {
	if (debug_flag)
	  printf ("prefixing::  %s%s%s\n", 
		  pl->prefix, machine_suffix ? machine_suffix : "", name);
	if (machine_suffix)
	  {
	    strcpy (temp, pl->prefix);
	    strcat (temp, machine_suffix);
	    strcat (temp, name);
	    if (access (temp, mode) == 0)
	      {
		if (pl->used_flag_ptr != 0)
		  *pl->used_flag_ptr = 1;
		return temp;
	      }
	    /* Some systems have a suffix for executable files.
	       So try appending that.  */

	    if (file_suffix[0] != 0)
	      {
		if (debug_flag)
		  printf ("prefixing::  %s%s%s%s\n", pl->prefix,
			  machine_suffix ? machine_suffix : "",
			  name, file_suffix);

		strcat (temp, file_suffix);
		if (access (temp, mode) == 0)
		  {
		    if (pl->used_flag_ptr != 0)
		      *pl->used_flag_ptr = 1;
		    return temp;
		  }
	      }
	  }
	/* Certain prefixes can't be used without the machine suffix
	   when the machine or version is explicitly specified.  */
	if (!pl->require_machine_suffix)
	  {
	    strcpy (temp, pl->prefix);
	    strcat (temp, name);
	    if (access (temp, mode) == 0)
	      {
		if (pl->used_flag_ptr != 0)
		  *pl->used_flag_ptr = 1;
		return temp;
	      }
	    /* Some systems have a suffix for executable files.
	       So try appending that.  */
	    if (file_suffix[0] != 0)
	      {
		strcat (temp, file_suffix);
		if (access (temp, mode) == 0)
		  {
		    if (pl->used_flag_ptr != 0)
		      *pl->used_flag_ptr = 1;
		    return temp;
		  }
	      }
	  }
      }

  free (temp);
  return 0;
}

/* Add an entry for PREFIX in PLIST.  If FIRST is set, it goes
   at the start of the list, otherwise it goes at the end.

   If WARN is nonzero, we will warn if no file is found
   through this prefix.  WARN should point to an int
   which will be set to 1 if this entry is used.  */

static void
add_prefix (struct path_prefix *pprefix, char *prefix, int first, int require_machine_suffix, int *warn)
{
  struct prefix_list *pl, **prev;
  int len;

  if (prefix==0)
    return;

  if (!first && pprefix->plist)
    {
      for (pl = pprefix->plist; pl->next; pl = pl->next)
	;
      prev = &pl->next;
    }
  else
    prev = &pprefix->plist;

  /* Keep track of the longest prefix */

  len = strlen (prefix);
  if (len > pprefix->max_len)
    pprefix->max_len = len;

  pl = (struct prefix_list *) xmalloc (sizeof (struct prefix_list));
  pl->prefix = save_string (prefix, len);
  pl->require_machine_suffix = require_machine_suffix;
  pl->used_flag_ptr = warn;
  if (warn)
    *warn = 0;

  if (*prev)
    pl->next = *prev;
  else
    pl->next = (struct prefix_list *) 0;
  *prev = pl;
}

/* Print warnings for any prefixes in the list PPREFIX that were not used.  */

static void
unused_prefix_warnings (struct path_prefix *pprefix)
{
  struct prefix_list *pl = pprefix->plist;

  while (pl)
    {
      if (pl->used_flag_ptr != 0 && !*pl->used_flag_ptr)
	{
	  error ("file path prefix `%s' never used",
		 pl->prefix);
	  /* Prevent duplicate warnings.  */
	  *pl->used_flag_ptr = 1;
	}
      pl = pl->next;
    }
}

/* stdin file number.  */
#define STDIN_FILE_NO 0

/* stdout file number.  */
#define STDOUT_FILE_NO 1

/* value of `pipe': port index for reading.  */
#define READ_PORT 0

/* value of `pipe': port index for writing.  */
#define WRITE_PORT 1

/* Pipe waiting from last process, to be used as input for the next one.
   Value is STDIN_FILE_NO if no pipe is waiting
   (i.e. the next command is the first of a group).  */

static int last_pipe_input;

/* Fork one piped subcommand.  FUNC is the system call to use
   (either execv or execvp).  ARGV is the arg vector to use.
   NOT_LAST is nonzero if this is not the last subcommand
   (i.e. its output should be piped to the next one.)  */

#ifdef __MSDOS__

#ifdef DOSENV
#include <process.h>

static int
pexecute (search_flag, program, argv, not_last)
     int search_flag;
     char *program;
     char *argv[];
     int not_last;
{
  char *scmd[3];
  FILE *argfile;
  int i;
  int (*func)() = (search_flag ? spawnv : spawnvp);

  scmd[0] = strrchr(program,'\\');
  scmd[0] = scmd[0]?scmd[0]:program;

  scmd[1] = (char *)alloca (strlen (temp_filename) + 6);
  sprintf (scmd[1], "@%s.gp", temp_filename);
  argfile = fopen (scmd[1] + 1, "w");
  if (argfile == 0)
    pfatal_with_name (scmd[1] + 1);

  for (i=1; argv[i]; i++)
  {
    char *cp;
    for (cp = argv[i]; *cp; cp++)
      {
	fputc (*cp, argfile);
      }
    fputc ('\n', argfile);
  }
  fclose (argfile);

  scmd[2]= NULL;

#ifdef DOS_MEM_STATS
  if (debug_flag)
    printf("Free memory at spawn: %u  stack: %u\n", _memavl(), stackavail());
#endif

  i=(*func) (P_WAIT, program, scmd, NULL);
  if (i == -1)
    pfatal_with_name ("spawn");
	
  remove (scmd[1] + 1);
  return i << 8;
}

#else /* not DOSENV */

static int
pexecute (int search_flag, char *program, char **argv, int not_last)
{
  char *scmd;
  FILE *argfile;
  int i;
  char *tfp; /* ptr to temp filename in command string, scmd. */

/* Number of extra char in scmd: ' ', '@', ".gp", '\0' */
#define EXTRA_CHARS 6
	
  scmd = xmalloc (strlen (program) + strlen (temp_filename) + EXTRA_CHARS);
  sprintf (scmd, "%s @%s.gp", program, temp_filename);

  tfp = scmd + strlen (program) + 2;
  argfile = fopen (tfp, "w");

  if (argfile == 0)
    pfatal_with_name (tfp);

  for (i=1; argv[i]; i++)
  {
    char *cp;


    for (cp = argv[i]; *cp; cp++)
      {
	if (*cp == '"' || *cp == '\'' )
	  fputc ('\\', argfile);
	fputc (*cp, argfile);
      }

    fputc ('\n', argfile);
  }
  fclose (argfile);

  i = system (scmd);
  remove (tfp);

  return i << 8;
}

#endif /* not DOSENV */
#endif /* __MSDOS__ */

#if !defined(__MSDOS__) && !defined(OS2) && !defined(_WIN32)

#if !defined(sun) && !defined (SVR4) /* Who is this for? */
extern int execv (const char *a, const char **b), execvp (const char *a, const char **b);
#endif

static int
pexecute (search_flag, program, argv, not_last)
     int search_flag;
     char *program;
     char *argv[];
     int not_last;
{
  int (*func)() = (search_flag ? execv : execvp);
  int pid;
  int pdes[2];
  int input_desc = last_pipe_input;
  int output_desc = STDOUT_FILE_NO;
  int retries, sleep_interval;

  /* If this isn't the last process, make a pipe for its output,
     and record it as waiting to be the input to the next process.  */

  if (not_last)
    {
      if (pipe (pdes) < 0)
	pfatal_with_name ("pipe");
      output_desc = pdes[WRITE_PORT];
      last_pipe_input = pdes[READ_PORT];
    }
  else
    last_pipe_input = STDIN_FILE_NO;

  /* Fork a subprocess; wait and retry if it fails.  */
  sleep_interval = 1;
  for (retries = 0; retries < 4; retries++)
    {
      pid = vfork ();
      if (pid >= 0)
	break;
      sleep (sleep_interval);
      sleep_interval *= 2;
    }

  switch (pid)
    {
    case -1:
#ifdef vfork
      pfatal_with_name ("fork");
#else
      pfatal_with_name ("vfork");
#endif
      /* NOTREACHED */
      return 0;

    case 0: /* child */
      /* Move the input and output pipes into place, if nec.  */
      if (input_desc != STDIN_FILE_NO)
	{
	  close (STDIN_FILE_NO);
	  dup (input_desc);
	  close (input_desc);
	}
      if (output_desc != STDOUT_FILE_NO)
	{
	  close (STDOUT_FILE_NO);
	  dup (output_desc);
	  close (output_desc);
	}

      /* Close the parent's descs that aren't wanted here.  */
      if (last_pipe_input != STDIN_FILE_NO)
	close (last_pipe_input);

      /* Exec the program.  */
      (*func) (program, argv);
      perror_exec (program);
      exit (-1);
      /* NOTREACHED */
      return 0;

    default:
      /* In the parent, after forking.
	 Close the descriptors that we made for this child.  */
      if (input_desc != STDIN_FILE_NO)
	close (input_desc);
      if (output_desc != STDOUT_FILE_NO)
	close (output_desc);

      /* Return child's process number.  */
      return pid;
    }
}

#endif /* not __MSDOS__ and not OS2 and not _WIN32 */

#if defined(OS2)

static int
pexecute (search_flag, program, argv, not_last)
     int search_flag;
     char *program;
     char *argv[];
     int not_last;
{
  return (search_flag ? spawnv : spawnvp) (1, program, argv);
}
#endif /* OS2 */

#if defined(_WIN32)

#include <process.h>

static int
pexecute (search_flag, program, argv, not_last)
     int search_flag;
     char *program;
     char *argv[];
     int not_last;
{
  int i;
  char *s, *t;
  int num_extra, num_backslash, quotes_needed;

  for (i = 1; argv[i]; i++)
    {
      /* Backslashes have no special meaning except before a double-quote.
	 Each backslash in a continuous string before a double-quote
	 must be doubled, and the double-quote must also be backslashed.
	   
	 Some examples:
		Command argv            Command argv
		\a      \a              \"      "
		\\a     \\a             \\" x"  \ x
		\\\a    \\\a            \\\"    \"
		\\\\a   \\\\a           \\\\""  \\
		\\\\\a  \\\\\a          \\\\\"  \\"
       */
      num_extra = num_backslash = quotes_needed = 0;
      for (s = argv[i]; *s; s++)
	if (*s == '\\')
	  num_backslash++;
	else
	  {
	    if (*s == ' ' || *s == '\t'
		|| *s == '*' || *s == '?' || *s == '[' || *s == ']')
	      quotes_needed = 2;
	    if (*s == '"')
	      num_extra += num_backslash + 1;
	    num_backslash = 0;
	  }
      if (!(num_extra + quotes_needed))
	continue;

      /* If we are going to quote the whole thing, and it ends with
	 backslashes, they need to be doubled. */
      if (quotes_needed)
	num_extra += num_backslash;

      t = alloca(s - argv[i] + num_extra + quotes_needed + 1);
      s = argv[i];
      argv[i] = t;
      if (quotes_needed)
	*t++ = '"';
      while (*s)
	{
	  if (*s == '"')
	    /* Write out as many backslashes as we just saw, plus one
	       more to escape the quote. */
	    while (num_backslash-- >= -1)
	      *t++ = '\\';
	  if (*s == '\\')
	    num_backslash++;
	  else
	    num_backslash = 0;
	  *t++ = *s++;
	}
      if (quotes_needed)
	{
	  /* Double any trailing backslashes. */
	  while (num_backslash--)
	    *t++ = '\\';
	  *t++ = '"';
	}
    }

  return (search_flag ? _spawnv : _spawnvp) (1, program, argv);
}
#endif /* _WIN32 */



/* Execute the command specified by the arguments on the current line of spec.
   When using pipes, this includes several piped-together commands
   with `|' between them.

   Return 0 if successful, -1 if failed.  */

static int
execute (void)
{
  int i;
  int n_commands;               /* # of command.  */
  char *string;
  struct command
    {
      char *prog;               /* program name.  */
      char **argv;              /* vector of args.  */
      int pid;                  /* pid of process for this command.  */
    };
  int status;

  struct command *commands;     /* each command buffer with above info.  */

  /* Count # of piped commands.  */
  for (n_commands = 1, i = 0; i < argbuf_index; i++)
    if (strcmp (argbuf[i], "|") == 0)
      n_commands++;

  /* Get storage for each command.  */
  commands
    = (struct command *) alloca (n_commands * sizeof (struct command));

  /* Split argbuf into its separate piped processes,
     and record info about each one.
     Also search for the programs that are to be run.  */

  commands[0].prog = argbuf[0]; /* first command.  */
  commands[0].argv = &argbuf[0];
  string = find_a_file (&exec_prefixes, commands[0].prog, X_OK);
  if (string)
    commands[0].argv[0] = string;

  for (n_commands = 1, i = 0; i < argbuf_index; i++)
    if (strcmp (argbuf[i], "|") == 0)
      {                         /* each command.  */
#ifdef __MSDOS__
	fatal ("-pipe not supported under MS-DOS");
#endif
	argbuf[i] = 0;  /* termination of command args.  */
	commands[n_commands].prog = argbuf[i + 1];
	commands[n_commands].argv = &argbuf[i + 1];
	string = find_a_file (&exec_prefixes, commands[n_commands].prog, X_OK);
	if (string)
	  commands[n_commands].argv[0] = string;
	n_commands++;
      }

  argbuf[argbuf_index] = 0;

  /* If -v, print what we are about to do, and maybe query.  */

  if (verbose_flag)
    {
/**/      fflush (stdout);
      /* Print each piped command as a separate line.  */
      for (i = 0; i < n_commands ; i++)
	{
	  char **j;

	  for (j = commands[i].argv; *j; j++)
	    fprintf (stderr, " %s", *j);

	  /* Print a pipe symbol after all but the last command.  */
	  if (i + 1 != n_commands)
	    fprintf (stderr, " |");
	  fprintf (stderr, "\n");
	}
      fflush (stderr);
#ifdef DEBUG
      fprintf (stderr, "\nGo ahead? (y or n) ");
      fflush (stderr);
      i = getchar ();
      if (i != '\n')
	while (getchar () != '\n') ;
      if (i != 'y' && i != 'Y')
	return 0;
#endif /* DEBUG */
    }

  /* Run each piped subprocess.  */

  last_pipe_input = STDIN_FILE_NO;
  for (i = 0; i < n_commands; i++)
    {
      char *string = commands[i].argv[0];

      commands[i].pid = pexecute (string != commands[i].prog,
				  string, commands[i].argv,
				  i + 1 < n_commands);

      if (string != commands[i].prog)
	free (string);
    }

  execution_count++;

  /* Wait for all the subprocesses to finish.
     We don't care what order they finish in;
     we know that N_COMMANDS waits will get them all.  */

  {
    int ret_code = 0;

    for (i = 0; i < n_commands; i++)
      {
	int pid;
	char *prog;

#ifdef __MSDOS__
	status = pid = commands[i].pid;
#else
#ifdef _WIN32
	pid = cwait (&status, commands[i].pid, 0);
	status <<= 8;
#else
	pid = wait (&status);
#endif
#endif
	if (pid < 0)
	  {
#ifdef DOSENV
	    fprintf(stdout,"File %s did not execute!\n", commands[i].prog);
#endif
	  abort ();
	  }

	if (status != 0)
	  {
	    int j;
	    for (j = 0; j < n_commands; j++)
	      if (commands[j].pid == pid)
		prog = commands[j].prog;

	    if ((status & 0x7F) != 0)
	      fatal ("Internal compiler error: program %s got fatal signal %d",
		     prog, (status & 0x7F));
	    if (((status & 0xFF00) >> 8) >= MIN_FATAL_STATUS)
	      ret_code = -1;
	  }
      }
    return ret_code;
  }
}

/* Find all the switches given to us
   and make a vector describing them.
   The elements of the vector are strings, one per switch given.
   If a switch uses following arguments, then the `part1' field
   is the switch itself and the `args' field
   is a null-terminated vector containing the following arguments.
   The `live_cond' field is 1 if the switch is true in a conditional spec,
   -1 if false (overridden by a later switch), and is initialized to zero.
   The `valid' field is nonzero if any spec has looked at this switch;
   if it remains zero at the end of the run, it must be meaningless.  */

struct switchstr
{
  char *part1;
  char **args;
  int live_cond;
  int valid;
};

static struct switchstr *switches;

static int n_switches;

struct infile
{
  char *name;
  char *language;
};

/* Also a vector of input files specified.  */

static struct infile *infiles;

static int n_infiles;

/* And a vector of corresponding output files is made up later.  */

static char **outfiles;

/* Create the vector `switches' and its contents.
   Store its length in `n_switches'.  */
#if defined (WARN_SWITCH)
static char *warn_switch[] = WARN_SWITCH;
#endif

#if defined (ADI) && (defined (A21C0) || defined (DSP21XX)) /*EK*/
static char *help_lines[] = 
{
    "-E                preprocess source files only",
    "-S                generate assembly source files only",
    "-c                generate assembly object files only",
    "-O2               optimize code using many optimizations",
    "-v                generate verbose output",
    "-mlistm           generate a merge listing file",
    "-mreserved=register-list not to use specified registers",
    "-mno-doloops      not to generate assembly do-loops",
    "-mno-csp01        no nop between DAG reg load and its use in dm pm mode",
    "-ansi             disable all non-ansi extension to the language",
    "-g                produce debuggable code for use with CBUG",
    "-a filename       specify alternate architecture file.",
    "-Ipath            specify addition paths to search for include files",
    "-Dmacro[=expansion]   define a macro for the preprocessor",
    "-Lpath            specify an additional path to search for library files",
    "-lxxx             include library libxxx.a in link line",
    "-Xlinker option   pass option to the linker",
    "-o filename       place output in filename",
    "-save-temps       save all temporary files ",
    "-w                inhibit all warning messages",
    "-Wall             combine all warnings in this list",
    "-Wimplicit        warn when a function is implicitly declared",
    "-Wfloat-convert   warn when a float number is converted to a double",
    "-Wmissing-prototypes warn if missing prototypes",
};
#elif defined (CRIPPLE)
static char *help_lines[] = 
{
    "-E                preprocess source files only",
    "-S                generate assembly source files only",
    "-c                generate assembly object files only",
    "-O2               optimize code using many optimizations",
    "-g                produce debuggable code for use with CBUG",
    "-Ipath            specify addition paths to search for include files",
    "-Dmacro[=expansion]   define a macro for the preprocessor",
    "-Lpath            specify an additional path to search for library files",
    "-lxxx             include library libxxx.a in link line",
    "-o filename       place output in filename",
};
#else
static char *help_lines[] = 
{
    "-E                preprocess source files only",
    "-S                generate assembly source files only",
    "-SO               generate preprocessed compacted assembly only",
    "-c                generate assembly object files only",
    "-O                optimize code using some optimizations",
    "-O2               optimize code using many optimizations",
    "-O3               optimize using the global code optimizer (compactor)",
    "-O4               application specific optimizations",
    "-CO               run the global code optimizer (does not include -O/-O2)",
    "-fno-short-double use 64bit doubles",
    "-v                generate verbose output",
    "-mlistm           generate a merge listing file.",
    "-mkeep-i13        load i13 with return address for compatibility.",
    "                  with old runtime model (21010 21020 only)",
    "-ansi             disable all non-ansi extension to the language",
    "-g                produce debuggable code for use with CBUG",
    "-a filename       specify alternate architecture file.",
    "-Ipath            specify addition paths to search for include files",
    "-Dmacro[=expansion]   define a macro for the preprocessor",
    "-Lpath            specify an additional path to search for library files",
    "-lxxx             include library libxxx.a in link line",
    "-o filename       place output in filename",
    "-save-temps       save all temporary files ",
    "-w                inhibit all warning messages",
    "-Wall             combine all warnings in this list",
    "-Wimplicit        warn when a function is implicitly declared",
    "-Wfloat-convert   warn when a float number is converted to a double",
    "-Wmissing-prototypes warn if missing prototypes",
};
#endif


static void
process_command (int argc, char **argv)
{
  register int i;
  char *temp;
  char *spec_lang = 0;
  int last_language_n_infiles;

  n_switches = 0;
  n_infiles = 0;
  spec_version = version_string;

  /* Set up the default search paths.  */

#ifdef USE_ADI_DSP
  temp = getenv ("ADI_DSP");

#ifdef ADI_DSP
  if (!temp) {
      /*EK* let us assume a reasonable default */
      temp = ADI_DSP;
  } else if (!*temp) {
      /*EK* let setting of ADI_DSP to "" mean something */
      temp = 0;
  }
#endif
  if (!temp) {
	fatal ("Must set ADI_DSP environment variable to root of ADI tools "
	   "directory.");
  }
    {  
      char *adi_dsp = temp;
      int len = strlen (adi_dsp) + 1;
      char *libp = (char *) alloca (len + sizeof LIB_LOCATION);
      char *execp = (char *) alloca (len + sizeof CC1_LOCATION);
      char *incp = xmalloc (len + sizeof INCLUDE_FILES_LOCATION);
      char *s1;

      if (debug_flag)
	printf("ADI_DSP: %s\n", adi_dsp);

  #ifdef CONVERT_BACKSLASH_TO_FORWARDSLASH
      adi_dsp = strcpy (alloca (len), adi_dsp);
      for (s1 = adi_dsp; *s1; s1++)
	if (*s1 == '\\')
	  *s1 = '/';

      if (debug_flag)
	printf ("Adjusted ADI_DSP: %s\n", adi_dsp);
#endif
#ifdef CONVERT_FORWARDSLASH_TO_BACKSLASH
      adi_dsp = strcpy (alloca (len), adi_dsp);
      for (s1 = adi_dsp; *s1; s1++)
	if (*s1 == '/')
	  *s1 = '\\';
    
      if (debug_flag)
	printf ("Adjusted ADI_DSP: %s\n", adi_dsp);
  #endif

      strcpy (libp, adi_dsp);
      strcpy (execp,adi_dsp);
      strcpy (incp, adi_dsp);

      if (adi_dsp[len - 2] != '/' && adi_dsp[len - 2] != DIR_SEPARATOR)
	{
	  strcat (libp, dir_separator_str);
	  strcat (execp, dir_separator_str);
	  strcat (incp, dir_separator_str);
	}
	
      strcat(libp, LIB_LOCATION);
      strcat(execp, CC1_LOCATION);
      strcat (incp, INCLUDE_FILES_LOCATION);
      cpp_default_include = incp;
	
      add_prefix (&exec_prefixes, execp, 0, 0, 0);
      add_prefix (&startfile_prefixes, libp, 0, 0, 0);
/* This ifdef is inserted because of VDSP Linker bug with duplicate -L */
#if !defined (ADI) || !defined (A21C0) || !defined (DSP21XX)
      add_prefix (&library_prefixes, libp, 0, 0, 0);
#endif
    }
#else /* !USE_ADI_DSP */

  gcc_exec_prefix = getenv ("GCC_EXEC_PREFIX");


  if (gcc_exec_prefix)
    {
      add_prefix (&exec_prefixes, gcc_exec_prefix, 0, 0, NULL_PTR);
      add_prefix (&startfile_prefixes, gcc_exec_prefix, 0, 0, NULL_PTR);
    }

  /* COMPILER_PATH and LIBRARY_PATH have values
     that are lists of directory names with colons.  */

  temp = getenv ("COMPILER_PATH");
  if (temp)
    {
      char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp-startp);
	      if (endp == startp)
		{
		  *nstore = '.';
		  strcat (nstore + 1, dir_separator_str);
		}
	      else if (endp[-1] != '/' && endp[-1] != DIR_SEPARATOR)
		{
		  nstore[endp-startp] = DIR_SEPARATOR;
		  nstore[endp-startp+1] = 0;
		}
	      else
		nstore[endp-startp] = 0;
	      add_prefix (&exec_prefixes, nstore, 0, 0, NULL_PTR);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

  temp = getenv ("LIBRARY_PATH");
  if (temp)
    {
      char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp-startp);
	      if (endp == startp)
		{
		  *nstore = '.';
		  strcat (nstore + 1, dir_separator_str);
		}
	      else if (endp[-1] != '/' && endp[-1] != DIR_SEPARATOR)
		{
		  nstore[endp-startp] = DIR_SEPARATOR;
		  nstore[endp-startp+1] = 0;
		}
	      else
		nstore[endp-startp] = 0;
	      add_prefix (&startfile_prefixes, nstore, 0, 0, NULL_PTR);
	      /* Make separate list of dirs that came from LIBRARY_PATH.  */
	      add_prefix (&library_prefixes, nstore, 0, 0, NULL_PTR);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

  /* Use LPATH like LIBRARY_PATH (for the CMU build program).  */
  temp = getenv ("LPATH");
  if (temp)
    {
      char *startp, *endp;
      char *nstore = (char *) alloca (strlen (temp) + 3);

      startp = endp = temp;
      while (1)
	{
	  if (*endp == PATH_SEPARATOR || *endp == 0)
	    {
	      strncpy (nstore, startp, endp-startp);
	      if (endp == startp)
		{
#ifdef DOSENV
		  strcpy (nstore, ".\\");
#else
		  strcpy (nstore, "./");
#endif
		}
	      else if (endp[-1] != DIR_SEPARATOR)
		{
		  nstore[endp-startp] = DIR_SEPARATOR;
		  nstore[endp-startp+1] = 0;
		}
	      else
		nstore[endp-startp] = 0;
	      add_prefix (&startfile_prefixes, nstore, 0, 0, NULL_PTR);
	      /* Make separate list of dirs that came from LIBRARY_PATH.  */
	      add_prefix (&library_prefixes, nstore, 0, 0, NULL_PTR);
	      if (*endp == 0)
		break;
	      endp = startp = endp + 1;
	    }
	  else
	    endp++;
	}
    }

#endif /* !USE_ADI_DSP */

  /* Scan argv twice.  Here, the first time, just count how many switches
     there will be in their vector, and how many input files in theirs.
     Here we also parse the switches that cc itself uses (e.g. -v).  */

  for (i = 1; i < argc; i++)
    {
#if defined (WARN_SWITCH)
      {
	int j;
	for(j=0;warn_switch[j];j++)
	  {
	    if (strcmp (warn_switch[j],argv[i])==0)
	      error("Warning: %s is an unsupported switch", warn_switch[j]);
	  }
      }
#endif

#ifdef ADI
      if ((! strcmp (argv[i], "-A"))|| (! strcmp (argv[i], "-a")))
	argv[i] = (char *)"-arch";       /* -a was -arch for g21 */
#endif

      if (! strcmp (argv[i], "-dumpspecs"))
	{
	  printf ("*asm:\n%s\n\n", asm_spec);
	  printf ("*asm_final:\n%s\n\n", asm_final_spec);
	  printf ("*cpp:\n%s\n\n", cpp_spec);
	  printf ("*cc1:\n%s\n\n", cc1_spec);
	  printf ("*cc1plus:\n%s\n\n", cc1plus_spec);
	  printf ("*endfile:\n%s\n\n", endfile_spec);
	  printf ("*link:\n%s\n\n", link_spec);
	  printf ("*lib:\n%s\n\n", lib_spec);
	  printf ("*startfile:\n%s\n\n", startfile_spec);
	  printf ("*switches_need_spaces:\n%s\n\n", switches_need_spaces);
	  printf ("*signed_char:\n%s\n\n", signed_char_spec);
	  printf ("*predefines:\n%s\n\n", cpp_predefines);
	  printf ("*cross_compile:\n%d\n\n", cross_compile);

	  exit (0);
	}
      else if (! strcmp (argv[i], "-dumpversion"))
	{
	  printf ("%s\n", version_string);
	  exit (0);
	}
      else if (! strcmp (argv[i], "-print-libgcc-file-name"))
	{
	  print_libgcc_file_name = 1;
	}
#ifdef ADI
      else if (! strcmp (argv[i], "-help") ||
	       ! strcmp (argv[i], "-Help") ||
	       ! strcmp (argv[i], "-HELP") ||
	       ! strcmp (argv[i], "-h") ||
	       ! strcmp (argv[i], "-?") ||
	       ! strcmp (argv[i], "-H"))
      {
	  char **hline;
	  char **hlines_end =
	      help_lines + sizeof (help_lines)/ sizeof(*help_lines);
	  printf ("Usage: %s <flags>", argv[0]);
	  for (hline = help_lines; hline < hlines_end; hline++)
	  {
	      putchar ('\n');
	      printf (*hline);
	  }
#ifndef __MSDOS__
	  putchar ('\n');
#endif
	  exit(0);
      }

#endif
      else if (! strncmp (argv[i], "-Wa,", 4))
	{
	  int prev, j;
	  /* Pass the rest of this option to the assembler.  */

	  n_assembler_options++;
	  if (!assembler_options)
	    assembler_options
	      = (char **) xmalloc (n_assembler_options * sizeof (char **));
	  else
	    assembler_options
	      = (char **) xrealloc (assembler_options,
				    n_assembler_options * sizeof (char **));

	  /* Split the argument at commas.  */
	  prev = 4;
	  for (j = 4; argv[i][j]; j++)
	    if (argv[i][j] == ',')
	      {
		assembler_options[n_assembler_options - 1]
		  = save_string (argv[i] + prev, j - prev);
		n_assembler_options++;
		assembler_options
		  = (char **) xrealloc (assembler_options,
					n_assembler_options * sizeof (char **));
		prev = j + 1;
	      }
	  /* Record the part after the last comma.  */
	  assembler_options[n_assembler_options - 1] = argv[i] + prev;
	}
      else if (! strncmp (argv[i], "-Wp,", 4))
	{
	  int prev, j;
	  /* Pass the rest of this option to the preprocessor.  */

	  n_preprocessor_options++;
	  if (!preprocessor_options)
	    preprocessor_options
	      = (char **) xmalloc (n_preprocessor_options * sizeof (char **));
	  else
	    preprocessor_options
	      = (char **) xrealloc (preprocessor_options,
				    n_preprocessor_options * sizeof (char **));

	  /* Split the argument at commas.  */
	  prev = 4;
	  for (j = 4; argv[i][j]; j++)
	    if (argv[i][j] == ',')
	      {
		preprocessor_options[n_preprocessor_options - 1]
		  = save_string (argv[i] + prev, j - prev);
		n_preprocessor_options++;
		preprocessor_options
		  = (char **) xrealloc (preprocessor_options,
					n_preprocessor_options * sizeof (char **));
		prev = j + 1;
	      }
	  /* Record the part after the last comma.  */
	  preprocessor_options[n_preprocessor_options - 1] = argv[i] + prev;
	}
#ifdef DSP21K
      else if (! strncmp (argv[i], "-Wo,", 4))
	{
	  int prev, j;
	  /* Pass the rest of this option to the compactor.  */

	  n_compactor_options++;
	  if (!compactor_options)
	    compactor_options
	      = (char **) xmalloc (n_compactor_options * sizeof (char **));
	  else
	    compactor_options
	      = (char **) xrealloc (compactor_options,
				    n_compactor_options * sizeof (char **));

	  /* Split the argument at commas.  */
	  prev = 4;
	  for (j = 4; argv[i][j]; j++)
	    if (argv[i][j] == ',')
	      {
		compactor_options[n_compactor_options - 1]
		  = save_string (argv[i] + prev, j - prev);
		n_compactor_options++;
		compactor_options
		  = (char **) xrealloc (compactor_options,
					n_compactor_options * sizeof (char **));
		prev = j + 1;
	      }
	  /* Record the part after the last comma.  */
	  compactor_options[n_compactor_options - 1] = argv[i] + prev;
	}
#endif
      else if (strncmp (argv[i], "-Wl,", 4) == 0)
	{
	  int prev, j;
	  /* Pass the rest of this option to the linker.  */

	  n_linker_options++;
	  if (!linker_options)
	    linker_options
	      = (char **) xmalloc (n_linker_options * sizeof (char **));
	  else
	    linker_options
	      = (char **) xrealloc (linker_options,
				    n_linker_options * sizeof (char **));

	  /* Split the argument at commas.  */
	  prev = 4;
	  for (j = 4; argv[i][j]; j++)
	    if (argv[i][j] == ',')
	      {
		linker_options[n_linker_options - 1]
		  = save_string (argv[i] + prev, j - prev);
		n_linker_options++;
		linker_options
		  = (char **) xrealloc (linker_options,
					n_linker_options * sizeof (char **));
		prev = j + 1;
	      }
	  /* Record the part after the last comma.  */
	  linker_options[n_linker_options - 1] = argv[i] + prev;
	}
      else if (! strcmp (argv[i], "-Xlinker"))
	{
	  /* Pass the argument of this option to the linker when we link.  */

	  if (i + 1 == argc)
	    fatal ("argument to `-Xlinker' is missing");

	  n_linker_options++;
	  if (!linker_options)
	    linker_options
	      = (char **) xmalloc (n_linker_options * sizeof (char **));
	  else
	    linker_options
	      = (char **) xrealloc (linker_options,
				    n_linker_options * sizeof (char **));

	  linker_options[n_linker_options - 1] = argv[++i];
	}
      else if (argv[i][0] == '-' && argv[i][1] != 0 && argv[i][1] != 'l')
	{
	  register char *p = &argv[i][1];
	  register int c = *p;

	  switch (c)
	    {
	    case 'b':
	      if (p[1] == 0 && i + 1 == argc)
		fatal ("argument to `-b' is missing");
	      if (p[1] == 0)
		spec_machine = argv[++i];
	      else
		spec_machine = p + 1;
	      break;

	    case 'B':
	      {
		int *temp = (int *) xmalloc (sizeof (int));
		char *value;
		if (p[1] == 0 && i + 1 == argc)
		  fatal ("argument to `-B' is missing");
		if (p[1] == 0)
		  value = argv[++i];
		else
		  value = p + 1;
		add_prefix (&exec_prefixes, value, 1, 0, temp);
		add_prefix (&startfile_prefixes, value, 1, 0, temp);
	      }
	      break;

	    case 'v':   /* Print our subcommands and print versions.  */
	      verbose_flag++;
	      n_switches++;
	      break;

	    case 'Z':   /* Print our subcommands and print versions.  */
	      debug_flag++;
	      break;

	    case 'V':
	      if (p[1] == 0 && i + 1 == argc)
		fatal ("argument to `-V' is missing");
	      if (p[1] == 0)
		spec_version = argv[++i];
	      else
		spec_version = p + 1;
	      break;

	    case 's':
	      if (!strcmp (p, "save-temps"))
		{
		  save_temps_flag = 1;
		  n_switches++;
		  break;
		}

	    default:
	      {
		n_switches++;

		/* print out warning for unrecognized switches */

		if (is_bad_switch (p))
		  error ("Warning: %s is an unsupported switch", p);

		if (SWITCH_TAKES_ARG (c) > (p[1] != 0))
		  i += SWITCH_TAKES_ARG (c) - (p[1] != 0);
		else if (WORD_SWITCH_TAKES_ARG (p))
		  i += WORD_SWITCH_TAKES_ARG (p);
	      }
	    }
	}
      else
	n_infiles++;
    }

  /* Set up the search paths before we go looking for config files.  */

  /* These come before the md prefixes so that we will find gcc's subcommands
     (such as cpp) rather than those of the host system.  */
  add_prefix (&exec_prefixes, standard_exec_prefix, 0, 2, NULL_PTR);
  add_prefix (&exec_prefixes, standard_exec_prefix_1, 0, 2, NULL_PTR);

  add_prefix (&startfile_prefixes, standard_exec_prefix, 0, 1, NULL_PTR);
  add_prefix (&startfile_prefixes, standard_exec_prefix_1, 0, 1, NULL_PTR);

  /* More prefixes are enabled in main, after we read the specs file
     and determine whether this is cross-compilation or not.  */


#ifdef DSP21K
  /* Leave space for a -CO at -O>2. */
  n_switches++;
#endif

  /* Then create the space for the vectors and scan again.  */

  switches = ((struct switchstr *)
	      xmalloc ((n_switches + 1) * sizeof (struct switchstr)));
  infiles = (struct infile *) xmalloc ((n_infiles + 1) * sizeof (struct infile));
  n_switches = 0;
  n_infiles = 0;
  last_language_n_infiles = -1;

  /* This, time, copy the text of each switch and store a pointer
     to the copy in the vector of switches.
     Store all the infiles in their vector.  */

  for (i = 1; i < argc; i++)
    {
      /* Just skip the switches that were handled by the preceding loop.  */
      if (! strncmp (argv[i], "-Wa,", 4))
	;
      else if (! strncmp (argv[i], "-Wp,", 4))
	;
      else if (! strncmp (argv[i], "-Wo,", 4))
	;
      else if (! strncmp (argv[i], "-Wl,", 4))
	;
      else if (!strcmp (argv[i], "-Xlinker"))
	i++;
      else if (! strcmp (argv[i], "-print-libgcc-file-name"))
	;
      else if (! strcmp (argv[i], "-Z"))
	;
      else if (argv[i][0] == '-' && argv[i][1] != 0 && argv[i][1] != 'l')
	{
	  register char *p = &argv[i][1];
	  register int c = *p;

	  if (c == 'B' || c == 'b' || c == 'V')
	    {
	      /* Skip a separate arg, if any.  */
	      if (p[1] == 0)
		i++;
	      continue;
	    }
	  if (c == 'x')
	    {
	      if (p[1] == 0 && i + 1 == argc)
		fatal ("argument to `-x' is missing");
	      if (p[1] == 0)
		spec_lang = argv[++i];
	      else
		spec_lang = p + 1;
	      if (! strcmp (spec_lang, "none"))
		/* Suppress the warning if -xnone comes after the last input
		   file, because alternate command interfaces like g++ might
		   find it useful to place -xnone after each input file.  */
		spec_lang = 0;
	      else
		last_language_n_infiles = n_infiles;
	      continue;
	    }
#if defined (DSP21K) && !defined (CRIPPLE)
	  if (c == 'O')
	    {
	      char *s = &p[1];
	      char c;
	      int optim;

	      while ((c = *s++))
		if (! (c >= '0' && c <= '9'))
		    break;

	      /* For -O>2, subtract one from the optimization level and
		 insert a -CO (at the front, so that a -no-CO has
		 precedence over a -O spec:
		 -no-CO -O3  ==  -CO -no-CO -O2  ==  -no-CO -O2  instead of
		 -no-CO -O3  ==  -no-CO -O2 -CO  ==  -O2 -CO. */

	      if (c == 0 && (optim = atoi(&p[1])) > 2)
		if (n_switches == 0 || strcmp(switches[0].part1, "CO") != 0)
		  {
		    int j;

		    /* Shift everything else to the right to make room. */
		    for (j = n_switches - 1; j >= 0; j--)
		      switches[j + 1] = switches[j];

		    switches[0].part1 = "CO";
		    switches[0].args = 0;
		    switches[0].live_cond = 0;
		    switches[0].valid = 0;

		    n_switches++;
		    p = xmalloc (strlen (p) + 1);
		    sprintf(p, "O%d", optim - 1);
		  }
	    }
#endif
	  switches[n_switches].part1 = p;
	  /* Deal with option arguments in separate argv elements.  */
	  if ((SWITCH_TAKES_ARG (c) > (p[1] != 0))
	      || WORD_SWITCH_TAKES_ARG (p))
	    {
	      int j = 0;
	      int n_args = WORD_SWITCH_TAKES_ARG (p);

	      if (n_args == 0)
		{
		  /* Count only the option arguments in separate argv elements.  */
		  n_args = SWITCH_TAKES_ARG (c) - (p[1] != 0);
		}
	      switches[n_switches].args
		= (char **) xmalloc ((n_args + 1) * sizeof (char *));
	      while (j < n_args)
		switches[n_switches].args[j++] = argv[++i];
	      /* Null-terminate the vector.  */
	      switches[n_switches].args[j] = 0;
	    }
	  else if (*switches_need_spaces != 0 && (c == 'o' || c == 'L'))
	    {
	      /* On some systems, ld cannot handle -o or -L without space.
		 So split the -o or -L from its argument.  */
	      switches[n_switches].part1 = (c == 'o' ? "o" : "L");
	      switches[n_switches].args = (char **) xmalloc (2 * sizeof (char *));
	      switches[n_switches].args[0] = xmalloc (strlen (p));
	      strcpy (switches[n_switches].args[0], &p[1]);
	      switches[n_switches].args[1] = 0;
	    }
	  else
	    switches[n_switches].args = 0;

	  switches[n_switches].live_cond = 0;
	  switches[n_switches].valid = 0;
	  /* This is always valid, since gcc.c itself understands it.  */
	  if (!strcmp (p, "save-temps"))
	    switches[n_switches].valid = 1;
	  n_switches++;
	}
      else
	{
	  infiles[n_infiles].language = spec_lang;
#ifdef CONVERT_FORWARDSLASH_TO_BACKSLASH
	  if (strchr(argv[i], '/'))
	    {
	      char *s = xmalloc (strlen (argv[i]) + 1);

	      infiles[n_infiles++].name = s;
	      strcpy (s, argv[i]);
	      for ( ; *s; s++)
		if (*s == '/')
		  *s = '\\';
	    }
	  else
#endif
	    infiles[n_infiles++].name = argv[i];
	}
    }

  if (n_infiles == last_language_n_infiles)
    error ("Warning: `-x %s' after last input file has no effect", spec_lang);

  switches[n_switches].part1 = 0;
  infiles[n_infiles].name = 0;

  /* If we have a GCC_EXEC_PREFIX envvar, modify it for cpp's sake.  */
  if (gcc_exec_prefix)
    {
      temp = (char *) xmalloc (strlen (gcc_exec_prefix) + strlen (spec_version)
			       + strlen (spec_machine) + 3);
      strcpy (temp, gcc_exec_prefix);
      strcat (temp, spec_version);
      strcat (temp, dir_separator_str);
      strcat (temp, spec_machine);
      strcat (temp, dir_separator_str);
      gcc_exec_prefix = temp;
    }
}

/* Process a spec string, accumulating and running commands.  */

/* These variables describe the input file name.
   input_file_number is the index on outfiles of this file,
   so that the output file name can be stored for later use by %o.
   input_basename is the start of the part of the input file
   sans all directory names, and basename_length is the number
   of characters starting there excluding the suffix .c or whatever.  */

static char *input_filename;
static int input_file_number;
static int input_filename_length;
static int basename_length;
static char *input_basename;
static char *input_suffix;

/* These are variables used within do_spec and do_spec_1.  */

/* Nonzero if an arg has been started and not yet terminated
   (with space, tab or newline).  */
static int arg_going;

/* Nonzero means %d or %g has been seen; the next arg to be terminated
   is a temporary file name.  */
static int delete_this_arg;

/* Nonzero means %w has been seen; the next arg to be terminated
   is the output file name of this compilation.  */
static int this_is_output_file;

/* Nonzero means %s has been seen; the next arg to be terminated
   is the name of a library file and we should try the standard
   search dirs for it.  */
static int this_is_library_file;

/* Process the spec SPEC and run the commands specified therein.
   Returns 0 if the spec is successfully processed; -1 if failed.  */

static int
do_spec (char *spec)
{
  int value;

  clear_args ();
  arg_going = 0;
  delete_this_arg = 0;
  this_is_output_file = 0;
  this_is_library_file = 0;

  value = do_spec_1 (spec, 0, NULL_PTR);

  /* Force out any unfinished command.
     If -pipe, this forces out the last command if it ended in `|'.  */
  if (value == 0)
    {
      if (argbuf_index > 0 && !strcmp (argbuf[argbuf_index - 1], "|"))
	argbuf_index--;

      if (argbuf_index > 0)
	value = execute ();
    }

  return value;
}

/* Process the sub-spec SPEC as a portion of a larger spec.
   This is like processing a whole spec except that we do
   not initialize at the beginning and we do not supply a
   newline by default at the end.
   INSWITCH nonzero means don't process %-sequences in SPEC;
   in this case, % is treated as an ordinary character.
   This is used while substituting switches.
   INSWITCH nonzero also causes SPC not to terminate an argument.

   Value is zero unless a line was finished
   and the command on that line reported an error.  */

static int
do_spec_1 (char *spec, int inswitch, char *soft_matched_part)
{
  register char *p = spec;
  register int c;
  int i;
  char *string;
  int value;

  while ((c = *p++))
    /* If substituting a switch, treat all chars like letters.
       Otherwise, NL, SPC, TAB and % are special.  */
    switch (inswitch ? 'a' : c)
      {
      case '\n':
	/* End of line: finish any pending argument,
	   then run the pending command if one has been started.  */
	if (arg_going)
	  {
	    obstack_1grow (&obstack, 0);
	    string = obstack_finish (&obstack);
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }
	arg_going = 0;

	if (argbuf_index > 0 && !strcmp (argbuf[argbuf_index - 1], "|"))
	  {
	    for (i = 0; i < n_switches; i++)
	      if (!strcmp (switches[i].part1, "pipe"))
		break;

	    /* A `|' before the newline means use a pipe here,
	       but only if -pipe was specified.
	       Otherwise, execute now and don't pass the `|' as an arg.  */
	    if (i < n_switches)
	      {
		switches[i].valid = 1;
		break;
	      }
	    else
	      argbuf_index--;
	  }

	if (argbuf_index > 0)
	  {
	    value = execute ();
	    if (value)
	      return value;
	  }
	/* Reinitialize for a new command, and for a new argument.  */
	clear_args ();
	arg_going = 0;
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	break;

      case '|':
	/* End any pending argument.  */
	if (arg_going)
	  {
	    obstack_1grow (&obstack, 0);
	    string = obstack_finish (&obstack);
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }

	/* Use pipe */
	obstack_1grow (&obstack, c);
	arg_going = 1;
	break;

      case '\t':
      case ' ':
	/* Space or tab ends an argument if one is pending.  */
	if (arg_going)
	  {
	    obstack_1grow (&obstack, 0);
	    string = obstack_finish (&obstack);
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }
	/* Reinitialize for a new argument.  */
	arg_going = 0;
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	break;

      case '%':
	switch (c = *p++)
	  {
	  case 0:
	    fatal ("Invalid specification!  Bug in cc.");

	  case 'b':
	    obstack_grow (&obstack, input_basename, basename_length);
	    arg_going = 1;
	    break;

	  case 'd':
	    delete_this_arg = 2 * (! save_temps_flag);
	    break;

	  /* Dump out the directories specified with LIBRARY_PATH,
	     followed by the absolute directories
	     that we search for startfiles.  */
	  case 'D':
	    for (i = 0; i < 2; i++)
	      {
		struct prefix_list *pl
		  = (i == 0 ? library_prefixes.plist :
		     startfile_prefixes.plist);
		unsigned int bufsize = 100;
		char *buffer = (char *) xmalloc (bufsize);
		int idx;

		for (; pl; pl = pl->next)
		  {
#ifdef RELATIVE_PREFIX_NOT_LINKDIR
		    /* Used on systems which record the specified -L dirs
		       and use them to search for dynamic linking.  */
		    /* Relative directories always come from -B,
		       and it is better not to use them for searching
		       at run time.  In particular, stage1 loses  */
		    if (pl->prefix[0] != '/' && pl->prefix[0] != DIR_SEPARATOR)
		      continue;
#endif
		    if (machine_suffix)
		      {
			if (is_linker_dir (pl->prefix, machine_suffix))
			  {
			    do_spec_1 ("-L", 0, NULL_PTR);
#ifdef SPACE_AFTER_L_OPTION
			    do_spec_1 (" ", 0, NULL_PTR);
#endif
			    do_spec_1 (pl->prefix, 1, NULL_PTR);
			    /* Remove slash from machine_suffix.  */
			    if (strlen (machine_suffix) >= bufsize)
			      bufsize = strlen (machine_suffix) * 2 + 1;
			    buffer = (char *) xrealloc (buffer, bufsize);
			    strcpy (buffer, machine_suffix);
			    idx = strlen (buffer);
			    if (buffer[idx - 1] == '/'
				|| buffer[idx - 1] == DIR_SEPARATOR)
			      buffer[idx - 1] = 0;
			    do_spec_1 (buffer, 1, NULL_PTR);
			    /* Make this a separate argument.  */
			    do_spec_1 (" ", 0, NULL_PTR);
			  }
		      }
		    if (!pl->require_machine_suffix)
		      {
			if (is_linker_dir (pl->prefix, ""))
			  {
			    do_spec_1 ("-L", 0, NULL_PTR);
#ifdef SPACE_AFTER_L_OPTION
			    do_spec_1 (" ", 0, NULL_PTR);
#endif
			    /* Remove slash from pl->prefix.  */
			    if (strlen (pl->prefix) >= bufsize)
			      bufsize = strlen (pl->prefix) * 2 + 1;
			    buffer = (char *) xrealloc (buffer, bufsize);
			    strcpy (buffer, pl->prefix);
			    idx = strlen (buffer);
			    if (buffer[idx - 1] == '/'
				|| buffer[idx - 1] == DIR_SEPARATOR)
			      buffer[idx - 1] = 0;
			    do_spec_1 (buffer, 1, NULL_PTR);
			    /* Make this a separate argument.  */
			    do_spec_1 (" ", 0, NULL_PTR);
			  }
		      }
		  }
		free (buffer);
	      }
	    break;

	  case 'e':
	  case '!':
	    /* {...:%efoo} means report an error with `foo' as error message
	       and don't execute any more commands for this file.
	       {...:%!foo} is similar but doesn't stop execution: a warning. */
	    {
	      char *q = p;
	      char *buf;
	      while (*p != 0 && *p != '\n') p++;
	      buf = (char *) alloca (p - q + 1);
	      strncpy (buf, q, p - q);
	      buf[p - q] = 0;
	      error ("%s", buf);

	      if (c == 'e')
		return -1;
	    }
	    break;

	  case 'g':
	    if (save_temps_flag)
	      obstack_grow (&obstack, input_basename, basename_length);
	    else
	      {
		obstack_grow (&obstack, temp_filename, temp_filename_length);
		delete_this_arg = 1;
	      }
	    arg_going = 1;
	    break;

	  case 'i':
	    obstack_grow (&obstack, input_filename, input_filename_length);
	    arg_going = 1;
	    break;

	  case 'I':
	    if (gcc_exec_prefix)
	      {
		do_spec_1 ("-imacros", 1, NULL_PTR);
		/* Make this a separate argument.  */
		do_spec_1 (" ", 0, NULL_PTR);
		do_spec_1 (gcc_exec_prefix, 1, NULL_PTR);
		do_spec_1 (" ", 0, NULL_PTR);
	      }
	    break;

	  case 'o':
	    {
	      register int f;
	      for (f = 0; f < n_infiles; f++)
		store_arg (outfiles[f], 0, 0);
	    }
	    break;

	  case 's':
	    this_is_library_file = 1;
	    break;

	  case 'w':
	    this_is_output_file = 1;
	    break;

	  case 'W':
	    {
	      int index = argbuf_index;
	      /* Handle the {...} following the %W.  */
	      if (*p != '{')
		abort ();
	      p = handle_braces (p + 1);
	      if (p == 0)
		return -1;
	      /* If any args were output, mark the last one for deletion
		 on failure.  */
	      if (argbuf_index != index)
		record_temp_file (argbuf[argbuf_index - 1], 0, 1);
	      break;
	    }

	  /* %x{OPTION} records OPTION for %X to output.  */
	  case 'x':
	    {
	      char *p1 = p;
	      char *string;

	      /* Skip past the option value and make a copy.  */
	      if (*p != '{')
		abort ();
	      while (*p++ != '}')
		;
	      string = save_string (p1 + 1, p - p1 - 2);

	      /* See if we already recorded this option.  */
	      for (i = 0; i < n_linker_options; i++)
		if (! strcmp (string, linker_options[i]))
		  {
		    free (string);
		    return 0;
		  }

	      /* This option is new; add it.  */
	      n_linker_options++;
	      if (!linker_options)
		linker_options
		  = (char **) xmalloc (n_linker_options * sizeof (char **));
	      else
		linker_options
		  = (char **) xrealloc (linker_options,
					n_linker_options * sizeof (char **));

	      linker_options[n_linker_options - 1] = string;
	    }
	    break;

	  /* Dump out the options accumulated previously using %x.  */
	  case 'X':
	    for (i = 0; i < n_linker_options; i++)
	      {
		do_spec_1 (linker_options[i], 1, NULL_PTR);
		/* Make each accumulated option a separate argument.  */
		do_spec_1 (" ", 0, NULL_PTR);
	      }
	    break;

	  /* Dump out the options accumulated previously using -Wa,.  */
	  case 'Y':
	    for (i = 0; i < n_assembler_options; i++)
	      {
		do_spec_1 (assembler_options[i], 1, NULL_PTR);
		/* Make each accumulated option a separate argument.  */
		do_spec_1 (" ", 0, NULL_PTR);
	      }
	    break;

	  /* Dump out the options accumulated previously using -Wp,.  */
	  case 'Z':
	    for (i = 0; i < n_preprocessor_options; i++)
	      {
		do_spec_1 (preprocessor_options[i], 1, NULL_PTR);
		/* Make each accumulated option a separate argument.  */
		do_spec_1 (" ", 0, NULL_PTR);
	      }
	    break;

#ifdef DSP21K
	  /* Dump out the options accumulated previously using -Wo,.  */
	  case 'V':
	    for (i = 0; i < n_compactor_options; i++)
	      {
		do_spec_1 (compactor_options[i], 1, NULL_PTR);
		/* Make each accumulated option a separate argument.  */
		do_spec_1 (" ", 0, NULL_PTR);
	      }
	    break;
#endif

	    /* Here are digits and numbers that just process
	       a certain constant string as a spec.  */

	  case '1':
	    value = do_spec_1 (cc1_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case '2':
	    value = do_spec_1 (cc1plus_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case '3':
	    value = do_spec_1 (cpp_default_include, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'a':
	    value = do_spec_1 (asm_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'A':
	    value = do_spec_1 (asm_final_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'c':
	    value = do_spec_1 (signed_char_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'C':
	    value = do_spec_1 (cpp_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'E':
	    value = do_spec_1 (endfile_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'l':
	    value = do_spec_1 (link_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'L':
	    value = do_spec_1 (lib_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	  case 'p':
	    {
	      char *x = (char *) alloca (strlen (cpp_predefines) + 1);
	      char *buf = x;
	      char *y;

	      /* Copy all of the -D options in CPP_PREDEFINES into BUF.  */
	      y = cpp_predefines;
	      while (*y != 0)
		{
		  if (! strncmp (y, "-D", 2))
		    /* Copy the whole option.  */
		    while (*y && *y != ' ' && *y != '\t')
		      *x++ = *y++;
		  else if (*y == ' ' || *y == '\t')
		    /* Copy whitespace to the result.  */
		    *x++ = *y++;
		  /* Don't copy other options.  */
		  else
		    y++;
		}

	      *x = 0;

	      value = do_spec_1 (buf, 0, NULL_PTR);
	      if (value != 0)
		return value;
	    }
	    break;

	  case 'P':
	    {
	      char *x = (char *) alloca (strlen (cpp_predefines) * 4 + 1);
	      char *buf = x;
	      char *y;

	      /* Copy all of CPP_PREDEFINES into BUF,
		 but put __ after every -D and at the end of each arg.  */
	      y = cpp_predefines;
	      while (*y != 0)
		{
		  if (! strncmp (y, "-D", 2))
		    {
		      int flag = 0;

		      *x++ = *y++;
		      *x++ = *y++;

		      if (strncmp (y, "__", 2))
			{
			  /* Stick __ at front of macro name.  */
			  *x++ = '_';
			  *x++ = '_';
			  /* Arrange to stick __ at the end as well.  */
			  flag = 1;
			}

		      /* Copy the macro name.  */
		      while (*y && *y != '=' && *y != ' ' && *y != '\t')
			*x++ = *y++;

		      if (flag)
			{
			  *x++ = '_';
			  *x++ = '_';
			}

		      /* Copy the value given, if any.  */
		      while (*y && *y != ' ' && *y != '\t')
			*x++ = *y++;
		    }
		  else if (*y == ' ' || *y == '\t')
		    /* Copy whitespace to the result.  */
		    *x++ = *y++;
		  /* Don't copy -A options  */
		  else
		    y++;
		}
	      *x++ = ' ';

	      /* Copy all of CPP_PREDEFINES into BUF,
		 but put __ after every -D.  */
	      y = cpp_predefines;
	      while (*y != 0)
		{
		  if (! strncmp (y, "-D", 2))
		    {
		      *x++ = *y++;
		      *x++ = *y++;

		      if (strncmp (y, "__", 2))
			{
			  /* Stick __ at front of macro name.  */
			  *x++ = '_';
			  *x++ = '_';
			}

		      /* Copy the macro name.  */
		      while (*y && *y != '=' && *y != ' ' && *y != '\t')
			*x++ = *y++;

		      /* Copy the value given, if any.  */
		      while (*y && *y != ' ' && *y != '\t')
			*x++ = *y++;
		    }
		  else if (*y == ' ' || *y == '\t')
		    /* Copy whitespace to the result.  */
		    *x++ = *y++;
		  /* Don't copy -A options  */
		  else
		    y++;
		}
	      *x++ = ' ';

	      /* Copy all of the -A options in CPP_PREDEFINES into BUF.  */
	      y = cpp_predefines;
	      while (*y != 0)
		{
		  if (! strncmp (y, "-A", 2))
		    /* Copy the whole option.  */
		    while (*y && *y != ' ' && *y != '\t')
		      *x++ = *y++;
		  else if (*y == ' ' || *y == '\t')
		    /* Copy whitespace to the result.  */
		    *x++ = *y++;
		  /* Don't copy other options.  */
		  else
		    y++;
		}

	      *x = 0;

	      value = do_spec_1 (buf, 0, NULL_PTR);
	      if (value != 0)
		return value;
	    }
	    break;

	  case 'S':
	    value = do_spec_1 (startfile_spec, 0, NULL_PTR);
	    if (value != 0)
	      return value;
	    break;

	    /* Here we define characters other than letters and digits.  */

	  case '{':
	    p = handle_braces (p);
	    if (p == 0)
	      return -1;
	    break;

	  case '%':
	    obstack_1grow (&obstack, '%');
	    break;

	  case '$':
	    {
	      char * back = (char *)strrchr (soft_matched_part, '.');
	      if (back == 0) {
		do_spec_1 (soft_matched_part, 1, NULL);
	      }
	      else {
		char *string = save_string (soft_matched_part, back - soft_matched_part );
		do_spec_1 (string, 1, NULL);
	      }
	    }
	    break;
		
	  case '*':
	    do_spec_1 (soft_matched_part, 1, NULL_PTR);
	    do_spec_1 (" ", 0, NULL_PTR);
	    break;

	    /* Process a string found as the value of a spec given by name.
	       This feature allows individual machine descriptions
	       to add and use their own specs.
	       %[...] modifies -D options the way %P does;
	       %(...) uses the spec unmodified.  */
	  case '(':
	  case '[':
	    {
	      char *name = p;
	      struct spec_list *sl;
	      int len;

	      /* The string after the S/P is the name of a spec that is to be
		 processed. */
	      while (*p && *p != ')' && *p != ']')
		p++;

	      /* See if it's in the list */
	      for (len = p - name, sl = specs; sl; sl = sl->next)
		if (strncmp (sl->name, name, len) == 0 && !sl->name[len])
		  {
		    name = sl->spec;
		    break;
		  }

	      if (sl)
		{
		  if (c == '(')
		    do_spec_1 (name, 0, NULL_PTR);
		  else
		    {
		      char *x = (char *) alloca (strlen (name) * 2 + 1);
		      char *buf = x;
		      char *y = name;

		      /* Copy all of NAME into BUF, but put __ after
			 every -D and at the end of each arg,  */
		      while (1)
			{
			  if (! strncmp (y, "-D", 2))
			    {
			      *x++ = '-';
			      *x++ = 'D';
			      *x++ = '_';
			      *x++ = '_';
			      y += 2;
			    }
			  else if (*y == ' ' || *y == 0)
			    {
			      *x++ = '_';
			      *x++ = '_';
			      if (*y == 0)
				break;
			      else
				*x++ = *y++;
			    }
			  else
			    *x++ = *y++;
			}
		      *x = 0;

		      do_spec_1 (buf, 0, NULL_PTR);
		    }
		}

	      /* Discard the closing paren or bracket.  */
	      if (*p)
		p++;
	    }
	    break;

	  default:
#ifdef DOSENV
	    fprintf(stdout,"Unknown %\n");
#endif
	    abort ();
	  }
	break;

      case '\\':
	/* Backslash: treat next character as ordinary.  */
	c = *p++;

	/* fall through */
      default:
	/* Ordinary character: put it into the current argument.  */
	obstack_1grow (&obstack, c);
	arg_going = 1;
      }

  return 0;             /* End of string */
}

/* Return 0 if we call do_spec_1 and that returns -1.  */

static char *
handle_braces (register char *p)
{
  char *savep;
  register char *q;
  char *filter;
  int pipe = 0;
  int negate = 0;
  int suffix = 0;
  savep = p;

  if (*p == '|')
    /* A `|' after the open-brace means,
       if the test fails, output a single minus sign rather than nothing.
       This is used in %{|!pipe:...}.  */
    pipe = 1, ++p;

  if (*p == '!')
    /* A `!' after the open-brace negates the condition:
       succeed if the specified switch is not present.  */
    negate = 1, ++p;

  if (*p == '.')
    /* A `.' after the open-brace means test against the current suffix.  */
    {
      if (pipe)
	{
#ifdef DOSENV
	  fprintf(stdout,"Couldn't pipe\n");
#endif
	abort ();
	}

      suffix = 1;
      ++p;
    }

  filter = p;
  while (*p != ':' && *p != '}') p++;
  if (*p != '}')
    {
      register int count = 1;
      q = p + 1;
      while (count > 0)
	{
	  if (*q == '{')
	    count++;
	  else if (*q == '}')
	    count--;
	  else if (*q == 0)
	    {
#ifdef DOSENV
	      fprintf(stdout,"Hit end of string too soon\n%s\n",savep);
#endif
	    abort ();
	    }
	  q++;
	}
    }
  else
    q = p + 1;

  if (suffix)
    {
      int found = (input_suffix != 0
		   && strlen (input_suffix) == p - filter
		   && strncmp (input_suffix, filter, p - filter) == 0);

      if (p[0] == '}')
	{
#ifdef DOSENV
	  fprintf(stdout,"Unexpected '}'\n");
#endif
	abort ();
	}

      if (negate != found
	  && do_spec_1 (save_string (p + 1, q - p - 2), 0, NULL_PTR) < 0)
	return 0;

      return q;
    }
  else if (p[-1] == '*' && p[0] == '}')
    {
      /* Substitute all matching switches as separate args.  */
      register int i;
      --p;
      for (i = 0; i < n_switches; i++)
	if (!strncmp (switches[i].part1, filter, p - filter)
	    && check_live_switch (i, p - filter))
	  give_switch (i, 0);
    }
  else
    {
      /* Test for presence of the specified switch.  */
      register int i;
      int present = 0;

      /* If name specified ends in *, as in {x*:...},
	 check for %* and handle that case.  */
      if (p[-1] == '*' && !negate)
	{
	  int substitution;
	  int bsubstitution;
	  char *r = p;

	  /* First see whether we have %*.  */
	  substitution = 0;
	  bsubstitution = 0;
	  while (r < q)
	    {
	      if (*r == '%' && r[1] == '*')
		substitution = 1;
	      if (*r == '%' && r[1] == '$')
		bsubstitution = 1;
	      r++;
	    }

	  /* If we do, handle that case.  */
	  if (substitution || bsubstitution)
	    {
	      /* Substitute all matching switches as separate args.
		 But do this by substituting for %*
		 in the text that follows the colon.  */

	      unsigned hard_match_len = p - filter - 1;
	      char *string = save_string (p + 1, q - p - 2);

	      for (i = 0; i < n_switches; i++)
		if (!strncmp (switches[i].part1, filter, hard_match_len)
		    && check_live_switch (i, -1))
		  {
		    if (!bsubstitution) {
		      do_spec_1 (string, 0, &switches[i].part1[hard_match_len]);
		      /* Pass any arguments this switch has.  */
		      give_switch (i, 1);
		    }
		    else {
		      if (switches[i].args != 0)
			{
			  char **p;
			  for (p = switches[i].args; *p; p++)
			    {
			      do_spec_1 (string, 0, *p);
			      do_spec_1 (" ", 0, NULL);
			    }
			}
		    }
		  }

	      return q;
	    }
	}

      /* If name specified ends in *, as in {x*:...},
	 check for presence of any switch name starting with x.  */
      if (p[-1] == '*')
	{
	  for (i = 0; i < n_switches; i++)
	    {
	      unsigned hard_match_len = p - filter - 1;

	      if (!strncmp (switches[i].part1, filter, hard_match_len)
		  && check_live_switch (i, hard_match_len))
		{
		  present = 1;
		}
	    }
	}
      /* Otherwise, check for presence of exact name specified.  */
      else
	{
	  for (i = 0; i < n_switches; i++)
	    {
	      if (!strncmp (switches[i].part1, filter, p - filter)
		  && switches[i].part1[p - filter] == 0
		  && check_live_switch (i, -1))
		{
		  present = 1;
		  break;
		}
	    }
	}

      /* If it is as desired (present for %{s...}, absent for %{-s...})
	 then substitute either the switch or the specified
	 conditional text.  */
      if (present != negate)
	{
	  if (*p == '}')
	    {
	      give_switch (i, 0);
	    }
	  else
	    {
	      char *x;
	      if (do_spec_1 (x = save_string (p + 1, q - p - 2), 0, NULL_PTR)
		  < 0)
		{
		  free (x);
		  return 0;
		}
	      free (x);
	    }
	}
      else if (pipe)
	{
	  /* Here if a %{|...} conditional fails: output a minus sign,
	     which means "standard output" or "standard input".  */
	  do_spec_1 ("-", 0, NULL_PTR);
	}
    }

  return q;
}

/* Return 0 iff switch number SWITCHNUM is obsoleted by a later switch
   on the command line.  PREFIX_LENGTH is the length of XXX in an {XXX*}
   spec, or -1 if either exact match or %* is used.

   A -O switch is obsoleted by a later -O switch.  A -f, -m, or -W switch
   whose value does not begin with "no-" is obsoleted by the same value
   with the "no-", similarly for a switch with the "no-" prefix.
   A -CO switch is obsoleted by a later -no-CO switch, and vice-versa. */

static int
check_live_switch (int switchnum, int prefix_length)
{
  char *name = switches[switchnum].part1;
  int i;

  /* In the common case of {<at-most-one-letter>*}, a negating
     switch would always match, so ignore that case.  We will just
     send the conflicting switches to the compiler phase.  */
  if (prefix_length >= 0 && prefix_length <= 1)
    return 1;

  /* If we already processed this switch and determined if it was
     live or not, return our past determination.  */
  if (switches[switchnum].live_cond != 0)
    return switches[switchnum].live_cond > 0;

  /* Now search for duplicate in a manner that depends on the name.  */
  switch (*name)
    {
    case 'O':
	for (i = switchnum + 1; i < n_switches; i++)
	  if (switches[i].part1[0] == 'O')
	    {
	      switches[switchnum].valid = 1;
	      switches[switchnum].live_cond = -1;
	      return 0;
	    }
      break;

    case 'W':  case 'f':  case 'm':
      if (! strncmp (name + 1, "no-", 3))
	{
	  /* We have Xno-YYY, search for XYYY. */
	  for (i = switchnum + 1; i < n_switches; i++)
	    if (switches[i].part1[0] == name[0]
		&& ! strcmp (&switches[i].part1[1], &name[4]))
	    {
	      switches[switchnum].valid = 1;
	      switches[switchnum].live_cond = -1;
	      return 0;
	    }
	}
      else
	{
	  /* We have XYYY, search for Xno-YYY.  */
	  for (i = switchnum + 1; i < n_switches; i++)
	    if (switches[i].part1[0] == name[0]
		&& switches[i].part1[1] == 'n'
		&& switches[i].part1[2] == 'o'
		&& switches[i].part1[3] == '-'
		&& !strcmp (&switches[i].part1[4], &name[1]))
	    {
	      switches[switchnum].valid = 1;
	      switches[switchnum].live_cond = -1;
	      return 0;
	    }
	}
      break;

    case 'C':
      if (name[1] == 'O' && name[2] == '\0')
	{
	  /* We have CO, search for no-CO */
	  for (i = switchnum + 1; i < n_switches; i++)
	    if (!strcmp (switches[i].part1, "no-CO"))
	      {
		switches[switchnum].valid = 1;
		switches[switchnum].live_cond = -1;
		return 0;
	      }
	}
      break;

    case 'n':
      if (!strcmp (name + 1, "o-CO"))
	{
	  /* We have no-CO, search for CO */
	  for (i = switchnum + 1; i < n_switches; i++)
	    if (!strcmp (switches[i].part1, "CO"))
	      {
		switches[switchnum].valid = 1;
		switches[switchnum].live_cond = -1;
		return 0;
	      }
	}
      break;
    }

  /* Otherwise the switch is live.  */
  switches[switchnum].live_cond = 1;
  return 1;
}

/* Pass a switch to the current accumulating command
   in the same form that we received it.
   SWITCHNUM identifies the switch; it is an index into
   the vector of switches gcc received, which is `switches'.
   This cannot fail since it never finishes a command line.

   If OMIT_FIRST_WORD is nonzero, then we omit .part1 of the argument.  */

static void
give_switch (int switchnum, int omit_first_word)
{
  if (!omit_first_word)
    {
      do_spec_1 ("-", 0, NULL_PTR);
      do_spec_1 (switches[switchnum].part1, 1, NULL_PTR);
    }
  do_spec_1 (" ", 0, NULL_PTR);
  if (switches[switchnum].args != 0)
    {
      char **p;
      for (p = switches[switchnum].args; *p; p++)
	{
	  do_spec_1 (*p, 1, NULL_PTR);
	  do_spec_1 (" ", 0, NULL_PTR);
	}
    }
  switches[switchnum].valid = 1;
}

/* Search for a file named NAME trying various prefixes including the
   user's -B prefix and some standard ones.
   Return the absolute file name found.  If nothing is found, return NAME.  */

static char *
find_file (char *name)
{
  char *newname;

  newname = find_a_file (&startfile_prefixes, name, R_OK);
  return newname ? newname : name;
}

/* Determine whether a -L option is relevant.  Not required for certain
   fixed names and for directories that don't exist.  */

static int
is_linker_dir (char *path1, char *path2)
{
  int len1 = strlen (path1);
  int len2 = strlen (path2);
  char *path = (char *) alloca (3 + len1 + len2);
  char *cp;
  struct stat st;

  /* Construct the path from the two parts.  Ensure the string ends with "/.".
     The resulting path will be a directory even if the given path is a
     symbolic link.  */
  bcopy (path1, path, len1);
  bcopy (path2, path + len1, len2);
  cp = path + len1 + len2;
  if (cp[-1] != '/' && cp[-1] != DIR_SEPARATOR)
    *cp++ = DIR_SEPARATOR;
  *cp++ = '.';
  *cp = '\0';

  /* Exclude directories that the linker is known to search.  */
  if ((cp - path == 6 && strcmp (path, "/lib/.") == 0)
      || (cp - path == 10 && strcmp (path, "/usr/lib/.") == 0))
    return 0;

  return (stat (path, &st) >= 0 && S_ISDIR (st.st_mode));
}

/* On fatal signals, delete all the temporary files.  */

void
fatal_error (int signum)
{
  signal (signum, SIG_DFL);
  delete_failure_queue ();
  delete_temp_files ();
  /* Get the same signal again, this time not handled,
     so its normal effect occurs.  */
#ifdef DOSENV
  raise (signum);
  exit(-1);  /* Hopefully we won't get to this line, anyways! */
#else
  kill (getpid (), signum);
#endif
}


#if !defined (GO32)
static int main_1 (int, char **, char **);

int 
main (int argc, char **argv, char **envp)
{
  process_atfiles(&argc, &argv);
  return main_1 (argc, argv, envp);
}
static int 
main_1 (int argc, char **argv, char **envp)
#else
int 
main (int argc, char **argv, char **envp)
#endif
{
  register int i;
  int j;
  int value;
  int error_count = 0;
  int linker_was_run = 0;
  char *explicit_link_files;
  char *specs_file;

  if (argc > 1 && !strcmp(argv[1], "-Z"))
    debug_flag++;

#ifdef MSDOS
  /* Redirect the output of stderr to stdout */
  dup2 (1,2); 
#ifdef DOS_MEM_STATS
  if (debug_flag)
    printf("Free memory at start: %u  stack: %u\n", _memavl(), stackavail());
#endif
#endif

  programname = argv[0];

						   /*EK*/
#if defined (ADI) && !defined (A21C0) && !defined (DSP21XX) && !defined (CRIPPLE)
  /* read the ach file and put the appropriate switches
     into argv */
  if (argc > 1) {
    extern void process_architecture_file ( int *, char  ***);
    process_architecture_file(&argc, &argv);
  }
#endif

#if defined ( __WATCOMC__)
  if(!getenv("DOS4G"))
    putenv("DOS4G=quiet");
#endif


#ifdef MSC
  if (!getenv("EMXOPT"))
    putenv("EMXOPT=-s16000 -c");
  if (!getenv("RSXOPT"))
    putenv("RSXOPT=-s16000 -c");
#endif

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
    signal (SIGINT, fatal_error);
#ifdef SIGHUP
  if (signal (SIGHUP, SIG_IGN) != SIG_IGN)
    signal (SIGHUP, fatal_error);
#endif
  if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
    signal (SIGTERM, fatal_error);
#ifdef SIGPIPE
  if (signal (SIGPIPE, SIG_IGN) != SIG_IGN)
    signal (SIGPIPE, fatal_error);
#endif

  argbuf_length = 10;
  argbuf = (char **) xmalloc (argbuf_length * sizeof (char *));

  obstack_init (&obstack);

  /* Set up to remember the pathname of gcc and any options
     needed for collect.  */
  obstack_init (&collect_obstack);
  obstack_grow (&collect_obstack, "COLLECT_GCC=", sizeof ("COLLECT_GCC=")-1);
  obstack_grow (&collect_obstack, programname, strlen (programname)+1);
  putenv (obstack_finish (&collect_obstack));

  /* Choose directory for temp files.  */

  choose_temp_base ();

  /* Make a table of what switches there are (switches, n_switches).
     Make a table of specified input files (infiles, n_infiles).
     Decode switches that are handled locally.  */

  process_command (argc, argv);

  /* Initialize the vector of specs to just the default.
     This means one element containing 0s, as a terminator.  */

  compilers = (struct compiler *) xmalloc (sizeof default_compilers);
  bcopy (default_compilers, compilers, sizeof default_compilers);
  n_compilers = n_default_compilers;

  /* Read specs from a file if there is one.  */

#ifndef DSP21K
  machine_suffix = concat (spec_machine, dir_separator_str,
			   concat (spec_version, dir_separator_str, ""));
#endif

  specs_file = find_a_file (&startfile_prefixes, "specs", R_OK);
  /* Read the specs file unless it is a default one.  */
  if (specs_file != 0 && strcmp (specs_file, "specs"))
    read_specs (specs_file);

  /* If not cross-compiling, look for startfiles in the standard places.  */
  /* The fact that these are done here, after reading the specs file,
     means that it cannot be found in these directories.
     But that's okay.  It should never be there anyway.  */
  if (!cross_compile)
    {
#ifdef MD_EXEC_PREFIX
      add_prefix (&exec_prefixes, md_exec_prefix, 0, 0, NULL_PTR);
      add_prefix (&startfile_prefixes, md_exec_prefix, 0, 0, NULL_PTR);
#endif

#ifdef MD_STARTFILE_PREFIX
      add_prefix (&startfile_prefixes, md_startfile_prefix, 0, 0, NULL_PTR);
#endif

#ifdef MD_STARTFILE_PREFIX_1
      add_prefix (&startfile_prefixes, md_startfile_prefix_1, 0, 0, NULL_PTR);
#endif

      add_prefix (&startfile_prefixes, standard_startfile_prefix, 0, 0,
		  NULL_PTR);
      add_prefix (&startfile_prefixes, standard_startfile_prefix_1, 0, 0,
		  NULL_PTR);
      add_prefix (&startfile_prefixes, standard_startfile_prefix_2, 0, 0,
		  NULL_PTR);
#if 0 /* Can cause surprises, and one can use -B./ instead.  */
      add_prefix (&startfile_prefixes, "./", 0, 1, NULL_PTR);
#endif
    }

  /* Now we have the specs.
     Set the `valid' bits for switches that match anything in any spec.  */

  validate_all_switches ();

  /* Warn about any switches that no pass was interested in.  */

  for (i = 0; i < n_switches; i++)
    if (! switches[i].valid)
      if (!is_bad_switch (switches[i].part1))
	if (is_suppressed_option (switches[i].part1))
	  error ("Warning: The `-%s' switch is not supported in the lite "
		 "compiler.", switches[i].part1);
	else if (is_default_option (switches[i].part1))
	  error ("The `-%s' switch is not needed, because it is always in "
		 "effect in the lite compiler.", switches[i].part1);
	else
	  error ("unrecognized option `-%s'", switches[i].part1);

  if (print_libgcc_file_name)
    {
      printf ("%s\n", find_file ("libgcc.a"));
      exit (0);
    }

  /* Obey some of the options.  */

  if (verbose_flag)
    {
      fprintf (stdout, "gcc version %s%s\n", version_string, PATCHLEVEL);
      if (n_infiles == 0)
	exit (0);
    }

  if (n_infiles == 0)
    fatal ("No input files specified.");

  /* Make a place to record the compiler output file names
     that correspond to the input files.  */

  outfiles = (char **) xmalloc (n_infiles * sizeof (char *));
  bzero (outfiles, n_infiles * sizeof (char *));

  /* Record which files were specified explicitly as link input.  */

  explicit_link_files = xmalloc (n_infiles);
  bzero (explicit_link_files, n_infiles);

  for (i = 0; i < n_infiles; i++)
    {
      register struct compiler *cp = 0;
      int this_file_error = 0;

      /* Tell do_spec what to substitute for %i.  */

      input_filename = infiles[i].name;
      input_filename_length = strlen (input_filename);
      input_file_number = i;

      /* Use the same thing in %o, unless cp->spec says otherwise.  */

      outfiles[i] = input_filename;

      /* Figure out which compiler from the file's suffix.  */

      cp = lookup_compiler (infiles[i].name, input_filename_length,
			    infiles[i].language);

      if (cp)
	{
	  /* Ok, we found an applicable compiler.  Run its spec.  */
	  /* First say how much of input_filename to substitute for %b  */
	  register char *p;
	  int len;

	  input_basename = input_filename;
	  for (p = input_filename; *p; p++)
	    if (*p == '/' || *p == DIR_SEPARATOR)
	      input_basename = p + 1;

	  /* Find a suffix starting with the last period,
	     and set basename_length to exclude that suffix.  */
	  basename_length = strlen (input_basename);
	  p = input_basename + basename_length;
	  while (p != input_basename && *p != '.') --p;
	  if (*p == '.' && p != input_basename)
	    {
	      basename_length = p - input_basename;
	      input_suffix = p + 1;
	    }
	  else
	    input_suffix = "";

	  len = 0;
	  for (j = 0; j < sizeof cp->spec / sizeof cp->spec[0]; j++)
	    if (cp->spec[j])
	      len += strlen (cp->spec[j]);

	  p = (char *) xmalloc (len + 1);

	  len = 0;
	  for (j = 0; j < sizeof cp->spec / sizeof cp->spec[0]; j++)
	    if (cp->spec[j])
	      {
		strcpy (p + len, cp->spec[j]);
		len += strlen (cp->spec[j]);
	      }


	  value = do_spec (p);
	  free(p);
	  if (value < 0)
	    this_file_error = 1;
	}

      /* If this file's name does not contain a recognized suffix,
	 record it as explicit linker input.  */

      else
	explicit_link_files[i] = 1;

      /* Clear the delete-on-failure queue, deleting the files in it
	 if this compilation failed.  */

      if (this_file_error)
	{
	  delete_failure_queue ();
	  error_count++;
	}
      /* If this compilation succeeded, don't delete those files later.  */
      clear_failure_queue ();
    }

  /* Run ld to link all the compiler output files.  */

  if (error_count == 0)
    {
      int tmp = execution_count;
      int i;
      int first_time;

      /* Rebuild the COMPILER_PATH and LIBRARY_PATH environment variables
	 for collect.  */
      putenv_from_prefixes (&exec_prefixes, "COMPILER_PATH=");
      putenv_from_prefixes (&startfile_prefixes, "LIBRARY_PATH=");

      /* Build COLLECT_GCC_OPTIONS to have all of the options specified to
	 the compiler.  */
      obstack_grow (&collect_obstack, "COLLECT_GCC_OPTIONS=",
		    sizeof ("COLLECT_GCC_OPTIONS=")-1);

      first_time = TRUE;
      for (i = 0; i < n_switches; i++)
	{
	  char **args;
	  if (!first_time)
	    obstack_grow (&collect_obstack, " ", 1);

	  first_time = FALSE;
	  obstack_grow (&collect_obstack, "-", 1);
	  obstack_grow (&collect_obstack, switches[i].part1,
			strlen (switches[i].part1));

	  for (args = switches[i].args; args && *args; args++)
	    {
	      obstack_grow (&collect_obstack, " ", 1);
	      obstack_grow (&collect_obstack, *args, strlen (*args));
	    }
	}
      obstack_grow (&collect_obstack, "\0", 1);
      putenv (obstack_finish (&collect_obstack));

      value = do_spec (link_command_spec);
      if (value < 0)
	error_count = 1;
      linker_was_run = (tmp != execution_count);
    }

  /* Warn if a -B option was specified but the prefix was never used.  */
  unused_prefix_warnings (&exec_prefixes);
  unused_prefix_warnings (&startfile_prefixes);

  /* If options said don't run linker,
     complain about input files to be given to the linker.  */

  if (! linker_was_run && error_count == 0)
    for (i = 0; i < n_infiles; i++)
      if (explicit_link_files[i])
	error ("%s: linker input file unused since linking not done",
	       outfiles[i]);

  /* Delete some or all of the temporary files we made.  */

  if (error_count)
    delete_failure_queue ();
  delete_temp_files ();

  exit (error_count);
  /* NOTREACHED */
  return 0;
}

/* Find the proper compilation spec for the file name NAME,
   whose length is LENGTH.  LANGUAGE is the specified language,
   or 0 if none specified.  */

static struct compiler *
lookup_compiler (char *name, int length, char *language)
{
  struct compiler *cp;

  /* Look for the language, if one is spec'd.  */
  if (language != 0)
    {
      for (cp = compilers + n_compilers - 1; cp >= compilers; cp--)
	{
	  if (language != 0)
	    {
	      if (cp->suffix[0] == '@'
		  && !strcmp (cp->suffix + 1, language))
		return cp;
	    }
	}
      error ("language %s not recognized", language);
    }

#ifdef DSP21K

  /* The linker will append ".obj" in absence of extension */
  /* Only "-l" switch is special to the driver */

    if (strncmp(name, "-l",2) &&
	(strchr(name, '.') == NULL))
	error("Warning: %s has no extension, .obj is assumed", name);
#endif

  /* Look for a suffix.  */
  for (cp = compilers + n_compilers - 1; cp >= compilers; cp--)
    {
      if (strlen (cp->suffix) < length
	       /* See if the suffix matches the end of NAME.  */
	       && !strcmp (cp->suffix,
			   name + length - strlen (cp->suffix))
	       /* The suffix `-' matches only the file name `-'.  */
	       && !(!strcmp (cp->suffix, "-") && length != 1))
	{
	  if (cp->spec[0][0] == '@')
	    {
	      struct compiler *new;
	      /* An alias entry maps a suffix to a language.
		 Search for the language; pass 0 for NAME and LENGTH
		 to avoid infinite recursion if language not found.
		 Construct the new compiler spec.  */
	      language = cp->spec[0] + 1;
	      new = (struct compiler *) xmalloc (sizeof (struct compiler));
	      new->suffix = cp->suffix;
	      bcopy (lookup_compiler (NULL_PTR, 0, language)->spec,
		     new->spec, sizeof new->spec);
	      return new;
	    }
	  /* A non-alias entry: return it.  */
	  return cp;
	}
    }

  return 0;
}

void *
xmalloc (unsigned int size)
{
  register char *value = (char *) malloc (size);
  if (value == 0)
    fatal ("virtual memory exhausted");
  return value;
}

void *
xrealloc (void *ptr, unsigned int size)
{
  register char *value = (char *) realloc (ptr, size);
  if (value == 0)
    fatal ("virtual memory exhausted");
  return value;
}

/* Return a newly-allocated string whose contents concatenate those of s1, s2, s3.  */

static char *
concat (char *s1, const char *s2, const char *s3)
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

static char *
save_string (char *s, int len)
{
  register char *result = xmalloc (len + 1);

  bcopy (s, result, len);
  result[len] = 0;
  return result;
}

static void
pfatal_with_name (char *name)
{
  char *s;

  if (errno < sys_nerr)
    s = concat ("%s: ", my_strerror( errno ), "");
  else
    s = "cannot open `%s'";
  fatal (s, name);
}

static void
perror_with_name (char *name)
{
  char *s;

  if (errno < sys_nerr)
    s = concat ("%s: ", my_strerror( errno ), "");
  else
    s = "cannot open `%s'";
  error (s, name);
}

static void
perror_exec (char *name)
{
  char *s;

  if (errno < sys_nerr)
    s = concat ("installation problem, cannot exec `%s': ",
		my_strerror (errno), "");
  else
    s = "installation problem, cannot exec `%s'";
  error (s, name);
}

/* More 'friendly' abort that prints the line and file.
   config.h can #define abort fancy_abort if you like that sort of thing.  */

void
fancy_abort (void)
{
  fatal ("Internal gcc abort.");
}

#ifdef HAVE_VPRINTF

/* Output an error message and exit */

void
#ifdef __USE_STDARGS__
fatal (char *format, ...)
#else
fatal (va_alist)
     va_dcl
#endif
{
  va_list ap;
#ifdef __USE_STDARGS__
  va_start (ap, format);
#else
  char *format;
  va_start (ap);
  format = va_arg (ap, char *);
#endif
  fprintf (stdout, "%s: ", programname);
  vfprintf (stdout, format, ap);
  va_end (ap);
  fprintf (stdout, "\n");
  delete_temp_files ();
  exit (1);
}

void
#ifdef __USE_STDARGS__
error (char *format, ...)
#else
error (va_alist)
     va_dcl
#endif
{
  va_list ap;
#ifdef __USE_STDARGS__
  va_start(ap, format);
#else
  char *format;
  va_start (ap);
  format = va_arg (ap, char *);
#endif

  fprintf (stdout, "%s: ", programname);
  vfprintf (stdout, format, ap);
  va_end (ap);

  fprintf (stdout, "\n");
}

#else /* not HAVE_VPRINTF */

void
fatal (msg, arg1, arg2)
     char *msg; char *arg1; char *arg2;
{
  error (msg, arg1, arg2);
  delete_temp_files ();
  exit (1);
}

void
error (msg, arg1, arg2)
     char *msg; char *arg1; char *arg2;
{
  fprintf (stderr, "%s: ", programname);
  fprintf (stderr, msg, arg1, arg2);
  fprintf (stderr, "\n");
}

#endif /* not HAVE_VPRINTF */


static void
validate_all_switches (void)
{
  struct compiler *comp;
  register char *p;
  register char c;
  struct spec_list *spec;

  for (comp = compilers; comp->spec[0]; comp++)
    {
      int i;
      for (i = 0; i < sizeof comp->spec / sizeof comp->spec[0] && comp->spec[i]; i++)
	{
	  p = comp->spec[i];
	  while ((c = *p++))
	    if (c == '%' && *p == '{')
	      /* We have a switch spec.  */
	      validate_switches (p + 1);
	}
    }

  /* look through the linked list of extra specs read from the specs file */
  for (spec = specs; spec ; spec = spec->next)
    {
      p = spec->spec;
      while ((c = *p++))
	if (c == '%' && *p == '{')
	  /* We have a switch spec.  */
	  validate_switches (p + 1);
    }

  p = link_command_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  /* Now notice switches mentioned in the machine-specific specs.  */

  p = asm_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = asm_final_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = cpp_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = signed_char_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = cc1_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = cc1plus_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = link_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = lib_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);

  p = startfile_spec;
  while ((c = *p++))
    if (c == '%' && *p == '{')
      /* We have a switch spec.  */
      validate_switches (p + 1);
}

/* Look at the switch-name that comes after START
   and mark as valid all supplied switches that match it.  */

static void
validate_switches (char *start)
{
  register char *p = start;
  char *filter;
  register int i;
  int suffix = 0;

  if (*p == '|')
    ++p;

  if (*p == '!')
    ++p;

  if (*p == '.')
    suffix = 1, ++p;

  filter = p;
  while (*p != ':' && *p != '}') p++;

  if (suffix)
    ;
  else if (p[-1] == '*')
    {
      /* Mark all matching switches as valid.  */
      --p;
      for (i = 0; i < n_switches; i++)
	if (!strncmp (switches[i].part1, filter, p - filter))
	  switches[i].valid = 1;
    }
  else
    {
      /* Mark an exact matching switch as valid.  */
      for (i = 0; i < n_switches; i++)
	{
	  if (!strncmp (switches[i].part1, filter, p - filter)
	      && switches[i].part1[p - filter] == 0)
	    switches[i].valid = 1;
	}
    }
}


#ifdef __WATCOMC__
char *mktemp(char *template)
{
  char *p, 
  *choice="ABCDEFGHIJKLMNOPQRSTUVXYZ0123456789";
  while(p=strchr(template,'X'))
    {
      long r=(rand() * strlen(choice))/RAND_MAX;
      *p=choice[r];
    }
  return template;
}
#endif

static int
is_bad_switch(char *p)
{
  switch (*p)
    {

      /* switches which can have x number of characters after */

    case 'D':  /* Define macro name */
    case 'f':  /* -f switches for the compiler */
    case 'I':  /* Include file directories */
    case 'l':  /* Library names for linking */
    case 'L':  /* Switch for Library paths */
    case 'm':  /* -m machine dependent switches for compiler */
    case 'p':  /* Cpp -pedantic and -pedantic-errors types */
    case 't':  /* Cpp directives such as -trigraphs or -traditional */
    case 'U':  /* Undefine macro name */
    case 'w':  /* warning switches */
    case 'W':  /* warning switches */
#ifdef DEBUG_DUMPS
    case 'd':
#endif
      return FALSE;

      /* Switches which must not have chars after their names */

    case 'c':  /* Do not link, stop after assemble */
    case 'E':  /* Preprocess only */
    case 'g':  /* Debug info on */
    case 'o':  /* Define output file name */
    case 'P':  /* C preprocessor switch */
      return p[1] != '\0';

      /* Special case switches */

    case 'C':  
      return (strcmp(p, "C") != 0       /* C preprocessor switch */
#ifdef DSP21K
	      && strcmp(p, "CO") != 0   /* Code Optimizer 'gco' */
#endif
	      );

    case 'S':  /* Do not assemble, stop after compile*/
      return (strcmp(p, "S") != 0
	      && strcmp(p, "S1") != 0
	      && strcmp(p, "SO") != 0);

    case 'a':  /* check for switches starting with 'a' */
      return (strcmp(p, "ansi") != 0
	      && strcmp(p, "arch") != 0);

    case 'i':  /* check for switches starting with 'i' */
      return (strcmp(p, "imacros") != 0
	      && strcmp(p, "include") != 0);

    case 'n': /* switches that start with 'n' */
      return (strcmp (p, "nomem") != 0
	      && strcmp (p, "nostdinc") != 0
#ifdef DSP21K
	      && strcmp (p, "no-CO") != 0
#endif
	      );

    case 'O':  /* Optmizer on */
      return (strcmp (p, "O") != 0
	      && strcmp (p, "O2") != 0
	      && strcmp (p, "O3") != 0
	      && strcmp (p, "O4") != 0);

    case 'r':  /* runhdr switch */
      return (strcmp (p, "runhdr") != 0);

    case 's':  /* -save-temps switch */
      return (strcmp (p, "save-temps") != 0);

    case 'u':  /* -undef switch for Cpp allowing no predefines*/
      return (strcmp (p, "undef") != 0);

    default:
      return TRUE;
    }

}

#ifdef CRIPPLE
static int
is_suppressed_option (char *option)
{
  switch (option[0])
    {
    case 'd':
    case 'i':
    case 'f':
    case 'm':
    case 'A':
    case 'M':
    case 'O':
    case 'W':
      /* These all take any argument in the full version. */
      return TRUE;

    case 'w':
    case 'H':
    case 'Q':
      /* These are single-character options in the full version. */
      return option[1] == '\0';
	
    case 'a':
      return (strcmp (option, "a") == 0
	      || strcmp (option, "arch") == 0
	      || strcmp (option, "ansi") == 0
	      || strncmp (option, "aux-info", 8) == 0); /* aux-info* */

    case 'n':
      return (strcmp (option, "no-CO") == 0
	      || strcmp (option, "nostdlib") == 0
	      || strncmp (option, "nostdinc", 8) == 0); /* nostdinc* */

    case 'p':
      return (strcmp (option, "p") == 0 || strcmp (option, "pg") == 0
	      || strcmp (option, "pipe") == 0
	      || strcmp (option, "pedantic") == 0);

    case 'r':
      return (strcmp (option, "runhdr") == 0);

    case 't':
      return (strcmp (option, "traditional") == 0
	      || strcmp (option, "trigraphs") == 0
	      || strcmp (option, "traditional-cpp") == 0);

    case 'u':
      return (strcmp (option, "undef") == 0);

    case 'C':
      return (strcmp (option, "CO") == 0);

    case 'S':
      return (strcmp (option, "S1") == 0 || strcmp (option, "SO") == 0);

    default:
      return FALSE;
    }
}

static int
is_default_option (char *option)
{
    return (strcmp (option, "nomem") == 0);
}
#endif
