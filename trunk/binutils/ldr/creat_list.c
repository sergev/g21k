#include <stdio.h>
#include <string.h>

#include "app.h"
#include "util.h"
#include "a_out.h"
#include "ldr.h"

extern long delta_symptr;
extern long delta_section_ptr;

unsigned char long_buff[6];

int get_int_word(int word_size)
{
  	if( word_size == DM_WORD_SIZE )
		return long_buff[4];
	else
		return (long_buff[4]<<8) | long_buff[5];
}	

unsigned long int get_long_word(int word_size)
{
	long int hold;
	
	fread(long_buff, word_size, 1, input_fd);
	
	hold  = ((long)long_buff[0]) << 24;
	hold |= ((long)long_buff[1]) << 16;
	hold |= ((long)long_buff[2]) <<  8;
	hold |= ((long)long_buff[3]);
	
	return hold;
}

void create_init_list(struct my_section_list *my_section, int word_size)
{
	long end, i;
	unsigned int current_low, new_low;
	unsigned long int current_hi, new_hi;
	long int hold_pos;

	if( my_section->new_hdr.s_size == 0 )
		return;

/*	Hold the original position of the file pointer */
	hold_pos = ftell( input_fd );
	fseek(input_fd, my_section->old_hdr.s_scnptr, 0);
	init_ptr->next = 
	(struct init_info *)my_calloc( (long)sizeof(struct init_info) );
	init_ptr = init_ptr->next;

	init_ptr->start_address = my_section->old_hdr.s_paddr;
	end = my_section->old_hdr.s_paddr + 1;

	current_hi = init_ptr->px2_data = get_long_word(word_size);
	current_low = init_ptr->px1_data = get_int_word(word_size);

	for(i=0; i<((my_section->new_hdr.s_size/word_size)-1); i++)
	{
		new_hi = get_long_word(word_size);
		new_low = get_int_word(word_size);

		if( (current_low != new_low) || (current_hi != new_hi) )
		{
			init_ptr->end_address = end;
			init_ptr->next = 
			(struct init_info *)my_calloc( (long)sizeof(struct init_info) );
			init_ptr = init_ptr->next;
			init_ptr->start_address = end;
			current_low = init_ptr->px1_data = new_low;
			current_hi = init_ptr->px2_data = new_hi;
		}
		end++;
	}

	init_ptr->end_address = end;

	delta_symptr -= my_section->new_hdr.s_size;
	my_section->delta_section_ptr -= my_section->new_hdr.s_size;
	my_section->new_hdr.s_size = 0L;
	my_section->new_hdr.s_scnptr = 0L;

/*	Restore the old file pointer */
	fseek( input_fd, hold_pos, 0 );
}
