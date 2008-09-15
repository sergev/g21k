/* @(#)action.c 2.2 2/13/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "list.h"
#include "a_out.h"
#include "action.h"
#include "util.h"

#include "list_fp.h"
#include "action_fp.h"

LIST section_list;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      action_define_section                                           *
*                                                                      *
*   Synopsis                                                           *
*      void action_define_section( char *sect_name,                    *
*                                  unsigned short sect_type,           *
*                                  int attributes, unsigned long address) *
*   Description                                                        *
*      Build a linked list of input sections.                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/26/89       created                  -----             *
***********************************************************************/

void action_define_section( register char *sect_name, unsigned short sect_type ,int attributes, unsigned long address, unsigned long width)
{
   register ACTION *p;

   p = (ACTION *) my_calloc( (long)sizeof(ACTION) );
   p->type = ACTION_DEFINE_SECTION;
   p->sect_type = sect_type;
   p->attributes = attributes;
   p->address = address;
   p->width = width;
   strncpy( p->sect_name, sect_name, SYMNMLEN );
     
   list_add( LIST_SECTION, &section_list, (char *) p );
}

