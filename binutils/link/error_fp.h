/* @(#)error_fp.h       1.5  6/11/91 */

extern void interrupt_handler( int signum );
extern void error( long error_type, char *file, int line, char *error_string, ... );
extern void message( long error_type );
extern void yyerror( char *string );
extern void user_warn( char *file, int line, char *error_string, ... );
extern void user_error( char *file, int line, char *error_string, ... );
extern void linker_error( char *file, int line, char *error_string, ... );
extern void fatal_linker_error( char *file, int line, char *error_string, ... );
extern short check_if_errors( void );
extern void  delete_temp_files( void );
extern void exit_summary( void );
extern void linker_exit( int exit_code );

