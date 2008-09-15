/* This file provides support for the routines related to the architecture of
 * the target system. */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "util.h"
#include "error.h"
#include "memory.h"
#include "memory_fp.h"

#include "error_fp.h"
#include "achparse.h"
#include "a_out.h"
#include "create_object.h"
#include "action_fp.h"

extern char             Init_segment_name[];

int                     processor = 0;

void 
ach_system (char *label)
{
  strcpy (Arch_descrip_name, label);
}

void 
ach_processor (int processor_token)
{
  processor = processor_token;
}

void 
ach_segment (unsigned long begin,
	     unsigned long end,
	     int type,
	     int access,
	     char *symbol,
	     unsigned short have_uninit,
	     unsigned long width)
{
  int                     attributes, link_type;
  char                    error_msg[250];

  sprintf (error_msg,
	   "Segment %s, must be less than 4294967296 0x100000000 in length\n", symbol);
  if (access == ACH_RAM)
    attributes = ATTR_RAM;

  if (access == ACH_ROM)
    attributes = ATTR_ROM;

  if (access == ACH_PORT)
    attributes = ATTR_PORT;

  if (type == ACH_PM)
    link_type = TYPE_PM;
  else
    link_type = TYPE_DM;

  if ((begin == 0L) && (end == 0xffffffffL)) /* Check for segment to large */
    FATAL_ERROR (error_msg);

  memory (symbol, attributes, link_type,
	  begin, end - begin + 1, have_uninit, width);
  action_define_section (symbol, link_type, attributes, begin, width);
}

void 
ach_endsys (void)
{
}

void 
ach_error (char *msg)
{
  FATAL_ERROR (msg);
}

void 
ach_bank (unsigned long begin, unsigned long pgsize, \
	  unsigned long wtstates, unsigned short type, \
	  unsigned short wtmode, unsigned short which_bank,
	  unsigned short pgwten)
{
  int                     begin_index, mode_index;
  int                     page_scale, wait_scale;
  long int                mode_value = 2L;

  begin_index = -1;
  wait_scale = 0;
  if ((which_bank == ACH_PM0) || (which_bank == ACH_PM1))
    {
      page_scale = 10;
      mode_index = PMWAIT_OFFSET;
    }
  else
    {
      page_scale = 20;
      mode_index = DMWAIT_OFFSET;
    }

  if (which_bank == ACH_DM1)
    {
      begin_index = DMBANK1_OFFSET;
      wait_scale = 5;
    }
  if (which_bank == ACH_DM2)
    {
      begin_index = DMBANK2_OFFSET;
      wait_scale = 10;
    }
  if (which_bank == ACH_DM3)
    {
      begin_index = DMBANK3_OFFSET;
      wait_scale = 15;
    }
  if (which_bank == ACH_PM1)
    {
      begin_index = PMBANK1_OFFSET;
      wait_scale = 5;
    }

  if (wtmode == ACH_EXTERNAL)
    mode_value = 0;
  if (wtmode == ACH_INTERNAL)
    mode_value = 1;
  if (wtmode == ACH_BOTH)
    mode_value = 2;
  if (wtmode == ACH_EITHER)
    mode_value = 3;

  if ((begin != -1L) && (begin_index != -1))
    C_init_list[begin_index] = begin;

  if (pgsize != 0L)
    {
      int                     i;

      if (pgwten)
	{
	  C_init_list[mode_index] |= (8L << page_scale);
	}
      pgsize >>= 9;
      for (i = 0; pgsize != 0; i++)
	pgsize >>= 1;

      pgsize = i;
      C_init_list[mode_index] &= (~(7L << page_scale));
      C_init_list[mode_index] |= pgsize << page_scale;
    }

  if (wtstates != -1L)
    {
      C_init_list[mode_index] &= (~(7L << (wait_scale + 2)));
      C_init_list[mode_index] |= wtstates << (wait_scale + 2);

      C_init_list[mode_index] &= (~(3L << wait_scale));
      C_init_list[mode_index] |= mode_value << wait_scale;
    }
  type;
}

void 
ach_scratch_register (char *reg)
{
  reg;
}

void 
ach_reserved_register (char *reg)
{
  reg;
}

void 
ach_circular_register (char *dag, char *variable)
{
  dag;
  variable;
}

void 
ach_cheap (unsigned long length, int type, int access, char *label)
{
  struct heap_list       *heap;

  if (length == 0)
    FATAL_ERROR ("Heap must be less than 4294967296 (0x100000000) in length");

  if (access != ACH_RAM)
    FATAL_ERROR ("Heap must be located RAM\n");

  for (heap = &heap_base; heap->next != NULL; heap = heap->next)
    ;

  heap->next = (struct heap_list *) my_malloc ((long) sizeof (struct heap_list));
  heap = heap->next;
  heap->next = NULL;
  strncpy (heap->name, label, SYMNMLEN);
  heap->name[SYMNMLEN] = '\0';
  heap->length = length;
  if (type == ACH_PM)
    heap->location = 1;
  else
    heap->location = -1;
}

void 
ach_cstack (unsigned long length, int type, int access, char *label)
{
  if (length == 0)
    FATAL_ERROR ("Stack must be less than 4294967296 (0x100000000) in length\n");

  if (access != ACH_RAM)
    FATAL_ERROR ("Stack must be located in RAM\n");

  strncpy (Stack_segment_name, label, SYMNMLEN);
  Stack_segment_name[SYMNMLEN] = '\0';
  type;
}

void 
ach_cdefaults (ACH_TOKEN which, int type, char *label)
{
  if (which == ACH_CINIT)
    strcpy (Init_segment_name, label);
  type;
}

void
ach_compiler (short map_double_as_floats,
	      short jjb,
	      short param_passing,
	      short chip_rev)
{
  map_double_as_floats;
  jjb;
  param_passing;
  chip_rev;
}

void 
ach_overlay (char *store_seg, char *swap_seg, char *label)
{
  add_swap (label, swap_seg);
}
