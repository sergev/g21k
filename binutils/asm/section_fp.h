/* @(#)section_fp.h	1.5 1/24/91 1 */

extern short in_section;
extern long section_cnt;
extern SEC_DATA section_data[];
extern SCNHDR section_header[];

extern short make_section( SYMBOL *symbol, long attr );
extern void  change_section( short sec_num );
extern void  end_section( void );
extern void  set_current_section_type(long new_type);
extern long  get_current_section_type( void );
