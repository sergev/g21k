# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin, *yyout;
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#include <stddef.h>

#include "app.h"

#ifdef TRUE
#undef TRUE
#endif

#include "symbol.h"
#include "expr.h"
#include "token.h"
#include "pass1.h"
#include "main.h"
#include "y_tab.h"
#include "lexical.h"
#include "listing.h"
#include "util.h"
#include "fileio.h"

#ifdef input
#undef input
#endif

#define input() input_char()

#ifdef unput
#undef unput
#endif



#define unput(c) unput_char(c)

int yycol = 0;
char lex_buff[BUFSIZ];
char last_identifier[BUFSIZ];

extern  int yylex(void );
extern  int check_id(void );
extern  long aotol(char *str);
extern  long abtol(char *str);
extern  long ahtol(char *str);
extern  int yylook(void );
extern  int yyback(int *p,int m);
extern  int yyinput(void );
extern  int yyoutput(int c);
extern  int yyunput(int c);

# define COMMENT 2
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
		{
					    if( listing_flag )
						listing_carriage_return();

					    yylineno++;
					}
break;
case 2:
		BEGIN 0;
break;
case 3:
			;
break;
case 4:
			{
					    if( listing_flag )
						listing_carriage_return();

					    yylineno++;
					    BEGIN COMMENT;
					}
break;
case 5:
			{
					    if( listing_flag )
						listing_carriage_return();

					    yylineno++;
					}
break;
case 6:
	return( check_id() );
break;
case 7:
	 {
					    yylval.dval = atof(yytext);
					    LEX_RETURN( REAL );
					}
break;
case 8:
		 {
					    yylval.dval = atof(yytext);
					    LEX_RETURN( REAL );
					}
break;
case 9:
		{
					    if( strlen(yytext) != HEX40_STRING_LENGTH )
					    {
						sscanf(yytext+2,"%lx",&yylval.lval);
						LEX_RETURN( INT_CONST );
					    }
					    else
					    {
						expression_hex_string_to_uns( yytext + 2, yylval.hex40, DM_WORD_SIZE);
						LEX_RETURN( HEX40_CONST );
					    }
					}
break;
case 10:
				{
					    yylval.lval = atol(yytext);
					    LEX_RETURN( INT_CONST );
					}
break;
case 11:
			{
					    yylval.lval = atol(&yytext[2]);
					    LEX_RETURN( INT_CONST );
					}
break;
case 12:
			{
					    yylval.lval = aotol(&yytext[2]);
					    LEX_RETURN( INT_CONST );
					}
break;
case 13:
			{
					    yylval.lval = abtol(&yytext[2]);
					    LEX_RETURN( INT_CONST );
					}
break;
case 14:
		{
					    yylval.lval = ahtol(&yytext[2]);
					    LEX_RETURN( INT_CONST );
					}
break;
case 15:
				   { LEX_RETURN( LEFT_SHIFT ); }
break;
case 16:
				   { LEX_RETURN( RIGHT_SHIFT ); }
break;
case 17:
			{
					    yytext[yyleng-1] = '\0';
					    yylval.sval = (char *) my_malloc((long) yyleng-1 );
					    sscanf(yytext + 1, "%s", yylval.sval);
					    LEX_RETURN( FILENAME );
					}
break;
case 18:
			{
					    strncat( lex_buff, yytext, BUFSIZ - strlen(lex_buff));
					}
break;
case 19:
				   ;
break;
case 20:
				{
					    if( listing_flag && !file_init_processing )
						listing_carriage_return();

					    lex_buff[0] = '\0';

					    if( init_processing )
						yylineno++;

					    if( !init_processing )
						return( NEW_LINE );
					}
break;
case 21:
    {
					    sscanf(yytext,"\n#%d %[^\n]\n",&yylineno,src_name);
					}
break;
case 22:
				{
					    LEX_RETURN( yytext[0] );
					}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

  int check_id( void )
{
  char *ptr;
  TOKEN_CELL *token;
  SYMBOL *symbol;
    
  if ( strlen(yytext) > IDENTIFIER_LENGTH )
    yytext[IDENTIFIER_LENGTH] = '\0';
    
  if ( (token = token_lookup(yytext)) != NULL ) 
  {
    yylval.lval = token->value;
    LEX_RETURN( token->type );
  }
  else 
  {
    strcpy( last_identifier, yytext);
    yylval.symbol = NULL;
    LEX_RETURN( IDENTIFIER );
  }
symbol;
ptr;
}



