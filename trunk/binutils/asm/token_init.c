/* @(#)token_init.c	1.7 2/21/91 1 */

#include "app.h"

#ifdef TRUE
#undef TRUE
#endif

#include "symbol.h"
#include "expr.h"
#include "lexical.h"
#include "y_tab.h"
#include "token.h"
#include "token_init.h"

static struct dope
{
    char *name;
    short token;
    short value;
} dope [] =
{
    "ENDSEG",        ENDSEG,        0,
    "EXTERN",        EXTERN,        0,
    "GLOBAL",        GLOBAL,        0,
    "GCC_COMPILED",  GCC_COMPILED,  0,
    "NEWPAGE",       NEWPAGE,       0,
    "PORT",          PORT,          0,
    "PRECISION",     PRECISION,     0,
    "ROUND_NEAREST", ROUND_NEAREST, 0,
    "ROUND_ZERO",    ROUND_ZERO,    0,
    "ROUND_MINUS",   ROUND_MINUS,   0,
    "ROUND_PLUS",    ROUND_PLUS,    0,
    "SEGMENT",       SEGMENT,       0,
    "VAR",           VAR,           0,

    "BB",	     BB,	    0,
    "BF",	     BF, 	    0,
    "DEF",           DEF,           0,
    "EB",	     EB,	    0,
    "EF",	     EF,	    0,
    "ENDEF", 	     ENDEF, 	    0,
    "FILE",	     FILEDEF, 	    0,
    "LINE",	     LINE,	    0,
    "LN",	     LN,	    0,
    "SCL",           SCL,           0,
    "TYPE",	     TYPE,	    0,
    "VAL",	     VAL, 	    0,
    "SIZE",	     SIZE,	    0,
    "DIM",           DIM,           0,
    "TAG",           TAG,           0,
    "EOS",           EOS,           0,

    "ABS",       ABS,         0,
    "AC",        AC,          0,
    "ACT",       ACT,         0,
    "AND",       AND,         0,
    "ASHIFT",    ASHIFT,      0,
    "AV",        AV,          0,
    "BCLR",      BCLR,        0,
    "BIT",       BIT,         0,
    "BITREV",    BITREV,      0,
    "BM",        BM,          0,
    "BSET",      BSET,        0,
    "BTGL",      BTGL,        0,
    "BTST",      BTST,        0,
    "BY",        BY,          0,
    "CA",        CA,          CA,
    "CACHE",     CACHE,       CACHE,
    "CALL",      CALL,        0,
    "CH",        CH,          CH,
    "CI",        CARRY,       0,
    "CJUMP",     CJUMP,       0,
    "CL",        CL,          0,
    "CLIP",      CLIP,        0,
    "CLR",       CLR,         0,
    "COMP",      COMP,        0,
    "COPYSIGN",  COPYSIGN,    0,
    "COS",       COS,         0,
    "DB",        DB,          DB,
    "DEC",       DEC,         0,
    "DM",        DM,          0,
    "DO",        DO,          0,
    "ECE",       ECE,         0,
    "ELSE",      ELSE,        0,
    "EMUIDLE",   EMUIDLE,     0,
    "EQ",        EQ,          0,
    "EX",        EX,          EX,
    "EXP",       EXP,         0,
    "EXP2",      EXP2,        0,
    "FDEP",      FDEP,        0,
    "FEXT",      FEXT,        0,
    "FIX",       FIX,         0,
    "FLAG0_IN",  FLAG0_IN,    0,
    "FLAG1_IN",  FLAG1_IN,    0,
    "FLAG2_IN",  FLAG2_IN,    0,
    "FLAG3_IN",  FLAG3_IN,    0,
    "FLOAT",     FLOAT,       0,
    "FLUSH",     FLUSH,       0,
    "FMERG",     FMERG,       0,
    "FOREVER",   FOREVER,     0,
    "FPACK",     FPACK,       0,
    "FRACTIONAL",FRACTIONAL,  0,
    "FUNPACK",   FUNPACK,     0,
    "GE",        GE,          0,
    "GT",        GT,          0,
    "IDLE",      IDLE,        0,
    "IDLE16",    IDLE16,      0,
    "IF",        IF,          0,
    "INC",       INC,         0,
    "JUMP",      JUMP,        0,
    "LA",        LA,          LA,
    "LCE",       LCE,         0,
    "LE",        LE,          0,
    "LEFTO",     LEFTO,       0,
    "LEFTZ",     LEFTZ,       0,
    "LOAD",      LOAD,        LOAD,
    "LOG2",      LOG2,        0,
    "LOGB",      LOGB,        0,
    "LSHIFT",    LSHIFT,      0,
    "LT",        LT,          0,
    "MANT",      MANT,        0,
    "MAX",       MAX,         0,
    "MIN",       MIN,         0,
    "MOD",       MOD,         0,
    "MODIFY",    MODIFY,      0,
    "MS",        MS,          0,
    "MV",        MV,          0,
    "LR",        CARRY,       0,
    "SSI",       MULTIPLIER_MODIFIER,     SSI,
    "SUI",       MULTIPLIER_MODIFIER,     SUI,
    "USI",       MULTIPLIER_MODIFIER,     USI,
    "UUI",       MULTIPLIER_MODIFIER,     UUI,
    "SSF",       MULTIPLIER_MODIFIER,     SSF,
    "SUF",       MULTIPLIER_MODIFIER,     SUF,
    "USF",       MULTIPLIER_MODIFIER,     USF,
    "UUF",       MULTIPLIER_MODIFIER,     UUF,
    "SSIR",      MULTIPLIER_MODIFIER,     SSIR,
    "SUIR",      MULTIPLIER_MODIFIER,     SUIR,
    "USIR",      MULTIPLIER_MODIFIER,     USIR,
    "UUIR",      MULTIPLIER_MODIFIER,     UUIR,
    "SSFR",      MULTIPLIER_MODIFIER,     SSFR,
    "SUFR",      MULTIPLIER_MODIFIER,     SUFR,
    "USFR",      MULTIPLIER_MODIFIER,     USFR,
    "UUFR",      MULTIPLIER_MODIFIER,     UUFR,
    "SI",        MULTIPLIER_MODIFIER,     SI,
    "UI",        MULTIPLIER_MODIFIER,     UI,
    "SF",        MULTIPLIER_MODIFIER,     SF,
    "UF",        MULTIPLIER_MODIFIER,     UF,
    "SR",        MULTIPLIER_MODIFIER,     SR,
    "UR",        MULTIPLIER_MODIFIER,     UR,
    "NBM",       NBM,                     0,
    "NE",        NE,                      0,
    "NOFZ",      NOFZ,                    0,
    "NOFO",      NOFO,                    0,
    "NOP",       NOP,                     0,

    "NOPSPECIAL", NOPSPECIAL,             0,

    "NOT",       NOT,                     0,
    "NU",        NU,                      NU,
    "OR",        OR,                      0,
    "P20",       P20,                     P20,
    "P24",       P24,                     P24,
    "P32",       P32,                     P32,
    "P40",       P40,                     P40,
    "PACK",      PACK,                    0,
    "PASS",      PASS,                    0,
    "PM",        PM,                      0,
    "POP",       POP,                     0,
    "PUSH",      PUSH,                    0,
    "READ",      READ,                    READ,
    "RECIPS",    RECIPS,                  0,
    "RFRAME",    RFRAME,                  0,
    "RND",       RND,                     0,
    "ROT",       ROT,                     0,
    "RS",        RS,                      RS,
    "RSQRTS",    RSQRTS,                  0,
    "RTI",       RTI,                     0,
    "RTS",       RTS,                     0,
    "SAT",       SAT,                     0,
    "SCALB",     SCALB,                   0,
    "SE",        SE,                      SE,
    "SET",       SET,                     0,
    "SIN",       SIN,                     0,
    "SQR",       SQR,                     0,
    "ST",        ST,                      ST,
    "STEP",      STEP,                    0,
    "SV",        SV,                      0,
    "SZ",        SZ,                      0,
    "TF",        TF,                      0,
    "TGL",       TGL,                     0,
/*    "TRAP",      TRAP,                    0,  */
    "TRUE",      TRUE,                    0,
    "TRUNC",     TRUNC,                   0,
    "TST",       TST,                     0,
    "UNPACK",    UNPACK,                  0,
    "UNTIL",     UNTIL,                   0,
    "WITH",      WITH,                    0,
    "XOR",       XOR,                     0,

    "MRB",       MRB,                     MRB,
    "MRF",       MRF,                     MRF,
    "MR0F",      MR0F,                    MR0F,
    "MR1F",      MR1F,                    MR1F,
    "MR2F",      MR2F,                    MR2F,
    "MR0B",      MR0B,                    MR0B,
    "MR1B",      MR1B,                    MR1B,
    "MR2B",      MR2B,                    MR2B,
    "SHP",       SHP,                     SHP,
    "SHF",       SHF,                     SHP,
    "FR",        FR,                      FR,

    "R0",        DATA_REGISTER,           R0,
    "R1",        DATA_REGISTER,           R1,
    "R2",        DATA_REGISTER,           R2,
    "R3",        DATA_REGISTER,           R3,
    "R4",        DATA_REGISTER,           R4,
    "R5",        DATA_REGISTER,           R5,
    "R6",        DATA_REGISTER,           R6,
    "R7",        DATA_REGISTER,           R7,
    "R8",        DATA_REGISTER,           R8,
    "R9",        DATA_REGISTER,           R9,
    "R10",       DATA_REGISTER,           R10,
    "R11",       DATA_REGISTER,           R11,
    "R12",       DATA_REGISTER,           R12,
    "R13",       DATA_REGISTER,           R13,
    "R14",       DATA_REGISTER,           R14,
    "R15",       DATA_REGISTER,           R15,

    "F0",        FLOATING_REGISTER,       F0,
    "F1",        FLOATING_REGISTER,       F1,
    "F2",        FLOATING_REGISTER,       F2,
    "F3",        FLOATING_REGISTER,       F3,
    "F4",        FLOATING_REGISTER,       F4,
    "F5",        FLOATING_REGISTER,       F5,
    "F6",        FLOATING_REGISTER,       F6,
    "F7",        FLOATING_REGISTER,       F7,
    "F8",        FLOATING_REGISTER,       F8,
    "F9",        FLOATING_REGISTER,       F9,
    "F10",       FLOATING_REGISTER,       F10,
    "F11",       FLOATING_REGISTER,       F11,
    "F12",       FLOATING_REGISTER,       F12,
    "F13",       FLOATING_REGISTER,       F13,
    "F14",       FLOATING_REGISTER,       F14,
    "F15",       FLOATING_REGISTER,       F15,

    "I0",        INDEX_REGISTER,          I0,
    "I1",        INDEX_REGISTER,          I1,
    "I2",        INDEX_REGISTER,          I2,
    "I3",        INDEX_REGISTER,          I3,
    "I4",        INDEX_REGISTER,          I4,
    "I5",        INDEX_REGISTER,          I5,
    "I6",        INDEX_REGISTER,          I6,
    "I7",        INDEX_REGISTER,          I7,
    "I8",        INDEX_REGISTER,          I8,
    "I9",        INDEX_REGISTER,          I9,
    "I10",       INDEX_REGISTER,          I10,
    "I11",       INDEX_REGISTER,          I11,
    "I12",       INDEX_REGISTER,          I12,
    "I13",       INDEX_REGISTER,          I13,
    "I14",       INDEX_REGISTER,          I14,
    "I15",       INDEX_REGISTER,          I15,

    "M0",        MODIFY_REGISTER,         M0,
    "M1",        MODIFY_REGISTER,         M1,
    "M2",        MODIFY_REGISTER,         M2,
    "M3",        MODIFY_REGISTER,         M3,
    "M4",        MODIFY_REGISTER,         M4,
    "M5",        MODIFY_REGISTER,         M5,
    "M6",        MODIFY_REGISTER,         M6,
    "M7",        MODIFY_REGISTER,         M7,
    "M8",        MODIFY_REGISTER,         M8,
    "M9",        MODIFY_REGISTER,         M9,
    "M10",       MODIFY_REGISTER,         M10,
    "M11",       MODIFY_REGISTER,         M11,
    "M12",       MODIFY_REGISTER,         M12,
    "M13",       MODIFY_REGISTER,         M13,
    "M14",       MODIFY_REGISTER,         M14,
    "M15",       MODIFY_REGISTER,         M15,

    "L0",        LENGTH_REGISTER,         L0,
    "L1",        LENGTH_REGISTER,         L1,
    "L2",        LENGTH_REGISTER,         L2,
    "L3",        LENGTH_REGISTER,         L3,
    "L4",        LENGTH_REGISTER,         L4,
    "L5",        LENGTH_REGISTER,         L5,
    "L6",        LENGTH_REGISTER,         L6,
    "L7",        LENGTH_REGISTER,         L7,
    "L8",        LENGTH_REGISTER,         L8,
    "L9",        LENGTH_REGISTER,         L9,
    "L10",       LENGTH_REGISTER,         L10,
    "L11",       LENGTH_REGISTER,         L11,
    "L12",       LENGTH_REGISTER,         L12,
    "L13",       LENGTH_REGISTER,         L13,
    "L14",       LENGTH_REGISTER,         L14,
    "L15",       LENGTH_REGISTER,         L15,

    "B0",        BASE_REGISTER,           B0,
    "B1",        BASE_REGISTER,           B1,
    "B2",        BASE_REGISTER,           B2,
    "B3",        BASE_REGISTER,           B3,
    "B4",        BASE_REGISTER,           B4,
    "B5",        BASE_REGISTER,           B5,
    "B6",        BASE_REGISTER,           B6,
    "B7",        BASE_REGISTER,           B7,
    "B8",        BASE_REGISTER,           B8,
    "B9",        BASE_REGISTER,           B9,
    "B10",       BASE_REGISTER,           B10,
    "B11",       BASE_REGISTER,           B11,
    "B12",       BASE_REGISTER,           B12,
    "B13",       BASE_REGISTER,           B13,
    "B14",       BASE_REGISTER,           B14,
    "B15",       BASE_REGISTER,           B15,
   
    "LOOP",      LOOP,                     LOOP,
    "STS",       STS,                      STS,
    "PC",        PC,                       PC,
    "FADDR",     MISC_REGISTER,            FADDR,
    "DADDR",     MISC_REGISTER,            DADDR,
    "PCSTK",     MISC_REGISTER,            PCSTK,
    "PCSTKP",    MISC_REGISTER,            PCSTKP,
    "LADDR",     MISC_REGISTER,            LADDR,
    "LCNTR",     LCNTR,                    LCNTR,
    "CURLCNTR",  MISC_REGISTER,            CURLCNTR,
    "PX",        MISC_REGISTER,            PX,
    "PX1",       MISC_REGISTER,            PX1,
    "PX2",       MISC_REGISTER,            PX2,
    "TPERIOD",   MISC_REGISTER,            TPERIOD,
    "TCOUNT",    MISC_REGISTER,            TCOUNT,

    "USTAT1",    SYSTEM_REGISTER,          USTAT1,
    "USTAT2",    SYSTEM_REGISTER,          USTAT2,
    "IRPTL",     SYSTEM_REGISTER,          IRPTL,
    "MODE2",     SYSTEM_REGISTER,          MODE2,
    "MODE1",     SYSTEM_REGISTER,          MODE1,
    "ASTAT",     SYSTEM_REGISTER,          ASTAT,
    "IMASK",     SYSTEM_REGISTER,          IMASK,
    "STKY",      SYSTEM_REGISTER,          STKY,
    "IMASKP",    SYSTEM_REGISTER,          IMASKP,
    
    "DAI1",      MISC_REGISTER,            DAI1,
    "DAI2",      MISC_REGISTER,            DAI2,
    "DAL1",      MISC_REGISTER,            DAL1,
    "DAL2",      MISC_REGISTER,            DAL2,
    "DAB1",      MISC_REGISTER,            DAB1,
    "DAB2",      MISC_REGISTER,            DAB2,
    "PSA1S",     MISC_REGISTER,            PSA1S,
    "PSA1E",     MISC_REGISTER,            PSA1E,
    "PSA2S",     MISC_REGISTER,            PSA2S,
    "PSA2E",     MISC_REGISTER,            PSA2E,
    "PSA3S",     MISC_REGISTER,            PSA3S,
    "PSA3E",     MISC_REGISTER,            PSA3E,
    "PSA4S",     MISC_REGISTER,            PSA4S,
    "PSA4E",     MISC_REGISTER,            PSA4E,
    "EMUCLK",    MISC_REGISTER,            EMUCLK,
    "EMUCLK2",   MISC_REGISTER,            EMUCLK2,
    "EMUN",      MISC_REGISTER,            EMUN,

    "PMWAIT",    MISC_REGISTER,            PMWAIT,
    "PMBANK1",   MISC_REGISTER,            PMBANK1,
    "PMADR",     MISC_REGISTER,            PMADR,
    "PMDAS",     MISC_REGISTER,            PMDAS,
    "PMDAE",     MISC_REGISTER,            PMDAE,
    "POVL0",     MISC_REGISTER,            POVL0,
    "POVL1",     MISC_REGISTER,            POVL1,

    "DMWAIT",    MISC_REGISTER,            DMWAIT,
    "DMBANK1",   MISC_REGISTER,            DMBANK1,
    "DMBANK2",   MISC_REGISTER,            DMBANK2,
    "DMBANK3",   MISC_REGISTER,            DMBANK3,
    "DMADR",     MISC_REGISTER,            DMADR,
    "DMA1S",     MISC_REGISTER,            DMA1S,
    "DMA1E",     MISC_REGISTER,            DMA1E,
    "DMA2S",     MISC_REGISTER,            DMA2S,
    "DMA2E",     MISC_REGISTER,            DMA2E,
    "DOVL",      MISC_REGISTER,            DOVL,

    "FTA",       TEST_REGISTER,            FTA,
    "FTB",       TEST_REGISTER,            FTB,
    "FTC",       TEST_REGISTER,            FTC,
    "",          0,                        0
};  


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      token_init                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void token_init(void)                                           *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for initializing the token hash     *
*   table.                                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/16/89       created                  -----             *
***********************************************************************/

void token_init( void )
{
   register short i;

   i = 0;
   while ( *dope[i].name )
   {
           token_insert(dope[i].name, dope[i].token, dope[i].value );
           ++i;
   }
}