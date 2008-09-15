/* @(#)ilnode.h	2.7 10/1/96 */

#define COMPUTE_NULL        0L
#define COMPUTE_TYPE1       1L
#define COMPUTE_TYPE2       2L
#define COMPUTE_TYPE3       3L
#define COMPUTE_TYPE4       4L
#define COMPUTE_TYPE5       5L
#define COMPUTE_TYPE6       6L

/* This is how we indicate in the instruction that there is no compute */

#define COMPUTE_NOT_PRESENT COMPUTE_NULL

extern int compute_type;

#define INSTRUCTION_NULL		0L
#define INSTRUCTION_TYPE1		1L
#define INSTRUCTION_TYPE2		2L
#define INSTRUCTION_TYPE3		3L
#define INSTRUCTION_TYPE4		4L
#define INSTRUCTION_TYPE5		5L
#define INSTRUCTION_TYPE6		6L
#define INSTRUCTION_TYPE7		7L
#define INSTRUCTION_TYPE8		8L
#define INSTRUCTION_TYPE9		9L
#define INSTRUCTION_TYPE10		10L
#define INSTRUCTION_TYPE11		11L
#define INSTRUCTION_TYPE12		12L
#define INSTRUCTION_TYPE13		13L
#define INSTRUCTION_TYPE14		14L
#define INSTRUCTION_TYPE15		15L
#define INSTRUCTION_TYPE16		16L
#define INSTRUCTION_TYPE17		17L
#define INSTRUCTION_TYPE18		18L
#define INSTRUCTION_TYPE19		19L
#define INSTRUCTION_TYPE20		20L
#define INSTRUCTION_TYPE21		21L
#define INSTRUCTION_TYPE22		22L
#define INSTRUCTION_TYPE23		23L
#define INSTRUCTION_TYPE24		24L
#define INSTRUCTION_TYPE25		25L
#define INSTRUCTION_TYPE26		26L
#define INSTRUCTION_TYPE25A     30L     /* new instruction for '061		  */
										/* IDLE16,  opcode 00a0 0000 0000 */
									    /*                MODIFICATION_BL */
#ifndef ADSPZ3
#define INSTRUCTION_TYPE27		27L
#define INSTRUCTION_TYPE28		28L
#define INSTRUCTION_TYPE29		29L
#else 
#define INSTRUCTION_CCALL		27L		/*new instruction for Z3*/
#define INSTRUCTION_RFRAME		28L		/*new instruction for Z3*/
#endif /* ADSPZ3 */

extern int instruction_type;

#define NULL_OPERAND        0x0L
#define IS_NULL_OPERAND(x)  ((x) == NULL_OPERAND)
#define IS_VALID_OPERAND(x) (IS_REG_FILE(x) || IS_NULL_OPERAND(x))

#define NULL_TRANSFER       0xFFL

#define ASM_ERROR           0x01L

#define DF_REG(x)           ((x) & 0xFL)
#define MULTI_REG(x)        ((x) & 0x3L)
#define DAG_REG(x)          ((x) & 0x7L)
#define IS_DATA6(x)         (((x) & ~0x3FL) == 0)

#define ADDRESS_DIRECT       0x01L
#define ADDRESS_INDIRECT     0x02L
#define ADDRESS_PC_RELATIVE  0x03L

#define RET_TYPE_RTS         0x00L
#define RET_TYPE_RTI         0x01L

#define LCNTR_INIT_IMMED     0x00L
#define LCNTR_INIT_UNIV_REG  0x01L

#define NO_BIT_REVERSE       0x00L
#define BIT_REVERSE          0x01L

#define SHIFT_IMMEDIATE      0x01L
#define SHIFT_DREG           0x02L
#define SHIFT_BOTH           0x03L

/* The intermediate language node that a 21000 instruction is transformed into */

