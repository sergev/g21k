/* @(#)assemble.h	2.2 10/3/96 2 */


#define COMPUTE_TOP7(x)     (((x) & 0x007f0000L) >> 16L)
#define COMPUTE_MIDDLE8(x)  (((x) & 0x0000ff00L) >> 8L)
#define COMPUTE_BOTTOM8(x)  ((x) & 0x000000ffL)

#define PM_ADDRESS(x)          ((x) & 0xFFFFFFL)
#define PM_ADDRESS_TOP8(x)     (((x) & 0xFF0000L) >> 16L)
#define PM_ADDRESS_MIDDLE8(x)  (((x) & 0x00FF00L) >> 8L)
#define PM_ADDRESS_BOTTOM8(x)  ((x) & 0xFFL)

extern void assemble_instruction( ILNODE *instr );
extern long format_compute( ILNODE *instr );
extern void instr_type1( ILNODE *instr );
extern void instr_type2( ILNODE *instr );
extern void instr_type3( ILNODE *instr );
extern void instr_type4( ILNODE *instr );
extern void instr_type5( ILNODE *instr );
extern void instr_type6( ILNODE *instr );
extern void instr_type7( ILNODE *instr );
extern void instr_type8( ILNODE *instr );
extern void instr_type9( ILNODE *instr );
extern void instr_type10( ILNODE *instr );
extern void instr_type11( ILNODE *instr );
extern void instr_type12( ILNODE *instr );
extern void instr_type13( ILNODE *instr );
extern void instr_type14( ILNODE *instr );
extern void instr_type15( ILNODE *instr );
extern void instr_type16( ILNODE *instr );
extern void instr_type17( ILNODE *instr );
extern void instr_type18( ILNODE *instr );
extern void instr_type19( ILNODE *instr );
extern void instr_type20( ILNODE *instr );
extern void instr_type21( ILNODE *instr );
extern void instr_type22( ILNODE *instr );
extern void instr_type23( ILNODE *instr );
extern void instr_type24( ILNODE *instr );
extern void instr_type25( ILNODE *instr );
extern void instr_type25a( ILNODE *instr );
extern void instr_type27( ILNODE *instr );
extern void instr_type28( ILNODE *instr );
extern void instr_type29( ILNODE *instr );
extern int last_minute_error_checking( register ILNODE *instr );
