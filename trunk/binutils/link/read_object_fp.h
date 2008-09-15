/* @(#)read_object_fp.h	1.5 6/11/91 1 */

extern void process_object_file( char *filename, FILE *fd, long file_type );
extern void load_object_file( LDFILE *fd, char *file_name, int file_index, SYMENT *symbuf );
extern char *read_string_table( FILE *fd, INPUT_FILE *file_ptr, long *size );
