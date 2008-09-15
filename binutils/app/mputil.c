/* @(#)mputil.c 2.4 8/29/92 2 */

/*
 * Utility routines for 'mp', the C macro preprocessor.
 */

#ifdef  MSDOS
#include <ctype.h>
#endif

#include <stdio.h>
#include <stddef.h>


#ifdef  MSDOS
#undef  stderr
#define stderr  stdout
#endif

#include "app.h"
#include "mpdefs.h"
#include "util.h"
#include "protos.h"

/*
 * Print error message on standard output
 */

void printerr(msg, oa)
    char           *msg;
    char           *oa;         /* optional argument */

{
    err_cnt++;
    fprintf(stderr, "%%PPERROR [%d] line %d %s\n\t",
	    err_cnt, lineno[inclvl], fname[inclvl]);
    fprintf(stderr, msg, oa);
    fprintf(stderr, "\n");
    return;
}

/*
 * Print warning message on standard output
 */

void printwarn(msg, oa)
    char           *msg;
    char           *oa;         /* optional argument */

{
    if (!warning_flag)
	return;

    fprintf(stderr, "%%PPWARN line %d %s\n\t",
	    lineno[inclvl], fname[inclvl]);
    fprintf(stderr, msg, oa);
    fprintf(stderr, "\n");
    return;
}

/*
 * Internal bug catcher
 */

void screech(msg)
    char           *msg;
{
    fprintf(stderr, "\nPreprocessor cannot continue due to previous error(s)\n");
    msg;
    exit(1);
}

/*
 * Skip a string of blanks and tabs
 */

char           *
skipblnk(ptr)
    char           *ptr;
{
    char            ch;

    while ((ch = *ptr) == ' ' || ch == '\t' || ch == '\n')
	ptr++;
    return (ptr);
}



/*
 * get_mem - Allocate 'nbytes' of dynamic memory.  If none is available,
 * report an error and exit with nonzero status; otherwise return pointer to
 * the node.
 */

char
	       *
get_mem(nbytes)
    int             nbytes;
{
    char           *p;

    if ((p = (char *) my_malloc((long)nbytes)) == NULL) {
	printerr("No dynamic memory available", "");
	exit(2);
    }
    else
	return (p);
}


/*
 * Pop an item off of the if/elif/else stack
 */

char           *
pop(stkaddr)
    register struct stack *stkaddr;
{
    if (empty(stkaddr))
	screech("Stack underflow!");
    stkaddr->spcnt++, stkaddr->usecnt--;
    return ((char *) *--stkaddr->stackptr);
}

/*
 * Push an item on the stack specified
 */

int
push(item, stkaddr)
    char           *item;
    register struct stack *stkaddr;
{
    if (stkaddr->spcnt == 0)
	return (ERROR);
    stkaddr->spcnt--, stkaddr->usecnt++;
    *stkaddr->stackptr++ = item;
    return (OK);
}

/*
 * Push an item on the if/elif/else stack specified
 */

int
pushi(item, stkaddr)
    int             item;
    register struct stacki *stkaddr;
{
    if (stkaddr->spcnt == 0)
	return (ERROR);
    stkaddr->spcnt--;
    stkaddr->usecnt++;
    *++stkaddr->eliflogic = FALSE;
    *++stkaddr->elsecount = 0;
    *stkaddr->stackptr++ = item;
    return (OK);
}

/*
 * Pop an item off of the if/elif/else stack
 */

int
popi(stkaddr)
    struct stacki  *stkaddr;
{
    if (empty_ifstack(stkaddr))
	screech("Stack underflow!");
    stkaddr->spcnt++;
    stkaddr->usecnt--;
    --stkaddr->eliflogic;
    --stkaddr->elsecount;
    return (*--stkaddr->stackptr);
}



/*
 * Mark the fact that an else has occured (only called after checking for
 * empty)
 */

check_else_count(stkaddr)
    struct stacki  *stkaddr;
{
    int            *pi;
    int             i;


    pi = stkaddr->elsecount;
    i = *pi;
    *pi = *pi + 1;

    if (i)
	printerr("#endif expected", "");
    return (i);

}

/*
 * Update the elif logic   (only called if a true condition has occured from
 * either an #if or an #elif)
 */
void update_elif_logic(logic, stkaddr)
    int             logic;
    struct stacki  *stkaddr;
{
    *stkaddr->eliflogic |= logic;
}

/*
 */
get_elif_logic(stkaddr)
    struct stacki  *stkaddr;
{
    return (*stkaddr->eliflogic);
}



/*
 * Test whether a stack is empty
 */

int
empty(stkaddr)
    struct stack   *stkaddr;
{
    return (stkaddr->usecnt == 0);
}

/*
 * Test whether the if/elif/else stack is empty
 */

