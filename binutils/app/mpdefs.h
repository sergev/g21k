/* @(#)mpdefs.h 2.1 11/9/92 2 */
/* Standard C library routines */

#ifndef sun
#include        <stdlib.h>
#endif

#ifdef  MSDOS
#include        <malloc.h>
#include        <ctype.h>
#endif
#include        <setjmp.h>
#include        <string.h>

/* Compile-time constants */

/* #define      UNIMATION */    /* Define to enable page numbers        */

extern  jmp_buf jump_env;

#define stdio

#define TRUE    1
#define FALSE   0
#ifndef EOF
#define EOF     -1
#endif
#define NAME_BUFF_SIZE  128
#define ERROR   -1
#define OK      0

	/* directive kind defintions */
#define DEF     0
#define INCL    1
#define UNDEF   2
#define IF      3
#define IFD     4
#define IFN     5
#define ELSE    6
#define ENDIF   7
#define ERR     8
#define LINE    9
#define PRAGMA  10
#define ELIF    11
#define NOTA    12

#define EOS     '\0'
#define EQL     0
#define NEQ     1
#define LSS     2
#define LEQ     3
#define GTR     4
#define GEQ     5
#define NIL     0
#define SOH     001
#define OCTAL   8
#define DECIMAL 10
#ifndef stdio
#define STDIN   0               /* Standard input file descriptor */
#define STDOUT  1               /* Standard output */
#define STDERR  2               /* Standard error output */
#endif
#define LINESZ  1024            /* Size of a line */
#define MAXIDLEN 31             /* Maximum length of an identifier */
#define SRCHMAX 2               /* #include search list size */
#define ALASIZE 31              /* Maximum number of macro parameters */
#define INCDEPTH 40             /* Maximum #include depth */
#define IFDEPTH 100             /* Maximum #if nesting depth */
#define DEFDEPTH 25             /* Maximum nested definition level */
#define EXPBUFSZ 300            /* Maximum #if expression buffer size */
#define CATMAX  50              /* Size of 'concat' buffer */
#define MAXARGSZ 100            /* Maximum length of an actual parameter */
#define INCLUDE_PATH_MAX 20     /* Maximum number of directory paths for MSDOS*/

#include        <ctype.h>

extern  warning_flag;           /* whether or not to print warnings */
extern  debug_include_list;     /* print out list searched */


/* Structure defintions */


struct stacki {
	int spcnt;      /* Space count */
	int usecnt;     /* Use count */
	int *eliflogic; /* elif logic to date */
	int *elsecount; /* only one else allowed */
			/* infinite elif's as long as no else has appeared */
	int *stackptr;  /* Pointer into stack buffer */
};

struct stack {
	int spcnt;      /* Space count */
	int usecnt;     /* Use count */
	char **stackptr; /* Pointer into stack buffer */
};

struct sym {
	char name[MAXIDLEN+1];  /* Name buffer */
	int nargs;      /* Number of arguments */
	int ref;        /* Reference marker to stop recursive definitions */
	char *defptr;   /* Pointer to definition block */
	struct sym *next;       /* Forward link for bucketing */
};

/*
 * Directives
 */

extern  char *dir_tbl[];

struct  pragmastruct {
    char    *pragma_string;
    int     pragma_match;
};
extern  struct  pragmastruct pragma_list[];
extern  int dir_type[];
extern  int err_cnt;

extern  int nocomp;

extern  char syn_err[];

/*
 * I/O-related stuff
 */

extern  FILE    *curinfil;
extern  FILE    *outfil;

extern  FILE    *inclpdl[INCDEPTH];

extern  struct stack inclstk;
extern  int inclvl;
extern  int inclflag;
				/* #include line itself                 */
extern  char *srchlist[];

/*
 * Conditional compilation
 */

extern  int ifpdl[IFDEPTH];
extern  struct stacki ifstack;
extern  int father;
extern  int self;

/*
 * Input line processing related data structures
 */

extern  char    line[LINESZ];
extern  int     lineno[INCDEPTH];
extern  int     in_com;                 /* TRUE if in a comment         */
extern  char    fname[INCDEPTH][NAME_BUFF_SIZE];
					/* Include file names           */
extern  int     linefeeds;              /* Count line feeds as output   */

#define LINE_LOC_LEN    16
extern  struct  sym     *lineloc;       /* __LINE__ macro */
extern  struct  sym     *fileloc;       /* __FILE__ macro */
extern  struct  sym     *stdcloc;       /* __STDC__ macro */
extern  struct  sym     *timeloc;       /* __TIME__ macro */
extern  struct  sym     *dateloc;       /* __DATE__ macro */
extern  struct  sym     *definedloc;    /* defined() macro */

extern  char *lp;                       /* Line pointer                         */

/*
 * Macro definition and expansion data structures
 */

extern  char def_ala[ALASIZE][MAXIDLEN+1];
				/* Definition ala                       */
extern  char *call_ala[ALASIZE];/* Dynamically-allocatable call ala     */
extern  char *callpdl[DEFDEPTH];

extern  struct stack callstk;
				/* Call stack - used to process nested  */
				/* macro calls                          */
extern  struct sym *refpdl[DEFDEPTH];
extern  struct stack refstack;


/*      In file MPUTIL.C */
extern  char    *get_mem();
extern  char    *skipblnk();
extern  char    *concat();
extern  char    *skipcom();
extern  char    *get_id();
extern  char    *pop();
extern  char    *skipq();

/*      In file MP.C */
extern  char    *actual();
extern  char    *get_def();
extern  char    *is_defined();
extern  char    *get_buffer();
extern  char    *release_buffer();



/*      In file MPSYM.c */
extern  struct sym *lookup();
extern  struct sym *sym_enter();


extern  include_system_list;        /* where in array to start searcg for #include <>*/