long aotol( register char *str )
{
    register long val = 0;

    while(*str) 
    {
	val <<= 3;
	val |= (*(str++) - '0');
    }

    return(val);
}



long abtol( register char *str )
{
    register long val = 0;

    while(*str) 
    {
	val <<= 1;
	val |= *(str++) - '0';
    }

    return(val);
}



long ahtol( register char *str )
{
    register long val = 0;

    while(*str) 
    {
	val <<= 4;
	if (*str <= '9' && *str >= '0')
	    val |= *(str++) - '0';
	else if(*str <= 'F' && *str >= 'A')
	    val |= *(str++) - 'A' + 10;
	else
	    val |= *(str++) - 'a' + 10;
    }

    return(val);
}

int yyvstop[] = {
0,

10,
0,

10,
0,

10,
0,

10,
0,

22,
0,

18,
22,
0,

20,
0,

19,
22,
0,

22,
0,

22,
0,

6,
22,
0,

22,
0,

10,
22,
0,

10,
22,
0,

22,
0,

22,
0,

6,
22,
0,

6,
22,
0,

6,
22,
0,

6,
22,
0,

22,
0,

22,
0,

22,
0,

18,
22,
0,

1,
20,
0,

19,
22,
0,

22,
0,

22,
0,

22,
0,

6,
22,
0,

22,
0,

10,
22,
0,

10,
22,
0,

22,
0,

22,
0,

6,
22,
0,

6,
22,
0,

6,
22,
0,

6,
22,
0,

22,
0,

2,
22,
0,

18,
0,

5,
0,

17,
0,

6,
0,

8,
0,

7,
0,

10,
0,

9,
0,

15,
0,

16,
0,

13,
0,

11,
0,

14,
0,

12,
0,

4,
0,

3,
0,

1,
0,

2,
0,

18,
0,

1,
5,
0,

2,
0,

1,
0,

17,
0,

2,
0,

6,
0,

8,
0,

7,
0,

10,
0,

9,
0,

15,
0,

16,
0,

13,
0,

11,
0,

14,
0,

12,
0,

1,
4,
0,

2,
3,
0,

8,
0,

7,
0,

21,
0,

1,
0,

21,
0,

2,
0,

8,
0,

7,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,    0,0,    1,5,    0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    1,6,    1,7,    
0,0,    6,42,   1,8,    0,0,    
6,42,   0,0,    0,0,    0,0,    
24,66,  0,0,    0,0,    0,0,    
0,0,    66,66,  0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
2,8,    1,6,    1,9,    1,10,   
6,42,   1,11,   17,54,  3,8,    
18,55,  19,56,  20,57,  24,66,  
1,5,    61,92,  62,93,  1,12,   
66,66,  1,13,   1,14,   1,14,   
2,9,    83,83,  2,22,   54,54,  
54,54,  93,92,  1,14,   3,9,    
29,74,  1,15,   15,52,  1,16,   
16,53,  2,12,   1,11,   1,17,   
2,14,   1,18,   1,11,   34,81,  
3,12,   1,19,   35,82,  3,14,   
29,75,  95,94,  29,75,  2,15,   
1,20,   2,16,   31,77,  48,90,  
31,77,  29,75,  3,15,   4,23,   
3,16,   1,11,   86,86,  31,77,  
86,86,  0,0,    0,0,    4,24,   
4,25,   0,0,    0,0,    4,26,   
12,48,  12,48,  12,48,  12,48,  
12,48,  12,48,  12,48,  12,48,  
12,48,  12,48,  0,0,    0,0,    
33,78,  0,0,    33,79,  48,90,  
33,79,  0,0,    4,24,   4,27,   
4,28,   4,29,   4,30,   33,79,  
1,21,   0,0,    1,5,    0,0,    
84,84,  4,23,   84,84,  106,107,        
4,31,   106,107,        4,32,   4,33,   
4,33,   84,84,  0,0,    0,0,    
106,107,        0,0,    2,21,   4,33,   
0,0,    74,74,  4,34,   74,94,  
4,35,   3,21,   0,0,    4,30,   
4,36,   0,0,    4,37,   4,30,   
0,0,    0,0,    4,38,   9,43,   
32,78,  74,75,  32,79,  74,75,  
32,79,  4,39,   0,0,    9,43,   
9,44,   0,0,    74,75,  32,79,  
0,0,    75,95,  4,30,   13,49,  
0,0,    13,50,  13,50,  13,50,  
13,50,  13,50,  13,50,  13,50,  
13,50,  13,50,  13,50,  10,45,  
0,0,    75,75,  9,43,   75,75,  
9,43,   0,0,    9,43,   10,45,  
10,45,  0,0,    75,75,  0,0,    
0,0,    9,43,   32,80,  79,78,  
0,0,    79,79,  9,43,   79,79,  
9,43,   4,40,   77,77,  4,41,   
77,77,  0,0,    79,79,  9,43,   
0,0,    13,51,  10,45,  77,77,  
10,46,  0,0,    10,45,  9,43,   
9,43,   0,0,    9,43,   9,43,   
96,106, 10,45,  9,43,   77,96,  
0,0,    96,107, 10,45,  96,107, 
10,45,  9,43,   78,78,  107,107,        
78,78,  107,107,        96,107, 10,45,  
0,0,    0,0,    9,43,   78,78,  
107,107,        13,51,  104,104,        10,45,  
10,45,  0,0,    10,45,  10,45,  
0,0,    97,108, 10,45,  78,97,  
0,0,    80,80,  97,109, 80,80,  
97,109, 10,45,  104,104,        0,0,    
104,104,        22,61,  80,80,  97,109, 
0,0,    0,0,    10,45,  104,104,        
0,0,    0,0,    80,80,  80,80,  
0,0,    80,80,  80,80,  9,43,   
11,47,  22,62,  22,62,  22,62,  
22,62,  22,62,  22,62,  22,62,  
22,62,  22,62,  22,62,  0,0,    
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  0,0,    0,0,    
0,0,    0,0,    0,0,    10,45,  
0,0,    11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  0,0,    
0,0,    0,0,    0,0,    11,47,  
0,0,    11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  11,47,  
11,47,  11,47,  11,47,  14,49,  
30,76,  14,50,  14,50,  14,50,  
14,50,  14,50,  14,50,  14,50,  
14,50,  14,50,  14,50,  21,58,  
30,76,  108,109,        30,76,  108,109,        
109,109,        0,0,    109,109,        21,58,  
21,59,  30,76,  108,109,        0,0,    
0,0,    109,109,        0,0,    0,0,    
0,0,    30,76,  30,76,  0,0,    
30,76,  30,76,  0,0,    0,0,    
30,76,  0,0,    0,0,    23,63,  
0,0,    0,0,    21,58,  30,76,  
21,58,  0,0,    21,58,  23,63,  
23,64,  0,0,    0,0,    0,0,    
30,76,  21,58,  0,0,    0,0,    
0,0,    0,0,    21,58,  0,0,    
21,58,  0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    21,58,  
0,0,    0,0,    23,63,  0,0,    
23,63,  0,0,    23,63,  21,58,  
21,58,  0,0,    21,58,  21,58,  
0,0,    23,63,  21,58,  27,67,  
0,0,    0,0,    23,63,  0,0,    
23,63,  21,58,  0,0,    27,67,  
27,68,  0,0,    0,0,    23,63,  
0,0,    0,0,    21,58,  0,0,    
0,0,    0,0,    0,0,    23,63,  
23,63,  0,0,    23,63,  23,63,  
0,0,    0,0,    23,63,  28,70,  
0,0,    0,0,    27,67,  0,0,    
27,67,  23,63,  27,67,  28,70,  
28,71,  0,0,    0,0,    0,0,    
0,0,    27,67,  23,63,  0,0,    
0,0,    0,0,    27,67,  0,0,    
27,67,  0,0,    0,0,    21,60,  
0,0,    0,0,    0,0,    27,67,  
0,0,    0,0,    28,70,  0,0,    
28,72,  0,0,    28,70,  27,67,  
27,67,  0,0,    27,67,  27,67,  
0,0,    28,70,  27,67,  0,0,    
0,0,    0,0,    28,70,  0,0,    
28,70,  27,67,  0,0,    23,65,  
0,0,    0,0,    0,0,    28,70,  
0,0,    0,0,    27,67,  36,83,  
36,76,  0,0,    0,0,    28,70,  
28,70,  0,0,    28,70,  28,70,  
0,0,    0,0,    28,70,  0,0,    
36,76,  0,0,    36,76,  0,0,    
0,0,    28,70,  0,0,    0,0,    
0,0,    36,76,  0,0,    0,0,    
37,84,  37,76,  28,70,  0,0,    
0,0,    36,76,  36,76,  0,0,    
36,76,  36,76,  0,0,    27,69,  
36,76,  37,76,  0,0,    37,76,  
0,0,    0,0,    0,0,    36,76,  
0,0,    0,0,    37,76,  0,0,    
0,0,    38,85,  38,76,  0,0,    
36,76,  0,0,    37,76,  37,76,  
0,0,    37,76,  37,76,  0,0,    
0,0,    37,76,  38,76,  28,73,  
38,76,  0,0,    0,0,    0,0,    
37,76,  0,0,    0,0,    38,76,  
0,0,    0,0,    39,86,  39,76,  
0,0,    37,76,  0,0,    38,76,  
38,76,  0,0,    38,76,  38,76,  
0,0,    0,0,    38,76,  39,76,  
0,0,    39,76,  40,87,  0,0,    
85,85,  38,76,  85,85,  0,0,    
39,76,  0,0,    40,87,  40,88,  
0,0,    85,85,  38,76,  0,0,    
39,76,  39,76,  0,0,    39,76,  
39,76,  85,85,  85,85,  39,76,  
85,85,  85,85,  0,0,    0,0,    
0,0,    0,0,    39,76,  0,0,    
0,0,    40,87,  0,0,    40,87,  
0,0,    40,87,  0,0,    39,76,  
0,0,    0,0,    0,0,    0,0,    
40,87,  0,0,    0,0,    0,0,    
0,0,    40,87,  0,0,    40,87,  
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    40,87,  0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    40,87,  40,87,  
0,0,    40,87,  40,87,  0,0,    
0,0,    40,87,  0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
40,87,  0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    40,87,  49,49,  49,49,  
49,49,  49,49,  49,49,  49,49,  
49,49,  49,49,  49,49,  49,49,  
51,51,  51,51,  51,51,  51,51,  
51,51,  51,51,  51,51,  51,51,  
51,51,  51,51,  0,0,    49,91,  
0,0,    0,0,    0,0,    0,0,    
0,0,    51,51,  51,51,  51,51,  
51,51,  51,51,  51,51,  0,0,    
0,0,    0,0,    40,89,  55,55,  
55,55,  55,55,  55,55,  55,55,  
55,55,  55,55,  55,55,  55,55,  
55,55,  0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    49,91,  
0,0,    0,0,    0,0,    0,0,    
0,0,    51,51,  51,51,  51,51,  
51,51,  51,51,  51,51,  56,56,  
56,56,  56,56,  56,56,  56,56,  
56,56,  56,56,  56,56,  56,56,  
56,56,  0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
56,56,  56,56,  56,56,  56,56,  
56,56,  56,56,  57,57,  57,57,  
57,57,  57,57,  57,57,  57,57,  
57,57,  57,57,  57,57,  0,0,    
0,0,    76,76,  98,99,  98,99,  
98,99,  98,99,  98,99,  98,99,  
98,99,  98,99,  98,99,  98,99,  
0,0,    76,76,  0,0,    76,76,  
56,56,  56,56,  56,56,  56,56,  
56,56,  56,56,  76,76,  0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    76,76,  76,76,  
0,0,    76,76,  76,76,  0,0,    
0,0,    76,76,  0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
76,76,  0,0,    0,0,    0,0,    
0,0,    90,98,  0,0,    90,98,  
0,0,    76,76,  90,99,  90,99,  
90,99,  90,99,  90,99,  90,99,  
90,99,  90,99,  90,99,  90,99,  
91,100, 0,0,    91,100, 0,0,    
0,0,    91,101, 91,101, 91,101, 
91,101, 91,101, 91,101, 91,101, 
91,101, 91,101, 91,101, 92,92,  
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    92,92,  
92,92,  100,101,        100,101,        100,101,        
100,101,        100,101,        100,101,        100,101,        
100,101,        100,101,        100,101,        0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    94,94,  
0,0,    0,0,    92,92,  0,0,    
92,102, 0,0,    92,92,  94,94,  
94,103, 0,0,    0,0,    0,0,    
0,0,    92,92,  0,0,    0,0,    
0,0,    0,0,    92,92,  0,0,    
92,92,  0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    92,92,  
0,0,    0,0,    94,94,  0,0,    
94,104, 0,0,    94,94,  92,92,  
92,92,  0,0,    92,92,  92,92,  
0,0,    94,94,  92,92,  0,0,    
0,0,    0,0,    94,94,  0,0,    
94,94,  92,92,  0,0,    0,0,    
0,0,    0,0,    0,0,    94,94,  
0,0,    0,0,    92,92,  0,0,    
0,0,    0,0,    0,0,    94,94,  
94,94,  0,0,    94,94,  94,94,  
0,0,    0,0,    94,94,  0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    94,94,  0,0,    0,0,    
102,102,        0,0,    0,0,    0,0,    
0,0,    0,0,    94,94,  0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    92,92,  
102,102,        102,102,        102,102,        102,102,        
102,102,        102,102,        102,102,        102,102,        
102,102,        102,102,        0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    0,0,    
0,0,    0,0,    0,0,    94,105, 
0,0};
struct yysvf yysvec[] = {
0,      0,      0,
yycrank+-1,     0,              yyvstop+1,
yycrank+-19,    yysvec+1,       yyvstop+3,
yycrank+-26,    yysvec+1,       yyvstop+5,
yycrank+-86,    0,              yyvstop+7,
yycrank+0,      0,              yyvstop+9,
yycrank+4,      0,              yyvstop+11,
yycrank+0,      0,              yyvstop+14,
yycrank+0,      0,              yyvstop+16,
yycrank+-158,   0,              yyvstop+19,
yycrank+-186,   0,              yyvstop+21,
yycrank+248,    0,              yyvstop+23,
yycrank+52,     0,              yyvstop+26,
yycrank+129,    0,              yyvstop+28,
yycrank+325,    0,              yyvstop+31,
yycrank+2,      0,              yyvstop+34,
yycrank+2,      0,              yyvstop+36,
yycrank+3,      yysvec+11,      yyvstop+38,
yycrank+5,      yysvec+11,      yyvstop+41,
yycrank+6,      yysvec+11,      yyvstop+44,
yycrank+7,      yysvec+11,      yyvstop+47,
yycrank+-382,   0,              yyvstop+50,
yycrank+237,    0,              yyvstop+52,
yycrank+-410,   0,              yyvstop+54,
yycrank+-11,    yysvec+23,      yyvstop+56,
yycrank+0,      0,              yyvstop+59,
yycrank+0,      yysvec+23,      yyvstop+62,
yycrank+-454,   0,              yyvstop+65,
yycrank+-482,   0,              yyvstop+67,
yycrank+-28,    yysvec+23,      yyvstop+69,
yycrank+-336,   yysvec+23,      yyvstop+71,
yycrank+-34,    yysvec+23,      yyvstop+74,
yycrank+-114,   yysvec+23,      yyvstop+76,
yycrank+-66,    yysvec+23,      yyvstop+79,
yycrank+-11,    yysvec+23,      yyvstop+82,
yycrank+-12,    yysvec+23,      yyvstop+84,
yycrank+-508,   yysvec+23,      yyvstop+86,
yycrank+-533,   yysvec+23,      yyvstop+89,
yycrank+-558,   yysvec+23,      yyvstop+92,
yycrank+-583,   yysvec+23,      yyvstop+95,
yycrank+-633,   0,              yyvstop+98,
yycrank+0,      0,              yyvstop+100,
yycrank+0,      yysvec+6,       yyvstop+103,
yycrank+0,      yysvec+9,       0,      
yycrank+0,      0,              yyvstop+105,
yycrank+0,      yysvec+10,      0,      
yycrank+0,      0,              yyvstop+107,
yycrank+0,      yysvec+11,      yyvstop+109,
yycrank+14,     yysvec+12,      yyvstop+111,
yycrank+674,    0,              yyvstop+113,
yycrank+0,      yysvec+14,      yyvstop+115,
yycrank+684,    0,              yyvstop+117,
yycrank+0,      0,              yyvstop+119,
yycrank+0,      0,              yyvstop+121,
yycrank+7,      0,              yyvstop+123,
yycrank+711,    0,              yyvstop+125,
yycrank+739,    0,              yyvstop+127,
yycrank+762,    0,              yyvstop+129,
yycrank+0,      yysvec+21,      0,      
yycrank+0,      0,              yyvstop+131,
yycrank+0,      0,              yyvstop+133,
yycrank+11,     yysvec+22,      0,      
yycrank+14,     yysvec+22,      0,      
yycrank+0,      yysvec+23,      0,      
yycrank+0,      0,              yyvstop+135,
yycrank+0,      0,              yyvstop+137,
yycrank+-16,    yysvec+23,      yyvstop+139,
yycrank+0,      yysvec+27,      0,      
yycrank+0,      0,              yyvstop+141,
yycrank+0,      yysvec+9,       yyvstop+144,
yycrank+0,      yysvec+28,      0,      
yycrank+0,      yysvec+10,      yyvstop+146,
yycrank+0,      yysvec+23,      yyvstop+148,
yycrank+0,      yysvec+10,      yyvstop+150,
yycrank+-113,   yysvec+23,      0,      
yycrank+-141,   yysvec+23,      0,      
yycrank+-785,   yysvec+23,      yyvstop+152,
yycrank+-162,   yysvec+23,      yyvstop+154,
yycrank+-190,   yysvec+23,      yyvstop+156,
yycrank+-157,   yysvec+23,      yyvstop+158,
yycrank+-213,   yysvec+23,      yyvstop+160,
yycrank+0,      yysvec+23,      yyvstop+162,
yycrank+0,      yysvec+23,      yyvstop+164,
yycrank+-5,     yysvec+23,      yyvstop+166,
yycrank+-80,    yysvec+23,      yyvstop+168,
yycrank+-588,   yysvec+23,      yyvstop+170,
yycrank+-42,    yysvec+23,      yyvstop+172,
yycrank+0,      yysvec+40,      0,      
yycrank+0,      0,              yyvstop+174,
yycrank+0,      0,              yyvstop+177,
yycrank+826,    0,              0,      
yycrank+841,    0,              0,      
yycrank+-898,   0,              0,      
yycrank+23,     0,              0,      
yycrank+-926,   0,              0,      
yycrank+-43,    yysvec+23,      0,      
yycrank+-185,   yysvec+23,      0,      
yycrank+-214,   yysvec+23,      0,      
yycrank+774,    0,              0,      
yycrank+0,      yysvec+98,      yyvstop+180,
yycrank+861,    0,              0,      
yycrank+0,      yysvec+100,     yyvstop+182,
yycrank+976,    0,              yyvstop+184,
yycrank+0,      yysvec+92,      yyvstop+186,
yycrank+-218,   yysvec+23,      yyvstop+188,
yycrank+0,      yysvec+92,      yyvstop+190,
yycrank+-83,    yysvec+23,      0,      
yycrank+-191,   yysvec+23,      yyvstop+192,
yycrank+-337,   yysvec+23,      0,      
yycrank+-340,   yysvec+23,      yyvstop+194,
0,      0,      0};
struct yywork *yytop = yycrank+1051;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,011 ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
040 ,01  ,'"' ,01  ,'$' ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,01  ,'+' ,01  ,01  ,
'0' ,'0' ,'2' ,'2' ,'2' ,'2' ,'2' ,'2' ,
'2' ,'9' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'B' ,'A' ,'D' ,'E' ,'A' ,'$' ,
'H' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'O' ,
'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,
'X' ,'$' ,'$' ,01  ,01  ,01  ,01  ,'$' ,
01  ,'A' ,'B' ,'A' ,'D' ,'E' ,'A' ,'$' ,
'H' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'O' ,
'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,'$' ,
'X' ,'$' ,'$' ,01  ,01  ,'}' ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static  char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)       /* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {              /* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)       /* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)       /* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){            /* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
