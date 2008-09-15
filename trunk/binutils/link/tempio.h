/* @(#)tempio.h	1.4 6/11/91 1 */

#define MAX_TEMPS     10
#define TEMP_PREFIX   "ldtmp"

extern int num_open_files;
extern char *temp_file[];

extern FILE *temp_file_create( char *type );
extern void copy_section( char *file, FILE *output_fd );
