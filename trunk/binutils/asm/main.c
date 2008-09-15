/* @(#)main.c   1.21 2/21/91 1 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#ifdef MSDOS
#include <stdlib.h>
#endif
#include "app.h"
#include "error.h"
#include "symbol.h"
#include "token.h"
#include "token_init.h"
#include "fileio.h"
#include "ieee.h"
#include "pass1.h"
#include "release.h"
#include "main.h"
#include "util.h"
#include "listing.h"

#ifdef ATFILE
#include "atfile.h"
#endif

short error_debug = FALSE;
short listing_flag = FALSE;
short Debug_symbols = FALSE;
short File_directive_needed = TRUE;
short Ansi_c_code = FALSE;
short Dsp_c_code = FALSE;
short Crts = TRUE;
short Pmstack = FALSE;
short True_doubles = TRUE;
short Flag_undefineds = FALSE;
short Gcc_compiled;
short Z3;

char list_name[MAX_FILE_NAME_SIZE];
char obj_name[MAX_FILE_NAME_SIZE], *file_name_ptr;
char src_name[MAX_FILE_NAME_SIZE];
char coff_name[MAX_FILE_NAME_SIZE];
char tryagain[MAX_FILE_NAME_SIZE];
FILE *src_stream = (FILE *) NULL;
FILE *duplicate_src_stream;
FILE *listing_file = (FILE *)NULL;

int   obj_name_flag = 0;
short pass = 0;
short processor = ADSP_21020;
short delete_preprocessor_output = FALSE;
char round_mode = RND_NEAREST;
short warn_flag = 1;

static void usage( void );
static void banner (void);


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      main                                                            *
*                                                                      *
*   Synopsis                                                           *
*      int main(argc, argv)                                            *
*      int argc;                                                       *
*      char **argc;                                                    *
*                                                                      *
*   Description                                                        *
*      This routine gets the command line arguments and start the      *
*   ball rolling by calling pass1.                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

int main( int argc, char **argv )
{
    register int arg;
    char *arg_str;
    char filename_seen;
    char root_name[MAX_FILE_NAME_SIZE];
    unsigned short verbose;

    Gcc_compiled = FALSE;
    Z3 = FALSE;

    init_std_io();

#ifdef ATFILE
    process_atfiles(&argc, &argv);
#endif

/* Check for verbose flag first */
   verbose = 0;
   for( arg = 1; arg < argc; arg++) {
     if( *argv[arg] == '-' ) {
       switch(*(argv[arg] + 1) ) {
	 case 'v':
	 case 'V':
	 case 'h':
	 case 'H':
	   verbose = 1;
	   break;
	 }
     }
   }
   if (verbose) {
     banner ();
   }




    if( argc < 2 )
    {
	banner ();
	usage();
	asm_exit( FATAL );
    }

    filename_seen = FALSE;

    for( arg=1; arg < argc; arg++ )
    {
	if( *argv[arg] == '-')
	{
	    arg_str = argv[arg];
	    arg_str++;

	    switch((*arg_str))
	    {

		    case 'l':   /* Listing file */
		    case 'L':   /* Listing file */
		       listing_flag = 1;
		       break;

		    case 'h':   /* Usage */
		    case 'H':   /* Usage */
		       usage ();
		       asm_exit (SUCCESS);

		    case 'r':  /* remove preprocessor output file */
		    case 'R':  /* remove preprocessor output file */
		       delete_preprocessor_output = AFFIRMATIVE;
		       break;

		    case 'e':
		    case 'E':
		       error_debug = 1;
		       break;

		    case 'v':
		    case 'V':
		       break;

		    case 'w':
		    case 'W':
		       warn_flag = 0;
		       break;

		    case 'o':
		    case 'O':
		       arg++;
		       obj_name_flag = 1;
		       file_name_ptr = make_my_slash( argv[arg], FALSE );
		       strncpy( obj_name, file_name_ptr, MAX_FILE_NAME_SIZE );
		       append_ext (obj_name, ".obj");
		       break;

		    case 'g':
		    case 'G':
			Debug_symbols = 1;
			break;

		    case 'c':
		    case 'C':
			if ((*(arg_str+1) == 'd') || (*(arg_str+1) == 'D'))
				Dsp_c_code = TRUE;
			else
				Ansi_c_code = TRUE;

/* this is obsolete
			if ((*(arg_str+2) == 'p') || (*(arg_str+2) == 'P'))
				Pmstack = TRUE;
			else
				Pmstack = FALSE;

			if ((*(arg_str+3) == 'c') || (*(arg_str+3) == 'C'))
				Crts = TRUE;
			else
				Crts = FALSE;

			if ((*(arg_str+4) == 'f') || (*(arg_str+4) == 'F'))
				True_doubles = FALSE;
			else
				True_doubles = TRUE;
*/
			break;
		    case 'u':
		    case 'U':
			Flag_undefineds = TRUE;
			break;

		     case 'a':
		     case 'A':
		       if(!strncmp((char *)(arg_str+4),"21060",5))
			   Z3 = TRUE;
		       else {
			   fprintf( stderr,"Illegal switch '%s'\n", arg_str );
			   usage();
			   asm_exit(FATAL);
		       }
		       break;

		    default :   /* illegal switch */
		       fprintf( stderr,"Illegal switch '%c'\n", *arg_str );
		       usage();
		       asm_exit(FATAL);
		}

	}
	else
	{
	    if( filename_seen )
	    {
		USER_ERROR( "Only one source file may be specified" );
		asm_exit(FATAL);
	    }

	    filename_seen = 1;
	    file_name_ptr = make_my_slash(argv[arg], FALSE);
	    strcpy(src_name, file_name_ptr);
	}
    }

