/* @(#)syms.c   1.13 5/13/91 1 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "a_out.h"
#include "action.h"
#include "input_file.h"
#include "input_section.h"
#include "output_section.h"
#include "process_args.h"
#include "linker_sym.h"
#include "linker_aux.h"
#include "main.h"
#include "error.h"
#include "pass1.h"
#include "coff_io.h"
#include "util.h"

#include "error_fp.h"
#include "list_fp.h"
#include "output_fp.h"
#include "syms_fp.h"

static int         extra_sym_cnt = 0;
static int         extra_aux_cnt = 0;
static LINKER_SYM  *extra_sym_ptr[MAX_EXTRA];
static LINKER_AUX  *extra_aux_ptr[MAX_EXTRA];
static LINKER_SYM  symbol_table[SYMBOL_TABLE_SIZE + 1];
static LINKER_AUX  auxillary_symbol_table[AUX_TABLE_SIZE + 1];
static long        hashtab[HASHSIZE];
static char        cur_x_fname[FILNMLEN];

long   total_aux=0l;        /* running total of number of auxillary entries */
long   total_symbols=0l;    /* running total of number of symbols */
long   unresolved=0l;       /* number of unresolved symbols */
long   max_relocatable_syms=0l;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      strsave                                                         *
*                                                                      *
*   Synopsis                                                           *
*      char *strsave( char *string )                                   * 
*                                                                      *
*   Description                                                        *
*      Save a string in memory. Lifted from K & R.                     * 
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            * 
*     mkc     6/22/89       created                  -----             * 
***********************************************************************/

char *strsave( register char *string )
{
    register char *p;
	
    if( (p = my_malloc( (long)(strlen(string) + 1))) )
    {
	 strcpy( p, string );
	 return( p );
    }
    else
	 FATAL_ERROR( "Memory allocation error." );
}
	
/***********************************************************************
*                                                                      *
*   Name                                                               *
*      hash                                                            *
*                                                                      *
*   Synopsis                                                           *
*      int hash( char *name )                                          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for calculating the hash value of   *
*   a symbol name.                                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/31/89       created                  -----             *
***********************************************************************/

