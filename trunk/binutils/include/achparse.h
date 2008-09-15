/* @(#)achparse.h	2.12 9/30/96 */

/******************************************************/
/* basic size stuff */
#define COLUMN_HEIGHT_060      8192uL
#define COLUMN_HEIGHT_061      4096uL  /* 3.3 Release change */
#define COLUMN_HEIGHT_062      4096uL
#define MAX_PM_BLOCK_060       40960uL
#define MAX_PM_BLOCK_061       8192uL  /* 3.3 Release Change */
#define MAX_PM_BLOCK_062       20480uL

/* '060 (4 Meg part) defines */
#define BLOCK0_START_060       0x00020000uL  /* block0 */
#define BLOCK0_END_060         0x0002ffffuL
#define BLOCK1_START_060       0x00030000uL  /* block1 */
#define BLOCK1_END_060         0x0003ffffuL
#define BLOCK0_SHORT_START_060 0x00040000uL  /* block0 shorts */
#define BLOCK0_SHORT_END_060   0x0005ffffuL
#define BLOCK1_SHORT_START_060 0x00060000uL  /* block1 shorts */
#define BLOCK1_SHORT_END_060   0x0007ffffuL


/* '062 (2 Meg part) defines */
#define BLOCK0_START_062       0x00020000uL  /* block0 */
#define BLOCK0_END_062         0x00027fffuL
#define BLOCK1_START_062       0x00028000uL  /* block1 */
#define BLOCK1_END_062         0x0002ffffuL
#define ALIAS_START1_062       0x00030000uL  /* block1 alias1 */
#define ALIAS_END1_062         0x00037fffuL
#define ALIAS_START2_062       0x00038000uL  /* block1 alias2 */
#define ALIAS_END2_062         0x0003ffffuL
#define BLOCK0_SHORT_START_062 0x00040000uL  /* block0 short */
#define BLOCK0_SHORT_END_062   0x0004ffffuL
#define BLOCK1_SHORT_START_062 0x00050000uL  /* block1 short */
#define BLOCK1_SHORT_END_062   0x0005ffffuL
#define ALIAS_SHORT_START1_062 0x00060000uL  /* block1 short alias1 */
#define ALIAS_SHORT_END1_062   0x0006ffffuL
#define ALIAS_SHORT_START2_062 0x00070000uL  /* block1 short alias2 */
#define ALIAS_SHORT_END2_062   0x0007ffffuL


/* '061 (1 Meg part) defines */
#define BLOCK0_START_061       0x00020000uL  /* block0 */
#define BLOCK0_END_061         0x00023fffuL
#define BLOCK1_START_061       0x00024000uL  /* block1 */
#define BLOCK1_END_061         0x00027fffuL
#define ALIAS_START1_061       0x00028000uL  /* block1 alias1 */
#define ALIAS_END1_061         0x0002bfffuL
#define ALIAS_START2_061       0x0002c000uL  /* block1 alias2 */
#define ALIAS_END2_061         0x0002ffffuL
#define ALIAS_START3_061       0x00030000uL  /* block1 alias3 */
#define ALIAS_END3_061         0x00033fffuL
#define ALIAS_START4_061       0x00034000uL  /* block1 alias4 */
#define ALIAS_END4_061         0x00037fffuL
#define ALIAS_START5_061       0x00038000uL  /* block1 alias5 */
#define ALIAS_END5_061         0x0003bfffuL
#define ALIAS_START6_061       0x0003c000uL  /* block1 alias6 */
#define ALIAS_END6_061         0x0003ffffuL
#define BLOCK0_SHORT_START_061 0x00040000uL  /* block0 short */
#define BLOCK0_SHORT_END_061   0x00047fffuL
#define BLOCK1_SHORT_START_061 0x00048000uL  /* block1 short */
#define BLOCK1_SHORT_END_061   0x0004ffffuL
#define ALIAS_SHORT_START1_061 0x00050000uL  /* block1 short alias1 */
#define ALIAS_SHORT_END1_061   0x00057fffuL
#define ALIAS_SHORT_START2_061 0x00058000uL  /* block1 short alias2 */
#define ALIAS_SHORT_END2_061   0x0005ffffuL
#define ALIAS_SHORT_START3_061 0x00060000uL  /* block1 short alias3 */
#define ALIAS_SHORT_END3_061   0x00067fffuL
#define ALIAS_SHORT_START4_061 0x00068000uL  /* block1 short alias4 */
#define ALIAS_SHORT_END4_061   0x0006ffffuL
#define ALIAS_SHORT_START5_061 0x00070000uL  /* block1 short alias5 */
#define ALIAS_SHORT_END5_061   0x00077fffuL
#define ALIAS_SHORT_START6_061 0x00078000uL  /* block1 short alias6 */
#define ALIAS_SHORT_END6_061   0x0007ffffuL

