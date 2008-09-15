/* @(#)error.c  1.6 5/13/91 1 */

#include "app.h"
#include "error.h"
#include "process_args.h"
#include "tempio.h"
#include <stdarg.h>

#include "error_fp.h"

extern FILE *map_file_fd;

static long linker_errors = 0l;
static long warnings = 0l;
static long user_errors = 0l;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      interrupt_handler                                               *
*                                                                      *
*   Synopsis                                                           *
*      void interrupt_handler()                                        *
*                                                                      *
*   Description                                                        *
*      This routine is called when a control-c or a kill signal is     *
*   sent to the linker. We trap these signals so that we can take      *
*   appropriate action before we exit.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void interrupt_handler( int signum )
{
   linker_exit( FATAL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      error                                                           *
*                                                                      *
*   Synopsis                                                           *
*      void error( long error_type, char *file, long line,             *
*                  char *error_string, ... )                           *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for actually printing the error     *
*  message.                                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
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
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void message( long error_type )
{
    putc( '\n', stderr );
    fprintf( stderr, "%%ld21000 - " );

    switch( (int) error_type )
    {
	case ERROR_WARNING:
	   fprintf( stderr, "Warning\n\n\t" );
	   break;

	case ERROR_USER_ERROR:
	   fprintf( stderr, "User Error\n\n\t" );
	   break;

	case ERROR_LINKER_ERROR:
	   fprintf( stderr, "Linker Error\n\n\t" );
	   break;

	case ERROR_SYNTAX_ERROR:
	   fprintf( stderr, "Syntax Error\n\n" );
	   break;

	case ERROR_FATAL:
	   fprintf( stderr, "FATAL ERROR\n\n" );
	   break;
    }
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      user_warn                                                       *
*                                                                      *
*   Synopsis                                                           *
*      void user_warn( char *file, long line, char *error_string, ... )*
*                                                                      *
*   Description                                                        *
*      Print a warning to the user.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void user_warn( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

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
*      user_error                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void user_error(char *file, long line, char *error_string, ...) *
*                                                                      *
*   Description                                                        *
*      Print a error to the user.                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
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

   if( user_errors >= MAX_ERRORS )
   {
       fprintf( stderr, "Maximum number of user errors exceeded.\n\n" );
       linker_exit( FATAL );
   }

}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      linker_error                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void linker_error( char *file, long line,                       *
*                         char *error_string, ... )                    *
*                                                                      *
*   Description                                                        *
*      Print a linker error.                                           * 
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void linker_error( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    if( user_errors )
    {
	fprintf( stderr, "Fatal error.\n\n\tCannot continue due to previous user errors.\n\n" );
	linker_exit( FATAL );
    }

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++linker_errors;
    error( ERROR_LINKER_ERROR, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      fatal_linker_error                                              *
*                                                                      *
*   Synopsis                                                           *
*      void fatal_linker_error( char *file, long line,                 *
*                                 char *error_string, ... )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing a fatal linker error.  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void fatal_linker_error( char *file, int line, char *error_string, ... ) 
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++linker_errors;
    error( ERROR_FATAL, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
    linker_exit( FATAL );
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
*      Determine if any errors have occurred.                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

short check_if_errors( void )
{
   return( (short) (user_errors | linker_errors) );
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
*   before the linker exits.                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void exit_summary( void )
{
    register int total_errors;

    total_errors = (int) (linker_errors + user_errors);

    fprintf( stderr, "\n%%ld21000 - Completed with " );
    fprintf( stderr, "%d Warning%c and ", warnings, warnings == 1 ? '\0' : 's' );
    fprintf( stderr, "%d Error%c.\n", total_errors, total_errors == 1 ? '\0' : 's' );

    if( user_errors || linker_errors )
	fprintf( stderr, "      No executable has been produced.\n" );
    putc( '\n', stderr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      linker_exit                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void linker_exit( int exit_code )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for any final chores before the     *
*   linker exits.                                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void linker_exit( int exit_code )
{
    if( check_if_errors() )
	unlink( output_file_name );

    if( mflag && map_file_fd )
	fclose( map_file_fd );

    if( check_if_errors() || warnings )
	exit_summary();

    delete_temp_files();
    exit( exit_code );
}

