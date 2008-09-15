/* @(#)error.h	1.4  6/11/91 */

#define ERROR_WARNING          0x1L
#define ERROR_USER_ERROR       0x2L
#define ERROR_LINKER_ERROR     0x3L
#define ERROR_SYNTAX_ERROR     0x4L
#define ERROR_FATAL            0x5L

#define MAX_ERRORS             30

#define USER_WARN(x)               user_warn( __FILE__, __LINE__, x )
#define USER_WARN1(x,a)            user_warn( __FILE__, __LINE__, x, a )
#define USER_WARN2(x,a,b)          user_warn( __FILE__, __LINE__, x, a, b )
#define USER_WARN3(x,a,b,c)        user_warn( __FILE__, __LINE__, x, a, b, c )
#define USER_WARN4(x,a,b,c,d)      user_warn( __FILE__, __LINE__, x, a, b, c, d )

#define USER_ERROR(x)              user_error( __FILE__, __LINE__, x )
#define USER_ERROR1(x,a)           user_error( __FILE__, __LINE__, x, a )
#define USER_ERROR2(x,a,b)         user_error( __FILE__, __LINE__, x, a, b )
#define USER_ERROR3(x,a,b,c)       user_error( __FILE__, __LINE__, x, a, b, c )
#define USER_ERROR4(x,a,b,c,d)     user_error( __FILE__, __LINE__, x, a, b, c, d )

#define LINKER_ERROR(x)            linker_error( __FILE__, __LINE__, x )
#define LINKER_ERROR1(x,a)         linker_error( __FILE__, __LINE__, x, a )
#define LINKER_ERROR2(x,a,b)       linker_error( __FILE__, __LINE__, x, a, b )
#define LINKER_ERROR3(x,a,b,c)     linker_error( __FILE__, __LINE__, x, a, b, c )
#define LINKER_ERROR4(x,a,b,c,d)   linker_error( __FILE__, __LINE__, x, a, b, c, d )

#define FATAL_ERROR(x)             fatal_linker_error( __FILE__, __LINE__, x )
#define FATAL_ERROR1(x,a)          fatal_linker_error( __FILE__, __LINE__, x, a )
#define FATAL_ERROR2(x,a,b)        fatal_linker_error( __FILE__, __LINE__, x, a, b )
#define FATAL_ERROR3(x,a,b,c)      fatal_linker_error( __FILE__, __LINE__, x, a, b, c )
#define FATAL_ERROR4(x,a,b,c,d)    fatal_linker_error( __FILE__, __LINE__, x, a, b, c, d )
