/* @(#)reloc_fp.h	1.4 1/4/91 1 */

extern long num_reloc;
extern void make_reloc( SYMBOL *symbol, long type );
int write_internal_relocation_info( INTERNAL_RELOC *rel_ptr, FILE *fp );
int read_internal_relocation_info( INTERNAL_RELOC *rel_ptr, FILE *fp );
