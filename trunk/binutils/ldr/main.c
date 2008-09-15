/* @(#)main.c   1.4 5/18/91 1 */

#include <stdio.h>
#ifdef MSDOS
#include <stddef.h>
#endif
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "app.h"
#include "process_args.h"
#include "util.h"
#include "error.h"
#include "release.h"
#include "a_out.h"
#include "ldr.h"

#ifdef ATFILE
#include "atfile.h"
#endif

#define TEMP_PREFIX "memtmp"
#define BUFFER_SIZE 1024

static char buffer[BUFFER_SIZE];

static char tmp_file[MAX_FILENAME_LENGTH];

FILE *input_fd, *output_fd;;

static void banner (void);
static short banner_printed = 0;

extern long int mem_errors;

static FILE *temp_file_create( char *type )
{
    FILE *desc;

    strcpy(tmp_file, tempnam( ".", TEMP_PREFIX ));
    desc = fopen( tmp_file, type );
    if( !desc )
	FATAL_ERROR("Could not open temporary file");
    return desc;
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      main                                                            *
*                                                                      *
*   Synopsis                                                           *
*      int main( int argc, char *argv[] )                              *
*                                                                      *
*   Description                                                        *
*      21000 main driver.                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

int main( int argc, char *argv[] )
{
    unsigned short arg;
    unsigned short verbose;
    int i;
      
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
		verbose = 1;
		break;
	    }
	}
    }
    if (verbose) {
	if (!banner_printed) {
	    banner ();
	    banner_printed = 1;
	}
    }
       
    init_pass1();
    process_arguments( argc, argv );
       
    /* Make sure we have a file to link */
    if (!(*output_file_name)) 
    {
	if (!*input_file_name) 
	{
	    if (!banner_printed) 
	    {
		banner ();
		banner_printed = 1;
	    }
	    usage ();
	    exit (1);
	}
    }
       
    if( strstr(input_file_name, ".") == NULL )
	strcat(input_file_name, ".exe");

    if( !(*output_file_name) )
    {
	strcpy( output_file_name, input_file_name );
    }

    if( strcmp(input_file_name, output_file_name) == 0 )
	output_fd = tmpfile();
    else
	if( (output_fd = fopen(output_file_name, WRITE_BINARY)) == NULL )
		FATAL_ERROR1( "Can't open file %s for output", output_file_name );

    if( (input_fd = fopen(input_file_name, READ_BINARY)) == NULL )
	FATAL_ERROR1( "Can't open file %s for input", input_file_name );
       
    make_standalone();

    fclose(input_fd);
    if( (strcmp(input_file_name, output_file_name) == 0) && (mem_errors == 0) )
    {
	fseek(output_fd, 0L, 0);
    
	unlink(input_file_name);

	if( (input_fd = fopen(input_file_name, WRITE_BINARY)) == NULL )
		FATAL_ERROR1( "Can't open file %s for output", input_file_name );
	
	while( (i = fread(buffer, 1, BUFFER_SIZE, output_fd)) != 0 )
		fwrite(buffer, 1, i, input_fd);
	fclose(input_fd);
   } 
   else
	fclose(output_fd);

   mem_exit( PASSED );
   return 0;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      init_pass1                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void init_pass1( void )                                         *
*                                                                      *
*   Description                                                        *
*      Do any pass 1 initialization.                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/22/89       created                  -----             *
***********************************************************************/

void init_pass1( void )
{
    if (signal(SIGINT, SIG_IGN) == SIG_DFL)
	signal(SIGINT, interrupt_handler);

    if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
	signal(SIGTERM, interrupt_handler);
}






/***********************************************************************
*                                                                      *
*   Name                                                               *
*      usage                                                           *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

void usage (void) {

	fprintf( stderr, "\n" );
	fprintf( stderr, "Usage: mem21k [-a file -h -o file -v ] infile\n" );
	fprintf( stderr, "Where:\n");
	fprintf( stderr, "\t-a\tSpecify architecture description file\n");
	fprintf( stderr, "\t-h\tDisplay usage\n");
	fprintf( stderr, "\t-v\tVerbose\n");
	fprintf( stderr, "\t-o\tSpecify output file name\n");
	fprintf( stderr, "\n" );

      }



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      banner                                                          *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

void banner (void) {

  printf( "\nAnalog Devices ADSP-210x0 Initializer\n");
  printf( "Release %s, Version %s\n", get_release_number(), get_version_number());
  printf( "%s\n", get_copyright_notice() );
  printf( "%s\n", get_warranty_notice() ); /*EK*/
  printf( "%s\n\n", get_license_notice() ); /*EK*/

}
