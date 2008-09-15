/* @(#)slotvec_fp.h	1.4  6/11/91 */
extern void slotvec_allocate( long numslots );
extern void slotvec_create( long sym_index, long orig_vaddr, long new_vaddr, long new_index, 
                     int secnum, int flags );
extern SLOTVEC *slotvec_read( long sym_index );
