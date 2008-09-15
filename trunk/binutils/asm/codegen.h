/* @(#)codegen.h	1.5 3/21/91 1 */


typedef struct
{
    union
    {
       PM_WORD       instr_value;
       DM_WORD       data_value;
       long          value;
    } code_value;

    unsigned short code_action;
    char *code_name_ptr;
    unsigned short code_size;     /* in bytes */
} CODE;

extern void code_gen( CODE *code, FILE *fd );
extern void code_process( char *file, long start, short section );
