
/**************************************************************************/
/*** Lisp-like cons, except that the "atom" (str) is actually           ***/
/*** the part of the cons itself. It is not 1 character, but a variable ***/
/*** number malloc'ed by in read_arg_list. CDR is an alias for "next"   ***/
/*** (lisp again...), while CAR is the string itself (... and again)    ***/
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atfile.h"

extern void exit(int);

static CONS *allocated_pool = 0;

static char **NewArgv = 0;
static CONS *read_arg_list(char *);
static CONS *nconc ( CONS *, CONS *);

/*********************************************************************/
/* This routine must be the first thing to call from a "main"        */
/* before any argument processing. This function, which has no effect*/ 
/* if no argument starts with a @-sign, is typically used            */
/* in the following manner: 					     */
/*-------------------------------------------------------------------*/
/* main(argc, argv)					*/
/* char argc; char *argv[];				*/
/* {							*/
/*     process_atfiles(&argc, &argv);			*/
/* }							*/
/********************************************************/

void
process_atfiles(pargc, pargv)
    int *pargc;
    char ***pargv;
{
    int an;
    for(an=0; an<*pargc; an++) {
	if (*(*pargv)[an] == '@') {
	    CONS *arglist = read_arg_list((*pargv)[an]+1);
	    int     atlen = atfile_get_length(arglist);
	    
	    *pargv=atfile_expand_argv(*pargc, *pargv, an, atlen);
	    atfile_copy_list(*pargv+an, arglist);
	    *pargc += atlen-1;
	    an     += atlen-1;
	}
    }
}


/** Read tokens from the file named FILE_NAME and **/
/** return a list of conses                       **/

static CONS *
read_arg_list(char *file_name)
{
    FILE *f;
    char buf[256];
    CONS *last_cons=0, *first_cons=0;
    if (!(f = fopen(file_name, "r"))) {
	fprintf(stderr,"Can't open @-file \"%s\"\n", file_name);
	exit(2);
    }
/* This is to allow spaces to be an acceptable component
   of a filename.  
   - cnoel&mmh   fgets (buf, 256, f) */
    while (fscanf (f,"%s", buf) == 1) {
	int len = strlen(buf);
	CONS *new_cons = (CONS *)malloc(sizeof (CONS)+len);
	new_cons->next=0;
	strcpy(CAR(new_cons), buf);
	if (last_cons) {
	    CDR(last_cons) = new_cons;
	    last_cons = new_cons;
	}
	else last_cons = first_cons = new_cons;
    }
    
    if (allocated_pool)
	nconc(allocated_pool,first_cons);
    else
	allocated_pool = first_cons;

    fclose(f);

    return(first_cons);
}

/** Return the lenght of LIST **/

int 
atfile_get_length(CONS *list)
{
    int len=0;
    CONS *c;
    for(c=list; c; c=CDR(c))
	len++;
    return len;
}

/*
 * Concatenate lists by altering them.
 * The last argument is not altered.
 * Returns n.
 */
static CONS *
nconc ( CONS *f, CONS *n)
{
    if (f)
    {
	for ( ; CDR(f) ; f = CDR(f)) ;
	return CDR(f) = n;
    }
    return n;
}
	

/*** Substitute GAP nulls for the element number AFTER in ***/
/*** the array ARGV of length ARGC ****/

char **
atfile_expand_argv(int argc, char *argv[], int after, int gap)
{
    int i, newsize=argc+gap-1;
    char **newargv;
    
    newargv = (char**)malloc(newsize * sizeof(char*));
    for(i=newsize-1;   i>=after+gap; i--) newargv[i] = argv[i-gap+1];
    for(i=after+gap-1; i>=after;     i--) newargv[i] = 0;
    for(i=after-1;     i>=0;         i--) newargv[i] = argv[i];

    if (NewArgv)		/* never free the system argv. */
	free(NewArgv);

    NewArgv = newargv;

    return(newargv);
}

/*** Copy list of conses ARGLIST to array of strings ARGV ***/

void 
atfile_copy_list(char *argv[], CONS *arglist)
{
    CONS *c=arglist;
    char *x;
    int i=0;
    for(c=arglist; c; c=CDR(c)) {
      argv[i++]=CAR(c);
    }
}

void 
free_argv()
{
    CONS *f;
    if (NewArgv)
	free(NewArgv);
    while(allocated_pool) {
	f = CDR(allocated_pool);
	free(allocated_pool);
	allocated_pool = f;
    }
}
