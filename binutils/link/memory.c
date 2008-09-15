/* @(#)memory.c 1.8 6/11/91 1 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "error.h"
#include "memory.h"
#include "list.h"
#include "util.h"
#include "a_out.h"
#include "action.h"
#include "action_fp.h"

#include "memory_fp.h"
#include "error_fp.h"
#include "list_fp.h"
#include "achparse.h"

LIST                    memory_list;

struct overlay_segment
{
  char                    name[SYMNMLEN];

  struct overlay_segment *next;
}                       overlay_base, *overlay_ptr;

/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      memory                                                          *
 *                                                                      *
 *   Synopsis                                                           *
 *      void memory( char *name, short attr, short type, long addr      *
		    *                        , long length )                               *
		    *                                                                      *
		    *   Description                                                        *
		    *      This routine is responsible for building a linked list of       *
		    *   memory descriptions from the architecture description file.        *
		    *                                                                      *
		    *   Revision History                                                   *
		    *                                                                      *
		    *     name    date          description              number            *
		    *     mkc     5/24/89       created                  -----             *
		    ***********************************************************************/

void 
memory (char *name,
	short attr,
	short type,
	long addr,
	long length,
	short have_uninit,
	unsigned long width)
{
  register MEMORY        *m;
  register MEMORY        *mptr;

  if (memory_list.head)
    {
      mptr = (MEMORY *) memory_list.head;
      while (mptr)
	{
	  if (!strcmp (name, mptr->sec_name))
	    FATAL_ERROR1 ("Duplicate segment name %s in architecture\
 description file.", name);
	  mptr = mptr->mem_next;
	}
    }

  m = (MEMORY *) my_malloc ((long) sizeof (MEMORY));

  if (name)
    strncpy (m->sec_name, name, NAME_SIZE);

  m->mem_attr = attr;
  m->mem_type = type;
  m->start_address = addr;
  m->mem_length = length;
  m->width = width;
  m->mem_next = (MEMORY *) NULL;
  m->addr_head = (ADDRESS *) NULL;
  m->addr_tail = (ADDRESS *) NULL;
  m->mem_uninit = (int) have_uninit;
  list_add (LIST_MEMORY, &memory_list, (char *) m);
}


/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      memory_check                                                    *
 *                                                                      *
 *   Synopsis                                                           *
 *      void memory_check( void )                                       *
 *                                                                      *
 *   Description                                                        *
 *      This routine is responsible for making sure that memory regions *
 *   given in the architecture description file do not overlap.         *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     5/24/89       created                  -----             *
 ***********************************************************************/

void 
memory_check (void)
{
  register MEMORY        *mptr;
  register MEMORY        *nextptr;
  register long           length;

  if (memory_list.head)
    {
      if (is_z (processor))
	{
	  /* Z family -- no pm,dm distinction */
	  mptr = (MEMORY *) memory_list.head;
	  while (mptr->mem_next)
	    {
	      nextptr = mptr->mem_next;
	      if (!nextptr)
		break;
	      length = mptr->start_address + mptr->mem_length;
	      if (length > nextptr->start_address)
		{
		  overlay_ptr = overlay_base.next;
		  while (overlay_ptr)
		    {
		      if (strncmp (overlay_ptr->name, nextptr->sec_name, SYMNMLEN) == 0)
			break;

		      overlay_ptr = overlay_ptr->next;
		    }

		  if (!overlay_ptr)
		    USER_WARN2 ("Memory areas %s and %s overlap.",
				mptr->sec_name, nextptr->sec_name);
		}
	      mptr = mptr->mem_next;
	    }
	}
      else
	{                       /* 21k pm and dm are sep spaces */
	  mptr = (MEMORY *) memory_list.head;
	  while (mptr->mem_next)
	    {
	      if (mptr->mem_type == TYPE_PM)
		{
		  nextptr = mptr->mem_next;
		  /* get next pm address */
		  while (nextptr && nextptr->mem_type == TYPE_DM)
		    nextptr = nextptr->mem_next;

		  if (!nextptr)
		    break;
		  length = mptr->start_address + mptr->mem_length;
		  if (length > nextptr->start_address)
		    {
		      USER_WARN2 ("Memory areas %s and %s overlap.", mptr->sec_name,
				  nextptr->sec_name);
		    }
		}
	      mptr = mptr->mem_next;
	    }
	  /* now go through dm list */
	  mptr = (MEMORY *) memory_list.head;
	  while (mptr->mem_next)
	    {
	      if (mptr->mem_type == TYPE_DM)
		{
		  nextptr = mptr->mem_next;
		  /* get next dm address */
		  while (nextptr && nextptr->mem_type == TYPE_PM)
		    nextptr = nextptr->mem_next;

		  if (!nextptr)
		    break;
		  length = mptr->start_address + mptr->mem_length;
		  if (length > nextptr->start_address)
		    {
		      USER_WARN2 ("Memory areas %s and %s overlap.", mptr->sec_name,
				  nextptr->sec_name);
		    }
		}
	      mptr = mptr->mem_next;
	    }
	}
    }
  else
    FATAL_ERROR ("No memory description of system.");
}

