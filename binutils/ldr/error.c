/* @(#)error.c	1.8 1.8 6/21/93 */

#include <stdio.h>
#include <stdlib.h>
#include "app.h"
#include "a_out.h"
#include "ldr.h"
#include "error.h"
#include "process_args.h"
#include <stdarg.h>
#ifdef MSDOS
#include <string.h>
#endif /* MSDOS */

long mem_errors = 0;

static long warnings = 0;
static long user_errors = 0;


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
*   sent to the mem. We trap these signals so that we can take      *
*   appropriate action before we exit.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void interrupt_handler( void )
{
   mem_exit( FATAL );
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
    fprintf( stderr, "%%mem21k - " );

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
       mem_exit( FATAL );
   }

}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      mem_error                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void mem_error( char *file, long line,                       *
*                         char *error_string, ... )                    *
*                                                                      *
*   Description                                                        *
*      Print a mem error.                                           * 
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void mem_error( char *file, int line, char *error_string, ... )
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    if( user_errors )
    {
        fprintf( stderr, "Fatal error.\n\n\tCannot continue due to previous user errors.\n\n" );
        mem_exit( FATAL );
    }

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++mem_errors;
    error( ERROR_LINKER_ERROR, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      fatal_mem_error                                              *
*                                                                      *
*   Synopsis                                                           *
*      void fatal_mem_error( char *file, long line,                 *
*                                 char *error_string, ... )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing a fatal mem error.  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void fatal_mem_error( char *file, int line, char *error_string, ... ) 
{
    char *oa1, *oa2, *oa3, *oa4, *oa5;
    va_list optional;

    va_start(optional, error_string);
    oa1 = va_arg( optional, char * );
    oa2 = va_arg( optional, char * );
    oa3 = va_arg( optional, char * );
    oa4 = va_arg( optional, char * );
    oa5 = va_arg( optional, char * );

    ++mem_errors;
    error( ERROR_FATAL, file, line, error_string, oa1, oa2, oa3, oa4, oa5 );
    mem_exit( FATAL );
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
   return( (short) (user_errors | mem_errors) );
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
*   before the mem exits.                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void exit_summary( void )
{
    register int total_errors;

    total_errors = (int) (mem_errors + user_errors);

    fprintf( stderr, "\n%%mem21k - Completed with " );
    fprintf( stderr, "%ld Warning%s and ", warnings, warnings == 1 ? "" : "s" );
    fprintf( stderr, "%d Error%s.\n", total_errors, total_errors == 1 ? "" : "s" );

    if( user_errors || mem_errors )
        fprintf( stderr, "      No executable has been produced.\n" );
    putc( '\n', stderr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      mem_exit                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void mem_exit( int exit_code )                                  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for any final chores before the     *
*   mem exits.                                                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void mem_exit( int exit_code )
{
    if( check_if_errors() )
	if( strcmp(output_file_name, input_file_name) )
        	unlink( output_file_name );

    if( check_if_errors() || warnings )
        exit_summary();

    exit( exit_code );
}

