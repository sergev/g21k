/* @(#)linker_sym.h	1.4 6/11/91 1 */

typedef struct sym
{
    SYMENT      obj_sym;           /* Symbol entry */
    INPUT_SECT  *input_sect;        /* Pointer to input section */
    long         acid;              /* Access ID */
    long         chain_id;          /* ID Pointer to hash collison chain */ 
    long         aux_id;            /* ID to auxillary entry */
    long         new_val;           /* Value after relocation */
    long         new_virtual;       /* New virtual value or offset */
    long         sym_index;         /* Index of symbol in output symbol table */
    int          name_length;       /* length of name */
    char         local_flag;        /* Flag saying this entry goes in the local part
                                     * of the symbol table, for entries such as functions.
                                     */
    char         undefined_msg;     /* Indicates whether a undefined message was printed */         
} LINKER_SYM;


#define LINKER_SYM_SIZE    sizeof(LINKER_SYM)

#define HASHSIZE           521
#define MAX_EXTRA          100
#define EXTRA_SYM_SIZE     100
#define EXTRA_AUX_SIZE     50
#define SYMBOL_TABLE_SIZE  200
#define AUX_TABLE_SIZE     100