/* Make sure we have the source file */
   if (!filename_seen) {
     usage ();
     asm_exit (FATAL);
   }

/* Open the source file */
    append_ext (src_name, ".asm");
    strcpy(coff_name, src_name);
    if( (src_stream = fopen(src_name, READ_TEXT) ) != NULL ) {
      fclose( src_stream );
      src_stream = 0;
    } else {
      FATAL_ERROR1( "Cannot open input file %s ", src_name );
    }

/* Construct the other filenames */
    strcpy(root_name, src_name);
    strip_path (root_name);
    strip_ext (root_name);

    if( !obj_name_flag )
    {
	strcpy(obj_name, root_name);
	append_ext (obj_name, ".obj");
    }

    strcpy(list_name, root_name);
    append_ext (list_name, ".lst");

    if( listing_flag )
    {
	if( (duplicate_src_stream = fopen(src_name, READ_TEXT)) == NULL )
	{
	    fprintf( stderr, "Error opening %s for input.\n", src_name );
	    asm_exit( FATAL );
	}

       if( (listing_file = fopen(list_name, WRITE_TEXT)) == NULL)
       {
	   fprintf( stderr, "Error opening %s for output.\n", list_name );
	   asm_exit( FATAL );
       }

       listing_banner ();

    }

    token_make_null();
    token_init();
    symbol_make_null();
    yylineno = 1;

    pass1();
#ifdef ATFILE
    free_argv();
#endif
    asm_exit( SUCCESS );
    return 0;
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      usage                                                           *
*                                                                      *
*   Synopsis                                                           *
*      static void usage                                               *
*                                                                      *
*   Description                                                        *
*      Show the user how to use it.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     10/20/89      created                  -----             *
***********************************************************************/

static void usage( void )
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "Usage: a21000 [-h -l -o name -r -v -w] filename\n");
    fprintf( stderr, "Where:\n");
    fprintf( stderr, "\t-h\tDisplay usage\n" );
    fprintf( stderr, "\t-l\tGenerate a listing file\n" );
    fprintf( stderr, "\t-o name\tName the output object file\n" );
    fprintf( stderr, "\t-r\tRemove pre-processor output\n" );
    fprintf( stderr, "\t-v\tVerbose\n" );
    fprintf( stderr, "\t-w\tSuppress warning messages\n" );
    fprintf( stderr, "\n" );
}




/***********************************************************************
*                                                                      *
*   Name                                                               *
*      banner                                                          *
*                                                                      *
*   Synopsis                                                           *
*      static void banner                                               *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

static void banner (void) {

     printf( "\nAnalog Devices ADSP-210x0 Assembler\n");
     printf( "Release %s, Version %s\n", get_release_number(), get_version_number());
     printf( "%s\n", get_copyright_notice() );
     printf( "%s\n", get_warranty_notice() ); /*EK*/
     printf( "%s\n\n", get_license_notice() ); /*EK*/
   }
