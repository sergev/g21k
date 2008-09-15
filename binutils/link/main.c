/* @(#)main.c   2.7 6/1/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "error.h"
#include "main.h"
#include "process_args.h"
#include "release.h"
#include <signal.h>
#include "util.h"

#include "main_fp.h"
#include "error_fp.h"
#include "process_args_fp.h"
#include "pass1_fp.h"

#ifdef ATFILE
#include "atfile.h"
#endif

char map_file[MAX_FILENAME_LENGTH];
FILE *map_file_fd = NULL;

#define MAX_LIBRARY_SEARCH_PATHS 10
char *library_search_paths[MAX_LIBRARY_SEARCH_PATHS];
int lib_paths;

int Verbose;

static void banner (void);
static short banner_printed = 0;

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

#ifdef ATFILE
    process_atfiles(&argc, &argv);
#endif

   lib_paths = 0;

/* Check for verbose flag first */
   Verbose = 0;
   for( arg = 1; arg < argc; arg++) {
     if( *argv[arg] == '-' ) {
       switch(*(argv[arg] + 1) ) {
	 case 'v':
	 case 'V':
	 case 'h':
	 case 'H':
	   Verbose = 1;
	   break;

/* We do the library paths here so that they will be available
 * for the -lxx switches later on.
 */
	 case 'L':
	   library_search_paths[lib_paths++] = make_my_slash(&argv[arg][2], 1);
	   if( lib_paths > MAX_LIBRARY_SEARCH_PATHS )
	       FATAL_ERROR( "Too many library search paths specified\n" );
	   break;
	        

	 }
     }
   }
   if (Verbose) {
     if (!banner_printed) {
       banner ();
       banner_printed = 1;
     }
   }

    init_pass1();
    process_arguments( argc, argv );

/* Make sure we have a file to link */
   if (!(*output_file_name)) {
     if (!file_names[0]) {
       if (!banner_printed) {
	 banner ();
	 banner_printed = 1;
       }
       usage ();
       exit (1);
     }
   }

    /* If the user didn't give us a name for the executable,
     * use the first file in the list and append '.exe' to it.
     */

    if( !(*output_file_name) )
    {
	strncpy( output_file_name, file_names[0], MAX_FILENAME_LENGTH );
	strip_path (output_file_name);
	strip_ext (output_file_name);
    }
    append_ext (output_file_name, ".exe");

    if( rflag && sflag )
    {
	USER_WARN( "The -r and -s flags are set. -s flag turned off" );
	sflag = 0;
    }

    if( rflag && (aflag < 2) )
	aflag = 0;


    if( mflag )
    {
      /* Create map file name and open it */
      strncpy( map_file, output_file_name, MAX_FILENAME_LENGTH );
      strip_path (map_file);
      strip_ext (map_file);
      append_ext (map_file, MAP_FILE_EXTENSION);
      if( (map_file_fd = fopen( map_file, UPDATE_TEXT)) == NULL )
      {
	FATAL_ERROR( "Error opening map file %s for output." );
      }
    }

    pass1();

    if( mflag )
	fclose( map_file_fd );
#ifdef ATFILE
    free_argv();
#endif
    linker_exit( PASSED );
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
	fprintf( stderr, "Usage: ld21k [-a file -h -i file -m -o file -lxxx -r -s -v -x] file1 file2 etc...\n" );
	fprintf( stderr, "Where:\n");
	fprintf( stderr, "\t-a\tSpecify architecture description file\n");
	fprintf( stderr, "\t-i\tSpecify file containing list of object files\n");
	fprintf( stderr, "\t-h\tDisplay usage\n");
	fprintf( stderr, "\t-m\tGenerate memory map file\n");
	fprintf( stderr, "\t-o\tSpecify executable filename\n");
	fprintf( stderr, "\t-lxxx\tAdd library name libxxx.a to link\n");
	fprintf( stderr, "\t-Lpath\tAdd path name to search of libraries\n");
/*         fprintf( stderr, "\t-r\tIgnore unresolved references\n"); */
	fprintf( stderr, "\t-s\tStrip symbol table from executable\n");
	fprintf( stderr, "\t-v\tVerbose\n");
	fprintf( stderr, "\t-x\tStrip local symbol info\n");
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

  printf( "\nAnalog Devices ADSP-210x0 Linker\n");
  printf( "Release %s, Version %s\n", get_release_number(), get_version_number());
  printf( "%s\n", get_copyright_notice() );
  printf( "%s\n", get_warranty_notice() ); /*EK*/
  printf( "%s\n\n", get_license_notice() ); /*EK*/

}
