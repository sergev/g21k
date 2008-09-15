/* @(#)allocate.c       2.4 2/13/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "error.h"
#include "address.h"
#include "memory.h"
#include "a_out.h"
#include "input_section.h"
#include "output_section.h"
#include "list.h"
#include "process_section.h"
#include "util.h"
#include "action.h"

#include "allocate_fp.h"
#include "list_fp.h"
#include "error_fp.h"

LIST avail_list;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void allocate( void )                                           *
*                                                                      *
*   Description                                                        *
*      This is the master routine for binding sections to the addresses*
*   given in the architecture description file.                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void allocate( void )
{
    allocate_address_space();
    allocate_section_to_memory();
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate_address_space                                          *
*                                                                      *
*   Synopsis                                                           *
*      void allocate_address_space( void )                             *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for building the free address space *
*   from the memory description given by the user.                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void allocate_address_space( void )
{
    register MEMORY  *mptr;
    register ADDRESS *aptr;

    for( mptr = (MEMORY *) memory_list.head; mptr != NULL; mptr = mptr->mem_next )
    {
	 aptr = (ADDRESS *) my_calloc( (long)sizeof(ADDRESS) );
	 aptr->addr_type = ADDRESS_FREE;
	 aptr->addr = mptr->start_address;
	 aptr->size = mptr->mem_length;
	 aptr->mem_ptr = mptr;
	 mptr->addr_head = mptr->addr_tail = aptr;

	 list_add( LIST_ADDRESS, &avail_list, (char *) aptr );
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate_section_to_memory                                      *
*                                                                      *
*   Synopsis                                                           *
*      void allocate_section_to_memory( void )                         *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for allocating memory for a given   *
*   section.                                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void allocate_section_to_memory( void )
{
    ADDRESS              *addr1;
    ADDRESS              *addr2;
    register OUTPUT_SECT *out_sect_ptr;
    register MEMORY      *mptr;
    int                  failure;

    for( out_sect_ptr = (OUTPUT_SECT *) output_section_list.head; out_sect_ptr != NULL;
	 out_sect_ptr = out_sect_ptr->next )
    {
	 failure = 1;
	 for( mptr = (MEMORY *) memory_list.head; mptr != NULL; mptr = mptr->mem_next )
	 {
	      if( !strncmp(out_sect_ptr->sec_hdr.s_name, mptr->sec_name, SYMNMLEN)
		  && allocate_check_memory( mptr->addr_head, mptr->addr_tail, out_sect_ptr, 
					    &addr1, &addr2) )
	      {
		  allocate_it( addr1, addr2, out_sect_ptr );
		  failure = 0;
		  break;
	      }
	 }

	 if( failure )
	     USER_ERROR1( "The memory size allocated for section %.8s in the architecture file\
 is insufficient.", out_sect_ptr->sec_hdr.s_name );
    }
}    


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate_check_memory                                           *
*                                                                      *
*   Synopsis                                                           *
*      int allocate_check_memory( ADDRESS *start, ADDRESS *end,        *
*                                 OUTPUT_SECT *out_sect_ptr,           *
*                                 ADDRESS **start_ptr,                 *
*                                 ADDRESS **end_ptr )                  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for determining if a given area of  *
*   memory can be allocated to a section.                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/
	
int allocate_check_memory( ADDRESS *start, ADDRESS *end, OUTPUT_SECT *out_sect_ptr,
			   ADDRESS **start_ptr, ADDRESS **end_ptr )
{
    register ADDRESS *addr_ptr1;
    register ADDRESS *addr_ptr2;
    unsigned long     what_we_have;  
    unsigned long     what_we_need;

    addr_ptr1 = addr_ptr2 = start;

    while( addr_ptr1->addr_type != ADDRESS_FREE )
	   if( addr_ptr1 == end )
	       return( 0 );
	   else
	       addr_ptr1 = addr_ptr1->next;

    addr_ptr2 = addr_ptr1;

    what_we_have = addr_ptr1->size;
    what_we_need = (long)(out_sect_ptr->sec_hdr.s_size / 
			  ((out_sect_ptr->sec_hdr.s_flags & SECTION_PM) == SECTION_PM
			  ? (long)PM_WORD_SIZE : (long)DM_WORD_SIZE));

    if( what_we_need > what_we_have )
	return( 0 );

    if( what_we_have > what_we_need )
	allocate_adjust_memory( addr_ptr2, what_we_need + addr_ptr1->addr );

    *start_ptr = addr_ptr1;
    *end_ptr   = addr_ptr2;

    return( 1 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate_adjust_memory                                          *
*                                                                      *
*   Synopsis                                                           *
*    void allocate_adjust_memory(ADDRESS *addr_ptr,uns long address )  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for 'splitting' up a area of memory *
*   in the case where one section does not use up all the memory.      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void allocate_adjust_memory( register ADDRESS *addr_ptr, long unsigned address )
{
    register ADDRESS *new;

    new = (ADDRESS *) my_calloc( (long)sizeof(ADDRESS) );
    *new = *addr_ptr;

    addr_ptr->size = address - addr_ptr->addr;
    new->size -= addr_ptr->size;
    new->addr = address;
    new->previous = addr_ptr;
    addr_ptr->next = new;

    if( addr_ptr->mem_ptr->addr_tail == addr_ptr )
	addr_ptr->mem_ptr->addr_tail = new;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      allocate_it                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void allocate_it( ADDRESS *addr_ptr1, ADDRESS *addr_ptr2,       *
*                        register OUTPUT_SECT *out_sect_ptr )          *
*                                                                      *
*   Description                                                        *
*      This routine binds a section to a physical address.             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void allocate_it( ADDRESS *addr_ptr1, ADDRESS *addr_ptr2, register OUTPUT_SECT *out_sect_ptr )
{
   register ADDRESS *p;

   for( p = addr_ptr1; p != addr_ptr2->next; p= p->next )
   {
	p->addr_type = ADDRESS_ALLOCATED;
	p->sect_ptr = out_sect_ptr;
   }

   out_sect_ptr->sec_hdr.s_paddr = out_sect_ptr->sec_hdr.s_vaddr = addr_ptr1->addr;
}