/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      segment start                                                   *
 *                                                                      *
 *   Synopsis                                                           *
 *      unsigned long int segment_start( char *name )                  *
 *                                                                      *
 *   Description                                                        *
 *      This routine finds the starting address of a segment from the   *
 *   ach file                                                           *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     gas     7/19/91       created                  -----             *
 ***********************************************************************/
long int 
segment_start (char *name)
{
  register MEMORY        *mptr;

  if (memory_list.head)
    {
      mptr = (MEMORY *) memory_list.head;
      while (mptr)
	{
	  if (!strncmp (name, mptr->sec_name, SYMNMLEN))
	    return mptr->start_address;
	  mptr = mptr->mem_next;
	}
    }
  FATAL_ERROR1 ("Unable to locate segment %s in architecture description file.", name);
}

/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      segment length                                                  *
 *                                                                      *
 *   Synopsis                                                           *
 *      unsigned long int segment_length( char *name )                 *
 *                                                                      *
 *   Description                                                        *
 *      This routine finds the length of a segment from the             *
 *   ach file                                                           *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     gas     7/19/91       created                  -----             *
 ***********************************************************************/
long int 
segment_length (char *name)
{
  register MEMORY        *mptr;

  if (memory_list.head)
    {
      mptr = (MEMORY *) memory_list.head;
      while (mptr)
	{
	  if (!strncmp (name, mptr->sec_name, SYMNMLEN))
	    return mptr->mem_length;
	  mptr = mptr->mem_next;
	}
    }
  FATAL_ERROR1 ("Unable to locate segment %s in architecture description file.", name);
}

/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      segment uninit                                                  *
 *                                                                      *
 *   Synopsis                                                           *
 *      int segment_uninit( char *name )                                *
 *                                                                      *
 *   Description                                                        *
 *      This routine finds if a segment is declared UNINIT in the       *
 *   ach file                                                           *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *   gas & sbb 5/25/93       created                  -----             *
 ***********************************************************************/
int 
segment_uninit (char *name)
{
  register MEMORY        *mptr;

  if (memory_list.head)
    {
      mptr = (MEMORY *) memory_list.head;
      while (mptr)
	{
	  if (!strncmp (name, mptr->sec_name, SYMNMLEN))
	    return mptr->mem_uninit;
	  mptr = mptr->mem_next;
	}
    }
  FATAL_ERROR1 ("Unable to locate segment %s in architecture description file.", name);
}

void 
add_swap (char *segment_name, char *swap_name)
{
  register MEMORY        *mptr, *m;

  if (memory_list.head)
    {
      mptr = (MEMORY *) memory_list.head;
      for (; (strncmp (swap_name, mptr->sec_name, SYMNMLEN)) && mptr;)
	mptr = mptr->mem_next;

      if (mptr)
	{
	  m = (MEMORY *) my_malloc ((long) sizeof (MEMORY));

	  memcpy (m, mptr, sizeof (MEMORY));

	  strncpy (m->sec_name, segment_name, NAME_SIZE);

	  list_add (LIST_MEMORY, &memory_list, (char *) m);
	}
      else
	{
	  FATAL_ERROR1 ("Segment \"%s\" not yet defined.", swap_name);
	  return;
	}
    }
  else
    {
      FATAL_ERROR1 ("Unable to locate segment %s in architecture description file.", swap_name);
      return;
    }

  action_define_section (m->sec_name, m->mem_type, m->mem_attr, m->start_address, m->width);

  overlay_ptr = (struct overlay_segment *) my_malloc ((long) sizeof (struct overlay_segment));
  memcpy (overlay_ptr->name, m->sec_name, SYMNMLEN);
  overlay_ptr->next = overlay_base.next;
  overlay_base.next = overlay_ptr;
}


unsigned short
is_z (int proc)
{
  switch (proc)
    {
    case ACH_ADSP210Z3:
    case ACH_ADSP21061:
    case ACH_ADSP210Z4:
      return 1;
    default:
      return 0;
    }
}
