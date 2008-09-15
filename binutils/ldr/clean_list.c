#include <stdio.h>
#include <string.h>
#include "app.h"
#include "util.h"
#include "a_out.h"
#include "ldr.h"
#ifndef MSDOS
#define huge  
#endif

#define PX2_SCALE 65536.0

unsigned long int zero_count_dm;
unsigned long int zero_count_pm;

unsigned long int init_length_dm;
unsigned long int init_length_pm;

unsigned long int max_count_dm;
unsigned long int max_count_pm;

unsigned char huge *init_string_dm;
unsigned char huge *init_string_pm;

void count_blocks(struct init_info *base, unsigned long int *count)
{
	struct init_info *proto;	
	*count = 0L;

	for(proto=base->next; proto != NULL; proto=proto->next)
	{
	    (*count) += proto->end_address - proto->start_address;
	}
}

void join_blocks(struct init_info *base, unsigned long int *count)
{
	struct init_info *proto, *candidate, *lc;
	double full_data, test_data;

	
	*count = 0L;

/*	Scan list for contigious blocks with identical data */
	for(proto=base->next; proto != NULL; proto=proto->next)
	{
		(*count) += proto->end_address - proto->start_address;
/*	.	Compute data into a double word for easy compare */
		full_data = proto->px2_data * PX2_SCALE + proto->px1_data;
		lc = proto;
		for(candidate=base->next; candidate != NULL; candidate=candidate->next)
		{
			test_data = candidate->px2_data * PX2_SCALE + candidate->px1_data;
/*	.	.	If the data in both matchs, (and they are not the same block, coalesce */
			if( (test_data == full_data ) && (candidate != proto) )
			{
/*	.	.	.	If end of proto match start of candidate, make proto span to end of cand*/
				if( proto->end_address == candidate->start_address )
				{
					proto->end_address = candidate->end_address;
					lc->next = candidate->next;
					my_free( candidate );
				}
/*	.	.	.	If start of proto matchs end of candidate, make proto begin at cand->start*/
				if( candidate->end_address == proto->start_address )
				{
					proto->start_address = candidate->start_address;
					lc->next = candidate->next;
					my_free( candidate );
				}
			}
/*	.	.	Hold last candidate pointer */
			lc = candidate;
		}
	}
}

struct init_info *remove_bss(struct init_info *base, unsigned long int *count)
{
	struct init_info bss_base, *bss;
	struct init_info *proto, *lc, *nc;
	unsigned long int length;

/*	Initialize pointers */
	*count = 0L;
	bss_base.next = NULL;
	bss = &bss_base;
	lc = base;
	
/*	Scan through list, place zero inits into seperate list */
	for(proto=base->next; proto != NULL; proto=nc)
	{
		nc = proto->next;
/*	.	If data is zero, place in bss list */
		if( (proto->px1_data == 0) && (proto->px2_data == 0L) )	
		{
/*	.	.	Also test for at least 3 locations */
			if( (proto->end_address - proto->start_address) > 2 )
			{
/*	.	.	.	Remove proto from init list, place in bss list */
				lc->next = proto->next;
				bss->next = proto;
				bss = bss->next;
				bss->next = NULL;
				length = proto->end_address - proto->start_address;
/*	.	.	.	Length of init must be less than 65536 */
				for( ;length > 65535L; )
				{
					bss->next = (struct init_info *)my_malloc( (long)sizeof(struct init_info) );
					(*count)++;
					memcpy(bss->next, bss, sizeof(struct init_info));
					bss->end_address = bss->start_address + 65535L;
					bss->next->start_address = bss->end_address;
					bss = bss->next;
					bss->next = NULL;
					length = bss->end_address - bss->start_address;
				}
				proto = lc;
				(*count)++;
			}
		}
		lc = proto;
	}
	return bss_base.next;
}

