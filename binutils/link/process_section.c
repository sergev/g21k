/* @(#)process_section.c        2.3 2/13/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "a_out.h"
#include "action.h"
#include "input_file.h"
#include "input_section.h"
#include "output_section.h"
#include "list.h"
#include "read_object.h"
#include "process_args.h"
#include "util.h"

#include "process_section_fp.h"
#include "list_fp.h"
#include "memory_fp.h"

long num_output_sects = 0l;
LIST output_section_list;

#ifdef DEBUG
void print_input_list( void );
#endif


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_section_list                                            *
*                                                                      *
*   Synopsis                                                           *
*      void process_section_list( void )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating a output section for   *
*   every input section, and if there are a bunch of input sections in *
*   different files, with the same name, bringing them together in one *
*   section.                                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void process_section_list( void )
{
    ACTION               *action_ptr1;
    ACTION               *action_ptr2;
    register OUTPUT_SECT *out_sect_ptr;
    register INPUT_SECT  *input_sect_ptr;
    register INPUT_FILE  *input_file_ptr;

    if( section_list.head )
    {
	action_ptr1 = (ACTION *) section_list.head;
	while( action_ptr1 )
	{
	       out_sect_ptr = (OUTPUT_SECT *) my_calloc( (long)sizeof(OUTPUT_SECT) );

	       /* Here's the plan:
		* Search all files for unallocated sections with name 'action_ptr1->sect_name'
		* and add them to the current output section. So if we have a bunch of, say
		* '.text' segments, they will all end up in one section.
		*/

#ifdef DEBUG
		    printf("\n\nprocess_section_list: action section = %s\n", action_ptr1->sect_name);
		    fflush(stdout);
#endif
	       for( input_file_ptr = (INPUT_FILE *) input_file_list.head; input_file_ptr;
		    input_file_ptr = input_file_ptr->infile_next )
	       {
#ifdef DEBUG
		    printf("process_section_list: file name = %s\n\n", input_file_ptr->file_name);
		    fflush(stdout);
#endif

		    for( input_sect_ptr = input_file_ptr->head; input_sect_ptr;
			 input_sect_ptr = input_sect_ptr->next )
		    {
#ifdef DEBUG
			 printf("process_section_list: input section = %.8s\n", 
				input_sect_ptr->sec_hdr.s_name);
			 fflush(stdout);
#endif
			 if( !input_sect_ptr->out_sec && !strncmp( action_ptr1->sect_name,
			     input_sect_ptr->sec_hdr.s_name, SYMNMLEN ) )
			 {
			     add_input_section( input_sect_ptr, out_sect_ptr );
			 }
		    }
#ifdef DEBUG
		    printf("\n");
#endif
	       }
		
	       out_sect_ptr->sec_hdr.s_flags = action_ptr1->sect_type;
	       if (is_z (processor))
		 {
		   /* roll in the section width info for the z family */
		   switch ((int)action_ptr1->width)
		     {
		     case 16:
		       out_sect_ptr->sec_hdr.s_flags |= SECTION_16;
		       break;
		     case 32:
		       out_sect_ptr->sec_hdr.s_flags |= SECTION_32;
		       break;
		     case 40:
		       out_sect_ptr->sec_hdr.s_flags |= SECTION_40;
		       break;
		     case 48:
		       out_sect_ptr->sec_hdr.s_flags |= SECTION_48;
		       break;
		     }
		 }
	       strncpy( out_sect_ptr->sec_hdr.s_name, action_ptr1->sect_name, 
			SYMNMLEN );

	       if( action_ptr1->type == ACTION_DEFINE_SECTION )
		   num_output_sects++;

	       out_sect_ptr->sec_hdr.s_paddr = -1L;
	       list_add( LIST_OUTPUT_SECTION, &output_section_list, (char *) out_sect_ptr );

	       action_ptr2 = action_ptr1;
	       action_ptr1 = action_ptr1->next;
	       my_free( (char *) action_ptr2 );
	}
    }

    /* Make sure all input sections are allocated into some output section */

    check_input_sections();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      add_input_section                                               *
*                                                                      *
*   Synopsis                                                           *
*      void add_input_section( INPUT_SECT *input_sect_ptr,             *
*                              OUTPUT_SECT *output_sect_ptr )          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for adding a input section to a     *
*   output section.                                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void add_input_section( register INPUT_SECT *input_sect_ptr, register OUTPUT_SECT *output_sect_ptr )
{
#ifdef DEBUG
    printf("add_input_section: input section = %.8s\n", input_sect_ptr->sec_hdr.s_name);
    fflush(stdout);
#endif
    input_sect_ptr->out_sec = output_sect_ptr;
    input_sect_ptr->data_disp = output_sect_ptr->sec_hdr.s_size;
    input_sect_ptr->line_disp = ((long) output_sect_ptr->sec_hdr.s_nlnno * (long) LINESZ);
    output_sect_ptr->sec_hdr.s_size += input_sect_ptr->sec_hdr.s_size;
    output_sect_ptr->sec_hdr.s_nlnno += input_sect_ptr->sec_hdr.s_nlnno;

    if( rflag )
    {
	input_sect_ptr->reloc_disp = ((long) output_sect_ptr->sec_hdr.s_nreloc * (long)RELOC_SIZE);
	output_sect_ptr->sec_hdr.s_nreloc += input_sect_ptr->sec_hdr.s_nreloc;
    }

    list_add( LIST_INC_INPUT_SECTS, (LIST *) output_sect_ptr, (char *) input_sect_ptr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      check_input_sections                                            *
*                                                                      *
*   Synopsis                                                           *
*      void check_input_sections( void )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for making sure that all input      *
*   sections are allocated into some output section.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/7/89        created                  -----             *
***********************************************************************/

void check_input_sections( void )
{
    register OUTPUT_SECT *out_sect_ptr;
    register INPUT_SECT  *input_sect_ptr;
    register INPUT_FILE  *input_file_ptr;

    for( input_file_ptr = (INPUT_FILE *) input_file_list.head; input_file_ptr;
	 input_file_ptr = input_file_ptr->infile_next )
    {
	 for( input_sect_ptr = input_file_ptr->head; input_sect_ptr;
	      input_sect_ptr = input_sect_ptr->next )
	 {
	      if( !input_sect_ptr->out_sec )
	      {
		  if( (out_sect_ptr = list_find_output_section(input_sect_ptr->sec_hdr.s_name)) !=NULL )
		       add_input_section( input_sect_ptr, out_sect_ptr );
		  else
		  {
		       out_sect_ptr = (OUTPUT_SECT *) my_calloc( (long)sizeof(OUTPUT_SECT) );
		       num_output_sects++;
		       strncpy( out_sect_ptr->sec_hdr.s_name, input_sect_ptr->sec_hdr.s_name, 
				SYMNMLEN );
		       out_sect_ptr->sec_hdr.s_paddr = -1L;
		       list_add( LIST_OUTPUT_SECTION, &output_section_list, (char *) out_sect_ptr );
		       add_input_section( input_sect_ptr, out_sect_ptr );
		  }
	      }
	 }
    }
}
