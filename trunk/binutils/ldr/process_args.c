/* @(#)process_args.c   1.7 1.7 6/29/94 */

#include <stdio.h>
#ifdef MSDOS
#include <stddef.h>
#endif
#include <string.h>
#include <ctype.h>
/* 
  #include <malloc.h> 
  malloc should be in stdlib.h
*/

#include <stdlib.h>


#include "app.h"
#include "a_out.h"
#include "util.h"
#include "main.h"
#include "process_args.h"
#include "error.h"
#include "ldr.h"


int aflag = 1;
int mflag;
int rflag;
int sflag;
int xflag;

char *file_name_ptr;

char arch_descrip_file[MAX_FILENAME_LENGTH] = "21k.ach";
short error_debug = FALSE;
char **file_names;
int  num_input_files;
char output_file_name[MAX_FILENAME_LENGTH] = "";
char input_file_name[MAX_FILENAME_LENGTH] = "";

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_arguments                                               *
*                                                                      *
*   Synopsis                                                           *
*      void process_arguments( int argc, char **argv)                  *
*                                                                      *
*   Description                                                        *
*      Process the command line arguments.                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void process_arguments( int argc, char **argv )
{
    register int i;
    file_names = (char **) my_calloc( (long) ((1+argc) * sizeof(char **)) );

    for( i = 1; i < argc; i++)
    {
	 if( *argv[i] == '-' )
	 {      
	     switch( argv[i][1] )
	     {
		     case 'a':                  /* Architecture description file name */
		     case 'A':                  /* Architecture description file name */
			i++;
			if( !argv[i] )
			    FATAL_ERROR("-a switch used without an architecture file name");
			file_name_ptr = make_my_slash(argv[i], FALSE);
			strncpy( arch_descrip_file, file_name_ptr, MAX_FILENAME_LENGTH );
			append_ext (arch_descrip_file, ".ach");
			continue;

#ifdef DEBUG
		     case 'e':
		     case 'E':
			error_debug = 1;
			continue;
#endif
		     case 'o':                  /* Give the output a name */
		     case 'O':                  /* Give the output a name */
			i++;
			file_name_ptr = make_my_slash(argv[i], FALSE);
			strncpy( output_file_name, file_name_ptr, MAX_FILENAME_LENGTH);
			append_ext (output_file_name, ".exe");
			continue;

		     case 'h':
		     case 'H':
			usage ();
			exit (0);

		     case 'v':
		     case 'V':
			continue;

		     default:
			USER_WARN1( "Illegal flag: %s ignored.", argv[i] );
			continue;
	     }
	 }
	 else
	 {
	     file_name_ptr = make_my_slash(argv[i], FALSE);
	     strcpy(input_file_name, file_name_ptr);
	 }
    }
}
