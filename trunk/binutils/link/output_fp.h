/* @(#)output_fp.h	1.5 6/11/91 1 */

extern void output_object( void );
extern void output_section_headers( FILE *output_fd );
extern void output_process_input_files( FILE *output_fd, FILE *symbol_fd, 
                                        FILE *string_fd, char *buf );
extern long output_local_symbols( FILE *symbol_fd, FILE *string_fd, FILE *input_file_fd, 
                                  INPUT_FILE *input_file_ptr );
extern int output_get_aux_class( SYMENT *sym ); 
extern void output_global_symbols( FILE *symbol_fd, FILE *string_fd );
extern void output_line_numbers( INPUT_SECT *input_sect_ptr, OUTPUT_SECT *output_sect_ptr,
                                 FILE *input_file_fd, FILE *symbol_fd, FILE *output_fd,
                                 INPUT_FILE *input_file_ptr );
extern void output_update_file_entry( FILE *symbol_fd, long sym_index );
extern void output_write_string( FILE *string_fd, SYMENT *sym, int string_length );
extern void output_section( register FILE *input_file_fd, INPUT_FILE *input_file_ptr, 
                            INPUT_SECT *input_sect_ptr, OUTPUT_SECT *output_sect_ptr, 
                            FILE *output_fd, char *sect_buffer, long buffer_size );
extern void output_init_arrays( INPUT_SECT *input_sect_ptr, int *new_section_num, 
                                long *old_address, long *new_address );
extern void output_file_header( FILHDR *output_file_header, FILE *output_fd );
extern void output_finish_up( FILE *symbol_fd, FILE *output_fd, FILE *string_fd );
