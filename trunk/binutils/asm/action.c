/* @(#)action.c 1.10 2/21/91 1 */

#include "app.h"
#include "codegen.h"
#include "error.h"
#include "symbol.h"
#include "a_out.h"
#include "obj_sym_misc.h"
#include "list.h"
#include "str.h"
#include "pass1.h"
#include "pass2.h"
#include "main.h"
#include "expr.h"
#include "ilnode.h"
#include "assemble.h"
#include "action.h"

#include "internal_reloc.h"
#include "reloc_fp.h"
#include "section_fp.h"
#include "object_fp.h"
#include "make_ilnode_fp.h"
#include "coff_io.h"

extern int Section_flag;
extern char coff_name[];
long num_line = 0;
unsigned short c_src_line = 0;

static AUXENT auxent;
static short dim_num = 0;
static LINENO   lineent;

static SYMENT obj_sym;
static SYMENT *obj_sym_ptr = &obj_sym;

static long tagndx_fix = 0l;
static long linnumber_fix = 0l;
static long section_fix = 0l;

static SYMBOL *save_sym;
static STACK_SYM *sym_stack_ptr;

static int      static_fix = FALSE;

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_file_symbol                                              *
*                                                                      *
*   Synopsis                                                           *
*      void define_file_symbol(symbol, code)                           *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the '.file' COFF symbol*
*   entry, which must the very first thing in the symbol table.        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

