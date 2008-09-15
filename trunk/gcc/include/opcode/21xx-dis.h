#ifndef _A21XX_DIS_
#define _A21XX_DIS_



/* These are the special flags that control disassmbly.
   info->flags |= value.   */
#define ADDRESS_IS_BYTE_OFFSET        0x10
#define CONSTANTS_IN_HEX_FORMAT       0x20
#define IGNORE_NEWLINES               0x40
#define IREGISTER_LOADS_ARE_ADDRESSES 0x80
#define USE_C0_REGISTER_FILE          0x80000000L
#define BUFFER_IS_LONGVECTOR          0x40000000L
#define BUFFER_IS_LITTLE_ENDIAN       0x20000000L

#define target_x1(info) (((info)->flags & 0xf) == 7)
#define target_c0(info) (((info)->flags & 0x3) == 3)
#define target_mach(info) ((info)->flags & 0xf)

#define TARGETS {\
 { "2100", 0},\
 { "2171", 2},\
 { "21xx", 1},\
 { "c0",   3},\
 { "x1",   7},\
 { "b3",   2},\
 { "21",   1},\
 { 0,0}}

struct decode_entry {
  long code, mask;
  int size;
  int type;
  char *decoder;
};

struct decode_logic {
  struct decode_entry *table;
  char **alufs;
  char **sffs;
  char **bert_alufs;
  char **c0_alufs;
  char **conds;
  char **terms;
  char * (* reguse_hook) (char *, int);
};

#define DECODE_LOGIC(info) ((struct decode_logic *)(info)->private_data)

#ifndef DLLINTERFACE
#define DLLINTERFACE
#endif

extern char * (* log_reguse_hook) (char *, int);
extern DLLINTERFACE print_insn_a21xx (bfd_vma memaddr, disassemble_info *info);
extern DLLINTERFACE print_insn_a21xx_1 (bfd_vma memaddr, long instruction, 
         disassemble_info *info);
int DLLINTERFACE print_insn_21 (bfd_vma memaddr, disassemble_info *info);
short DLLINTERFACE bertconst (long instruction, char kind);

extern struct decode_logic decode_c0, decode_21xx;

#endif

