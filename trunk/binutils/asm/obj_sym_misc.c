/* @(#)obj_sym_misc.c   1.7 2/21/91 1 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "a_out.h"
#include "error.h"
#include "symbol.h"
#include "obj_sym_misc.h"
#include "util.h"

#include "pass2.h"
#include "list.h"
#include "codegen.h"
#include "action.h"
#include "string.h"

#include "section_fp.h"
#include "object_fp.h"

STACK_SYM first_symbol = {0L, 0L, 0L, 0L, 0L, 0L, NULL};
static STACK_SYM *stack[MAX_STACK];
static STACK_SYM *symbol_head    = &first_symbol;
static STACK_SYM *symbol_tail    = &first_symbol;
static STACK_SYM *current_symbol = &first_symbol;
short stack_top = 0;

static void push_sym_ptr( STACK_SYM *value );


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      pop_sym_ptr                                                     *
*                                                                      *
*   Synopsis                                                           *
*      STACK_SYM *pop_sym_ptr(void)                                    *
*                                                                      *
*   Description                                                        *
*      This routine pops the symbol stack.                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

STACK_SYM *pop_sym_ptr( void )
{
    if( stack_top == 0 )
    {
	FATAL_ERROR("Unbalanced symbol table");
	return( (STACK_SYM *) NULL );
    }

    return( stack[--stack_top] );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      push_sym_ptr                                                    *
*                                                                      *
*   Synopsis                                                           *
*      static push_sym_ptr(value)                                      *
*      STACK_SYM *value;                                               *
*                                                                      *
*   Description                                                        *
*      This routine pushes a pointer to a STACK_SYM onto the symbol    *
*   stack.                                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

static void push_sym_ptr( STACK_SYM *value )
{
    if( stack_top == MAX_STACK - 1 )
	FATAL_ERROR("Symbol stack overflow");

    stack[stack_top++] = value;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      save_symbol_index                                               *
*                                                                      *
*   Synopsis                                                           *
*      void save_symbol_index( value )                                 *
*      short value;                                                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for saving the symbol table index of*
*   a symbol.                                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

void save_symbol_index( short value )
{
    current_symbol->symbol_index = num_syms + 1;
    current_symbol->function_length = (long) value;    

    if( symbol_tail != current_symbol )
	symbol_tail->next = current_symbol;

    symbol_tail = current_symbol;
    current_symbol = (STACK_SYM *) my_calloc((long)(sizeof(STACK_SYM)));
    push_sym_ptr( symbol_tail );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      fixup_symbol_table                                              *
*                                                                      *
*   Synopsis                                                           *
*      void fixup_symbol_table( fd )                                   *
*      FILE *fd;                                                       *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for fixing the symbol table by      *
*   backpatching the size of functions, and the index of the next      *
*   symbol entry for tags, functions, and beginning of block and       *
*   function.                                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

void fixup_symbol_table( register FILE *fd )
{
    int i;
    AUXENT auxent;
    SYMENT obj_sym;
    register SCNHDR *sec_hdr_ptr;
    long beginning;
    long long_buff;
    register STACK_SYM *sym_ptr;
    long line_ptr, reloc_ptr, sect_lp[MAX_SECTIONS+1];
    long line, rel, size;
    long fixup_value;


    sym_ptr = symbol_head;

    line = rel = size =0;

    for( i = 1, sec_hdr_ptr = &section_header[1]; i <= (short) section_cnt;
	 ++i,++sec_hdr_ptr )
    {
	 line += sec_hdr_ptr->s_nlnno;
	 rel  += sec_hdr_ptr->s_nreloc;
	 size += sec_hdr_ptr->s_size;
    }
   
    line_ptr = FILHSZ + AOUTSZ + section_cnt * SCNHSZ + size + rel * RELOC_SIZE;

    for( i = 1, sec_hdr_ptr = &section_header[1]; i <= (short) section_cnt; ++i, ++sec_hdr_ptr )
    {
	 if( sec_hdr_ptr->s_nlnno )
	     sec_hdr_ptr->s_lnnoptr = sect_lp[i] = line_ptr;
    } 
    if( sym_ptr->symbol_index > 0 )
    {
	beginning = ftell( fd );
	while( sym_ptr != NULL )
	{
	       if( sym_ptr->function_length >= 0 )
	       {
		   fseek( fd, (sym_ptr->symbol_index * SYMESZ) +
			 (long)(((char *) &auxent.x_sym.x_fcnary.x_fcn.x_lnnoptr) - ((char *) &auxent)), 0 );
		     
		   fixup_value = sect_lp[sym_ptr->section]+sym_ptr->linnumber;
		   FWRITE( &fixup_value, sizeof(sym_ptr->section), 1, fd ); 


/*      It is uncertain if this will be necessary!
		   fseek( fd, (sym_ptr->symbol_index * SYMESZ) +
			 (long)(((char *) &auxent.x_sym.x_tagndx) - ((char *) &auxent)), 0 );
		   port_put_long( sym_ptr->function_beginning, long_buff );
		   FWRITE( long_buff, sizeof(sym_ptr->function_beginning), 1, fd );
*/

		   fseek( fd, (sym_ptr->symbol_index * SYMESZ) +
			 (long)(((char *) &auxent.x_sym.x_misc.x_fsize) - ((char *) &auxent)), 0 );
		   FWRITE( &sym_ptr->function_length, sizeof(sym_ptr->function_length), 1, fd );
	       }
	       fseek( fd, (sym_ptr->symbol_index * SYMESZ) +
		     (long)(((char *) &auxent.x_sym.x_fcnary.x_fcn.x_endndx) - ((char *) &auxent)), 0 );
	       FWRITE( &sym_ptr->forward_index, sizeof(sym_ptr->forward_index), 1, fd );
	       sym_ptr = sym_ptr->next;
	}
	if (symbol_head != NULL) 
	{
		fseek( fd, (long)(((char *) &obj_sym.n_value ) - ((char *) &obj_sym)), 0);
		long_buff = num_syms + num_stat_def_syms + (2 * section_cnt);
		FWRITE( &long_buff, sizeof(long), 1, fd);
	}
	fseek( fd, beginning, 0 );
    }
reloc_ptr;
}
