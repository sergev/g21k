/* @(#)lexical.h	2.3 6/22/93 2 */

typedef struct
{
   long reg;
   long type;
} REGTYPE;

#define SCALAR_TYPE 0x4C2L
#define FLOAT_TYPE  0x3FL

#define IS_DATA_FILE_REG(x)  (((x) >= REG_R0 && (x) <= REG_R15))

#define IS_DATA_REG(x)  (((x).type == SCALAR_TYPE) && (IS_DATA_FILE_REG((x).reg)))
#define IS_FLOAT_REG(x) (((x).type == FLOAT_TYPE) && (IS_DATA_FILE_REG((x).reg)))
#define REG(x)          ((x).reg)

#define LEX_RETURN     strncat( lex_buff, yytext, BUFSIZ - strlen(lex_buff)); return

typedef union
{
   REGTYPE       regval;
   EXPR          expr;
   char          *sval;
   long          lval;
   double        dval;
   SYMBOL        *symbol;
   unsigned char hex40[DM_WORD_SIZE];
} YYSTYPE;


extern YYSTYPE yylval;
extern int    init_processing;
extern int    file_init_processing;
extern char   last_identifier[];

#define R0    0
#define R1    1
#define R2    2
#define R3    3
#define R4    4
#define R5    5
#define R6    6
#define R7    7
#define R8    8
#define R9    9
#define R10   10
#define R11   11
#define R12   12
#define R13   13
#define R14   14
#define R15   15

#define F0    16
#define F1    17
#define F2    18
#define F3    19
#define F4    20
#define F5    21
#define F6    22
#define F7    23
#define F8    24
#define F9    25
#define F10   26
#define F11   27
#define F12   28
#define F13   29
#define F14   30
#define F15   31

#define I0    32
#define I1    33
#define I2    34
#define I3    35
#define I4    36
#define I5    37
#define I6    38
#define I7    39
#define I8    40
#define I9    41
#define I10   42
#define I11   43
#define I12   44
#define I13   45
#define I14   46
#define I15   47

#define M0    48
#define M1    49
#define M2    50
#define M3    51
#define M4    52
#define M5    53
#define M6    54
#define M7    55
#define M8    56
#define M9    57
#define M10   58
#define M11   59
#define M12   60
#define M13   61
#define M14   62
#define M15   63

#define L0    64
#define L1    65
#define L2    66
#define L3    67
#define L4    68
#define L5    69
#define L6    70
#define L7    71
#define L8    72
#define L9    73
#define L10   74
#define L11   75
#define L12   76
#define L13   77
#define L14   78
#define L15   79

#define B0    80
#define B1    81
#define B2    82
#define B3    83
#define B4    84
#define B5    85
#define B6    86
#define B7    87
#define B8    88
#define B9    89
#define B10   90
#define B11   91
#define B12   92
#define B13   93
#define B14   94
#define B15   95


#define FADDR    130
#define DADDR    131
/*#define PCSTK    140*/
#define PCSTKP   141
#define LADDR    142
#define CURLCNTR 143
#define PX       144
#define PX1      145
#define PX2      146
#define TPERIOD  147
#define TCOUNT   148

#define USTAT1   150
#define USTAT2   151
#define IRPTL    153
#define MODE2    154
#define MODE1    155
#define ASTAT    156
#define IMASK    157
#define STKY     158
#define IMASKP   159

#define DAI1     160
#define DAI2     161
#define DAM1     162
#define DAM2     163
#define DAL1     164
#define DAL2     165
#define DAB1     166
#define DAB2     167

#define PSA1S    170
#define PSA1E    171
#define PSA2S    172
#define PSA2E    173
#define PSA3S    174
#define PSA3E    175
#define PSA4S    176
#define PSA4E    177
#define EMUCTL   178
#define EMUCLK   179

#define PMWAIT   180
#define PMBANK1  181
#define PMADR    182
#define PMDAS    183
#define PMDAE    184
#define POVL0    185
#define POVL1    186

#define DMWAIT   187
#define DMBANK1  188
#define DMBANK2  189
#define DMBANK3  190
#define DMADR    191
#define DMA1S    192
#define DMA1E    193
#define DMA2S    194
#define DMA2E    195
#define DOVL     196

/* Instruction modifiers */

#define SSI      197
#define SUI      198
#define USI      199
#define UUI      200
#define SSF      201
#define SUF      202
#define USF      203
#define UUF      204
#define SSIR     205
#define SUIR     206
#define USIR     207
#define UUIR     208
#define SSFR     209
#define SUFR     210
#define USFR     211
#define UUFR     212
#define SI       213
#define UI       214
#define SF       215
#define UF       216
#define SR       217
#define UR       218

#define FTA      219
#define FTB      220
#define FTC      221
#define EMUCLK2  222
#define EMUN     223

#ifndef MSDOS
extern double atof( char *nptr );
#endif

extern long aotol( char *str );
extern long abtol( char *str );
extern long ahtol( char *str );
extern int  yylex( void );
extern int  yyparse( void );
extern int  check_id( void );

