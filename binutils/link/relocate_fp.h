/* @(#)relocate_fp.h	1.5  6/11/91 */
extern void relocate_section( FILE *input_file_fd, INPUT_FILE *input_file_ptr, 
                              INPUT_SECT *input_sect_ptr, FILE *output_fd, long offset, 
                              char *sect_buffer, long buffer_size );
extern int relocate_addr_offset( short reloc_type );
