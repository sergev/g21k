/* @(#)lex.h	1.1 4/17/90 1 */

typedef union
{
   long lval;
   char *string;
} YYSTYPE;

extern YYSTYPE  yylval;
extern FILE    *yyin;


int  lookup( void);
char *strsave( char *string );
int  yywrap( void );

int yylex (void);
int yylook (void);
int yyback (int *p, int m);
int yyinput (void);
int yyoutput (int c);
int yyunput (int c);
void yyerror (char *msg);
int  yyparse (void);

