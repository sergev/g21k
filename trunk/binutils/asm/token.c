/* @(#)token.c	2.1 1/6/92 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "token.h"
#include "util.h"

static TOKEN_CELL *token_table[TOKEN_TABLE_SIZE];


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      token_make_null                                                 *
*                                                                      *
*   Synopsis                                                           *
*      void token_make_null(void)                                      *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for initializing the token hash     *
*  table to zero.                                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

void token_make_null( void )
{
    register short i;

    for(i = 0; i < TOKEN_TABLE_SIZE; i++)
        token_table[i] = NULL;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      token_hash                                                      *
*                                                                      *
*   Synopsis                                                           *
*      short token_hash(str)                                           *
*      char *str;                                                      *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for computing the hash value of a   *
*   string.                                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/15/89       created                  -----             *
***********************************************************************/

short token_hash( register char *str )
{
    register short i;
    register short sum;

    sum = 0;
    i = 0;

    while( str[i] )
        sum += str[i++];

    sum %= TOKEN_TABLE_SIZE;
    return( sum );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      token_lookup                                                    *
*                                                                      *
*   Synopsis                                                           *
*      TOKEN_CELL *token_lookup(str)                                   *
*      char *str;                                                      *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for looking up a string to determine*
*   if it is a reserved word token.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

TOKEN_CELL *token_lookup( register char *str )
{
    register TOKEN_CELL *current;
    register char       *tmp;

    tmp = (char *) my_malloc( (long) (strlen(str) + 1) );
    strcpy( tmp, str );
    strtoupper( tmp );

    current = token_table[token_hash(tmp)];

    while( current != NULL )
    {
           if ( strcmp(current->str,tmp) == 0 )
           {
	        my_free( tmp );
                return( current );
           }
           else
                current = current->next;
    }

    my_free( tmp );
    return( NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/22/89       created                  -----             *
***********************************************************************/

void token_insert( register char *str, short type, short value )
{
    register TOKEN_CELL *token_ptr;
    TOKEN_CELL *oldheader;
    register short bucket;


    if ( token_lookup(str) == NULL ) 
    {
        bucket = token_hash( str );
        oldheader = token_table[bucket];
        token_table[bucket] = (TOKEN_CELL *) my_malloc ((long)sizeof(TOKEN_CELL));
        token_ptr = token_table[bucket];
        token_ptr->str = str;
        token_ptr->type = type;
        token_ptr->value = (long) value;
        token_ptr->next = oldheader;
    }
}
