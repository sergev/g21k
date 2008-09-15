/* @(#)token.h	1.4 1/4/91 1 */

typedef struct TOKEN 
{
    char         *str;
    short         type;
    long          value;
    struct TOKEN *next;
} TOKEN_CELL;

extern void token_make_null( void );
extern short token_hash( char *str );
extern TOKEN_CELL *token_lookup( char *str );
extern void token_insert( char *str, short type, short value );