typedef struct ilnode
{
    struct
    {
        short x1_reg;
        short y1_reg;
        short result1_reg;

        short x2_reg;
        short y2_reg;
        short result2_reg;

        short x3_reg;
        short y3_reg;
        short result3_reg;

        short cu_reg;
        short data_reg;
        unsigned int t:1;

        long  compute_type;
        long  computation_unit;
        long  opcode;
    } compute;
    
    long instruction_type;
    short error_occurred;
    short cond;            /* This is the condition code field */
    unsigned int shift_data_move:1; /* Is the data move present in a shift */
    unsigned int ret:1;    /* Is the return a RTS or RTI */
    unsigned int linit:1;  /* Is the loop counter init a immed. or univ reg */
    unsigned int lpo:1;
    unsigned int spo:1;
    unsigned int ppo:1;
    unsigned int lpu:1;
    unsigned int spu:1;
    unsigned int ppu:1;
    unsigned int bit_rev:1; /* bit reverse ? */
    unsigned int dmd:1;    /* Data memory d bit */
    unsigned int pmd:1;    /* Program memory d bit */
    unsigned int d:1;      /* Memory access direction, 0 = read, 1 = write */
    unsigned int s:1;      /* Select bit */
    unsigned int g:1;      /* Program or Data Memory, 0 = DM, 1 = PM */
    unsigned int b:1;      /* Branch type, 0 = jump, 1 = call */
    unsigned int j:1;      /* Jump type, 0 = non-delayed jump, 1 = delayed */
    unsigned int a:1;      /* Loop abort, 0 = no abort, 1 = abort */
    unsigned int c:1;      /* Counter select, 0 = Loop counter, 1 = Event counter */
    unsigned int u:1;      /* index register update, pre or post modify */
    unsigned int cs:2;     /* Cache select code */
#ifndef ADSPZ3
    unsigned int oelse:1;  /* Indicates an optional ELSE compute */
#endif /* ADSPZ3 */
    unsigned int cios:1;   /* current int clear (jump/call -- os return rts */
    unsigned int length;   /* We want the length of a symbol so don't relocate it */
    short i;               /* index register when either PM or DM is used */
    short m;               /* modify register when either PM or DM is used */
    short pmi;             /* pm index register */
    short dmi;             /* dm index register */
    short pmm;             /* pm modify register */
    short dmm;             /* dm modify register */
    EXPR  addr;            /* address field */
    EXPR  reladdr;         /* relative address field */
    EXPR  data;            /* data field */
    short dreg;            /* data register */
    short term;            /* termination code */
    short creg;            /* cache register */

    short pm_dreg;         /* PM Data file register, used in PM(I,M) = dreg or vice versa */
    short dm_dreg;
    short src_ureg;        /* Source ureg, used only in ureg to ureg transfer */
    short dest_ureg;       /* Destination ureg, used only in ureg to ureg transfer */
    long  shift_immediate; /* Shift constant */
    long  shift_type;      /* Immediate or contained in aa register */
    short address_type;    /* Direct, PC relative, or indirect */
    short ureg;            /* A universal register, used in 'ureg = const' */
    short bop;             /* System register bit opcode */
    short sreg;            /* A system register */
#ifdef ADSPZ3
    short bit_25;	   /* enable ELSE clause if high for Z3.*/
    short cach_flush;	   /* enable instruction cache flush for Z3*/
#endif /* ADSPZ3 */

#ifdef ASM  /************ These fields are only used in the assembler */

    short cond_kludge;     /* Differentiate between LCE & NOT LCE */
    short pm_ireg;         /* PM Index register */
    short pm_mreg;         /* PM Modify register */
    short pm_ureg;         /* PM Universal reg., used in PM(I,M) = ureg or vice versa     */
    short dm_ireg;
    short dm_mreg;
    short dm_ureg;
    EXPR   pc_disp;         /* PC relative displacement, used in (PC,displacement) */
    EXPR lcntr_data;       /* Used in initializing the loop counter */
    short lcntr_ureg;      /* Universal register used in initializing the loop counter */
    EXPR  pm_addr;         /* A direct PM address */
    EXPR  dm_addr;         /* A direct DM address */
    EXPR  dm_disp;       /* DM(I, disp), where disp is a 32-bit two's-complement number */
    EXPR  pm_disp;       /* PM(I, disp), where disp is a 24-bit two's complement number */
    EXPR immediate;
    long immediate_type;   /* Float or int? */
    short cache_reg;       /* A cache register */

#endif  /*************************************************************/

} ILNODE;

