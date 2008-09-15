/* @(#)obj_sym_misc.h	1.6 5/9/91 1 */


typedef struct stack_sym
{
    long symbol_index;
    long function_length;
    long function_beginning;
    long forward_index;
    long section;
    long linnumber;
    struct stack_sym *next;
} STACK_SYM;

extern short stack_top;

extern STACK_SYM *pop_sym_ptr( void );
extern void save_symbol_index ( short value );
extern void fixup_symbol_table( FILE *fd );