#define MULTISPACE_ID1_START   0x00080000uL  /* multiprocessor space */
#define MULTISPACE_ID1_END     0x000FFFFFuL
#define MULTISPACE_ID2_START   0x00100000uL
#define MULTISPACE_ID2_END     0x0017FFFFuL
#define MULTISPACE_ID3_START   0x00180000uL
#define MULTISPACE_ID3_END     0x001FFFFFuL
#define MULTISPACE_ID4_START   0x00200000uL
#define MULTISPACE_ID4_END     0x0027FFFFuL
#define MULTISPACE_ID5_START   0x00280000uL
#define MULTISPACE_ID5_END     0x002FFFFFuL
#define MULTISPACE_ID6_START   0x00300000uL
#define MULTISPACE_ID6_END     0x0037FFFFuL
#define MULTISPACE_ALL_START   0x00380000uL
#define MULTISPACE_ALL_END     0x003FFFFFuL

#define EXTERNAL_MEMORY_START  0x00400000uL  /* external memory start */
#define EXT_PM_END             0x00ffffffuL  /* external pm end */

/* These token values are returned by the parser:     */

typedef enum {
  ACH_SYSTEM,
  ACH_PROCESSOR,
  ACH_SEGMENT,
  ACH_OVERLAY,
  ACH_BANK,
  ACH_REGISTER,
  ACH_COMPILER,
  ACH_ENDSYS,
  ACH_ADSP21010,
  ACH_ADSP21020,
  ACH_ADSP210Z3,
  ACH_ADSP21061,
  ACH_ADSP210Z4,
  ACH_BEGIN,
  ACH_END,
  ACH_CDEFAULT,
  ACH_CSTRINGS,
  ACH_CGLOBALS,
  ACH_CSWITCH,
  ACH_CSTACK,
  ACH_CHEAP,
  ACH_CDATA,
  ACH_CCODE,
  ACH_CINIT,
  ACH_UNINIT,
  ACH_PM,
  ACH_DM,
  ACH_RAM,
  ACH_ROM,
  ACH_PORT,
  ACH_RESERVED_REG,
  ACH_SCRATCH_REG,
  ACH_CIRC_REG,
  ACH_PM0,
  ACH_PM1,
  ACH_DM0,
  ACH_DM1,
  ACH_DM2,
  ACH_DM3,
  ACH_DMU,
  ACH_PGSIZE,
  ACH_WTSTATES,
  ACH_DRAM,
  ACH_SRAM,
  ACH_WTMODE,
  ACH_PGWAIT,
  ACH_PGMODE,
  ACH_NORMAL,
  ACH_PGFAULT,
  ACH_PARITY,
  ACH_DMAPARITY,
  ACH_INTERNAL,
  ACH_EXTERNAL,
  ACH_EITHER,
  ACH_BOTH,
  ACH_PCRTS,
  ACH_RTRTS,
  ACH_STORE,
  ACH_SWAP,
  ACH_PPY,
  ACH_PPN,
  ACH_DAF,
  ACH_DAD,
  ACH_SREV_X,
  ACH_WIDTH,
  ACH_PROCESS_ID,
  ACH_EMOVERLAY,
  ACH_LONG48,		/*directive for 48_bit words in Z3*/
  ACH_BADTOKEN,
} ACH_TOKEN;
extern ACH_TOKEN Processor_type;

/******************************************************/
/* These modules are supplied by the parser:          */ 

int ach_parse      (char *filename);
int ach_find_stack (char *filename);

/******************************************************/


/******************************************************/
/* These modules must be supplied by the application: */

void ach_system    (char *label);
void ach_processor (int processor_token);
void ach_segment   (unsigned long begin, unsigned long end,
                    int type_token, int access_token, char *symbol, unsigned short have_uninit,unsigned long width);
void ach_bank      (unsigned long begin, unsigned long pgsize,
                    unsigned long wtstates, unsigned short type,
                    unsigned short wtmode, unsigned short which_bank, 
		    unsigned short pgwten);
void ach_overlay(char *store_seg, char *swap_seg, char *label);
void ach_scratch_register(char *reg);
void ach_reserved_register(char *reg);
void ach_circular_register(char *dag_register, char *variable);
void ach_cheap (unsigned long length, int type, int access, char *label);
void ach_cstack (unsigned long length, int type, int access, char *label);
void ach_cdefaults (ACH_TOKEN which_def, int type, char *label);
void ach_compiler (short map_double_to_float, short jjb, short param_passing, short chip_rev);
void ach_endsys    (void);
void ach_error     (char *msg);

/******************************************************/




