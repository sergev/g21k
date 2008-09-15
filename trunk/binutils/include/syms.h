struct syment
{
    union
    {
	char    _n_name[SYMNMLEN];  /* Symbol name (if .LE. 8) */
	struct                      /* if _n_name[0-3] == 0 */
	{
	    long    _n_zeroes;      /* then _n_name[4-7] is an */
	    long    _n_offset;      /* offset into the string table */
	} _n_n;
	struct
	{
		long  _n_padding;
		char *_n_nptr;      /* allows for overlaying */
	} _n_p;
      } _n;
    long            n_value;        /* value of symbol */
    short           n_scnum;        /* section number */
    unsigned short  n_type;         /* type and derived type */
    char            n_sclass;       /* storage class */
    char            n_numaux;       /* number of aux. entries */
} PACKED ;

#define     SYMENT      struct syment
#define     SYMESZ      sizeof(SYMENT)

#define     n_name      _n._n_name
#define     n_nptr      _n._n_nptr
#define     n_zeroes    _n._n_n._n_zeroes
#define     n_offset    _n._n_n._n_offset

#define     BTMASK      0xFL


/* Brought over from /usr/xsoft/21k/include/obj_sym.h with appropriate name fixes */
#define OBJ_SYM_NAME(symbol)            ((symbol)->_n._n_name)
#define OBJ_SYM_ZEROES(symbol)          ((symbol)->n_zeroes)
#define OBJ_SYM_OFFSET(symbol)          ((symbol)->n_offset)
#define OBJ_SYM_VALUE(symbol)           ((symbol)->n_value)
#define OBJ_SYM_SECT(symbol)            ((symbol)->n_scnum)
#define OBJ_SYM_TYPE(symbol)            ((symbol)->n_type)
#define OBJ_SYM_SCLASS(symbol)          ((symbol)->n_sclass)
#define OBJ_SYM_NUMAUX(symbol)          ((symbol)->n_numaux)
#define IN_OBJ_STRING_TABLE(symbol)     (OBJ_SYM_ZEROES(symbol) == 0)


/* the rest is verbatum from /usr/xsoft/21k/include/obj_sym.h */

/* type information */

#define T_NULL    0
#define T_VOID    1
#define T_CHAR    2
#define T_SHORT   3 
#define T_INT     4 
#define T_LONG    5
#define T_FLOAT   6
#define T_DOUBLE  7 
#define T_STRUCT  8 
#define T_UNION   9 
#define T_ENUM    10 
#define T_MOE     11 
#define T_UCHAR   12 
#define T_USHORT  13 
#define T_UINT    14 
#define T_ULONG   15


/* derived types */

#define DT_NON    0   /* no derived type */
#define DT_PTR    0x10   /* pointer to ... */
#define DT_FCN    0x20   /* function returning ... */
#define DT_ARY    0x30   /* array of ... */

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
#define C_CRTL     107  /* like C_EXT, but from C RTL */
#define C_SECTION  108  /* COFF section name */
#define C_PROLOG   109  /* End of C function prolog marker */

/* section numbers */

#define N_DEBUG    -2   /* special symbolic debugging symbol */  
#define N_ABS      -1   /* absolute symbol */
#define N_UNDEF     0   /* undefined external symbol */
#define N_SCNUM         /* section number where symbol is defined */

#define OBJ_TYPE_MASK          0x30
#define OBJ_BASIC_TYPE_MASK    0xF
#define TYPE_SHIFT             0x2
#define OBJ_SYM_BASIC_TYPE(x)  ((x) & OBJ_BASIC_TYPE_MASK)


#define IS_FCN(x)               ((x & OBJ_TYPE_MASK) == DT_FCN)
#define IS_ARY(x)               ((x & OBJ_TYPE_MASK) == DT_ARY)
#define IS_PTR(x)               ((x & OBJ_TYPE_MASK) == DT_PTR)
#define IS_TAG(x)               ((x) == C_STRTAG || (x) == C_UNTAG || (x) == C_ENTAG)

#define DECREF(x) (((x >> TYPE_SHIFT) & ~OBJ_BASIC_TYPE_MASK) | (x & OBJ_BASIC_TYPE_MASK))
