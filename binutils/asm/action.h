/* @(#)action.h 1.4 1/4/91 1 */


#define NO_ACTION              0
#define DEFINE_FILE_SYMBOL     1
#define DEFINE_SYMBOL          2
#define DEFINE_SYMBOL_VALUE    3
#define DEFINE_SYMBOL_SCLASS   4
#define DEFINE_SYMBOL_TAG      5
#define DEFINE_SYMBOL_LINE     6
#define DEFINE_SYMBOL_SIZE     7
#define DEFINE_SECTION_AUXENT  8
#define DEFINE_DIM1            9
#define DEFINE_DIM2            10
#define END_SYMBOL_DEFINE      11
#define LINE_NUMBER            12
#define RELOC_ADDR_VAR         13
#define RELOC_ADDR24           14
#define RELOC_ADDR32           15
#define RELOC_PC_REL_SHORT     16
#define RELOC_PC_REL_LONG      17
#define DEFINE_SYMBOL_TYPE     18
#define NEW_STATEMENT          19


#define IS_VALID_ACTION(x)     (((unsigned)(x)) <= NEW_STATEMENT) /*EK* avoid gcc warning */

extern void (*(array_ptr_functions[]))();
extern long num_line;

extern void define_file_symbol( SYMBOL *symbol, CODE *code );
extern void define_symbol( SYMBOL *symbol, CODE *code );
extern void define_symbol_value( SYMBOL *symbol, CODE *code );
extern void define_symbol_type( SYMBOL *symbol, CODE *code );
extern void define_symbol_sclass( SYMBOL *symbol, CODE *code );
extern void define_symbol_tag( SYMBOL *symbol, CODE *code );
extern void define_symbol_line( SYMBOL *symbol, CODE *code );
extern void define_symbol_size( SYMBOL *symbol, CODE *code );
extern void define_section_auxent( long size, long num_reloc, long num_line );
extern void final_preparations( void );
extern void end_symbol_define( SYMBOL *symbol, CODE *code );
extern int get_aux_class( SYMBOL *sym );
extern void line_number( SYMBOL *symbol, CODE *code );
extern void define_dim1( SYMBOL *symbol, CODE *code );
extern void define_dim2( SYMBOL *symbol, CODE *code );
extern void reloc_addr_var( SYMBOL *symbol, CODE *code );
extern void reloc_addr24( SYMBOL *symbol, CODE *code );
extern void reloc_addr32( SYMBOL *symbol, CODE *code );
extern void reloc_pc_rel_short( SYMBOL *symbol, CODE *code );
extern void reloc_pc_rel_long( SYMBOL *symbol, CODE *code );
extern void new_statement( SYMBOL *symbol, CODE *code );