int hash( register char *name )
{
    register unsigned int hash_value;

    hash_value = 0;
    while( *name )
	   hash_value = hash_value * 10 + *name++;

    return( (int)(hash_value % HASHSIZE) );

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      findsym                                                         *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_SYM *findsym( char *name )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for looking up a symbol. All        *
*   symbols that hash to the same value are connected in a one-way,    *
*   circular linked list. The most recently referenced symbol on the   *
*   chain is considered the head of the chain, and its id is stored in *
*   the hash table. Get the head of the collision chain, and walk down *
*   it looking for a match.                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/31/89       created                  -----             *
***********************************************************************/

LINKER_SYM *findsym( char *name )
{
    register long        acid;
	     int         hash_value;
	     long        head;             /* Head of collison chain      */
    register LINKER_SYM  *p;
	     char        *sym_name;
	     char        local_symname[SYMNMLEN+1];

    hash_value = hash( name );
    if( (acid = hashtab[hash_value]) == 0 )
	 return( NULL );
    head = acid;

    do
    {
	p = getsym( acid );
	if( 0L == p->obj_sym.n_zeroes )
	  sym_name = p->obj_sym._n._n_p._n_nptr;
	else 
	{
	  strncpy( local_symname, p->obj_sym._n._n_name, SYMNMLEN);
	  local_symname[SYMNMLEN] = '\0';
	  sym_name = local_symname;
	}

	if( !strncmp(sym_name, name, p->name_length) )
	{
	    hashtab[hash_value] = acid;
	    return( p );
	}

	acid = p->chain_id;
    }
    while( acid != head );

    return( NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      findaux                                                         *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_AUX *findaux( LINKER_SYM *sym, int index )               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for looking up a auxillary entry    *
*   for a linker symbol table entry.                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/31/89       created                  -----             *
***********************************************************************/

LINKER_AUX *findaux( LINKER_SYM *sym, int index )
{
    register LINKER_AUX *a; 
    register long       acid;

    acid = sym->aux_id;

    while( acid != 0 )
    {
	   a = getaux(acid);
	   if( a->pos == index )
	       return( a );

	   acid = a->chain_id;
    }
    return( NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      makesym                                                         *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_SYM *makesym( SYMENT *sym, INFILE *file_ptr )           *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for either inserting a new symbol   *
*   in the symbol table, or updating an existing symbol table emtry.   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/31/89       created                  -----             *
***********************************************************************/

LINKER_SYM  *makesym( register SYMENT *sym, INPUT_FILE *file_ptr )
{
    register long       acid;
    int                 hash_value;
    char                *name;
    register LINKER_SYM *p;
    LINKER_SYM          *p2;
    char                 local_symname[SYMNMLEN+1];

    if( 0L == OBJ_SYM_ZEROES(sym) )
      name = sym->_n._n_p._n_nptr;
    else 
    {
      strncpy( local_symname, sym->_n._n_name, SYMNMLEN);
      local_symname[SYMNMLEN] = '\0';
      name = local_symname;
    }

    if( (p = findsym(name)) == NULL )
    {
	/*
	 * Case 1: The symbol is not in the ld symbol table. Add a
	 * new entry, making it the head of the collision chain
	 * Update the hash table
	 */

	if( total_symbols >= SYMBOL_TABLE_SIZE + (extra_sym_cnt * EXTRA_SYM_SIZE) )
	{
	    if (extra_sym_cnt >= MAX_EXTRA)
		LINKER_ERROR( "Symbol table overflow." );
	    extra_sym_ptr[extra_sym_cnt++] = (LINKER_SYM *) my_calloc((long)EXTRA_SYM_SIZE * LINKER_SYM_SIZE);
	}

	acid = ++total_symbols;
	p = getsym(acid);
	p->obj_sym = *sym;
	p->name_length = strlen(name) + 1;
	if( sym->n_scnum == 0 && sym->n_value == 0 )
	    unresolved++;

	if( (IN_OBJ_STRING_TABLE(sym)) && ((file_ptr == NULL) ||
	    (file_ptr && (file_ptr->strings == NULL))) )
	{
	    p->obj_sym._n._n_p._n_nptr = strsave(sym->_n._n_p._n_nptr);
	}

	p->obj_sym.n_numaux = 0;
	p->acid = acid;

	hash_value = hash(name);
	if( hashtab[hash_value] > 0 )
	{
	    p2 = getsym(hashtab[hash_value]);
	    p->chain_id = p2->chain_id;
	    p2->chain_id = acid;
	}
	else
	    p->chain_id = acid;

	hashtab[hash_value] = acid;
	return( p );
    }

    /*
     * Case 2: The symbol is already in the symbol table Update the
     * existing entry
     */

    if( sym->n_scnum != 0 )
    {
	if( p->obj_sym.n_scnum == 0 )
	{
	    /*
	     * The existing ld symbol table entry was from a
	     * reference. Merge the information from the function argument and
	     * the ld symbol table.
	     */

	    unresolved--;
	    p->obj_sym.n_value = sym->n_value;
	    p->obj_sym.n_scnum = sym->n_scnum;
	    p->obj_sym.n_type = sym->n_type;
	    p->obj_sym.n_sclass = sym->n_sclass;
	}
	else if( p->obj_sym.n_scnum != -1 )
	    USER_ERROR3( "Symbol %s in file: %s is multiply defined. First defined in %s",
			name, current_file_name, p->input_sect->file_ptr->file_name );
    }

    return( p );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      makeaux                                                         *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_AUX( LINKER_SYM *sym, AUX *aux, int num_aux )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for adding a auxillary entry to a   *
*   existing symbol table entry.                                       *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

LINKER_AUX * makeaux( register LINKER_SYM *sym, register AUXENT *aux, int num_aux )
{
    register LINKER_AUX *a;
    long     acid;
    char  local_symname[SYMNMLEN+1];
    char *name;
	

    if( (acid = sym->aux_id) == 0 )
    {
	if( num_aux != 0 )
	  {
	    if( 0L == sym->obj_sym.n_zeroes )
	      name = sym->obj_sym._n._n_p._n_nptr;
	    else 
	    {
	      strncpy( local_symname, sym->obj_sym._n._n_name, SYMNMLEN);
	      local_symname[SYMNMLEN] = '\0';
	      name = local_symname;
	    }
	    LINKER_ERROR2( "Making aux entry %d for symbol %s out of sequence",
			  num_aux + 1, name);
	  }

	if( total_aux >= AUX_TABLE_SIZE + (extra_aux_cnt * EXTRA_AUX_SIZE) )
	{
	    if( extra_aux_cnt >= MAX_EXTRA )
		LINKER_ERROR( "Auxillary symbol table overflow." );
	    extra_aux_ptr[extra_aux_cnt++] = (LINKER_AUX *) my_calloc ((long)EXTRA_AUX_SIZE * LINKER_AUX_SIZE);
	}

	acid = ++total_aux;
	a = getaux(acid);
	a->aux = *aux;
	a->pos = 1;
	a->acid = acid;
	a->chain_id = 0;

	sym->obj_sym.n_numaux++;
	sym->aux_id = acid;

	return( a );
    }

    if( num_aux == 0 )
    {
      if( 0L == sym->obj_sym.n_zeroes )
	name = sym->obj_sym._n._n_p._n_nptr;
      else 
      {
	strncpy( local_symname, sym->obj_sym._n._n_name, SYMNMLEN);
	local_symname[SYMNMLEN] = '\0';
	name = local_symname;
      }
      FATAL_ERROR1( "Symbol %s has more than one aux entry.", name);
    }
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      getsym                                                          *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_SYM *getsym( long acid )                                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning a pointer to a symbol *
*   table entry, given a access ID.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

LINKER_SYM *getsym( register long acid )
{
    if( acid <= 0 )
	FATAL_ERROR( "Negative symbol table id." );

    if( acid <= SYMBOL_TABLE_SIZE )
	return( &symbol_table[acid] );

    acid -= (SYMBOL_TABLE_SIZE + 1);
    if( acid < extra_sym_cnt * EXTRA_SYM_SIZE )
    {
	return( (LINKER_SYM *) ((long) extra_sym_ptr[acid / EXTRA_SYM_SIZE] + 
				((acid % EXTRA_SYM_SIZE) * LINKER_SYM_SIZE)) );
    }

    FATAL_ERROR( "Invalid symbol table id.");
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      getaux                                                          *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_AUX *getaux( long acid )                                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning a pointer to a        *
*   auxillary symbol table entry, given a access ID.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

LINKER_AUX *getaux( long acid )
{
    if( acid <= 0 )
	FATAL_ERROR( "Negative auxillary table id." );

    if( acid <= AUX_TABLE_SIZE )
	return( &auxillary_symbol_table[acid] );

    acid -= (AUX_TABLE_SIZE + 1);
    if( acid < extra_aux_cnt * EXTRA_AUX_SIZE )
    {
	return ((LINKER_AUX *) ((long) extra_aux_ptr[acid / EXTRA_AUX_SIZE] +
				((acid % EXTRA_AUX_SIZE) * LINKER_AUX_SIZE)) );
    }

    FATAL_ERROR( "Invalid auxillary table id." );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_symbol_table                                            *
*                                                                      *
*   Synopsis                                                           *
*      long process_symbol_table( FILE *fd, long numsyms,              *
*                                 INFILE *file_ptr, SYMENT *symbuf,   *
*                                 char *string_table_ptr )             *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for processing the symbol table     *
*   table entries from every object file and archive that is given to  *
*   as input to the linker. Basically all the entries from a given     *
*   file are added to the cumulative linker symbol table.              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

long process_symbol_table( FILE *fd, long numsyms, INPUT_FILE *file_ptr, 
			   register SYMENT *symbuf, char *string_table_ptr )
{
    register AUXENT      *aux;
    int                  i;
    long                 l;
    AUXENT               local_auxbuf;
    SYMENT               local_symbuf;
    long                 local_symbols;
    register long        n;
    long                 relocatable_symbols;
    register SYMENT     *sym;  
    register LINKER_SYM *symp;
    char                 tmp_numaux;
    char                 tmp_sclass;
    char                *p;

    sym = &local_symbuf;
    aux = &local_auxbuf;

    local_symbols = 0L;
    relocatable_symbols = 0L;

    for( n = 0L; n < numsyms; n++ )
    {
	if( read_symbol(sym, fd) != 1 )
	    FATAL_ERROR1( "Failed to read symbol table of file %s", file_ptr->file_name);

	/* If the name is in the string table, change the string table offset into a
	 * pointer to the name.
	 */

	if( IN_OBJ_STRING_TABLE(sym) )
	{
	    if( string_table_ptr == NULL )
		FATAL_ERROR1( "Name referenced when no string table exists in file %s", 
			     file_ptr->file_name);
	    sym->_n._n_p._n_nptr = string_table_ptr + OBJ_SYM_OFFSET(sym);
	}

	switch( sym->n_sclass )
	{
	       /*
		* Symbols of type C_EXT are global, and must be
		* added to ld's cumulative symbol table
		*/

		case C_EXT:
		   /* if its an global from the RTL, permote sits torage class. */
		   if (strncmp(cur_x_fname, "___RTL21020___", 14) == 0)
		     sym->n_sclass = C_CRTL;
		   relocatable_symbols++;
		   symp = makesym(sym, file_ptr);

		   if( sym->n_scnum > 0 )
		   {
#ifdef DEBUG
		       printf("process_symbol_table: name = %s\n", sym->_n._n_name);
		       fflush(stdout);
#endif
		       symp->input_sect = list_find_input_section(sym->n_scnum, file_ptr);
		   }

		   /*
		    * Functions, although added to ld's symbol table,
		    * must be put in the "local symbol" section of the
		    * final output file
		    */

		   if( IS_FCN(sym->n_type) && (sym->n_scnum != N_ABS) )
		   {
		       symp->sym_index = local_symbols;
		       local_symbols += (long) (sym->n_numaux + 1);
		       symp->local_flag = 1;
		   }

		   n += (long) sym->n_numaux;
		   tmp_numaux = sym->n_numaux;
		   for( i = 0; i < tmp_numaux; i++ )
		   {
			if( read_aux_info(aux, fd, output_get_aux_class(sym)) != 1 )
			    FATAL_ERROR1( "Failed to read aux entries of file %s", file_ptr->file_name);

			makeaux(symp, aux, i);
		   }
		   break;

		case C_FILE:
		   local_symbols += 1 + (long) sym->n_numaux;
		   if( sym->n_numaux )
		   {
		       n += sym->n_numaux;
/*
		       if( fseek(fd, (long) ((AUXESZ) * sym->n_numaux), 1) != 0 )
*/
		       /* track the current ".file" name aux entry */
		       if (read_aux_info(aux, fd, 0) != 1)
			   FATAL_ERROR1( "Fail to skip aux entry of %s", file_ptr->file_name );

		       for (i=0, p=aux->x_file.x_fname; *p && i<FILNMLEN; i++)
			 cur_x_fname[i] = *p++;
		       for ( ; i<FILNMLEN; i++)
			 cur_x_fname[i] = '\0';
		   }
		   break;

		case C_STAT:
		   relocatable_symbols++;
		   local_symbols += 1 + sym->n_numaux;
		   if( sym->n_numaux == 1 )
		   {
		       n++;
		       if( fseek(fd, (long) AUXESZ, 1) != 0 )
			   FATAL_ERROR1( "Failed to skip the aux entry of %s", file_ptr->file_name );
		   };
		   continue;

		case C_HIDDEN:
		   relocatable_symbols++;

		default:
		   if( !xflag )
		       local_symbols++;

		   if( sym->n_numaux > 0 )
		   {
		       if( !xflag )
			   local_symbols += (long) sym->n_numaux;
		       n += (long) sym->n_numaux;
		       tmp_sclass = sym->n_sclass;
		       tmp_numaux = sym->n_numaux;

		       if( read_aux_info(aux, fd, output_get_aux_class(sym)) != 1 )
			   FATAL_ERROR1( "Fail to read aux entries of file: %s", file_ptr->file_name );

		       /*
			* Skip all aux entries after the first
			*/

		       if( tmp_numaux > 1 )
			   if( fseek(fd, (long) (AUXESZ * (tmp_numaux - 1)), 1) != 0 )
			       FATAL_ERROR1("Failed to skip the aux entry of file: %s", 
					   file_ptr->file_name);

		       /*
			* Skip all members of structures and unions
			*/

		      if( IS_TAG(tmp_sclass) )
		      {
			  l = aux->x_sym.x_fcnary.x_fcn.x_endndx - n - 1;
			  if( !xflag )
			      local_symbols += l;
			  n = aux->x_sym.x_fcnary.x_fcn.x_endndx - 1;
			  if( fseek(fd, l * SYMESZ, 1) != 0 )
			      FATAL_ERROR1( "Failed to skip a structure member in file: %s", 
					    file_ptr->file_name);
		      }
		  }
	}
   }

   if( relocatable_symbols > max_relocatable_syms )
       max_relocatable_syms = relocatable_symbols;

   return( local_symbols );
symbuf;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      get_aux_entry                                                   *
*                                                                      *
*   Synopsis                                                           *
*      LINKER_AUX *get_aux_entry( LINKER_SYM *sym, LINKER_AUX *aux,    *
*                                 int changed )                        *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning the auxillary entry   *
*   associated with a symbol table entry.                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

LINKER_AUX *find_aux_entry( LINKER_SYM *sym, LINKER_AUX *aux, int changed )
{
    char  local_symname[SYMNMLEN+1];
    char *name;

    if( aux == NULL )
    {
	if( sym->aux_id != 0 )
	    return ( getaux(sym->aux_id) );

	return( NULL );
    }

    /*
     * Find the next aux entry. As things stand right now we should never get
     * here because in the current COFF spec, there is never more than one
     * auxillary entry for a symbol. But just in case...
     */

   /* 
    if( 0L == sym->obj_sym.n_zeroes )
      name = sym->obj_sym._n._n_p._n_nptr;
    else 
    {
      strncpy( local_symname, sym->obj_sym._n._n_name, SYMNMLEN);
      local_symname[SYMNMLEN] = '\0';
      name = local_symname;
    }
	
    LINKER_ERROR1( "Symbol %s has more than one auxillary table entry.", name);
*/
    if( aux->chain_id != 0 )
	return( getaux(aux->chain_id) );

    return( NULL );
changed;
name;
local_symname;
}

