/* @(#)process_args.c   1.16 3/21/91 1 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>


#include "app.h"
#include "error.h"
#include "main.h"
#include "process_args.h"
#include "util.h"
#include <malloc.h>

#include "process_args_fp.h"
#include "error_fp.h"
#include "pass1.h"

int aflag = 1;
int mflag = 0;
int rflag = 0;
int sflag = 0;
int xflag = 0;

char *file_name_ptr;

char *file_names[MAX_NUMBER_INPUT_FILES];
int  num_input_files = 0;

char arch_descrip_file[MAX_FILENAME_LENGTH] = "21k.ach";
short error_debug = FALSE;
char output_file_name[MAX_FILENAME_LENGTH] = "";

static void process_file_name (char *filename);

int  Old_library_flag = 1;

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
    char library_string[MAX_FILENAME_LENGTH];

    for( i = 1; i < argc; i++)
    {
	 if( *argv[i] == '-' )
	 {      
	     switch( argv[i][1] )
	     {
		     case 'a':                  /* Architecture description file name */
		     case 'A':                  /* Architecture description file name */
			i++;
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
		     case 'm':                  /* Outputs a memory map */
		     case 'M':                  /* Outputs a memory map */
			mflag++;
			continue;

		     case 'o':                  /* Give the output a name */
		     case 'O':                  /* Give the output a name */
			i++;
			file_name_ptr = make_my_slash(argv[i], FALSE);
			strncpy( output_file_name, file_name_ptr, MAX_FILENAME_LENGTH);
			append_ext (output_file_name, ".exe");
			continue;

		     case 'i':
		     case 'I':
			i++;
			process_file_list( argv[i] );
			continue;                    

		     case 'h':
		     case 'H':
			usage ();
			exit (0);

		      case 'L':
			continue;

		      case 'l':
			Old_library_flag = 0;
			strcpy(library_string, "lib");
			if( argv[i][2] != 0)
			    strcat(library_string, &argv[i][2]);
			else
			    strcat(library_string, argv[++i]);
			if( !strchr(library_string, '.') )
			    strcat(library_string, ".a");
			process_file_name(library_string);
			fopen_library(num_input_files-1, "ADI_DSP");
			continue;

		     case 'r': /* Retains reloc info, (output is not executable) */
		     case 'R': /* Retains reloc info, (output is not executable) */
			USER_WARN1("Illegal flag: %s no longer supported.", argv[i]);
/*                      rflag++; */
			continue;

		     case 's':                /* Strips all relocation and symbol info from output */
		     case 'S':                /* Strips all relocation and symbol info from output */
			sflag++;
			continue;

		     case 'x':                  /* Strips symbol info except */
		     case 'X':                  /* Strips symbol info except */
			xflag++;
			continue;

		     case 'v':
		     case 'V':
			continue;

		     default:
			USER_WARN1( "Illegal flag: %s ignored.", argv[i] );
			continue;
	     }
	 }
	 else
	     process_file_name (argv[i]);
    }
}





/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_file_name                                               *
*                                                                      *
*   Description                                                        *
*      adds a file into the list of object files to process            *
*                                                                      *
***********************************************************************/

static void process_file_name (char *filename) {

   char tmp_name[MAX_FILENAME_LENGTH];
   char *s;
   char *tmp_ptr;

/* Make sure there is room */
   if( MAX_NUMBER_INPUT_FILES == num_input_files )
       FATAL_ERROR ("Too many object files");

/* Make the slashes for the system */
   tmp_ptr = make_my_slash(filename, FALSE);

/* Copy the name */
   strncpy (tmp_name, tmp_ptr, MAX_FILENAME_LENGTH);

/* Append in extension if necessary */
   append_ext (tmp_name, ".obj");

/* Save the name */
   if (!(s = strdup (tmp_name))) {
     FATAL_ERROR1 ("Error adding %s to the list of object files.", tmp_name);
   }

/* Add it to the list */
   file_names[num_input_files++] = s;

/* Return */
 }


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_file_list                                               *
*                                                                      *
*   Description                                                        *
*      Adds object filenames to the list when passed in as a file      *
*                                                                      *
***********************************************************************/
	
void process_file_list( char *filename )
{
    FILE *fd;
    short count;
    char *line;

/* Malloc a buffer to read in a line from the file */
   line = my_malloc ((long)BUFSIZ);

/* Open the file */
    if ((fd = fopen( filename, READ_TEXT )) == NULL ) {
      FATAL_ERROR1( "Cannot open file %s.", filename );
    }

/* FOR (each line in the file) */
   while( fgets(line, BUFSIZ, fd) != NULL ) {

/* . Ignore everything after the first whitespace and control character (this allows comments) */
     strtok (line, " \n\t");
     for (count = 0; count < strlen (line); count++) {
       if (!isprint (line[count])) {
	 line[count] = '\0';
	 break;
       }
     }

/* . Add the filename into the list */
     process_file_name (line);

/* ENDFOR */
   }

/* Close the file */
   fclose (fd);

/* Return */
  }
