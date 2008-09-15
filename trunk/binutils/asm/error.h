/* @(#)error.h  1.5 1/4/91 1 */


#define ERROR_WARNING          0x1L
#define ERROR_USER_ERROR       0x2L
#define ERROR_ASSEMBLER_ERROR  0x3L
#define ERROR_SYNTAX_ERROR     0x4L
#define ERROR_FATAL            0x5L

#define MAX_ERRORS             20

#define USER_WARN(x)                 user_warn( __FILE__, __LINE__, x )
#define USER_WARN1(x,a)              user_warn( __FILE__, __LINE__, x, a )
#define USER_WARN2(x,a,b)            user_warn( __FILE__, __LINE__, x, a, b )
#define USER_WARN3(x,a,b,c)          user_warn( __FILE__, __LINE__, x, a, b, c )
#define USER_WARN4(x,a,b,c,d)        user_warn( __FILE__, __LINE__, x, a, b, c, d )

#define USER_ERROR(x)                user_error( __FILE__, __LINE__, x )
#define USER_ERROR1(x,a)             user_error( __FILE__, __LINE__, x, a )
#define USER_ERROR2(x,a,b)           user_error( __FILE__, __LINE__, x, a, b )
#define USER_ERROR3(x,a,b,c)         user_error( __FILE__, __LINE__, x, a, b, c )
#define USER_ERROR4(x,a,b,c,d)       user_error( __FILE__, __LINE__, x, a, b, c, d )

#define ASSEMBLER_ERROR(x)           assembler_error( __FILE__, __LINE__, x )
#define ASSEMBLER_ERROR1(x,a)        assembler_error( __FILE__, __LINE__, x, a )
#define ASSEMBLER_ERROR2(x,a,b)      assembler_error( __FILE__, __LINE__, x, a, b )
#define ASSEMBLER_ERROR3(x,a,b,c)    assembler_error( __FILE__, __LINE__, x, a, b, c )
#define ASSEMBLER_ERROR4(x,a,b,c,d)  assembler_error( __FILE__, __LINE__, x, a, b, c, d )

#define SYNTAX_ERROR(x)              syntax_error( __FILE__, __LINE__, x )
#define SYNTAX_ERROR1(x)             syntax_error( __FILE__, __LINE__, x, a )

#define FATAL_ERROR(x)               fatal_assembler_error( __FILE__, __LINE__, x )
#define FATAL_ERROR1(x,a)            fatal_assembler_error( __FILE__, __LINE__, x, a )
#define FATAL_ERROR2(x,a,b)          fatal_assembler_error( __FILE__, __LINE__, x, a, b )
#define FATAL_ERROR3(x,a,b,c)        fatal_assembler_error( __FILE__, __LINE__, x, a, b, c )
#define FATAL_ERROR4(x,a,b,c,d)      fatal_assembler_error( __FILE__, __LINE__, x, a, b, c, d )

extern void interrupt_handler( int );
extern void error( long error_type, char *file, int line, char *error_string, ... );
extern void message( long error_type );
extern void yyerror( char *string );
extern void user_warn( char *file, int line, char *error_string, ... );
extern void user_error( char *file, int line, char *error_string, ... );
extern void assembler_error( char *file, int line, char *error_string, ... );
extern void syntax_error( char *file, int line, char *error_string, ... );
extern void fatal_assembler_error( char *file, int line, char *error_string, ... );
extern short check_if_errors( void );
extern void delete_temp_files( void );
extern void exit_summary( void );
extern void asm_exit( int exit_code );
