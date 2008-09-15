/* @(#)list.c	1.7 3/21/91 1 */

#include <stdio.h>
#include <stddef.h>
#include "app.h"
#include "a_out.h"
#include "string.h"
#include "symbol.h"
#include "list.h"
#include "util.h"
#include "str.h"

static LIST *list[C_ENTAG + 1] =
{
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_insert_index                                               *
*                                                                      *
*   Synopsis                                                           *
*      void list_insert_index(symbol, sclass, index)                   *
*      SYMBOL *symbol;                                                 *
*      short   sclass;                                                 *
*      long    index;                                                  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for saving the index of a object    *
*   symbol table entry.                                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

void list_insert_index( SYMBOL *symbol, short sclass, long index )
{
    register LIST *new_entry;

    switch( sclass )
    {
            case C_EXT:
            case C_EXTDEF:
            case C_STAT:
            case C_STRTAG:
            case C_UNTAG:
            case C_ENTAG:
               new_entry = (LIST *) my_malloc( (long)LIST_SIZE );

               if( IN_STRING_TABLE(symbol) )
               {
                   new_entry->tag = STRING_TABLE_INDEX;
                   new_entry->offset = OFFSET( symbol );
               }
               else
               {
                   new_entry->tag = NAME_IN_MEMORY;
                   new_entry->symbol = NAME( symbol );
               }

               new_entry->index = index;
               new_entry->next = list[sclass];
               list[sclass] = new_entry;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_get_index                                                  *
*                                                                      *
*   Synopsis                                                           *
*      long list_get_index(symbol_name, sclass)                        *
*      register char *symbol_name;                                     *
*      short          sclass;                                          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for retrieving the symbol index of  *
*   a symbol.                                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

long list_get_index( register char *symbol_name, short sclass )
{
    register LIST *list_ptr;
    register char *p;
    register char *p1;

    for( list_ptr = list[sclass]; list_ptr != NULL; list_ptr = list_ptr->next )
    {
         p = symbol_name;
         if( list_ptr->tag == STRING_TABLE_INDEX )
             p1 = &string_table[list_ptr->offset];
         else
             p1 = list_ptr->symbol;

         if( p[1] == p1[1] && !strcmp(p, p1) )
             return( list_ptr->index );
    }
    return( -1L );
}
