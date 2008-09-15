/* @(#)memory_fp.h	2.2  2/13/95 */

extern void memory( char *name, short attr, short type, long addr, long length, short have_uninit, unsigned long width);
extern void memory_check( void );
extern int segment_uninit(char *name);
extern void add_swap (char *segment_name, char *swap_name);
extern unsigned short is_z (int proc);
