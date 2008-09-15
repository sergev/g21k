

/* WARNING: The size of a symbol has to be the same size as auxillary
 * entry. If necessary padding must be added, if anything changes. In
 * addition, any changes to the symbol structure must be reflected in
 * the routines that read and write the object file format in a portable
 * manner.
 */

#define SYMBOL_NAME_LENGTH 8

typedef struct
{
    union
    {
        char name[SYMBOL_NAME_LENGTH];  /* symbol name */

        struct
        {
            long zeroes;           /* symbol name */
            long offset;           /* location in string table */
        } strtab_entry;

        char *name_ptr[2];         /* allows overlaying */
    } what;

    unsigned long value;           /* value of symbol; type dependent */
    short         section_num;     /* section number */
    unsigned long type;            /* symbol type */
    char          sclass;          /* storage class */
    char          num_aux;         /* number of auxillary entries */
    short         mem_type;        /* memory type: code, dmdata, or pmdata */
} OBJ_SYM;
     
#define OBJ_SYM_SIZE sizeof(OBJ_SYM)

#define OBJ_SYM_NAME(symbol)  ((symbol)->what.name)
#define OBJ_SYM_ZEROES(symbol) ((symbol)->what.strtab_entry.zeroes)
#define OBJ_SYM_OFFSET(symbol) ((symbol)->what.strtab_entry.offset)
#define OBJ_SYM_VALUE(symbol) ((symbol)->value)
#define OBJ_SYM_SECT(symbol)  ((symbol)->section_num)
#define OBJ_SYM_TYPE(symbol)  ((symbol)->type)
#define OBJ_SYM_SCLASS(symbol) ((symbol)->sclass)
#define OBJ_SYM_NUMAUX(symbol)   ((symbol)->num_aux)
#define IN_OBJ_STRING_TABLE(symbol) (OBJ_SYM_ZEROES(symbol) == 0)

/* type information */

#define T_NULL    0L
#define T_VOID    1L
#define T_CHAR    2L
#define T_SHORT   3L 
#define T_INT     4L 
#define T_LONG    5L
#define T_FLOAT   6L
#define T_DOUBLE  7L 
#define T_STRUCT  8L 
#define T_UNION   9L 
#define T_ENUM    10L 
#define T_MOE     11L 
#define T_UCHAR   12L 
#define T_USHORT  13L 
#define T_UINT    14L 
#define T_ULONG   15L
 
/* derived types */

#define DT_NON    0L   /* no derived type */
#define DT_PTR    0x10L   /* pointer to ... */
#define DT_FCN    0x20L   /* function returning ... */
#define DT_ARY    0x30L   /* array of ... */

/* storage class */

#define C_EFCN    -1    /* physical end of function */
#define C_NULL     0    /* just what is says */
#define C_AUTO     1    /* automatic variable */
#define C_EXT      2    /* external symbol */
#define C_STAT     3    /* static symbol */
#define C_REG      4    /* register variable */
#define C_EXTDEF   5    /* external definition */
#define C_LABEL    6    /* label */
#define C_ULABEL   7    /* undefined label */
#define C_MOS      8    /* member of structure */
#define C_ARG      9    /* function argument */
#define C_STRTAG   10   /* structure tag */
#define C_MOU      11   /* member of union */
#define C_UNTAG    12   /* union tag */
#define C_TPDEF    13   /* type definition */
#define C_USTATIC  14   /* uninitialized static */
#define C_ENTAG    15   /* enumeration tag */
#define C_MOE      16   /* member of enumeration */
#define C_REGPARM  17   /* register parameter */
#define C_FIELD    18   /* bit field */

/* storage class for special symbols */

#define C_BLOCK    100  /* beginning and end of block */
#define C_FCN      101  /* beginning and end of function */
#define C_EOS      102  /* end of structure/union */
#define C_FILE     103  /* filename */
#define C_LINE     104  /* used only by utility programs */
#define C_ALIAS    105  /* duplicated tag */
#define C_HIDDEN   106  /* like static, used to avoid name conflicts */

/* section numbers */

#define N_DEBUG    -2   /* special symbolic debugging symbol */  
#define N_ABS      -1   /* absolute symbol */
#define N_UNDEF     0   /* undefined external symbol */
#define N_SCNUM         /* section number where symbol is defined */

#define OBJ_TYPE_MASK          0x30L
#define OBJ_BASIC_TYPE_MASK    0xFL
#define TYPE_SHIFT             0x2L
#define OBJ_SYM_BASIC_TYPE(x)  ((x) & OBJ_BASIC_TYPE_MASK)


#define IS_FCN(x)               ((x & OBJ_TYPE_MASK) == DT_FCN)
#define IS_ARY(x)               ((x & OBJ_TYPE_MASK) == DT_ARY)
#define IS_PTR(x)               ((x & OBJ_TYPE_MASK) == DT_PTR)
#define IS_TAG(x)               ((x) == C_STRTAG || (x) == C_UNTAG || (x) == C_ENTAG)

#define DECREF(x) (((x >> TYPE_SHIFT) & ~OBJ_BASIC_TYPE_MASK) | (x & OBJ_BASIC_TYPE_MASK))
