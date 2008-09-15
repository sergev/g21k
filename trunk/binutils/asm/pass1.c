/* @(#)pass1.c  2.2 8/20/92 2 */

#include "app.h"
#include "symbol.h"
#include "expr.h"
#include "str.h"
#include "error.h"
#include "fileio.h"
#include "ieee.h"
#include "main.h"
#include "pass1.h"
#include "pass2.h"
#include "ilnode.h"
#include "lexical.h"
#include "tempio.h"
#include <signal.h>

long   newpc = 0;
SYMBOL *pc = (SYMBOL *) NULL;
char  precision = EXTENDED_PRECISION;
short size_char = 0;
short size_long = 0;
extern short in_section;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      pass1                                                           *
*                                                                      *
*   Synopsis                                                           *
*      short pass1(void)                                               *
*                                                                      *
*   Description                                                        *
*      This routine is the master control routine for pass1, it calls  *
*   yyparse to parse the assembly source file, flushes the section     *
*   buffers and calls pass2 if there were no errors.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

short pass1( void )
{
    short parse_error;

    /* Let's use '.' as the current location counter */

    if( (src_stream = fopen(src_name, READ_TEXT)) == NULL )
	 FATAL_ERROR("Unable to open input file");

    set_file_ptr( src_stream );   /* Direct the input of lex to this file */

    if( signal(SIGINT, SIG_IGN) == SIG_DFL )
	signal( SIGINT, interrupt_handler );
    if( signal(SIGTERM, SIG_IGN) == SIG_DFL )
	signal( SIGTERM, interrupt_handler );

    string_table_init();
    pc = symbol_insert( ".", 0L, 0L, STYPE_UNDEFINED, 0L );
    pc->value = newpc = 0;
    pc->section_num   = 0;
    pass = 1;
    if ((parse_error = yyparse()))
	USER_ERROR("Badly Formed Instruction.");
    fclose( src_stream );
    src_stream = 0;

    if (!parse_error && in_section) {
	USER_ERROR("Missing endseg directive.");
	parse_error = TRUE;
    }

    flush_temp_files();

    if ( !check_if_errors() && (!parse_error) )
	 return( pass2() );
    else
    {
	 asm_exit( FATAL );
    }
}
