/* @(#)reloc.c	2.8 5/17/94 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "symbol.h"
#include "a_out.h"
#include "pass1.h"
#include "pass2.h"
#include "error.h"
#include "coff_io.h"
#include "internal_reloc.h"
#include "util.h"

#include "reloc_fp.h"
#include "section_fp.h"

long num_reloc = 0L;
extern short  Flag_undefineds;

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_reloc                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void make_reloc(symbol,type)                                    *
*      SYMBOL *symbol;                                                 *
*      long    type;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a relocation entry.    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/28/89       created                  -----             *
***********************************************************************/

void make_reloc( SYMBOL *symbol, long type )
{
    INTERNAL_RELOC reloc_entry;
    register char *rsym;
    long sym_type;
/*
    char buf[200];

    if (symbol->what.name[0] != '\0' && symbol->value == -1)
      {
        sprintf(buf,"Symbol %s undefined.",symbol->what.name);
        FATAL_ERROR(buf);
      }
*/



    sym_type = BASIC_TYPE(symbol->type);
    switch( (short)sym_type )
    {
      case STYPE_ABS:
	return;
	
      case STYPE_PM:
      case STYPE_DM:
/*
	reloc_entry.relname.r_name = section_header[symbol->section_num].s_name;
*/
	reloc_entry.relname.r_name = NAME( symbol );
	break;
	
      case STYPE_UNDEFINED:
	if ( symbol->type == STYPE_EXTERN)
	    reloc_entry.relname.r_name = NAME( symbol );
	else if ( Flag_undefineds)
	    USER_ERROR1("Undefined symbol %s",NAME(symbol));
	else if ( symbol->type != STYPE_SECTION )
	    symbol->type |= STYPE_EXTERN;
	reloc_entry.relname.r_name = NAME( symbol );
	break;

      default:
	ASSEMBLER_ERROR( "Cannot relocate invalid symbol type." );
	break;
    }

    reloc_entry.r_address = newpc;
    reloc_entry.r_type = (short) type;
    write_internal_relocation_info( &reloc_entry, rel_fd );
    num_reloc++;
    symbol->num_relocs++;
rsym;
}





/***********************************************************************

  Name:		read_internal_relocation_info 

  Description:	write an INTERNAL_RELOC structure to a file - made to
                work similarly to the routines in coff_io.c

  Return Value:	0 on error, 1 for success

  **********************************************************************/
int read_internal_relocation_info( INTERNAL_RELOC *rel_ptr, FILE *fp )
{
  if( 1 == fread( (unsigned char *)rel_ptr, sizeof(INTERNAL_RELOC), 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:		write_internal_relocation_info

  Description:	read an INTERNAL_RELOC structure to a file - made to
                work similarly to the routines in coff_io.c

  Return Value:	0 on error, 1 for success

  **********************************************************************/
int write_internal_relocation_info( INTERNAL_RELOC *rel_ptr, FILE *fp )
{
  if( 1 == fwrite( (unsigned char *)rel_ptr, sizeof(INTERNAL_RELOC), 1, fp) )
    return(1);
  else
    return( 0);
}



