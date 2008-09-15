/* @(#)symbol.c	2.2 9/14/92 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "str.h"
#include "symbol.h"
#include "util.h"

static SYMBOL *symbol_table[SYMBOL_TABLE_SIZE];


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      symbol_hash()                                                   *
*                                                                      *
*   Synopsis                                                           *
*      short symbol_hash(name)                                         *
*      char *name;                                                     *
*                                                                      *
*   Description                                                        *
*      This is the has function for the symbol name.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/21/89       created                  -----             *
***********************************************************************/

short symbol_hash( register char *name )
{
    register short i;
    register short sum;

    sum = 0;
    i = 0;

    while( name[i] )
           sum+=name[i++];

    sum %= SYMBOL_TABLE_SIZE;

    return( sum );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      symbol_make_null()                                              *
*                                                                      *
*   Synopsis                                                           *
*      void symbol_make_null()                                         *
*                                                                      *
*   Description                                                        *
*      Initialize the symbol table.                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/21/89       created                  -----             *
***********************************************************************/

void symbol_make_null( void )
{
    register int i;

    for( i=0; i < SYMBOL_TABLE_SIZE; i++)
         symbol_table[i] = NULL;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      symbol_lookup()                                                 *
*                                                                      *
*   Synopsis                                                           *
*      SYMBOL *symbol_lookup(name)                                     *
*      char *name;                                                     *
*                                                                      *
*   Description                                                        *
*      Lookup the name in the symbol table.                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/21/89       created                  -----             *
***********************************************************************/

SYMBOL *symbol_lookup( register char *name )
{
    register SYMBOL *current;
    char *current_name;

#ifdef DEBUG
    printf("symbol_lookup: name = %s\n", name);
#endif
    current = symbol_table[symbol_hash(name)];


    while( current != NULL )
    {
           current_name = IN_STRING_TABLE(current) ? &string_table[OFFSET(current)] :
                          NAME(current);

           if ( strcmp(current_name, name) == 0 )
           {
               return( current );
           }
           else
           {
               current = current->next;
           }
     }

    return( (SYMBOL *) NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      symbol_insert()                                                 *
*                                                                      *
*   Synopsis                                                           *
*      SYMBOL *symbol_insert(name,value,section_num,type,attr,length)  *
*      char *name;                                                     *
*      long int value;                                                 *
*      long int section_num;                                           *
*      long int type;                                                  *
*      long int attr;                                                  *
*      long int length;                                                *
*                                                                      *
*   Description                                                        *
*      Insert a symbol in the symbol table.                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/21/89       created                  -----             *
***********************************************************************/

SYMBOL *symbol_insert( register char *name, long int value, long int section_num,
                       long int type, long int length )
{
    SYMBOL *oldheader;
    register SYMBOL *sym_ptr;
    register char *ptr;
    register int bucket;

#ifdef DEBUG
    printf("symbol_insert: name = %s\n", name);
#endif

    if( symbol_lookup(name) == NULL ) 
    {
        bucket = symbol_hash( name );
        oldheader = symbol_table[bucket];
        symbol_table[bucket] = (SYMBOL *) my_malloc ((long)sizeof(SYMBOL));
        sym_ptr = symbol_table[bucket];
        sym_ptr->what.strtab_entry.zeroes = -1L;
        sym_ptr->what.strtab_entry.offset = -1L;

        /* Do we need to put the string in the string table? */

        if( strlen(name) > MAX_SYMBOL_NAME_SIZE )
        {
            sym_ptr->what.strtab_entry.zeroes = 0L;
            sym_ptr->what.strtab_entry.offset = string_table_index;
            string_add( name );
        }
        else {
            for( ptr = NAME(sym_ptr); *ptr++ = *name++; )
                 ;
        }

        sym_ptr->section_num = section_num;
        sym_ptr->type = type;
        sym_ptr->value = value;
        sym_ptr->length = length;
        sym_ptr->next = oldheader;
	sym_ptr->num_relocs = 0L;
    }
    
    if (Debug_symbol_definition)
	sym_ptr->Debug_symbol = TRUE;
    else
	sym_ptr->Debug_symbol = FALSE;

    return( sym_ptr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      symbol_traverse                                                 *
*                                                                      *
*   Synopsis                                                           *
*      void symbol_traverse(function)                                  *
*      int (*function)();                                              *
*                                                                      *
*   Description                                                        *
*      Go through the symbol table, an entry at a time, and apply a    *
*   function to each entry.                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/1/89        created                  -----             *
*************************************a**********************************/

void symbol_traverse( void (*function)() )
{
    register int i;
    register SYMBOL *current;

    for( i=0; i < SYMBOL_TABLE_SIZE; i++ ) 
    {
         current = symbol_table[i];

         while( current != NULL ) 
         {
               (*function)(current);
               current = current->next;
         }
    }
}
