/* @(#)error.c  2.1 5/4/92 2 */

#include "app.h"
#include "main.h"
#include "symbol.h"
#include "tempio.h"
#include "error.h"
#include <signal.h>
#include <stdarg.h>

static int assembler_errors = 0;
static int warnings = 0;
static int user_errors = 0;
static int syntax_errors = 0;
extern FILE *file_ptr;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      interrupt_handler                                               *
*                                                                      *
*   Synopsis                                                           *
*      void interrupt_handler( void )                                  *
*                                                                      *
*   Description                                                        *
*      This routine is called when a control-c or a kill signal is     *
*   sent to the assembler. We trap these signals so that we can delete *
*   the temporary files.                                               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void interrupt_handler( int signum )
{
   asm_exit( FATAL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      error                                                           *
*                                                                      *
*   Synopsis                                                           *
*      void error( long error_type, char *file, int line,             *
*                  char *error_string, ... )                           *
*   Description                                                        *
*      This routine is responsible for actually printing the error     *
*  message.                                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
*     mkc     7/4/89        updated with va_arg                        *
***********************************************************************/

void error( long error_type, char *file, int line, char *error_string, ... )
{
    char *a1, *a2, *a3, *a4, *a5;
    va_list optional;

    if( error_debug )
	printf("__FILE__ = %s, __LINE__ = %d\n", file, line);

    va_start(optional, error_string);
    a1 = va_arg( optional, char * );
    a2 = va_arg( optional, char * );
    a3 = va_arg( optional, char * );
    a4 = va_arg( optional, char * );
    a5 = va_arg( optional, char * );

    message( error_type );
    fprintf( stderr, error_string, a1, a2, a3, a4, a5 );
    putc( '\n', stderr );
    putc( '\n', stderr );

    if( (assembler_errors + user_errors + syntax_errors) > MAX_ERRORS )
    {
	 fprintf( stderr, "Maximum number of errors exceeded.\n\n" );
	 asm_exit( FATAL );
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      message                                                         *
*                                                                      *
*   Synopsis                                                           *
*      void message( long error_type )                                 *
*                                                                      *
*   Description                                                        *
*      This routine prints the type of error and puts a canned header  *
*   on each error message.                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void message( long error_type )
{
    long           pos;
    char           buf[BUFSIZ];
    register int   i;
    char           *ptr;
    int            num_tabs;

    putc( '\n', stderr );
    fprintf( stderr, "%%asm21000 - %s, line %d: ", src_name, (long) yylineno );

    switch( (int)error_type )
    {
	case ERROR_WARNING:
	   fprintf( stderr, "Warning\n\n\t" );
	   break;

	case ERROR_USER_ERROR:
	   fprintf( stderr, "User Error\n\n\t" );
	   break;

	case ERROR_ASSEMBLER_ERROR:
	   fprintf( stderr, "Assembler Error\n\n\t" );
	   break;

	case ERROR_SYNTAX_ERROR:
	   fprintf( stderr, "Syntax Error\n\n\t" );
	   break;

	case ERROR_FATAL:
	   fprintf( stderr, "Fatal Error\n\n\t" );
	   break;
    }


    if( pass == 1 )
    {
	fprintf( stderr, "%s", lex_buff );
	fflush( stderr );

	pos = ftell( file_ptr );
	fgets( buf, BUFSIZ, file_ptr );

	fseek( file_ptr, pos, 0 );
	fprintf( stderr, "%s", buf );
	fflush( stderr );

#ifndef MSDOS
	if( error_type == ERROR_SYNTAX_ERROR )
	{
	    fputc( '\t', stderr );

	    /* If there are tabs at the beginning of a line we
	     * must account for it on the output.
	     */

	    ptr = lex_buff;
	    num_tabs = 0;
	    while( ptr && *ptr++ == '\t' )
	    {
		   fputc( '\t', stderr );
		   num_tabs++;          
	    }

	    for( i = 0; i < strlen(lex_buff) - strlen(yytext) - num_tabs; ++i )
		 fputc( '-', stderr );

	    fputc( '^', stderr );
	    for( i= 0; i < strlen(buf) + strlen(yytext) - ((strlen(buf) <= 1) ? 3 : 2); i++ )
		 fputc( '-', stderr );    
	  }

	fprintf( stderr, "\n\n\t" );    
#endif
      }
ptr;
num_tabs;
i;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      yyerror                                                         *
*                                                                      *
*   Synopsis                                                           *
*      yyerror( char *string )                                         *
*                                                                      *
*   Description                                                        *
*      This routine is called by the parser when it detects a syntax   *
*   error in the input stream.                                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void yyerror( char *string )
{
    string;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      user_warn                                                       *
*                                                                      *
*   Synopsis                                                           *
*      void user_warn( char *file, int line, char *error_string, ... )*
*                                                                      *
*   Description                                                        *
*      Print a warning to the user.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
*     chill   8/4/89        updated with va_arg handling               *
***********************************************************************/

void user_warn( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    if( !warn_flag )
	return;
    
    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

   ++warnings;
   error( ERROR_WARNING, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
   fflush( stderr );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      syntax_error                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void syntax_error( char *file, int line,                       *
*                         char *error_string, ... )                    *
*                                                                      *
*   Description                                                        *
*      Print a error syntax to the user.                               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/7/89       created                  -----             *
***********************************************************************/

void syntax_error( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

   ++syntax_errors;
   error( ERROR_SYNTAX_ERROR, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
   fflush( stderr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      user_error                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void user_error(char *file, int line, char *error_string, ...) *
*                                                                      *
*   Description                                                        *
*      Print a error to the user.                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void user_error( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

   ++user_errors;
   error( ERROR_USER_ERROR, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
   fflush( stderr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      assembler_error                                                 *
*                                                                      *
*   Synopsis                                                           *
*      void assembler_error( char *file, int line,                    *
*                            char *error_string, ... )                 *
*                                                                      *
*   Description                                                        *
*      Print a assembler error.                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
*     mkc     8/4/89        updated with va_arg handling               *
***********************************************************************/

void assembler_error( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    if( user_errors )
    {
	fprintf( stderr, "Fatal error.\n\n\tCannot continue due to previous user errors.\n\n" );
	asm_exit( FATAL );
    }

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++assembler_errors;
    error( ERROR_ASSEMBLER_ERROR, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      fatal_assembler_error                                           *
*                                                                      *
*   Synopsis                                                           *
*      void fatal_assembler_error( char *file, int line,              *
*                         char *error_string, ... )                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing a fatal assembler      *
*  error.                                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
*     mkc     8/4/89        updated with va_arg handling               *
***********************************************************************/

void fatal_assembler_error( char *file, int line, char *error_string, ... ) 
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++assembler_errors;
    error( ERROR_FATAL, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
    asm_exit( FATAL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      check_if_errors                                                 *
*                                                                      *
*   Synopsis                                                           *
*      short check_if_errors( void )                                   *
*                                                                      *
*   Description                                                        *
*      Determine if any errors have occurred.                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

short check_if_errors( void )
{
   return( user_errors | assembler_errors | syntax_errors );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      delete_temp_files                                               *
*                                                                      *
*   Synopsis                                                           *
*      void delete_temp_files( void )                                  *
*                                                                      *
*   Description                                                        *
*      Delete the temporary files.                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void delete_temp_files( void )
{
    register short i;

    for( i = 0; i < num_open_files; ++i )
	 unlink( temp_file[i] );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      exit_summary                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void exit_summary( void )                                       *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing a summary message      *
*   before the assembler exits.                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
*     mkc     11/2/89       No longer include yynerrs in total_errors  *
***********************************************************************/

void exit_summary( void )
{
    register int total_errors;

    total_errors = assembler_errors + user_errors + syntax_errors;

    fprintf( stderr, "\n%%asm21000 - Completed with " );
    fprintf( stderr, "%d Warning%c, ", warnings, warnings == 1 ? '\0' : 's' );
    fprintf( stderr, "and %d Error%c.\n", total_errors, total_errors == 1 ? '\0' : 's' );
    fprintf( stderr, "      At line %d in file %s\n", yylineno, src_name );

    if( total_errors )
    {
	if( listing_flag )
	    fprintf( stderr, "      No object code or listing file has been produced.\n" );
	else
	    fprintf( stderr, "      No object code has been produced.\n" );
    }


    putc( '\n', stderr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      asm_exit                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void asm_exit( int exit_code )                                  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for any final chores before the     *
*   assembler exits.                                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/21/89       created                  -----             *
***********************************************************************/

void asm_exit( int exit_code )
{

    if( src_stream != NULL )
	fclose( src_stream );

    if( duplicate_src_stream != NULL )
	fclose( src_stream );

    if( listing_file != NULL )
    {
	fclose( listing_file );
	if( exit_code == FATAL || check_if_errors() )
	    unlink( list_name );        
    }
     
    if( exit_code == FATAL )
	unlink( obj_name );

    if( delete_preprocessor_output )
	unlink( src_name );

    if( check_if_errors() || warnings )
	exit_summary();

    delete_temp_files();
    exit( exit_code );
}

