/* @(#)ieee.h	1.5 1/4/91 1 */


#define SINGLE_PRECISION       1
#define EXTENDED_PRECISION     2

#define EXPONENT_BIAS     127

#define SINGLE_MANT       23
#define EXTENDED_MANT     31

#define RND_ZERO          1
#define RND_PLUS_INF      2
#define RND_MINUS_INF     3
#define RND_NEAREST       4

extern void convert_to_ieee( double d ,char rnd, char precision, unsigned long *mant, int *exp );
extern void format_ieee( unsigned long mant, int exp, unsigned char *number, char precision );
