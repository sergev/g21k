/* @(#)pass2.h	1.5 5/9/91 1 */


extern long header_ptr;
extern FILE *glob_sym_fd;
extern FILE *line_fd;
extern FILE *obj_fd;
extern FILE *rel_fd;
extern FILE *sym_fd;

extern short pass2( void );
extern void write_string_table( void );
extern void flush_files( void );

extern FILE *stat_sym_fd;


