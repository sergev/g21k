/* @(#)pass1.c  2.6 5/28/93 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#ifdef MSDOS
#include <io.h>
#endif /* MSDOS */

#include "app.h"
#include "a_out.h"
#include "ar.h"
#include "error.h"
#include "ldfcn.h"
#include "process_args.h"
#include "util.h"

#include "pass1_fp.h"
#include "pass2_fp.h"
#include "read_object_fp.h"
#include "error_fp.h"
#include "achparse.h"

extern int Verbose;
extern int Old_library_flag;
extern char *getenv();
extern int yyparse( void );
char *current_file_name = NULL;

extern char *library_search_paths[];
extern int lib_paths;

void fopen_library(int i, char *path)
{
    char *pc, *pc2;
    char *save_env;
    char library_name[MAX_FILENAME_LENGTH];
    int j;
	
    strcpy(library_name, file_names[i]);
	
    if( lib_paths )
    {
	for(j=0; j<lib_paths; j++)
	{
	    file_names[i] = (char *)my_malloc((long)(strlen(library_search_paths[j])+15));
	    strcpy(file_names[i], library_search_paths[j]);
	    strcat(file_names[i], library_name);
	    if( !access(file_names[i],0) )
	    {
		if( Verbose )
		    fprintf(stderr, "Using library %s\n", file_names[i]);
		return;
	    }
	    my_free(file_names[i]);
	}
    }
	
    pc = getenv(path);
    if( pc ) 
    {    
	save_env=(char *)my_malloc((long)(15+strlen(pc)+strlen(file_names[i])));
	pc2=save_env;
	for(; *pc != ';' && *pc != '\0'; )
	    *pc2++ = *pc++;
	*pc2-- = '\0';
	if( *pc2 == FILENAME_SEPARATOR )
	    *pc2 = '\0';
	
#ifdef MSDOS
	strcat(save_env, "\\21k\\lib\\");
#else
	strcat(save_env, "/21k/lib/");
#endif 
	strcat(save_env, library_name);
	file_names[i] = save_env;
    }

    if( Verbose && !access(file_names[i],0) )
    {
	fprintf(stderr, "Using library %s\n", file_names[i]);
	return;
    }

    if( access(file_names[i],0) )
	FATAL_ERROR1("Unable to open library %s\n", library_name);
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      pass1                                                           *
*                                                                      *
*   Synopsis                                                           *
*     void pass1( void )                                               * 
*                                                                      *
*   Description                                                        *
*      This routine reads the architecture description file, and also  *
*   reads in and builds info for each input (object) file.             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/23/89       created                  -----             *
***********************************************************************/

void pass1( void )
{
    char           *pc;
    char            atype[SARMAG];
    FILE           *filedes;
    long            firstword;
    register int    i;
    unsigned short  magic;


    if (ach_parse( arch_descrip_file ))
	FATAL_ERROR( "Cannot continue due to previous errors\n" );

    for( i = 0; i < num_input_files; ++i )
    {
	pc = strchr(file_names[i], '.');
	if ( Old_library_flag && pc && \
		(strncmp(file_names[i], "lib", 3) == 0 || \
		 strncmp(file_names[i], "LIB", 3) == 0) && \
		(strncmp(pc, ".a", 2) == 0 || \
		 strncmp(pc, ".A", 2) == 0))
			fopen_library(i, "ADI_DSP");
	if( (filedes = fopen(file_names[i], READ_BINARY)) == NULL )
		FATAL_ERROR1( "Can't open file %s for input", file_names[i] );
	current_file_name = file_names[i];
	      
	 fseek( filedes, 0L, 0);
	 if( fread( &magic, sizeof(magic), 1, filedes) != 1)
	   FATAL_ERROR1( "Error reading magic number of file %s", file_names[i] );
	 firstword = (long)magic;
	      
	 /* Seek to beginning of file */

	 fseek( filedes, 0L, 0 );    
	 if( fread(atype, SARMAG, 1, filedes) != 1 )
	     atype[0] = '\0';

	 if( ISCOFF(magic) )
	 {
	     process_object_file( current_file_name, filedes, firstword );
	     /* fclose( filedes ); */
	 }
	 else if( strncmp(atype, ARMAG, SARMAG) == 0 )
	 {
	     process_object_file( current_file_name, filedes, (long) ARCHIVE_TYPE );
	     /* fclose( filedes ); */
	 }
	 else
	 {
	     FATAL_ERROR2( "File %s is of unknown type: magic number = %6.2x",
			   current_file_name, firstword );
	     fclose( filedes );
	 }       
    }

    if( !check_if_errors() )
    {
	pass2();
    }
    else
    {
	linker_exit( FATAL );
    }

}
