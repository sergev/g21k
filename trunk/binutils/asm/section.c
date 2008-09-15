/* @(#)section.c        2.1 2/13/95 2 */

#include "app.h"
#include "error.h"
#include "symbol.h"
#include "a_out.h"
#include "tempio.h"
#include "str.h"
#include "pass1.h"

#include "section_fp.h"

static long current_section_type = (-1); /* strictly local - do not 'extern' */

short in_section = FALSE;
long section_cnt = 0;
SEC_DATA section_data[MAX_SECTIONS + 1];
SCNHDR section_header[MAX_SECTIONS + 1];


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_section()                                                  *
*                                                                      *
*   Synopsis                                                           *
*      short make_section(symbol,attr)                                 *
*      SYMBOL *symbol;                                                 *
*      long int attr;                                                  *
*                                                                      *
*   Description                                                        *
*      Make a new section with the given attributes.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/22/89       created                  -----             *
***********************************************************************/

short make_section( SYMBOL *symbol, long attr )
{
    FILE *descriptor;
    register SEC_DATA *sect_data_ptr;
    register SCNHDR *sect_hdr_ptr;

    /* Check to see if we encountered a end segment directive */

    if( in_section == AFFIRMATIVE )
	return( 0 );

    if( !IS_SECTION_TYPE(symbol->type) )
    {
	if( symbol->type != STYPE_UNDEFINED )
	    FATAL_ERROR("Section name is defined in another context");
	else if( section_cnt >= MAX_SECTIONS )
	    FATAL_ERROR("Maximum number of sections exceeded");
	else
	{
	    ++section_cnt;
	    sect_data_ptr = &section_data[section_cnt];
	    sect_hdr_ptr  = &section_header[section_cnt];

	    if ((attr & SECTION_PM) == SECTION_PM)
		sect_data_ptr->sec_type = STYPE_PM;
	    else
		sect_data_ptr->sec_type = STYPE_DM;

	    set_current_section_type( attr );
	    descriptor = temp_file_create( WRITE_BINARY );
	    sect_data_ptr->temp_file_index = num_open_files - 1;
	    sect_data_ptr->file_desc = descriptor;
	    sect_data_ptr->is_open = AFFIRMATIVE;
	    sect_hdr_ptr->s_flags = attr;
	    if( IN_STRING_TABLE(symbol) )
		strncpy( sect_hdr_ptr->s_name, &string_table[OFFSET(symbol)], 
			 SYMNMLEN );
	    else
		strncpy( sect_hdr_ptr->s_name, NAME(symbol), SYMNMLEN );

	    symbol->type = STYPE_SECTION;
	    symbol->section_num = section_cnt;
	    symbol->value = attr;
	}

    }
    else if( attr && (symbol->value != attr) )
	     USER_ERROR("Section attributes do not match");
     
    return( (short) symbol->section_num );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      change_section()                                                *
*                                                                      *
*   Synopsis                                                           *
*      void change_section(sec_num)                                    *
*      short sec_num;                                                  *
*                                                                      *
*   Description                                                        *
*      Change the current section to a previously defined one.         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/22/89       created                  -----             *
***********************************************************************/

void change_section( short sec_num )
{
    register SEC_DATA *sect_data_ptr;
    register SCNHDR *sect_hdr_ptr;

    pc->section_num = sec_num;
    sect_data_ptr = &section_data[sec_num];
    sect_hdr_ptr = &section_header[sec_num];
    pc->type = sect_data_ptr->sec_type;

    if( !sect_data_ptr->is_open )
    {
	sect_data_ptr->file_desc = fopen( temp_file[sect_data_ptr->temp_file_index],
					  APPEND_BINARY );
	sect_data_ptr->is_open = AFFIRMATIVE;
    }

    pc->value = newpc = sect_hdr_ptr->s_size / 
      ((sect_hdr_ptr->s_flags & SECTION_PM) == SECTION_PM
       ? PM_WORD_SIZE : DM_WORD_SIZE);
    in_section = AFFIRMATIVE;
    set_current_section_type( sect_hdr_ptr->s_flags );

}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      end_section                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void end_section(void)                                          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for handling any end of section     *
*   processing.                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/17/89       created                  -----             *
***********************************************************************/

void end_section( void )
{
    register SCNHDR *sect_hdr_ptr;
    register SEC_DATA   *sect_data_ptr;


    sect_hdr_ptr = &section_header[pc->section_num];
    sect_data_ptr = &section_data[pc->section_num];

    in_section = FALSE;

    sect_hdr_ptr->s_size
      = newpc * ((sect_hdr_ptr->s_flags & SECTION_PM) == SECTION_PM
		 ? PM_WORD_SIZE : DM_WORD_SIZE);

    sect_data_ptr->is_open = FALSE;
    fclose( sect_data_ptr->file_desc );
}


/**********************************************************
 * set/get current_section_type                           *
 *                                                        *
 * routines to maintain the attributes of the most recent *
 * section                                                *
 **********************************************************/
void set_current_section_type( long new_type )
{
  current_section_type = new_type;
}


long get_current_section_type( void )
{
  return( current_section_type );
}
