/* @(#)mpsym.c  2.4 5/5/92 2 */

/*
 * Symbol table manipulation routines for mp.  The general symbol table
 * organization is linked list representation with bucketing to decrease
 * search time.  The buckets are determined by the first character of the
 * symbol (and thus 54 buckets are needed).
 * 
 * If this implementation turns out to be a bottleneck, some other type of
 * bucket mapping function may be defined to reduce crowding.  If this fails
 * to be adequate, then a completely different symbol table mechanism could
 * be tried (but remember that we need the ability to delete symbols, which
 * makes a simple hasing technique untenable).
 */

/*
 * Include global constant definitions
 */

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


struct sym     *sym_bkts[54];   /* Symbol table buckets */

/*
 * Symbol table lookup
 */

struct sym     *
lookup(symbol)
    register char  *symbol;
{
    register struct sym *ptr = NIL;

	if( !*symbol )
		return( ptr );

    for (ptr = sym_bkts[bkt_map(*symbol)]; ptr != (struct sym *) NIL;
	    ptr = ptr->next)
	if (strcmp(symbol, ptr->name) == 0)
	    break;
    return (ptr);
}

/*
 * Enter a symbol into the symbol table
 */

struct sym     *
sym_enter(symbol, nargs, defptr)
    char           *symbol,
		   *defptr;
    int             nargs;
{
    register int    bucket;
    register struct sym *p,
		   *tp;

    bucket = bkt_map(*symbol);
    if ((p = lookup(symbol)) != (struct sym *) NIL)
	my_free(p->defptr);     /* free old definition buffer    */
    else {

	/*
	 * Allocate a new symbol table node
	 */
	p = (struct sym *) get_mem(sizeof *sym_bkts[0]);
	p->next = (struct sym *) NIL;
	if (sym_bkts[bucket] == (struct sym *) NIL)
	    sym_bkts[bucket] = p;
	else {
	    for (tp = sym_bkts[bucket];
		    tp->next != (struct sym *) NIL;
		    tp = tp->next);
	    tp->next = p;
	}
    }
    strcpy(p->name, symbol);    /* Copy name into entry          */
    p->nargs = nargs;           /* Set number of parameters      */
    p->defptr = defptr;         /* Set ptr to definition block   */
    p->ref = 0;                 /* Clear reference flag          */
    return (p);
}

/*
 * Delete a symbol from the symbol table
 */

int 
sym_del(symbol)
    char           *symbol;
{
    register struct sym *p,
		   *tp;
    register int    bucket;



    bucket = bkt_map(*symbol);
    for (p = sym_bkts[bucket]; p != (struct sym *) NIL; tp = p, p = p->next)
	if (strcmp(symbol, p->name) == 0) {

	    if (reserved_macro(p)) {
		printerr("not allowed to undefine %s", p->name);
		return (OK);    /* avoid two error messages */
	    }

	    my_free(p->defptr); /* Free old definition   */
	    if (p == sym_bkts[bucket])
		sym_bkts[bucket] = p->next;
	    else
		tp->next = p->next;
	    my_free(p);         /* And release node      */
	    return (OK);
	}
    return (ERROR);
}

/*
 * Map an alphabetic character into a symbol table bucket index
 */

int 
bkt_map(register char c)
{
    if (!c_alpha(c))
	screech("Invalid call to bkt_map()");
    if (c == '_')
	return (52);
    else if (c == '$')
	return (53);
    else if (c >= 'A' && c <= 'Z')
	return (c - 'A');
    else
	return (c - ('a' - 26));
}

/* Dump contents of symbol table */

void sym_print()
{
    register int    i;
    register struct sym *p;
    register char  *cp;

    printf("\n\nSymbol table dump follows:\n");
    for (i = 0; i < 54; i++)
	for (p = sym_bkts[i]; p != (struct sym *) NIL; p = p->next) {
	    printf("%s<%d,%d>=", p->name, p->nargs, p->ref);
	    for (cp = p->defptr; *cp != EOS; cp++)
		if (*cp & 0200)
		    printf("#%c", (*cp & 0177) + '0');
		else
		    printf("%c", *cp);
	    printf("\n");
	}
    return;
}

/* Initialize symbol table buckets */

void sym_init()
{
    char           *tp1,
		   *tp2;

    register int    i;

    for (i = 0; i < 54; i++)
	sym_bkts[i] = (struct sym *) NIL;

    tp1 = get_local_time(); /*EK*/

    tp2 = get_mem(14);
    tp2[0] = '\"';
    tp2[1] = tp1[4];
    tp2[2] = tp1[5];
    tp2[3] = tp1[6];
    tp2[4] = tp1[7];
    if (tp2[8] == '0')
	tp2[5] = ' ';
    else
	tp2[5] = tp1[8];
    tp2[6] = tp1[9];
    tp2[7] = ' ';
    tp2[8] = tp1[20];
    tp2[9] = tp1[21];
    tp2[10] = tp1[22];
    tp2[11] = tp1[23];
    tp2[12] = '\"';
    tp2[13] = '\0';
    dateloc = sym_enter("__DATE__", 0, tp2);

    tp2 = get_mem(11);
    for (i = 1; i < 9; i++)
	tp2[i] = tp1[i + 10];
    tp2[0] = tp2[9] = '\"';
    tp2[10] = '\0';
    timeloc = sym_enter("__TIME__", 0, tp2);

    tp1 = get_mem(LINE_LOC_LEN);
    lineloc = sym_enter("__LINE__", 0, tp1);

    stdcloc = sym_enter("__STDC__", 0, "1");

    fileloc = sym_enter("__FILE__", 0, tp1 = get_mem(NAME_BUFF_SIZE + 3));
    *tp1 = '"';

    definedloc = sym_enter("defined", 0, tp1 = get_mem(4));

    sym_enter("ADI", 0, "1");
    sym_enter("adi", 0, "1");

#ifdef  MSDOS
    sym_enter("msdos", 0, "1");
    sym_enter("MSDOS", 0, "1");
#endif

#ifdef sun
    sym_enter("sun", 0, "1");
    sym_enter("SUN", 0, "1");
    sym_enter("mc68000", 0, "1");
#endif

#if !defined MSDOS && !defined WIN31 /*EK*/
    sym_enter("unix", 0, "1");
    sym_enter("UNIX", 0, "1");
#endif

    return;
}
