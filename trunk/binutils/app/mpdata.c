/* @(#)mpdata.c 2.2 5/5/92 2 */

#include        <stdio.h>

#ifdef  MSDOS
#undef  stderr
#define stderr  stdout
#endif

#include "app.h"
#include        "mpdefs.h"

/*
 * Directives
 */

int             warning_flag = TRUE;    /* whether or not to print warnings */
int             debug_include_list = FALSE;

char           *dir_tbl[] = {   /* Directive name table */
    "define",
    "include",
    "undef",
    "if",
    "ifdef",
    "ifndef",
    "else",
    "endif",
    "error",
    "line",
    "pragma",
    "elif",
    "",
    (char *) NIL,

};

struct pragmastruct pragma_list[] = {
    "ADSP2100", TRUE,
    "adsp2100", TRUE,
    (char *) 0, FALSE,
};

int             dir_type[] = {  /* Directive mnemonics for 'switch' */
    DEF,
    INCL,
    UNDEF,
    IF,
    IFD,
    IFN,
    ELSE,
    ENDIF,
    ERR,
    LINE,
    PRAGMA,
    ELIF,
    NOTA,
};

int             err_cnt = 0;    /* Error counter                         */


int             nocomp = TRUE;  /* No <SOH> stuff */



/*
 * I/O-related stuff
 */

FILE           *curinfil = (FILE *) NULL;       /* Pointer to current input
						 * file  */
FILE           *outfil = (FILE *) NULL; /* Pointer to output file                */

/*
 * #include
 */

FILE           *inclpdl[INCDEPTH];

struct stack    inclstk = {
    INCDEPTH,
    0,
    (char **) inclpdl
};
int             inclvl = 0;     /* #include nesting level                */
int             inclflag = FALSE;       /* Ugly fix for extraneous <SOH>
					 * before        */

/* #include line itself                  */

char           *srchlist[INCLUDE_PATH_MAX] = {
    "",                         /* This must be first!                   */
    (char *) NIL,
};

/*
 * Conditional compilation
 */

int             ifpdl[IFDEPTH];
int             eliflogic[IFDEPTH];
int             elsecount[IFDEPTH];
struct stacki   ifstack = {
    IFDEPTH,
    0,
    eliflogic,
    elsecount,
    ifpdl
};
int             father = TRUE;  /* Previous level expansion flag         */
int             self = TRUE;    /* Current level expansion flag          */

/*
 * Input line processing related data structures
 */

char            line[LINESZ];
int             lineno[INCDEPTH];
int             in_com = FALSE; /* TRUE if in a comment          */
char            fname[INCDEPTH][NAME_BUFF_SIZE] = {0};

/* Include file names            */
int             pageno[INCDEPTH];       /* Include page number           */
int             linefeeds = 1;  /* Count line feeds as output    */

struct sym     *lineloc;        /* __LINE__ macro */
struct sym     *fileloc;        /* __FILE__ macro */
struct sym     *stdcloc;        /* __STDC__ macro */
struct sym     *timeloc;        /* __TIME__ macro */
struct sym     *dateloc;        /* __DATE__ macro */
struct sym     *definedloc;     /* defined() macro */



char           *lp = NULL;              /* Line pointer                          */

/*
 * Macro definition and expansion data structures
 */

char            def_ala[ALASIZE][MAXIDLEN + 1];

/* Definition ala                        */
char           *call_ala[ALASIZE];      /* Dynamically-allocatable call ala      */
char           *callpdl[DEFDEPTH];

struct stack    callstk = {
    DEFDEPTH,
    0,
    callpdl
};

/* Call stack - used to process nested   */
/* macro calls                           */
struct sym     *refpdl[DEFDEPTH];
struct stack    refstack = {
    DEFDEPTH,
    0,
    (char **) refpdl
};

/* Stack for referenced symbols          */

/*
 * For longjmp
 */
jmp_buf         jump_env;


int include_system_list = 1;
