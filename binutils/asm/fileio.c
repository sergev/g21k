/* @(#)fileio.c 2.3 10/1/96 2 */

#include "app.h"

#ifdef TRUE
#undef TRUE
#endif

#include "symbol.h"
#include "fileio.h"
#include "expr.h"
#include "lexical.h"
#include "error.h"
#include "y_tab.h"


extern long num_initializers;
FILE *file_ptr = NULL;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      input_char                                                      *
*                                                                      *
*   Synopsis                                                           *
*      short input_char(void)                                          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning the next input        *
*   character from a file.                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/15/89       created                  -----             *
***********************************************************************/

int input_char( void )
{
    register int c;

    if( (c = getc(file_ptr)) == EOF )
	 return( 0 );
    else
	 return( c );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      unput_char                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void unput_char(c)                                              *
*      char c;                                                         *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for pushing the given character     *
*   back onto the source file stream;                                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

void unput_char(char c)
{
    ungetc( c, file_ptr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      yywrap                                                          *
*                                                                      *
*   Synopsis                                                           *
*      int yywrap(void)                                                *
*                                                                      *
*   Description                                                        *
*      This routine is called by lex whenever an end-of-file is        *
*   encountered, we return a 1 to instruct lex to perform normal wrapup*
*   when an end-of-file is encountered.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

int yywrap( void )
{
    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      read_data_file                                                  *
*                                                                      *
*   Synopsis                                                           *
*      void read_data_file( char *filename )                           *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for reading a data file in order    *
*   to initialize a array. To get around having to parse the data file *
*   ourselves, we redirect the input of lex to the data file and       *
*   let lex parse the input for us.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     9/14/89       created                  -----             *
***********************************************************************/

void read_data_file( char *filename )
{
    FILE          *data_file;
    EXPR           expr;
    register EXPR *expr_ptr = &expr;
    register int   token;
    int            lineno;
    int            minus_flag;

    data_file = fopen( filename, READ_TEXT );
    if( data_file == NULL )
	FATAL_ERROR1( "Error opening data file %s for input.", filename );

    num_initializers = 0;
    set_file_ptr( data_file );
    minus_flag = 0;
    lineno = 1;
    while( (token = yylex()) )
    {
	   switch( token )
	   {
		   case INT_CONST:
		      expr_ptr->expr_type = EXPR_TYPE_INT;
		      if( minus_flag )
		      {
			  expr_ptr->value.int_val = -yylval.lval;
			  minus_flag = 0;                         
		      }
		      else
			  expr_ptr->value.int_val = yylval.lval;
		      expr_ptr->sym_ptr = NULL;
		      expression_process_initializers( expr_ptr );
		      num_initializers++;
		      break;

		   case HEX40_CONST:
		      expr_ptr->expr_type = EXPR_TYPE_HEX40;
		      if( minus_flag )
		      {
			fprintf( stderr, "%%asm21000 -  %s, line %d: FATAL ERROR\n\n",
			       filename, lineno  );
			fprintf( stderr, "Illegal format in initialization file.\n" );
			asm_exit( FATAL );
		      }
		      memcpy(expr_ptr->value.hex40_val,yylval.hex40,5);
		      expr_ptr->sym_ptr = NULL;
		      expression_process_initializers( expr_ptr );
		      num_initializers++;
		      break;


		   case REAL:
		      expr_ptr->expr_type = EXPR_TYPE_FLOAT;
		      if( minus_flag )
		      {
			  expr_ptr->value.float_val = -yylval.dval;
			  minus_flag = 0;                         
		      }
		      else
			  expr_ptr->value.float_val = yylval.dval;
		      expr_ptr->sym_ptr = NULL;
		      expression_process_initializers( expr_ptr );
		      num_initializers++;
		      break;

		   case NEW_LINE:
		      lineno++;
		      break;

		   case '-':
		      minus_flag = 1;
		      break;

		   case '+':
		      break;

		   default:
		      fprintf( stderr, "%%asm21000 -  %s, line %d: FATAL ERROR\n\n",
			       filename, lineno  );
		      fprintf( stderr, "Illegal format in initialization file.\n" );
		      asm_exit( FATAL );
		      break;
	   }

    }
    set_file_ptr( src_stream );
    fclose( data_file );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*       init_std_io                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void init_std_io( void )                                        *
*                                                                      *
*   Description                                                        *
*      Initialize lex standard input and output. stdin and stdout      *
*      were assumed to be constants.                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     egil    7/14/2001     created                  -----             *
***********************************************************************/

void init_std_io( void )
{
   extern FILE *yyin, *yyout;

   /* for lex_yy.c */
   yyin = stdin;
   yyout = stdout;
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      set_file_ptr                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void set_file_ptr( FILE *file )                                 * 
*                                                                      *
*   Description                                                        *
*      Direct the input of lex to the given file pointer.              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     9/14/89       created                  -----             *
***********************************************************************/

void set_file_ptr( FILE *file )
{
   file_ptr = file;    
}


void advance_to_semi( void )
{
    int c;
    while( (c = getc(file_ptr)) != ';' && c != EOF )
       ;

    /* Stops case where missing semicolon at the end of a file (e.g 
       .ENDSEG produces a barrage of error messages */

    if (c == EOF)
    {
	 fprintf( stderr, "End of File encountered.\n" );
	 asm_exit( FATAL );
    }
}
