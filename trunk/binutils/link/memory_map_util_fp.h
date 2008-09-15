/* @(#)memory_map_util_fp.h	1.5 6/11/91 1 */

MEM_MAP *mem_map_init( char *filename, int num_symbols );
void mem_map_add_symbol( SYMENT *obj_sym_ptr, long address, INPUT_SECT *input_sect_ptr, 
                         MEM_MAP *map_ptr, long mem_type );
