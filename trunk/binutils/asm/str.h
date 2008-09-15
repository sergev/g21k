/* @(#)str.h	1.1 2/21/91 1 */


#define THERE_IS_A_STRING_TABLE   (string_table_index > 4)

extern char *string_table;
extern long string_table_index;

extern void string_add( char *string );
extern void string_table_init( void );