void define_file_symbol( SYMBOL *symbol, register CODE *code )
{
    register short i;
    register char *name_ptr, *p;
    register SYMBOL *file_symbol;

    i = 0;
    if( num_syms > 0 || num_glob_def_syms > 0 )
    {
	USER_WARN("The '.file' must be the first symbol in the symbol table");
	return;
    }

    file_symbol = symbol_insert(".file", 0L, 0L, STYPE_UNDEFINED, 0L );
    define_symbol( file_symbol, code );
    obj_sym_ptr->n_numaux = 1;
    name_ptr = ((AUXENT *)(&auxent))->x_file.x_fname;

/*  The old way...
    if (IN_STRING_TABLE(symbol))
	for(i=0, p=&string_table[OFFSET(symbol)]; i<FILNMLEN; i++)
		*name_ptr++ = *p++;
    else
    {
	for(i=0, p=NAME(symbol); i<SYMNMLEN; i++)
		*name_ptr++ = *p++;
	for( ;i<FILNMLEN; i++)
		*name_ptr++ = '\0';
    }
*/

/* The new way... */
    for(i=0, p=coff_name; *p && i<FILNMLEN; i++)
      *name_ptr++ = *p++;
    for( ;i<FILNMLEN; i++)
      *name_ptr++ = '\0';
       
    code->code_value.value = (long) C_FILE;
    define_symbol_sclass( (SYMBOL*) NULL, code );
    end_symbol_define( (SYMBOL *) NULL, code );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_symbol                                                   *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol( symbol, code )                              *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for setting up a symbol that will   *
*   eventually be written to a symbol table file. This routine and it's*
*   associated routines are only called if the compiler has embedded   *
*   symbolic debug information in it's output.                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol( SYMBOL *symbol, register CODE *code )
{
    register short i;

    save_sym = symbol;

    if( IN_STRING_TABLE(symbol) )
    {
	OBJ_SYM_ZEROES(obj_sym_ptr) = 0L;
	OBJ_SYM_OFFSET(obj_sym_ptr) = OFFSET(symbol);
    }
    else
    {
	for( i = 0; (i < SYMNMLEN) && 
	     (obj_sym_ptr->_n._n_name[i] = symbol->what.name[i]); ++i )
	     ;

	for( ; i < SYMNMLEN; ++i )
	     obj_sym_ptr->_n._n_name[i] = '\0';
    }

    obj_sym_ptr->n_value = 0L;
    obj_sym_ptr->n_scnum = 0;
    obj_sym_ptr->n_type = 0;
    obj_sym_ptr->n_sclass = 0;
    obj_sym_ptr->n_numaux = 0;
    for(i=0; i < FILNMLEN; i++ )
	AUX_FILENAME(auxent)[i] = 0;

code;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_symbol_value                                             *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol_value( symbol, code )                        *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for defining a symbol's value.      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_value( SYMBOL *symbol, register CODE *code )
{
    if( symbol != (SYMBOL *) NULL )
    {
	if (code->code_value.value == 0)
		code->code_value.value += symbol->value;
	obj_sym_ptr->n_scnum = (short) symbol->section_num;
    }
    else
	obj_sym_ptr->n_scnum = N_ABS;

    if( code->code_value.value == 0x80000000L)
	code->code_value.value = newpc;

    if( code->code_value.value == 0x80000001L)
      code->code_value.value = 0;

    obj_sym_ptr->n_scnum = (short) symbol->section_num;
    obj_sym_ptr->n_value = code->code_value.value;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol_type( symbol, code )                         *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for defining a symbols type.        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_type( SYMBOL *symbol, register CODE *code )
{
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/

    obj_sym_ptr->n_type = (unsigned short)code->code_value.value;
symbol;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol_sclass( symbol, code )                       *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsbile for defining a symbols storage      *
*   class.                                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_sclass( SYMBOL *symbol, register CODE   *code )
{
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/

    if( symbol != (SYMBOL *) NULL )
    {
	if( code->code_value.value == C_FIELD )
	{
		if( obj_sym_ptr->n_value == symbol->value )
		{
			obj_sym_ptr->n_value = 0;
		}
	}
    }

    if( (code->code_value.value == C_AUTO) || (code->code_value.value == C_ARG) ||
	(code->code_value.value == C_REG)  || (code->code_value.value == C_MOS) ||
	(code->code_value.value == C_MOU)  || (code->code_value.value == C_MOE) ||
	(code->code_value.value == C_EOS)  || (code->code_value.value == C_REGPARM) ||
	(code->code_value.value == C_FIELD))
		obj_sym_ptr->n_scnum = N_ABS;

    obj_sym_ptr->n_sclass = (char) code->code_value.value;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_symbol_tag                                               *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol_tag( symbol, code )                          *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine processes the auxillary entry of a structure,      *
*   union or enum by setting it's tag index field to the index of the  *
*   structure tag in the object symbol table.                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_tag( SYMBOL *symbol, register CODE   *code )
{
    register long i;
    register char *name;

    if( !symbol )
	return;

    if( IN_STRING_TABLE(symbol) )
	name = &string_table[OFFSET(symbol)];
    else
	name = NAME(symbol);

    if( ((i = list_get_index(name, C_STRTAG)) < 0) &&
	((i = list_get_index(name, C_UNTAG)) < 0) &&
	((i = list_get_index(name, C_ENTAG)) < 0))
    {
	  ASSEMBLER_ERROR("Illegal structure, union, or enumeration tag");
    }

    obj_sym_ptr->n_numaux = 1;
    AUX_TAGINDEX(auxent) = i;
code;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_symbol_line                                              *
*                                                                      *
*   Synopsis                                                           *
*      void define_symbol_line( symbol, code )                         *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for setting the line number entry   *
*   of a object file symbol.                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_line( SYMBOL *symbol, register CODE   *code )
{
    obj_sym_ptr->n_numaux = 1;
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/
     
    AUX_LINENO(auxent) = (unsigned short)(code->code_value.value);
symbol;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_symbol_size                                              *
*                                                                      *
*   Synopsis                                                           *
*      define_symbol_size( symbol, code )                              *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for setting the auxillary size field*
*   of a array, or structure type variable.                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_symbol_size( SYMBOL *symbol, register CODE   *code )
{
    obj_sym_ptr->n_numaux = 1;
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/
     
    AUX_SIZE(auxent) = (unsigned short)(code->code_value.value);
symbol;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_section_auxent                                           *
*                                                                      *
*   Synopsis                                                           *
*      void define_section_auxent( size, nreloc, nline )               *
*      long size;                                                      *
*      long nreloc;                                                    *
*      long nline;                                                     *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for defining auxillary entries for  *
*   each section.                                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void define_section_auxent( long size, long nreloc, long nline )
{
    obj_sym_ptr->n_numaux = 1;
    AUX_SCNLEN(auxent)   = size;
    AUX_NRELOC(auxent)   = (unsigned short) nreloc;
    AUX_NLINENO(auxent)  = (unsigned short) nline;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      final_preparations                                              *
*                                                                      *
*   Synopsis                                                           *
*      void final_preparations()                                       *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for calculating different pieces of *
*   information on a symbol (depending on it's storage class) prior    *
*   to it's being written to a symbol table file.                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void final_preparations( void )
{
    switch( obj_sym_ptr->n_sclass )
    {
	    case C_STRTAG:
	    case C_UNTAG:
	    case C_ENTAG:
		save_symbol_index( -1 );

	    case C_TPDEF:
	    case C_FILE:
		obj_sym_ptr->n_scnum = N_DEBUG;
		break;

	    case C_BLOCK:    /* '.bb' (begin block) and '.eb' (end block) */
		if( obj_sym_ptr->_n._n_name[1] == 'b' )
		    save_symbol_index( -1 );
		else
		    pop_sym_ptr()->forward_index = num_syms + 1 + obj_sym_ptr->n_numaux;
		break;

	    case C_FCN:      /* '.bf' and '.ef' */
		if( obj_sym_ptr->_n._n_name[1] == 'b' )
		{       
			static_fix = TRUE;
			tagndx_fix = num_syms;
			save_symbol_index( -1 );
		}
		else
		{
			static_fix = FALSE;
		    sym_stack_ptr = pop_sym_ptr();                    
		    sym_stack_ptr->forward_index = num_syms + 1 + obj_sym_ptr->n_numaux;
/*
		    if( stack_top > 0 )
			FATAL_ERROR("Symbol table is unbalanced (.ef)");
*/
		}
		break;

	    case C_EOS:
		pop_sym_ptr()->forward_index = num_syms + obj_sym_ptr->n_numaux + 1;
		break;

	    default:
		if( IS_FCN(obj_sym_ptr->n_type) )
		{
		    sym_stack_ptr = NULL;
		    lineent.l_lnno = 0;
		    lineent.l_addr.l_symndx = num_syms;
		    write_line_info( &lineent, line_fd );
		    obj_sym_ptr->n_numaux = 1;
		    AUX_LINEPTR(auxent) = num_line * LINESZ;
		    linnumber_fix = num_line * LINESZ;
		    section_fix = obj_sym_ptr->n_scnum;
		     ++num_line;
		    save_symbol_index( 0 );
		}
		break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_dim1                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void define_dim1( symbol, code )                                *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for setting up the auxillary entry  *
*   dimension size information for arrays.                             *       
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/20/89       created                  -----             *
***********************************************************************/

void define_dim1( SYMBOL *symbol, register CODE *code )
{
    obj_sym_ptr->n_numaux = 1;
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/

    AUX_DIMEN(auxent)[dim_num = 0] = (unsigned short)(code->code_value.value);
symbol;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_dim2                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void define_dim2( symbol, code )                                *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for setting up the auxillary entry  *
*   dimension size information for arrays.                             *       
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/20/89       created                  -----             *
***********************************************************************/

void define_dim2( SYMBOL *symbol, register CODE *code )
{
/*
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;
*/

    if( ++dim_num < DIMNUM )
	AUX_DIMEN(auxent)[dim_num] = (unsigned short)(code->code_value.value);
    else
	USER_WARN("Too many dimensions for symbol debug");
symbol;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      end_symbol_define                                               *
*                                                                      *
*   Synopsis                                                           *
*      void end_symbol_define( symbol, code )                          *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing a symbol to the object  *
*   symbol file.                                                       *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void end_symbol_define( SYMBOL *symbol, register CODE *code )
{
    register long *count;
    FILE *fd;

    /* Is this the physical end of a function?  */

    if( obj_sym_ptr->n_sclass == (char) C_EFCN )
    {
	if( 1 || (sym_stack_ptr == NULL ))
	{
	    sym_stack_ptr = pop_sym_ptr();    
	    sym_stack_ptr->forward_index = num_syms;
	    if( stack_top > 0 )
		FATAL_ERROR("C_EFCN Symbol table is unbalanced");
	}

	sym_stack_ptr->function_length = newpc - save_sym->value;
	sym_stack_ptr->function_beginning = tagndx_fix;
	sym_stack_ptr->section = section_fix;
	sym_stack_ptr->linnumber = linnumber_fix;
	return;
    }

    final_preparations();
    if ( !(IS_FCN(obj_sym_ptr->n_type)) 
	 && (obj_sym_ptr->n_sclass == C_EXT))
    {
	fd = glob_sym_fd;
	count = &num_glob_def_syms;
    }
    else if( !(IS_FCN(obj_sym_ptr->n_type)) 
	     && (!Section_flag) 
	     && (obj_sym_ptr->n_sclass == C_STAT) 
	     && !(static_fix) )
    {
	fd = stat_sym_fd;
	count = &num_stat_def_syms;
    }
    else 
    {
	fd = sym_fd;
	count = &num_syms;
	list_insert_index( save_sym, obj_sym_ptr->n_sclass, num_syms );
    }

/*    FWRITE( &obj_sym, SYMESZ, 1, fd );   */
    write_symbol( &obj_sym, fd );
    (*count)++;

       
    if( obj_sym_ptr->n_numaux != 0 )
    {
/*        FWRITE( &auxent, AUXENT_SIZE, 1, fd );   */
	if( symbol == (SYMBOL *) NULL && code->code_value.value == (long) C_FILE )
	    write_aux_info( &auxent, fd, AUX_CLASS_FILE_NAME );
	else
	    write_aux_info( &auxent, fd, get_aux_class( symbol ) );

	(*count)++;
    }

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      get_aux_class                                                   *
*                                                                      *
*   Description                                                        *
*      Determine the auxillary class this symbol belongs to.           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/04/90       created                  -----             *
***********************************************************************/

int get_aux_class( SYMBOL *sym )
{
    if( !sym )
	return AUX_CLASS_SYM;

    switch( (int) sym->type )
    {
	    case STYPE_UNDEFINED:
	       if( !IN_STRING_TABLE(sym) && !strcmp( NAME(sym), ".file") )
		   return( (int) AUX_CLASS_FILE_NAME );
	       else
		   return( (int) AUX_CLASS_SYM );

	    case STYPE_SECTION:
	       return( (int) AUX_CLASS_SECTION );

	    default:
	       return( (int) AUX_CLASS_SYM );
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      define_line_number                                              *
*                                                                      *
*   Synopsis                                                           *
*   void define_line_number( symbol, code )                            *
*   SYMBOL *symbol;                                                    *
*   CODE   *code;                                                      *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a line number entry.   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/10/89       created                  -----             *
***********************************************************************/

void line_number( SYMBOL *symbol, register CODE *code )
{
    if( symbol != (SYMBOL *) NULL )
	code->code_value.value += symbol->value;

    lineent.l_lnno =  c_src_line = (unsigned short) (code->code_value.value);
    lineent.l_addr.l_paddr = newpc;
    write_line_info( &lineent, line_fd );
/*    FWRITE( &lineent, LINESZ, 1, line_fd );   */
    ++num_line;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      reloc_addr_var                                                  *
*                                                                      *
*   Synopsis                                                           *
*   void reloc_addr_var( SYMBOL *symbol, CODE *code )                  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry     *
*   when a variable is set equal to the address of another in a data   *
*   section.                                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/20/89       created                  -----             *
***********************************************************************/

void reloc_addr_var( SYMBOL *symbol, register CODE *code )
{
    register unsigned long value;

    value = (long)((long)code->code_value.data_value[0] << 24) | ((long)code->code_value.data_value[1] << 16) |
	((long)code->code_value.data_value[2] << 8)  | ((long)code->code_value.data_value[3]);

    if( symbol != (SYMBOL *) NULL )
    {
	value += symbol->value;
	make_reloc( symbol, R_ADDR_VAR );
    }

    code->code_value.data_value[0] = (char) DATA32_TOP8(value);
    code->code_value.data_value[1] = (char) DATA32_TOP_MIDDLE8(value);
    code->code_value.data_value[2] = (char) DATA32_MIDDLE8(value);
    code->code_value.data_value[3] = (char) DATA32_BOTTOM8(value);
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      reloc_addr24                                                    *
*                                                                      *
*   Synopsis                                                           *
*   void reloc_addr24( SYMBOL *symbol, CODE *code )                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry     *
*   when a address in program memory is involved.                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/20/89       created                  -----             *
***********************************************************************/

void reloc_addr24( SYMBOL *symbol, register CODE *code )
{
    register unsigned long value;

    value = 0L;
    value = (long)((long)code->code_value.instr_value[3] << 16) | ((long)code->code_value.instr_value[4] << 8)  | 
	    ((long)code->code_value.instr_value[5]);

    if( symbol != (SYMBOL *) NULL )
    {
	value += symbol->value;
	make_reloc( symbol, R_ADDR24 );
    }

    code->code_value.instr_value[3] = (char) PM_ADDRESS_TOP8(value);
    code->code_value.instr_value[4] = (char) PM_ADDRESS_MIDDLE8(value);
    code->code_value.instr_value[5] = (char) PM_ADDRESS_BOTTOM8(value);
       
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      reloc_addr32                                                    *
*                                                                      *
*   Synopsis                                                           *
*   void reloc_addr32( SYMBOL *symbol, CODE *code )                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry     *
*   when a address in data memory is involved.                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/20/89       created                  -----             *
***********************************************************************/

void reloc_addr32( SYMBOL *symbol, register CODE *code )
{
    register unsigned long value;

    value = (long)((long)code->code_value.instr_value[2] << 24) | ((long)code->code_value.instr_value[3] << 16) |
	    ((long)code->code_value.instr_value[4] << 8)  | (long)(code->code_value.instr_value[5]);

    if( symbol != (SYMBOL *) NULL )
    {
	value += symbol->value;
	make_reloc( symbol, R_ADDR32 );
    }

    code->code_value.instr_value[2] = (char) DATA32_TOP8(value);
    code->code_value.instr_value[3] = (char) DATA32_TOP_MIDDLE8(value);
    code->code_value.instr_value[4] = (char) DATA32_MIDDLE8(value);
    code->code_value.instr_value[5] = (char) DATA32_BOTTOM8(value);
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      reloc_pc_rel_short                                              *
*                                                                      *
*   Synopsis                                                           *
*   void reloc_pc_rel_short( SYMBOL *symbol, CODE *code )              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry     *
*   when a pc relative short branch is involved.                       * 
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/20/89       created                  -----             *
***********************************************************************/

void reloc_pc_rel_short( SYMBOL *symbol, register CODE *code )
{
    register short value;
    register short rel_val;

    value = (short) (((short)code->code_value.instr_value[1] & (char) 0x01L) << 5) | 
	    ((unsigned short)(code->code_value.instr_value[2] & (char) 0xF8L) >> 3);
    if( symbol != (SYMBOL *) NULL )
	value += (short) symbol->value;

    rel_val = (short)(value - newpc);
    code->code_value.instr_value[1] |= (char) ((unsigned short)(rel_val & (short) 0x20L) >> 5);
    code->code_value.instr_value[2] =  (char) ((rel_val & (short) 0x1FL) << 3) | 
				       (char) (code->code_value.instr_value[2] & (char) 0x07L);

    if( (rel_val < -32) || (rel_val > 31) )
	 USER_ERROR( "PC relative short branch is out of range." );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      reloc_pc_rel_long                                               *
*                                                                      *
*   Synopsis                                                           *
*   void reloc_pc_rel_long( SYMBOL *symbol, CODE *code )               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry     *
*   when a pc relative long branch is involved.                        * 
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/20/89       created                  -----             *
***********************************************************************/

void reloc_pc_rel_long( SYMBOL *symbol, register CODE *code )
{
    register long value;

    value = 0L;
    value = (long) (((long)code->code_value.instr_value[3] << 16) | ((long)code->code_value.instr_value[4] << 8)
	    | ((long)code->code_value.instr_value[5]));
    if( value & 0x00800000L )
      value |=  0xFF000000L;

    if( symbol != (SYMBOL *) NULL )
    {
	value += symbol->value;
	make_reloc( symbol, R_PCRLONG );
    }

    code->code_value.instr_value[3] = (char) PM_ADDRESS_TOP8(value);
    code->code_value.instr_value[4] = (char) PM_ADDRESS_MIDDLE8(value);
    code->code_value.instr_value[5] = (char) PM_ADDRESS_BOTTOM8(value);
       
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      new_statement                                                   *
*                                                                      *
*   Synopsis                                                           *
*      void new_statement( symbol, code )                              *
*      SYMBOL *symbol;                                                 *
*      CODE   *code;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for resyncing the pc and setting    *
*   the line number on the second pass;                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/20/89       created                  -----             *
***********************************************************************/

void new_statement( SYMBOL *symbol, register CODE *code )
{
#ifdef MSDOS
    yylineno = (short) code->code_value.value;
#endif
    yylineno = (int) code->code_value.value;
    pc->value   = newpc;
symbol;
}

