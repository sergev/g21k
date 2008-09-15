/* @(#)tempio.c 1.5 2/21/91 1 */

#include "app.h"
#include "error.h"
#include "tempio.h"
#include "error_fp.h"
#include <string.h>

static char buffer[BUFSIZ];
char *temporary_file_name[MAX_FILENAME_LENGTH];
char *temp_file[MAX_TEMPS];
int   num_open_files = 0;

char Temporary_filename[125];


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      temp_file_create                                                *
*                                                                      *
*   Synopsis                                                           *
*      FILE temp_file_create(type)                                     *
*      char *type;                                                     *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating and opening a temp.    *
*   file.                                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/1/89        created                  -----             *
***********************************************************************/

FILE *temp_file_create( char *type )
{
    FILE *desc;

    temp_file[num_open_files] = tempnam( "", TEMP_PREFIX );
    strcpy(Temporary_filename, temp_file[num_open_files]);
    desc = fopen( temp_file[num_open_files], type );

    if( desc != NULL )
    {
	++num_open_files;
	return( desc );
    }
    else
	FATAL_ERROR("Could not open temporary file");
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      copy_section                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void copy_section(file)                                         *
*      char *file;                                                     *
*                                                                      *
*   Description                                                        *
*      Copy a section from a temp file to the object file.             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/2/89        created                  -----             *
***********************************************************************/

void copy_section( char *file, FILE *output_fd )
{
    register short num;
    FILE *input_fd;

    if( (input_fd = fopen(file, READ_BINARY)) == NULL )
	 FATAL_ERROR("Error opening temporary file");

    do
    {
	num = fread( buffer, sizeof(*buffer), BUFSIZ, input_fd );
	fwrite( buffer, sizeof(*buffer), num, output_fd );
    } while( num == BUFSIZ );

    fclose( input_fd );
}
