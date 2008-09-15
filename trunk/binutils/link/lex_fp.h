/* @(#)lex_fp.h	1.6  6/11/91 */

int  lookup( void);
#ifndef MSDOS
unsigned long strtoul( char *str, char **ptr, int base );
long strtol( char *str, char **ptr, int base );
#endif
char *strsave( char *string );
int  yywrap( void );
extern int yylex( void );

