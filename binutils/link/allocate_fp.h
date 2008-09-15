/* @(#)allocate_fp.h	1.5  6/11/91 */

extern void allocate( void );
extern void allocate_address_space( void );
extern void allocate_section_to_memory( void );
extern int  allocate_check_memory( ADDRESS *start, ADDRESS *end, OUTPUT_SECT *out_sect_ptr,
                                   ADDRESS **start_ptr, ADDRESS **end_ptr );
extern void allocate_adjust_memory( ADDRESS *addr_ptr, unsigned long address );
extern void allocate_it( ADDRESS *addr_ptr1, ADDRESS *addr_ptr2, OUTPUT_SECT *out_sect_ptr );
