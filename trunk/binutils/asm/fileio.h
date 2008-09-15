/* @(#)fileio.h 1.4 1/4/91 1 */


extern int  input_char( void );
extern void unput_char ( char c );
/*extern int  yywrap( void );*/
extern void read_data_file( char *filename );
extern void init_std_io( void );
extern void set_file_ptr( FILE *file );
extern void advance_to_semi( void );

