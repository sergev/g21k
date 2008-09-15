/* @(#)object_fp.h	1.4 1/4/91 1 */

extern long num_syms;
extern long num_glob_def_syms;
extern long num_stat_def_syms;
extern long sect_line_ptr[];

extern void object_headers( void );
extern void copy_section( char *file );
extern void write_all_relocation_info( long num_reloc );
extern void make_one_up( SYMBOL *symbol, long value, short section, short sclass,
                         long mem_type );
extern void make_symbol_entry( SYMBOL *symbol );
extern void dump_symbols( void );
