/* @(#)slotvec.c        2.1 5/25/93 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "error.h"
#include <malloc.h>
#include "slotvec.h"

#include "error_fp.h"
#include "slotvec_fp.h"
#include "util.h" 

SLOTVEC        *sv_page_ptr = NULL;    /* pointer to array of slots */
SLOTVEC        *sv_page_ptr1 = NULL;    /* pointer to array of slots */
unsigned short    sv_high_slot = 0;      /* index of next available slot */
static long     sv_high_index = 0L;    /* sym. tbl index of symbol assigned to last assigned slot */

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      getslot                                                         *
*                                                                      *
*   Synopsis                                                           *
*      SLOTVEC *getslot( unsigned short slot )                         *
*                                                                      *
*   Description                                                        *
*      This module returns the slot vector                             *
***********************************************************************/
SLOTVEC *getslot(unsigned short slot)
{
   if (slot >=MALLOC_CUTOFF_SIZE/SLOTSIZ) 
      return(&sv_page_ptr1[(slot-MALLOC_CUTOFF_SIZE/SLOTSIZ)]);
   else
      return(&sv_page_ptr[(slot)]);
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      slotvec_allocate                                                *
*                                                                      *
*   Synopsis                                                           *
*      void slotvec_allocate( long numslots )                          *
*                                                                      *
*   Description                                                        *
*      This module administers the slot vector used during output of   *
*  the object file.  A slot is used to store the information needed to *
*  permit the relocation of all references to a single symbol. Using a *
*  slot vector prevents the link editor from having to look up each    *
*  symbol in the global symbol table in order to relocate it. The size *
*  of the slot vector is dependant on the size of the largest input    *
*  symbol table, since it is rebuilt for every input object file.      *
*  The 'slotvector' is a unix system V concept.                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

void slotvec_allocate( long numslots )
{

    if (numslots > MALLOC_CUTOFF_SIZE/SLOTSIZ) {
	if( (sv_page_ptr = (SLOTVEC *) my_calloc((long) (MALLOC_CUTOFF_SIZE))) == NULL )
	{
		FATAL_ERROR( "Failed to allocate memory slotvector table" );
	}
	if( (sv_page_ptr1 = (SLOTVEC *) my_calloc((long) ((numslots-MALLOC_CUTOFF_SIZE/SLOTSIZ) * SLOTSIZ))) == NULL )
	{
	    FATAL_ERROR( "Failed to allocate memory slotvector table" );
	}
    } else {
	if( (sv_page_ptr = (SLOTVEC *) my_calloc((long) (numslots * SLOTSIZ))) == NULL )
	{
	    FATAL_ERROR( "Failed to allocate memory slotvector table" );
	}
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      slotvec_create                                                  *
*                                                                      *
*   Synopsis                                                           *
*      void slotvec_create( long sym_index, long orig_vaddr,           *
*                           long new_vaddr, long new_index, int secnum,* 
*                           int flags )                                *
*                                                                      *
*   Description                                                        *
*      Get and initialize the next available slot in the slot vector.  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

void slotvec_create( long sym_index, long orig_vaddr, long new_vaddr, long new_index, 
		     int secnum, int flags )
{
    register SLOTVEC *p;

    p = getslot( sv_high_slot++ );
    sv_high_index = sym_index;

    p->sv_symindex     = sym_index;
    p->sv_vaddr        = orig_vaddr;
    p->sv_new_vaddr    = new_vaddr;
    p->sv_new_symindex = new_index;
    p->sv_new_secnum   = (char) secnum;
    p->sv_flags        = (char) flags;

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      slotvec_read                                                    *
*                                                                      *
*   Synopsis                                                           *
*      SLOTVEC *slotvec_read( long sym_index )                         *
*                                                                      *
*   Description                                                        *
*      Return the pointer to the slot belonging to the symbol whose old*
*   (e.g., input) symbol table index is given by "sym_index"           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/13/89       created                  -----             *
***********************************************************************/

SLOTVEC *slotvec_read( long sym_index )
{
    register SLOTVEC *p;
    register unsigned int guess;
    register unsigned int high;
	     unsigned int low;
	     unsigned int diff;

    if( sym_index == sv_high_index )
	return( getslot(sv_high_slot - 1) );

    high = sv_high_slot - 1;
    low = 0;

    while ( (diff = high - low) != 0 )
    {
	     guess = low + (diff / 2);
	     p = getslot( guess );
	     if( p->sv_symindex == sym_index )
		 return( p );

	     if( p->sv_symindex > sym_index )
	     {
		if( high == guess )
		    break; 
		high = guess;
	     }
	     else
	     {
		 if( low == guess )
		     break;
		 low = guess;
	     }
    }

    return( NULL );
}