int
empty_ifstack(stkaddr)
    struct stacki  *stkaddr;
{
    return (stkaddr->usecnt == 0);
}

/*
 * skipq - Routine to return pointer to next unquoted character starting from
 * "src". All quoted text is flushed to either a text buffer or the output
 * file, as determined by "dst".  If "dst" == NIL, quoted text is written to
 * "outfil"; if "dst" is non-NIL, then any bypassed characters are moved to
 * the buffer specified.  The parameter "dstmax" specified the last address
 * into which a character may be deposited. All comments are also flushed
 * here.  For multi-line comments, a pointer to the end of line is returned.
 * When end of comment is encountered, scanning is resumed immediately
 * afterward.
 */

int             unmagic = FALSE;

char
	       *
skipq(src, dst, dstmax)
    register char  *src;
    char          **dst,
		   *dstmax;
{
    register char   quote;
    register int    full;

    if (in_com)
	src = skipcom(src);

    switch (*src) {
	case '\\':
	    unmagic = !unmagic;
	    return (src);
	case '\'':
	case '"':
	    if (unmagic)
		break;
	    myputch(quote = *src++, dst, dstmax);
	    for (;;) {
		if (*src == EOS) {
		    printerr("Unterminated quoted string", "");
		    break;
		}
		if (*src == '\\') {
		    myputch(*src++, dst, dstmax);
		    unmagic = !unmagic;
		    continue;
		}
		full = myputch(*src, dst, dstmax);
		if (*src++ == quote && !unmagic)
		    break;
		unmagic = FALSE;
	    }
	    return ((full == ERROR) ? (char *) ERROR : src);
	case '/':
	    if (*(src + 1) == '*') {
		in_com = TRUE;
		return (skipcom(src + 2));
	    }
	    else
		break;
    }
    unmagic = FALSE;
    return (src);
}


/*
 * skipcom - Routine to skip past any comment characters, until either end of
 * comment or end of line.  If end of comment is encountered, in_com is set
 * to FALSE.
 */

char
	       *
skipcom(src)
    register char  *src;
{
    while ((*src != '*' || *(src + 1) != '/')) {
	if (*src == EOS)
	    return (src);
	if ((*src == '/' && *(src + 1) == '*'))
	    printwarn("nested comments", "");
	src++;
    }
    src += 2;
    in_com = FALSE;
    return (src);
}

/*
 * Routine to emit a flushed character to either the current output file or
 * to the buffer whose address is specified.  Returns ERROR if the buffer is
 * overrun.
 */

