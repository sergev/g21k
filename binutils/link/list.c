/* @(#)list.c   2.3 6/1/93 2 */

#include "app.h"
#include "address.h"
#include "error.h"
#include "a_out.h"
#include "action.h"
#include "memory.h"
#include "input_section.h"
#include "output_section.h"
#include "input_file.h"
#include "list.h"
#include "process_section.h"
#include "read_object.h"

#include "error_fp.h"
#include "list_fp.h"


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_add                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void list_add( int list_type, LIST *owner, char *member )       *
*                                                                      *
*   Description                                                        *
*      This is a general purpose routine for adding a item to a linked *
*   list.                                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/12/89       created                  -----             *
***********************************************************************/

void list_add( int list_type, register LIST *owner, register char *member )
{
    switch( list_type )
    {
	    case LIST_SECTION:
	       if( owner->tail == NULL )
		   owner->head = member;
	       else
		   ((ACTION *) owner->tail)->next = (ACTION *) member;
	       owner->tail = member;
	       ((ACTION *) member)->next = NULL;
	       break;

	    case LIST_ADDRESS:
	       /* Ordered by address */

	       if( owner->head == NULL )
	       {
		   owner->head = member;
		   owner->tail = member;
		   ((ADDRESS *) member)->next = ((ADDRESS *) member)->previous = NULL;
	       }
	       else
	       {
		   ADDRESS *adr_ptr;

		   adr_ptr = (ADDRESS *) owner->head;
		   while( adr_ptr )
		   {
			  if( adr_ptr->addr > ((ADDRESS *) member)->addr)
			  {
			      if( adr_ptr->previous )
				  adr_ptr->previous->next = (ADDRESS *) member;
			      else
				  owner->head = member;
			      ((ADDRESS *) member)->next = adr_ptr;
			      ((ADDRESS *) member)->previous = adr_ptr->previous;
			      adr_ptr->previous = (ADDRESS *) member;
			      return;
			  }
			  adr_ptr = adr_ptr->next;
		   }
		   adr_ptr = (ADDRESS *) owner->tail;
		   owner->tail = member;
		   ((ADDRESS *) member)->next = NULL;
		   ((ADDRESS *) member)->previous = adr_ptr;
		   adr_ptr->next = (ADDRESS *) member;
	       }
	       break;

	    case LIST_MEMORY:
	       /* Ordered by address */

	       if( owner->head == NULL )
	       {
		   owner->head = member;
		   owner->tail = member;
		   ((MEMORY *) member)->mem_next = NULL;
	       }
	       else
	       {
		   MEMORY *mem_ptr, *prev_mem_ptr;

		   mem_ptr = (MEMORY *) owner->head;
		   prev_mem_ptr = NULL;
		   while( mem_ptr )
		   {
			  if( mem_ptr->start_address > ((MEMORY *) member)->start_address)
			  {
			      if( prev_mem_ptr )
				  prev_mem_ptr->mem_next = (MEMORY *) member;
			      else
				  owner->head = member;
			      ((MEMORY *) member)->mem_next = mem_ptr;
			      return;
			  }
			  prev_mem_ptr = mem_ptr;
			  mem_ptr = mem_ptr->mem_next;
		   }
		   prev_mem_ptr->mem_next = (MEMORY *) member;
		   owner->tail = member;
	       }
	       break;

	    case LIST_OUTPUT_SECTION:
	       if( owner->head == NULL )
		   owner->head = member;
	       else
		   ((OUTPUT_SECT *) owner->tail)->next = (OUTPUT_SECT *) member;

	       owner->tail = member;
	       ((OUTPUT_SECT *) member)->next = NULL;
	       break;

	    case LIST_INC_INPUT_SECTS:
	       if( ((OUTPUT_SECT *) owner)->in_sect_head == NULL )
		     ((OUTPUT_SECT *) owner)->in_sect_head = (INPUT_SECT *) member;
	       else
		     ((OUTPUT_SECT *) owner)->in_sect_tail->inc_next = (INPUT_SECT *) member;

	       ((OUTPUT_SECT *) owner)->in_sect_tail = (INPUT_SECT *) member;
	       ((INPUT_SECT *) member)->inc_next = NULL;
	       ((INPUT_SECT *) member)->out_sec = (OUTPUT_SECT *) owner;
	       break;

	    case LIST_INPUT_SECTION:
	       if( ((INPUT_FILE *) owner)->head == NULL )
		     ((INPUT_FILE *) owner)->head = (INPUT_SECT *) member;
	       else
		     ((INPUT_FILE *) owner)->tail->next = (INPUT_SECT *) member;

	       ((INPUT_FILE *) owner)->tail = (INPUT_SECT *) member;
	       ((INPUT_SECT *) member)->next = NULL;
	       ((INPUT_SECT *) member)->file_ptr = (INPUT_FILE *) owner;
	       break;

	    case LIST_INPUT_FILE:
	       if( owner->tail == NULL )
		   owner->head = member;
	       else
		   ((INPUT_FILE *) owner->tail)->infile_next = (INPUT_FILE *) member;
	
	       owner->tail = member;
	       ((INPUT_FILE *) member)->infile_next = NULL;
	       break;

	    default:
	       LINKER_ERROR( "Illegal list type." );
	       break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_find_input_file                                            *
*                                                                      *
*   Synopsis                                                           *
*      INFILE *list_find_input_file( char *file_name )                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for finding a input file on a list  *
*   of INPUT_FILE structures.                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/26/89       created                  -----             *
***********************************************************************/

INPUT_FILE *list_find_input_file( char *file_name )
{
    register INPUT_FILE *p;

    p = (INPUT_FILE *) input_file_list.head;
    while( p )
    {
	   if( !strcmp(file_name, p->file_name) )
	       return( p );
	   p = p->infile_next;
    }

    return( NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_find_input_section                                         *
*                                                                      *
*   Synopsis                                                           *
*      INPUT_SECTION *list_find_input_section( int i,                  *
*                                              INPUT_FILE *file_ptr )  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning a pointer to a        *
*   section, given the section number.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/1/89        created                  -----             *
***********************************************************************/

INPUT_SECT *list_find_input_section( int i, INPUT_FILE *file_ptr )
{
    register INPUT_SECT *section_ptr;

    if( i == 0 )
	return( NULL );

    for( section_ptr = file_ptr->head; section_ptr != NULL; section_ptr = section_ptr->next )
	 if( section_ptr->section_num == i )
	     return( section_ptr );
    return( NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_find_output_section                                        *
*                                                                      *
*   Synopsis                                                           *
*      OUTPUT_SECTION *list_find_output_section( char *name )          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning a pointer to a output *
*   section data item.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/12/89       created                  -----             *
***********************************************************************/

OUTPUT_SECT *list_find_output_section( register char *name )
{
    register OUTPUT_SECT *p;

    for( p = (OUTPUT_SECT *) output_section_list.head; p; p = p->next )
    {
	 if( !strncmp(name, p->sec_hdr.s_name, SYMNMLEN) )
	     return( p );
    }

    return( NULL );
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      list_find_ach_section                                           *
*                                                                      *
*   Synopsis                                                           *
*      int list_find_ach_section( char *name )                         *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning a pointer to the      *
*   section if the section was mentioned in the architecture file.     *       
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     gas     1/31/93       created                  -----             *
*     sbb     5/28/93       return ptr to section    -----             *
***********************************************************************/

ACTION *list_find_ach_section( register char *name )
{
    register ACTION *p;

    for( p = (ACTION *) section_list.head; p; p = p->next )
    {
	 if( !strncmp(name, p->sect_name, SYMNMLEN) )
	     return (p);
    }

    return((ACTION *)NULL);
}
