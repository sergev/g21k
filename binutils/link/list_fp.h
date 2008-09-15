/* @(#)list_fp.h	2.2  6/1/93 */

extern void list_add( int list_type, register LIST *owner, register char *member );
extern INPUT_FILE *list_find_input_file( char *file_name );
extern INPUT_SECT *list_find_input_section( int i, INPUT_FILE *file_ptr );
extern OUTPUT_SECT *list_find_output_section( register char *name );
extern ACTION *list_find_ach_section( register char *name );