int
myputch(register char ch, char **where, char *limit)
{

    if (where == (char **) NIL) {
	if ((ch == '\n') & !nocomp) {
	    if (linefeeds++ == 0)
		putc(ch, outfil);
	}
	else {
	    if (linefeeds > 1) {
		if (linefeeds == 2)
		    putc('\n', outfil);
		else {

#ifdef UNIMATION

#ifdef PREPRE
		    fprintf
			(outfil,
			 "#define pg #%d p.%d %s\npg\n",
#else
		    fprintf(outfil, "#%d p.%d %s\n",
#endif

			    lineno[inclvl],
			    pageno[inclvl],
			    fname[inclvl]);
#else
		    fprintf(outfil, "#%d %s\n",
			    lineno[inclvl], fname[inclvl]);
#endif
		}
	    }
	    linefeeds = 0;
	    putc(ch, outfil);
	}
	return (OK);
    }
    else {
	if (*where > limit)
	    return (ERROR);
	else
	    *(*where)++ = ch;
	return (OK);
    }
}

/*
 * Routine to release entries in the call_ala.  The last entry to free is
 * specified by 'last'.
 */

void rlse_ala(last)
    register int    last;
{
    register int    i;

    for (i = 0; i <= last; i++)
	my_free(call_ala[i]);
    return;
}


/*
 * get_id - Get an identifier (1-MAXIDLEN alphanumerics + '_', must start
 * with an alpha).  Note that alpha's include '_' and (in Dec-land) '$'.
 */

char
	       *
get_id(src, dst)
    register char  *src,
		   *dst;
{
    register int    i;

    /*
     * if( !isalpha(*src) && *src != '_' && *src != '$') { error_message =
     * errmsg = (char *)my_malloc((long)(strlen(src)+1)); while( *src && *src != ' ')
     * errmsg++ = *src++; errmsg = '\0'; printerr( "Idenitifier \"%s\" does
     * not begin with a valid character", error_message); return( src ); }
     */

    for (i = 0; i < MAXIDLEN && c_alnum(*src); i++) {
	*dst++ = *src++;
    }
    while (c_alnum(*src))
	src++;

    *dst = EOS;
    return (src);
}


/*
 * return(TRUE if c is a C-language alpha (A-Z,a-z, "_", "$")
 */
c_alpha(char c)
{
    if (c < 0)
	return (FALSE);
    return (isalpha(c) || c == '_' || c == '$');
}

c_alnum(c)
    register int    c;

/*
 * Return TRUE if c is a C-language alpha (A-Z, a-z, "_", "$") or a digit.
 */
{
    return (isalnum(c) || c == '_' || c == '$');
}


/*
 * Check for reserved macros
 */

reserved_macro(stp)
    struct sym     *stp;
{

    if (stp == lineloc || stp == fileloc || stp == timeloc || stp == definedloc ||
	    stp == dateloc || stp == stdcloc)
	return (TRUE);
    else
	return (FALSE);
}

/*
 * Routine to push the contents of the call ala onto the stack.
 */

void push_ala(numargs)
    register int    numargs;
{
    register int    i;

    for (i = 0; i < numargs; i++)
	if (push(call_ala[i], &callstk) == ERROR)
	    exp_err("Maximum call nesting depth exceeded", i);
    return;
}

/*
 * Routine to pop the contents of the call ala from the stack.
 */

void pop_ala(numargs)
    register int    numargs;
{
    register int    i;

    for (i = numargs - 1; i >= 0; i--)
	call_ala[i] = pop(&callstk);
    return;
}


/*
 * Routine to add command line includes to "include search list"
 */

void include_init_command_line(include_path)
    char           *include_path;       /* command line include file path */
{
    char           *pc,
		   *pc1;
    int             i;          /* srchlist array index */

    i = 1;

    if (include_path) {

	pc = include_path;

	while (*pc && (i < INCLUDE_PATH_MAX)) {

	    srchlist[i++] = pc;

	    pc1 = pc;
	    while (*pc1 && *pc1 != ';')
		pc1++;
	    if (*pc1 == ';')
		*pc1++ = '\0';

	    pc = pc1;
	}
    }

    srchlist[i] = (char *) NIL;
}

char *add_to_search_list( char *pc, int index)
{
    char   *pc1, *pc2;
    int     num;
	
    pc1 = pc;
    num = 0;
    while( *pc1 && *pc1 != ';' )
    {
	pc1++;
	num++;
    }
	
    pc2 = pc1 - 1;
	
    if( *pc1 == ';' )
	*pc1++ = '\0';

    srchlist[index] = get_mem( num + 3 );
    strcpy( srchlist[index], pc);
	
	
    /* Make sure the include string has the trailing '/' (unix) '\' (DOS) */
    while( *pc2 == ' ' || *pc2 == '\t')
	pc2--;

    if( *pc2 != FILENAME_SEPARATOR)
	strcat( srchlist[index], FILENAME_SEPARATOR_STR);
	
    return pc1;
}

/*
 * Routine to add include path as specified by ADII environment variable to
 * the "include search directory.
 */
void include_init()
{
    char           *pc, *pc1;
    int             i, j;               /* srchlist array index */
    char           *save_env;
	
#ifdef sun
    extern char    *getenv();
#endif
	
    i = 1;
    while (srchlist[i])
	i++;

    pc = getenv( "ADII_21K");
	
    if (pc) 
    {
	save_env = (char *) my_malloc((long)(strlen(pc) + 1));
	strcpy(save_env, pc);
	pc = save_env;
	
	while (*pc && (i < INCLUDE_PATH_MAX)) 
	{
	    pc = add_to_search_list( pc, i++);
	}
    }
    else
    {
	pc = get_adi_dsp(); /*EK*/
	if( pc )
	{
	    j = strlen(pc)-1;
	    save_env = (char *)malloc( j + 25 );
	    strcpy( save_env, pc );
	    if( *(save_env+j) == FILENAME_SEPARATOR )
		*(save_env+j) = '\0';
#ifdef MSDOS
	    strcat( save_env, "\\include\\21k\\");
#else /*EK*/
	    strcat( save_env, "/include/21k/");
#endif
	    pc = save_env;
		
	    pc = add_to_search_list( pc, i++);
	}
    }

	
#ifndef MSDOS
    srchlist[i++] = "/usr/include/";
#endif
	
    srchlist[i] = (char *) NIL;
}


/*
 * Process the  defined()       macro
 */
char           *
is_defined(src, ch)
    char           *src;
    char           *ch;
{
    char            name[MAXIDLEN + 1];
    int             right_paren = FALSE;

    src = skipblnk(src);
    if (*src == '(') {
	right_paren = TRUE;
	src++;
    }

    src = skipblnk(src);
    src = get_id(src, name);
    if (lookup(name) == (struct sym *) NIL)
	*ch = '0';
    else
	*ch = '1';

    if (right_paren)
	if (*src == ')')
	    src++;
	else
	    printerr("expected a \")\"", "");

    return src;
}