unsigned long int create_string(struct init_info *base,  unsigned char huge *string)
{

	unsigned long int blk_count;
	unsigned long int this_address;
	unsigned short int this_count;
	unsigned char huge *current_ch; 
        unsigned char huge *current_blk;
	struct init_info *proto, *free_old;
	unsigned long int i;
        unsigned long ret_val;

	if( base->next == NULL )
	{
		port_put_long(0L, string);
		port_put_short(0, string+4);
		return 6L;
	}

	blk_count = 1;
	current_blk = string + 6;
	current_ch = current_blk + 6;
	this_address = base->next->start_address;
	this_count = 0;
	for(proto=base->next; proto->next != NULL; )
	{
/*	.	Check for blocks that might have two points */
		for(i=proto->start_address; i<proto->end_address; i++)
		{
			port_put_long(proto->px2_data, current_ch);
			port_put_short(proto->px1_data, current_ch+4);
			this_count++;
			current_ch += 6;
		}
		if( (proto->end_address != proto->next->start_address ) || (this_count > 65530) )
		{
			blk_count++;
			port_put_long(this_address, current_blk);
			port_put_short(this_count, current_blk+4);
			current_blk = current_ch;
			current_ch +=6;
			this_count = 0;
			this_address = proto->next->start_address;
		}
		free_old = proto;
		proto = proto->next;
		my_free(free_old);
	}
/*	Write out data from last init block */
	for(i=proto->start_address; i<proto->end_address; i++)
	{
		port_put_long(proto->px2_data, current_ch);
		port_put_short(proto->px1_data, current_ch+4);
		this_count++;
		current_ch += 6;
	}
	port_put_long(this_address, current_blk);
	port_put_short(this_count, current_blk+4);
	port_put_long(blk_count, string);
	port_put_short(0, string+4);
	my_free(proto);
        ret_val = (unsigned long)(current_ch - string);
	return (ret_val);
}
	

void clean_list(void)
{
	unsigned char huge *hld_ptr;
        unsigned long i;

/*	Join all contigious blocks that contain identical data */
        max_count_dm=0L;
        max_count_pm=0L;
/* not needed gas/sbb
	join_blocks(&init_base_dm, &max_count_dm);
	join_blocks(&init_base_pm, &max_count_pm);
*/

/*	Remove blocks that are initialized to zero (BSS) */
	init_bss_dm_base.next = remove_bss(&init_base_dm, &zero_count_dm);
	init_bss_pm_base.next = remove_bss(&init_base_pm, &zero_count_pm);

/*	Update the new symbol pointer for the file */
	delta_symptr += PM_WORD_SIZE * (zero_count_dm + 1);
	delta_symptr += PM_WORD_SIZE * (zero_count_pm + 1);

/*      Count blocks */
	count_blocks(&init_base_dm, &max_count_dm);
	count_blocks(&init_base_pm, &max_count_pm);

/*	Malloc and array to hold init values */

	init_string_dm = (unsigned char huge *)my_calloc( (long)(2*PM_WORD_SIZE*(max_count_dm+1)) );
	init_length_dm = create_string(&init_base_dm, init_string_dm);

	hld_ptr = (unsigned char *)my_malloc( (long)init_length_dm );
/*
memcpy won't work with really big stuff
	memcpy(hld_ptr, init_string_dm, (size_t)init_length_dm);
*/
        for (i=0;i<init_length_dm;i++)
            hld_ptr[i] = init_string_dm[i];

	my_free(init_string_dm);
	init_string_dm = hld_ptr;

	init_string_pm = (unsigned char huge *)my_calloc( (long)(2*PM_WORD_SIZE*(max_count_pm+1)) );
	init_length_pm = create_string(&init_base_pm, init_string_pm);
	hld_ptr = (unsigned char *)my_malloc( (long)init_length_pm );
/*
memcpy won't work with really big stuff
	memcpy(hld_ptr, init_string_pm, (size_t)init_length_pm);
*/
        for (i=0;i<init_length_pm;i++)
            hld_ptr[i] = init_string_pm[i];
	my_free(init_string_pm);
	init_string_pm = hld_ptr;

/*	Update symbol pointer based on init_count */
	delta_symptr += init_length_dm;
	delta_symptr += init_length_pm;

}
