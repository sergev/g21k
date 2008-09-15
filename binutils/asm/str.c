/* @(#)str.c	2.1 5/6/92 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include <malloc.h>
#include "error.h"
#include "str.h"
#include "util.h"

char *string_table      = (char *) NULL;
long string_table_index = 0L;

static long size = 0l;
static long string_table_size = 0l;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      string_add                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void string_add(string)                                         *
*      char *string;                                                   *
*                                                                      *
*   Description                                                        *
*      Add a string the the string table, realloc if necessary.        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/1/89        created                  -----             *
***********************************************************************/

void string_add( register char *string )
{
    register short length;

    length = strlen( string );
    if( length + string_table_index >= size )
        if( (string_table = my_realloc(string_table,
            (unsigned long)(size += string_table_size))) == NULL )
        {
             FATAL_ERROR("String table overflow");
        }

    strcpy( &string_table[string_table_index], string );
    string_table_index += length + 1;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      string_table_init                                               *
*                                                                      *
*   Synopsis                                                           *
*      void string_table_init()                                        *
*                                                                      *
*   Description                                                        *
*      Get space for the string table.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/1/89        created                  -----             *
***********************************************************************/

void string_table_init( void )
{
    string_table_size = 4 * BUFSIZ;
    if( (string_table = my_malloc((long)(size = string_table_size))) == NULL )
         FATAL_ERROR("No memory for string table");

    string_table_index = 4;
}
