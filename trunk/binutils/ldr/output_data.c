#include <stdio.h>
#include <string.h>
#include "app.h"
#include "a_out.h"
#include "util.h"
#include "ldr.h"
#include "error.h"
#define MIN(a,b) (a<b?a:b)

extern unsigned long int Init_segment_ach_size;

void output_zero_data(struct init_info *base, unsigned long int zero_count)
{
	struct init_info *free_blk, *proto;
	unsigned short int length;
	unsigned char data[PM_WORD_SIZE];

	port_put_long(zero_count, data);
	port_put_short(0, data+4);
	fwrite(data, PM_WORD_SIZE, 1, output_fd);

	for(proto=base->next; proto != NULL; )
	{
		length = (short)(proto->end_address - proto->start_address);
		port_put_long(proto->start_address, data);
		port_put_short(length, data+4);
		fwrite(data, PM_WORD_SIZE, 1, output_fd);
		proto = (free_blk = proto)->next;
		my_free(free_blk);
	}
}

void output_init_section(struct my_section_list * my_section, unsigned long int s_scnptr)
{

	my_section->new_hdr.s_scnptr = s_scnptr - delta_section_ptr;
	my_section->new_hdr.s_size += PM_WORD_SIZE * (zero_count_dm + 1);
	my_section->new_hdr.s_size += PM_WORD_SIZE * (zero_count_pm + 1);
	my_section->new_hdr.s_size += init_length_pm + init_length_dm;
	my_section->delta_section_ptr = my_section->new_hdr.s_size - my_section->old_hdr.s_size;
	if( Init_segment_ach_size*6L < my_section->new_hdr.s_size )
	  {
	    char tmpbuf[256];
	    sprintf(tmpbuf,
		    "The initialization segment size (%ld) has exceed the size\n"
		    "specified in the architecture file (%ld) by %ld\n",
		    my_section->new_hdr.s_size/6L,
		    Init_segment_ach_size,
		    (long)(my_section->new_hdr.s_size - Init_segment_ach_size*6L)/6L);
	    FATAL_ERROR( tmpbuf);
	  }
}

#ifndef MSDOS
#define huge  
#endif
void output_init_data(void)
{
   long remaining,num_bytes;
   unsigned char huge *temp_init_string;

  	output_zero_data(&init_bss_dm_base, zero_count_dm);
	output_zero_data(&init_bss_pm_base, zero_count_pm);

        remaining = init_length_dm;
        temp_init_string = init_string_dm;
        while( remaining )
        {
                num_bytes = MIN(32767L, remaining);
                if( fwrite(temp_init_string, (int) num_bytes, 1, output_fd) != 1 )
                        FATAL_ERROR( "Unable to write initialization data\n" );
                remaining -= num_bytes;
		temp_init_string+= num_bytes;
        }

        remaining = init_length_pm;
        temp_init_string = init_string_pm;
        while( remaining )
        {
                num_bytes = MIN(32767L, remaining);
                if( fwrite(temp_init_string, (int) num_bytes, 1, output_fd) != 1 )
                        FATAL_ERROR( "Unable to write initialization data\n" );
                remaining -= num_bytes;
		temp_init_string+= num_bytes;
        }


	my_free(init_string_dm);
	my_free(init_string_pm);
	
}

