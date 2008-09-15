/* Definitions of target machine for GNU compiler.

			    Analog Devices
			      ADSP 21020


   Copyright (C) 1990 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef ADSP21K_H
#define ADSP21K_H
/* static char* __ident_tm_adsp21k_h__ = "@(#) tm.adsp21k.h 2.3@(#)"; */
#ifndef ACH_PARSE
/*
  ACH_PARSE is wrapped around everything except the TARGET_SWITCHES
  and TARGET_OPTIONS tables.  This is so the architecture file parser
  for the driver doesn't import the additional baggage in this file.

  MDA - 920716
*/
#include <stdio.h>
#include <math.h>

/* Note that some other tm- files include this one and then override
   many of the definitions that relate to assembler syntax.  */

#define dbgp(x) printf x

#define Z3_CAN_FILL_DELAY_BRANCH_SLOTS 0

#define TRUNC_060_BUG 0 /* ??? doesn't seem to be any anomolies with this. why? */

#ifdef MSCVER
#define __USE_STDARGS__
#endif
#define DSP21K
#define ADI
#define USE_CPPCOMMENTS 1

#define QImode SImode
#define MEMSEG
#define MAX_DOLOOP_DEPTH 6

#include "machmode.h"

#ifdef REG_OK_STRICT
# define STRICTNESS 1
#else  /* REG_OK_STRICT */
# define STRICTNESS 0
#endif /* REG_OK_STRICT */


/* This improves code significantly. */

#define DEFAULT_CALLER_SAVES

#define SWITCH_SUBREG(X) {while(GET_CODE(X)==SUBREG)X=SUBREG_REG(X);}

#define SUBREGNO(X)(GET_CODE(X)==SUBREG?REGNO(XEXP(X,0)):REGNO(X))

/* Addressing modes, and classification of registers for them.  */
/* The class value for index registers, and the one for base regs.  */
/* The class value for index registers, and the one for base regs.  */

/* With memory segments (pm/dm), the base class dependes on the mode: */
/* DM_MREGS for DMmode and PM_MREGS for PMmode, same for index class  **/

/*(shell-command "rm caller-save.o regclass.o reload.o reload1.o") */
#define INDEX_REG_CLASS(MODE) \
    (((MODE)==DMmode)?DM_MREGS:((MODE)==PMmode)?PM_MREGS:NO_REGS)

#define BASE_REG_CLASS(MODE) \
    (((MODE)==DMmode)?DM_IREGS:((MODE)==PMmode)?PM_IREGS:NO_REGS)

#define BASE_REG_CLASS1(mode,dummy)  BASE_REG_CLASS(mode)
#define INDEX_REG_CLASS1(mode,dummy) INDEX_REG_CLASS(mode)

/** In some rare cases we might still need class for all possible **/
/** base and index registers **/

#define ALL_BASE_REG_CLASS  IREGS_REGS
#define ALL_INDEX_REG_CLASS MREGS_REGS


/*   A number, the maximum number of registers that can appear in a
     valid memory address.  Note that it is up to you to specify a
     value equal to the maximum number that `GO_IF_LEGITIMATE_ADDRESS'
     would ever accept. */
#define MAX_REGS_PER_ADDRESS 2

/* Macros to check register numbers against specific register classes.  */

/* These assume that REGNO is a hard or pseudo reg number.
   They give nonzero only if REGNO is a hard reg of the suitable class
   or a pseudo reg currently allocated to a suitable hard reg.
   Since they use reg_renumber, they are safe only once reg_renumber
   has been allocated, which happens in local-alloc.c.  */

/* Recognize any constant value that is a valid address.  */
#define CONSTANT_ADDRESS_P(X) CONSTANT_P (X)


/* Nonzero if the constant value X is a legitimate general operand.
   It is given that X satisfies CONSTANT_P or is a CONST_DOUBLE.  */

#define LEGITIMATE_CONSTANT_P(X) 1
#define CONST_INT_P(X) (GET_CODE(X)==CONST_INT)


#define CONSTANT_6BIT_P(X) \
    (CONST_INT_P(X) && INTVAL(X) <= 31  && INTVAL(X) >= -32)

#define POST_MODIFY_P(X)\
  (GET_CODE (X) == POST_INC\
   || GET_CODE (X) == POST_DEC\
   || GET_CODE (X) == POST_MODIFY)

/* GO_IF_LEGITIMATE_ADDRESS recognizes an RTL expression
   that is a valid memory address for an instruction.
   The MODE argument is the machine mode for the MEM expression
   that wants to use this address.

        any constant including a symbol_ref
	ireg
        6bit + ireg
        mreg + ireg
	(post_inc ireg)
	(post_dec ireg)

(doc "Addressing Modes" "GO_IF_LEGITIMATE_ADDRESS (MODE, X, LABEL)")
(shell-command "rm explow.o recog.o reload.o") */

#define GO_IF_LEGITIMATE_ADDRESS(MODE, X, WIN) \
    if (go_if_legitimate_address(MODE,X,STRICTNESS)) goto WIN

/* Try machine-dependent ways of modifying an illegitimate address
   to be legitimate.  If we find one, return the new, valid address.
   This macro is used in only one place: `memory_address' in explow.c.

   OLDX is the address as it was before break_out_memory_refs was called.
   In some cases it is useful to look at this to decide what needs to be done.

   MODE and WIN are passed so that this macro can use
   GO_IF_LEGITIMATE_ADDRESS.

   It is always safe for this macro to do nothing.  It exists to recognize
   opportunities to optimize the output.

   For the 21k, do nothing for now. */
/*(shell-command "rm explow.o" nil)*/
#define LEGITIMIZE_ADDRESS(X,OLDX,MODE,WIN) {\
   extern rtx legitimize_address(); rtx _q;\
   if((_q = legitimize_address(X,OLDX,MODE))) { X = _q; goto WIN;}}


/* Go to LABEL if ADDR (a legitimate address expression)
   has an effect that depends on the machine mode it is used for.
   On the 21k, only postdecrement and postincrement address depend thus
   (the amount of decrement or increment being the length of the operand).  */

#define HAVE_POST_INCREMENT
#define HAVE_POST_DECREMENT
#define HAVE_POST_MODIFY

#define GO_IF_MODE_DEPENDENT_ADDRESS(ADDR,LABEL)  \
 if (GET_CODE (ADDR) == POST_INC || GET_CODE (ADDR) == POST_DEC) goto LABEL


/* Control the assembler format that we output.  */

/* Output at beginning of assembler file.  */
#define ASM_FILE_START(file) do { 						\
  output_options (file, f_options, sizeof (f_options) / sizeof (f_options[0]),	\
		        W_options, sizeof (W_options) / sizeof (W_options[0]));	\
  asm_file_start(file); } while(0)

#define ASM_FILE_END(file) asm_file_end(file)


#define ASM_IDENTIFY_GCC(FILE) \
  fprintf (FILE, ".gcc_compiled;\n");
/* Output to assembler file text saying following lines
   may contain character constants, extra white space, comments, etc.  */

#define ASM_APP_ON ""

/* Output to assembler file text saying following lines
   no longer contain unusual constructs.  */

#define ASM_APP_OFF ""

/* Output before read-only data.  */

/*(shell-command "rm crtstuff.o varasm.o")*/
#define TEXT_SECTION_ASM_OP \
(temp_segment_name = alloca(strlen(pmcode_segment)+25),\
 sprintf(temp_segment_name, ".endseg;\n.segment /pm %s;\n", pmcode_segment),\
 temp_segment_name)

/* Output before writable data.  */

/*(shell-command "rm varasm.o")*/
#define DATA_SECTION_ASM_OP \
(temp_segment_name = alloca(strlen(dmdata_segment)+25),\
 sprintf(temp_segment_name, ".endseg;\n.segment /dm %s;\n", dmdata_segment),\
 temp_segment_name)


#define EXTRA_SECTIONS in_pmda_section

#define EXTRA_SECTION_FUNCTIONS						\
pmda_section()								\
{									\
 if ( in_section != in_pmda_section) {					\
  fprintf(asm_out_file, ".endseg;\n.segment /pm %s;\n", pmdata_segment);\
  in_section = in_pmda_section;					        \
 }								        \
}

#define READONLY_DATA_SECTION data_section

/* How to renumber registers for dbx and gdb. */

#define DBX_REGISTER_NUMBER(REGNO) (REGNO)

/* This is how to output the definition of a user-level label named NAME,
   such as the label on a static function or variable NAME.  */

#define ASM_OUTPUT_LABEL(FILE,NAME)    \
{assemble_name (FILE, NAME); fputs(":\n", FILE);}


/* This is how to output a command to make the user-level label named NAME
   defined for reference from other files.  */

#define ASM_GLOBALIZE_LABEL(FILE,NAME)	\
{fputs(".global\t", FILE); assemble_name(FILE, NAME); fputs(";\n", FILE);}

/* This is how to output a reference to a user-level label named NAME.
   `assemble_name' uses this.  */
#define ASM_OUTPUT_LABELREF(FILE,NAME)\
  fprintf (FILE, "_%s", NAME)

/* This is how to output an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.  */
#define ASM_OUTPUT_INTERNAL_LABEL(FILE,PREFIX,NUM)\
  fprintf(FILE, "_%s$%d:\n", PREFIX, NUM)

/* (shell-command "rm -f dbxout.o dwarfout.o final.o varasm.o") */
/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */
#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM)\
  sprintf (LABEL, "%s$%d", PREFIX, NUM)

/* This is how to output an assembler line defining a `double' constant.  */

#define ASM_OUTPUT_DOUBLE(FILE,VALUE) \
{ int _a[2];                          \
  ieee_double_hex (VALUE, &_a);       \
  fprintf (FILE, " 0x%x,0x%x", _a[0], _a[1]);}

#define ASM_OUTPUT_FLOAT(FILE,VALUE) \
{  union { float f; long l; } matt;\
   matt.f = (float)VALUE;\
   fprintf (FILE, " 0x%0.8x", matt.l); }

#define ASM_OUTPUT_INT(FILE,VALUE)     output_addr_const (FILE, VALUE)

#define ASM_OUTPUT_SHORT(FILE,VALUE)   output_addr_const (FILE, VALUE)

#define ASM_OUTPUT_CHAR(FILE,VALUE)    output_addr_const (FILE, VALUE)

#define ASM_OUTPUT_BYTE(FILE,VALUE)    fprintf(FILE, "0x%0.2x", VALUE)

#define ASM_OUTPUT_ASCII(FILE,PTR,LEN)                         \
{    int _i, _lcount = 0;                                      \
     for (_i = 0; _i < LEN; _i++)                              \
       {  if (_i + 1 < LEN) fprintf (FILE, "0x%02x,", PTR[_i]);\
          else fprintf (FILE, "0x%02x", PTR[_i]);              \
	  if ((++_lcount % 20) == 0)                           \
	    fprintf (FILE, "\n"); }}

/* This is how to output an insn to push a register on the stack.
   It need not be very fast code since it is used only for profiling  */
#define ASM_OUTPUT_REG_PUSH(FILE,REGNO) asm_output_reg_push(FILE,REGNO)

/* This is how to output an insn to pop a register from the stack.
   It need not be very fast code since it is used only for profiling  */
#define ASM_OUTPUT_REG_POP(FILE,REGNO) asm_output_reg_pop(FILE,REGNO)

/* This is how to output an element of a case-vector that is absolute. */
#define ASM_OUTPUT_ADDR_VEC_ELT(FILE, VALUE)  asm_output_addr_vec_elt(FILE,VALUE)


/* This is how to output an element of a case-vector that is relative.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, VALUE, REL) asm_output_addr_diff_elt(FILE, VALUE, REL)

/* This is a "prologue" for "switch" */

#define ASM_OUTPUT_CASE_LABEL(file,pr,num,tab) asm_output_case_label(file,pr,num,tab)

/* This is an "epilogue" for "switch" */
#define ASM_OUTPUT_CASE_END(FILE,NUM,TAB)\
  fprintf (FILE, ";\n")

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */
#define ASM_OUTPUT_ALIGN(FILE,LOG)\
{ if ((LOG) != 0) fprintf(FILE, "ERROR (align %d)\n", LOG);}



#define ASM_OUTPUT_SKIP(FILE,SIZE)\
{ int _i;                         \
  for (_i=0;_i<SIZE-1;_i++)       \
    fprintf (FILE, "0,");         \
  fprintf (FILE,"0");}


/* This says how to output an assembler line
   to define a global common symbol.  */
#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)     \
do { fputs(".global ", FILE);    assemble_name (FILE, NAME);\
  fputs("; ", FILE);					\
  ASM_OUTPUT_LOCAL (FILE, NAME, SIZE, ROUNDED); } while(0)


/* This says how to output an assembler line
   to define a local common symbol.  */
#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE, ROUNDED) \
{fputs(".var\t", FILE);  assemble_name (FILE, NAME);\
 if (ROUNDED > 1) fprintf (FILE, "[%d]", ROUNDED);   \
 fprintf (FILE, ";\n");}

/* Store in OUTPUT a string (made with alloca) containing
   an assembler-name for a local static variable named NAME.
   LABELNO is an integer which is different for each call.  */
/*(shell-command "rm varasm.o")*/
#define ASM_FORMAT_PRIVATE_NAME(OUTPUT, NAME, LABELNO) \
  ((OUTPUT) = (char *) alloca (strlen ((NAME)) + 10),\
     sprintf ((OUTPUT), "%s_%d", (NAME), (LABELNO)))

/* Define the parentheses used to group arithmetic operations
   in assembler code.  */

#define ASM_OPEN_PAREN "("
#define ASM_CLOSE_PAREN ")"

/* Define results of standard character escape sequences.  */
#define TARGET_BELL      007
#define TARGET_BS        010
#define TARGET_TAB       011
#define TARGET_NEWLINE   012
#define TARGET_VT        013
#define TARGET_FF        014
#define TARGET_CR        015

#define PRINT_OPERAND_PUNCT_VALID_P(CODE)  print_operand_punct_valid_p(CODE)

/* A C statement, to be executed after all slot-filler instructions
   have been output.  If necessary, call `dbr_sequence_length' to
   determine the number of slots filled in a sequence (zero if not
   currently outputting a sequence), to decide how many no-ops to
   output, or whatever.

   Don't define this macro if it has nothing to do, but it is
   helpful in reading assembly output if the extent of the delay
   sequence is made explicit (e.g. with white space).

   Note that output routines for instructions with delay slots must
   be prepared to deal with not being output as part of a sequence
   (i.e.  when the scheduling pass is not run, or when no slot
   fillers could be found.)  The variable `final_sequence' is null
   when not processing a sequence, otherwise it contains the
   `sequence' rtx being output.  */
#define DBR_OUTPUT_SEQEND(STREAM)\
  dbr_output_seqend(STREAM)

/*
 * Define number of delay slots present in the epilogue
 */
#define DELAY_SLOTS_FOR_EPILOGUE (CALLING_PROTOCALL_RTRTS && ADSP210Z3u && (MAIN_JUST_RETURNS || strcmp(current_function_name, "main")))?0:0

/* Expression indicating if INSN can be placed in epilog delay slot */
#define ELIGIBLE_FOR_EPILOGUE_DELAY(INSN,N) eligible_for_epilogue_p (INSN,N)

/* Defined as non-zero if it is safe for the delay slot scheduler
   to place instructions in the delay slot of insn */

#define INSN_SETS_ARE_DELAYED(insn) ( get_attr_type (insn) == TYPE_CALL && \
				     get_attr_delayable (insn) == DELAYABLE_TRUE)

#define INSN_REFERENCES_ARE_DELAYED(insn) (get_attr_type (insn) == TYPE_CALL && \
					   get_attr_delayable (insn) == DELAYABLE_TRUE)

#define PRINT_OPERAND(FILE, X, CODE)\
  print_operand(FILE,X,CODE);

/* Print a memory address as an operand to reference that memory location.
   Defined in out-dsp21k.c */
#define PRINT_OPERAND_ADDRESS(FILE, ADDR)\
  print_operand_address(FILE,ADDR)

/* Print a symbol separating elements of an array or 
   a record */
#define ASM_OUTPUT_ELT_SEP(FILE)\
  fprintf(FILE, ",")

/* Print out the size of an initialized array.
   It normally follows the name of the array */
#define ASM_OUTPUT_INIT_ASSIGN(FILE)\
      fprintf (FILE, " = ")

/* For emiting ".var" statements: */
#define ASM_OUTPUT_VAR_LABEL(FILE,NAME)\
  fprintf(FILE, ".var\t_%s ", NAME)

#define ASM_OUTPUT_DECL_END(FILE)\
  fprintf(FILE,";\n")

#define ASM_OUTPUT_EXTERNAL(FILE,DECL,NAME)\
{ extern int flag_short_double; \
  int _n; \
  if (_n = does_function_have_polymorphic_property (NAME))\
    generate_polymorphic_library_extern (FILE, NAME, _n); \
  else if (flag_short_double && is_double_math_library (NAME))\
    fprintf (FILE, ".extern\t_%sf;\n", NAME);\
  else fprintf (FILE, ".extern\t_%s;\n", NAME); }

#define RET  return ""        /* Used in machine description */

#define ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)  \
{int _size = int_size_in_bytes (TREE_TYPE (DECL)); \
 if (_size < 0)	abort();                           \
 fprintf (FILE, ".var\t_%s", NAME);                \
 if (_size != 1) fprintf (FILE, "[%d] ", _size);}

#define ASM_DECLARE_INTERNAL_OBJECT_NAME(FILE, NAME, DECL) \
  ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)

/* How to refer to registers in assembler output.
   This sequence is indexed by compiler's hard-register-number (see above).  */
/*(shell-command "rm varasm.o print-rtl.o rtl.o regclass.o")*/
#define REGISTER_NAMES \
{"r0", "r1", "r2", "r3",  "r4",  "r5",  "r6",  "r7",	\
 "r8", "r9", "r10","r11", "r12", "r13", "r14", "r15",	\
 "i0", "b0", "l0", "m0",  "i1",  "b1",  "l1",  "m1",    \
 "i2", "b2", "l2", "m2",  "i3",  "b3",  "l3",  "m3",    \
 "i4", "b4", "l4", "m4",  "i5",  "b5",  "l5",  "m5",    \
 "i6", "b6", "l6", "m6",  "i7",  "b7",  "l7",  "m7",    \
 "i8", "b8", "l8", "m8",  "i9",  "b9",  "l9",  "m9",    \
 "i10","b10","l10","m10", "i11", "b11", "l11", "m11",    \
 "i12","b12","l12","m12", "i13", "b13", "l13", "m13",    \
 "i14","b14","l14","m14", "i15", "b15", "l15", "m15",    \
 "pc", "pcstk","pcstkp","faddr","daddr","laddr","curlcntr","lcntr", \
 "mode1","mode2","irptl","imask","imaskp","astat","stky","ustat1", \
 "ustat2","dmwait","dmbank1","dmbank2","dmbank3","dmadr","pmwait","pmbank1", \
 "pmadr","px","px1","px2","tperiod","tcount","mrf","mrb", \
 }

#define ALT_REGISTER_NAMES \
{"f0", "f1", "f2", "f3",  "f4",  "f5",  "f6",  "f7",	\
 "f8", "f9", "f10","f11", "f12", "f13", "f14", "f15",	\
 }

#define ASM_COMMENT_START "\t!! "

#define INIT_SECTION_ASM_OP "\t!! Initialization Code for Main Routine???"


/* Here we define machine-dependent flags and fields in cc_status
   (see `conditions.h').  */


/* Store in cc_status the expressions
   that the condition codes will describe
   after execution of an instruction whose pattern is EXP.
   Do not alter them if the instruction would not alter the cc's.
   Defined in out-dsp21k.c  */

#define CC_IN_MULT	04000	/* Sign of value materialized in MS flag */
#define CC_IN_SHIFT	02000	/* Zero|Non-Zero materialized in SZ flag */


#define NOTICE_UPDATE_CC(BODY, INSN) {extern void notice_update_cc(rtx,rtx); \
					  notice_update_cc(BODY, INSN);}

/* Compute the cost of computing a constant rtl expression RTX
   whose rtx-code is CODE.  The body of this macro is a portion
   of a switch statement.  If the code is computed here,
   return it with a return statement.  Otherwise, break from the switch.
   For the 21k a constant that fits in 24-bits is cheaper than one
   that only fits in 32-bits  */

/* Disasllow constants 2^6 > x to be CSE  to be done. */
#define CONST_COSTS(RTX,CODE, OUTER_CODE) 			\
  case CONST_INT:						\
    return 1;     						\
  case CONST:							\
  case LABEL_REF:						\
  case SYMBOL_REF:						\
    return 1;							\
  case CONST_DOUBLE:						\
    return 2;

/*
  Like CONST_COSTS but applies to nonconstant RTL expressions.
  This can be used, for example, to indicate how costly a multiply
  instruction is.  In writing this macro, you can use the construct
  COSTS_N_INSNS (n) to specify a cost equal to n fast
  instructions.

  This macro is optional; do not define it if the default cost assumptions
  are adequate for the target machine.

  For division of the form x/2 we should handle this case so we can
  recognize the instruction (a+b)/2 */
/*(shell-command "rm -f cse.o unroll.o")*/
#define RTX_COSTS(x, code, outer_code)			\
  case MULT:       total = 2; break;			\
  case ASHIFT:     total = 4; break;                    \
  case LSHIFT:     total = 4; break;                    \
  case MOD:						\
  case DIV:						\
      total = 10;					\
    break
/*
   Why was this ever here?
 case DIV:
 if (GET_MODE(x) == SImode 			\
		&& GET_CODE (XEXP(x,1)) == CONST_INT 	\
		&& exact_log2(INTVAL (XEXP(x,1))) >=0)  \
*/
/*`REGISTER_MOVE_COST (FROM, TO)'
     A C expression for the cost of moving data from a register in
     class FROM to one in class TO.  The classes are expressed using
     the enumeration values such as `GENERAL_REGS'.  A value of 2 is
     the default; other values are interpreted relative to that.

     It is not required that the cost always equal 2 when FROM is the
     same as TO; on some machines it is expensive to move between
     registers if they are not general registers.

     If reload sees an insn consisting of a single `set' between two
     hard registers, and if `REGISTER_MOVE_COST' applied to their
     classes returns a value of 2, reload does not check to ensure
     that the constraints of the insn are met.  Setting a cost of
     other than 2 will allow reload to verify that the constraints are
     met.  You should do this if the `movM' pattern's constraints do
     not allow such copying.
#define REGISTER_MOVE_COST(FROM,TO) */


/*`MEMORY_MOVE_COST (M)'
     A C expression for the cost of moving data of mode M between a
     register and memory.  A value of 2 is the default; this cost is
     relative to those in `REGISTER_MOVE_COST'.

     If moving between registers and memory is more expensive than
     between two registers, you should define this macro to express
     the relative cost.*/

#define MEMORY_MOVE_COST(M) (((M)==DImode||(M)==DFmode)?6:2)

/* Define this if zero-extension is slow (more than one real instruction).  */

/* Nonzero if access to memory by bytes is slow and undesirable.  */
#define SLOW_BYTE_ACCESS 0

/* Define this if addresses of constant functions shouldn't
   be put through pseudo-regs where they can be cse'd.
   Desirable on machines where ordinary constants are expensive
   but a CALL with constant address is cheap */
#define NO_FUNCTION_CSE

#define ADDRESS_COST(X) address_cost(X)

/* Macro to define tables used to set the flags.
   This is a list in braces of pairs in braces,
   each pair being { "NAME", VALUE }
   where VALUE is the bits to set or minus the bits to clear.
   An empty string NAME is used to identify the default VALUE.  */

extern char* arch_reserved_registers;
extern char* arch_scratch_registers;

extern char *strength_reduction_benefit_str;
extern char *cse_pathlength_str;
extern char *pmcode_segment;
extern char *dmdata_segment;
extern char *pmdata_segment;

extern char *temp_segment_name;	/* used to point to scratch space for macro '(TEXT|DATA)_SECTION_ASM_OP' */
/********************ENVIRONMENT********************/
/* Macro to define tables used to set the flags.
   This is a list in braces of pairs in braces,
   each pair being { "NAME", VALUE }
   where VALUE is the bits to set or minus the bits to clear.
   An empty string NAME is used to identify the default VALUE.  */

#endif /* ACH_PARSE */

#ifdef ACH_PARSE
#define ACH_SWITCH(x,y,z) { z, "-m"##x }
#else /*  ACH_PARSE */
#define ACH_SWITCH(x,y,z) { x, y }
#endif /*  ACH_PARSE */
/* (shell-command "rm -f toplev.o") */
#define TARGET_SWITCHES {\
	ACH_SWITCH( "xpcrts",			   -00000000001, SW_PCRTS),\
	ACH_SWITCH( "xrtrts",	         	    00000000001, SW_RTRTS),\
	ACH_SWITCH( "xstackparm",  	   	   -00000000002, SW_PPONSTACK),\
	ACH_SWITCH( "xregparm",  	   	    00000000002, SW_PPINREG),\
        ACH_SWITCH( "xpmstack",			   -00000000010, SW_PMSTACK),\
        ACH_SWITCH( "xdmstack",			    00000000010, SW_DMSTACK),\
	ACH_SWITCH( "listm",                        00000010000, SW_NONE),\
	ACH_SWITCH( "pcrel",			    00200000000, SW_NONE),\
	ACH_SWITCH( "keep-i13",                     00000000020, SW_NONE),\
        ACH_SWITCH( "dont-inline-sqrt",		    00000000100, SW_NONE),\
        ACH_SWITCH( "keep-code-small",		    00000000200, SW_NONE),\
	ACH_SWITCH( "signed-pointers",              00000000400, SW_NONE),\
	ACH_SWITCH( "ADSP21010",                    00000001000, SW_ADSP21010),\
	ACH_SWITCH( "ADSP21020",                    00000001000, SW_ADSP21020),\
	ACH_SWITCH( "ADSP21060",                    00000002000, SW_ADSP21060),\
	ACH_SWITCH( "ADSP21060",                    00000002000, SW_ADSP210Z3),\
	ACH_SWITCH( "ADSP21061",                    00000006000, SW_ADSP21061),\
	ACH_SWITCH( "ADSP21062",                    00000006000, SW_ADSP210Z4),\
	ACH_SWITCH( "no-doloops",                   00000020000, SW_NONE),\
	ACH_SWITCH( "ainrts",                       00000040000, SW_NONE),\
	ACH_SWITCH( "jdwrite",                      00000100000, SW_NONE),\
	ACH_SWITCH( "premod-ptr-warn",              00000200000, SW_NONE),\
	ACH_SWITCH( "premod-ptr-error",             00000600000, SW_NONE),\
	ACH_SWITCH( "rr",			    00001000000, SW_NONE),\
	ACH_SWITCH( "rr-flow",			    00002000000, SW_NONE),\
        ACH_SWITCH( "profile",			    00004000000, SW_NONE),\
  	ACH_SWITCH( "debug",			    00010000000, SW_NONE),\
	ACH_SWITCH( "stat",			    00020000000, SW_NONE),\
	ACH_SWITCH( "atleast2-iterations",          00040000000, SW_NONE),\
	ACH_SWITCH( "ap",                           00000000000, SW_NONE),\
	ACH_SWITCH( "", 		            00000000017, SW_NONE)}

#define TARGET_OPTIONS {\
	ACH_SWITCH( "reserved=", &arch_reserved_registers ,		SW_RESERVED),\
	ACH_SWITCH( "scratch=",  &arch_scratch_registers ,		SW_NONE),\
	ACH_SWITCH( "srlevel=",  &strength_reduction_benefit_str,	SW_NONE),\
	ACH_SWITCH( "cselength=",&cse_pathlength_str,	                SW_NONE),\
	ACH_SWITCH( "pmcode=",   &pmcode_segment,			SW_PMCODE),\
	ACH_SWITCH( "dmdata=",   &dmdata_segment,			SW_DMDATA),\
	ACH_SWITCH( "pmdata=",   &pmdata_segment,			SW_PMDATA),\
	}

#ifndef ACH_PARSE
#define ADI_MERGED_LISTING

#define PCREL_ONLY		    (target_flags & 00200000000)
#define CALLING_PROTOCALL_PCRTS   (!(target_flags & 00000000001))
#define CALLING_PROTOCALL_RTRTS     (target_flags & 00000000001)
#define TARGET_REGPARM              (target_flags & 00000000002)
#define DOUBLE_IS_32              (!(target_flags & 00000000004))
#define DOUBLE_IS_64                (target_flags & 00000000004)
#define PMSTACK                   (!(target_flags & 00000000010))
#define DMSTACK                     (target_flags & 00000000010)
#define KEEP_I13                    (target_flags & 00000000020)

#define DONT_INLINE_SQRT            (target_flags & 00000000100)
#define KEEP_CODE_SMALL             (target_flags & 00000000200)
#define SIGNED_POINTERS             (target_flags & 00000000400)
#define GUAREENTED_2ITERATIONS      (target_flags & 00040000000)
#define DODOLOOPS                 (!(target_flags & 00000020000))

#define ADSP21010u                  (target_flags & 00000002000)
#define ADSP21020u                  (target_flags & 00000001000)
#define ADSP210Z3u                  (target_flags & 00000002000)
#define ADSP210Z4u                  (target_flags & 00000004000)
#define JDWRITE                     (target_flags & 00000100000)

#define RE_RECOG		    (target_flags & 00001000000)
#define RE_RECOG_FLOW		    (target_flags & 00002000000)
#define PROFILE			    (target_flags & 00004000000)
#define DEBUGING       		    (target_flags & 00010000000)
#define KEEP_STAT       	    (target_flags & 00020000000)
#define LISTING_WITH_SOURCE         (target_flags & 00000010000)
#define MAIN_JUST_RETURNS           (target_flags & 00000040000)

#define PREMOD_PTR_WARNING          (target_flags & 00000200000)
#define PREMOD_PTR_ERROR            (target_flags & 00000600000)

/* Run-time compilation parameters selecting different hardware subsets.  */
extern long target_flags;
#define STACK_REF (PMSTACK?"pm":"dm")
#define FRAME_POINTER_STRING (PMSTACK?"I14":"I6")
#define STACK_POINTER_STRING (PMSTACK?"I15":"I7")

/* Macros used in the machine description to test the flags.  */

#define CHECK_PREMODIFY_MEMORY_ACCESS(x, strict) \
	if (strict \
	    && PREMOD_PTR_WARNING \
	    && ! RTX_ISPOINTER_P (x)) {\
	  debug_rtx (x); \
	  if (PREMOD_PTR_ERROR)\
	    error ("invalid memory reference non pointer register used.");\
	  else\
	    warning ("invalid memory reference non pointer register used.");\
	}


/*** Reset both RE-FLOW and RE-RELOAD if RR is reset: ***/

#define OVERRIDE_OPTIONS \
{ if (write_symbols != NO_DEBUG)\
    optimize = 0; \
  flag_force_mem = 1;\
  if (! RE_RECOG)\
    target_flags &= (~ 00001000000) & (~ 00002000000); }

#define SWITCH_TAKES_ARG(CHAR)      \
  ((CHAR) == 'D' || (CHAR) == 'U' || (CHAR) == 'o' \
   || (CHAR) == 'e' || (CHAR) == 'T' || (CHAR) == 'u' \
   || (CHAR) == 'I' || (CHAR) == 'm' \
   || (CHAR) == 'L' || (CHAR) == 'A')

#define WORD_SWITCH_TAKES_ARG(STR) \
	 (!strcmp (STR, "Tdata")   || \
	  !strcmp (STR, "include") || \
	  !strcmp (STR, "arch")    || \
	  !strcmp (STR, "imacros") || \
	  !strcmp (STR, "runhdr"))

#define OPTIMIZATION_OPTIONS(optimize){\
  extern int flag_short_double;\
  flag_short_double = 1; }


/* (shell-command "rm gcc.o") */
#define CPP_SPEC \
"-D__ADSP21000__ -DADSP21000 \
 %{mADSP21010:-D__21K__ -D__ADSP21010__ -DADSP21010} \
 %{mADSP21020:-D__21K__ -D__ADSP21020__ -DADSP21020} \
 %{mADSP21060:-D__2106x__ -D__ADSP21060__ -DADSP21060} \
 %{mADSP21061:-D__2106x__ -D__ADSP21061__ -DADSP21061} \
 %{mADSP21062:-D__2106x__ -D__ADSP21062__ -DADSP21062} \
 %{mADSP21063:-D__2106x__ -D__ADSP21063__ -DADSP21063} \
 %{mADSP21064:-D__2106x__ -D__ADSP21064__ -DADSP21064} \
 %{g:-D__DEBUG__} \
 %{!ansi: %{!fno-short-double: -D__DOUBLES_ARE_FLOATS__}} "

#define CC1_SPEC  "%{ansi: -fno-short-double} "

#define ASM_SIGNATURE_SPEC "-ccdcd \
  %{mADSP21060:-ADSP21060} %{mADSP21061:-ADSP21060} %{mADSP21062:-ADSP21060} \
  %{mADSP21063:-ADSP21060} %{mADSP21064:-ADSP21060}"

#ifdef RDP21K
#define ASM_SPEC ASM_SIGNATURE_SPEC ## "%{a:%{a*}}%{!a:-a 21k.ach%s} "
#else /*  RDP21K */
#define ASM_SPEC ASM_SIGNATURE_SPEC
#endif /* RDP21K */

#define LIB_SPEC  "%{mADSP21060:-l060} %{mADSP21061:-l060} %{mADSP21062:-l060} %{mADSP21063:-l060} %{mADSP21064:-l060} -lc"

#define LINK_SPEC "%{map:-m} "

#define STARTFILE_SPEC "%{!nostdlib:\
   %{!runhdr:%{mADSP21010:020_hdr.obj%s}\
             %{mADSP21020:020_hdr.obj%s}\
             %{mADSP21060:060_hdr.obj%s}\
             %{mADSP21061:060_hdr.obj%s}\
             %{mADSP21062:060_hdr.obj%s}} %{runhdr*:%*}}"

/* Names to predefine in the preprocessor for this target machine.  */

#define CPP_PREDEFINES ""


#define WARN_SWITCH \
{  "-fpretend-float",\
   "-fcse-follow-jumps",\
   "-fcse-skip-blocks",\
   "-fexpensive-optimizations",\
   "-ffloat-store",\
   "-fforce-addr",\
   "-fkeep-inline-functions",\
   "-fno-defer-pop",\
   "-fno-function-cse",\
   "-fomit-frame-pointer",\
   "-fthread-jumps",\
   "-funroll-all-loops",\
   "-funroll-loops",\
   "-fno-common",\
   "-fpcc-struct-return",\
   "-fpic",\
   "-fPIC",\
   "-fshared-data",\
   "-fshort-enums",\
   "-fvolatile",\
   0   }

#define TARGET_VERSION_STRING " 3.3a "

/* Print subsidiary information on the compiler version in use.  */
#define TARGET_VERSION fprintf (stderr,	   \
			  " (adsp21k Development" TARGET_VERSION_STRING "):");

#define DEFAULT_TARGET_MACHINE "21k"

#define CROSS_COMPILER

#define ADII                   "ADII_21K"
#define USE_ADI_DSP            "ADI_DSP"

#ifndef __MSDOS__
#define INCLUDE_FILES_LOCATION DEFAULT_TARGET_MACHINE ## "/include"
#define CC1_LOCATION           DEFAULT_TARGET_MACHINE ## "/etc/"
#define LIB_LOCATION           DEFAULT_TARGET_MACHINE ## "/lib/"
#else /* __MSDOS__ */
#define INCLUDE_FILES_LOCATION DEFAULT_TARGET_MACHINE ## "\\include"
#define CC1_LOCATION           DEFAULT_TARGET_MACHINE ## "\\etc\\"
#define LIB_LOCATION           DEFAULT_TARGET_MACHINE ## "\\lib\\"
#endif /* __MSDOS__ */

#define STANDARD_EXEC_PREFIX   0
#define STANDARD_STARTFILE_PREFIX 0
#define STANDARD_STARTFILE_PREFIX_1 0
#define STANDARD_STARTFILE_PREFIX_2 0
#define STANDARD_EXEC_PREFIX_1 0

#define INCLUDE_DEFAULTS {{ 0 , 0 }};
#ifdef __MSDOS__
#define EXECUTABLE_SUFFIX ".exe"
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR  '\\'
#define BROKEN_DJGCC_DOUBLE_UNSIGNED

/* This is for thoose dos boxes that don't let you redirect stderr. */

#endif /* __MSDOS__ */


#define TARGET_MEM_FUNCTIONS
/********************ENVIRONMENT********************/


/* Define this if the tablejump instruction expects the table
   to contain offsets from the address of the table.
   Do not define this if the table should contain absolute addresses.  */
/* #define CASE_VECTOR_PC_RELATIVE  */

/* Specify the tree operation to be used to convert reals to integers.  */
#define IMPLICIT_FIX_EXPR FIX_ROUND_EXPR

/* This is the kind of divide that is easiest to do in the general case.  */
#define EASY_DIV_EXPR TRUNC_DIV_EXPR


/* We currently define double as 32 bits on the 21k */

/* Max number of bytes we can move from memory to memory
   in one reasonably fast instruction.  */
#define MOVE_MAX UNITS_PER_WORD


/* Define if shifts truncate the shift count
   which implies one can omit a sign-extension or zero-extension
   of a shift count.  */
/* #define SHIFT_COUNT_TRUNCATED 	Not true on 21k */


/* Value is 1 if truncating an integer of INPREC bits to OUTPREC bits
   is done just by pretending it is already truncated.  */
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1


/* We assume that the store-condition-codes instructions store 0 for false
   and some other value for true.  This is the value stored for true.  */

#define STORE_FLAG_VALUE  1

/* When a prototype says `char' or `short', really pass an `int'.  */
/* But for now this feature is disabled **/

/****  #define PROMOTE_PROTOTYPES  *******/


/* Specify the machine mode that pointers have.
   After generation of rtl, the compiler makes no further distinction
   between pointers and any other objects of this machine mode.  */

#ifdef MEMSEG

/*(shell-command
"rm -f caller-save.o calls.o combine.o cp-expr.o cse.o emit-rtl.o explow.o \
expr.o final.o flow.o function.o integrate.o loop.o optabs.o recog.o \
reg-stack.o regclass.o reload.o reload1.o stmt.o stor-layout.o \
varasm.o")*/

enum machine_mode extract_tree_pmode();
enum machine_mode extract_rtx_pmode();


/* Specify the machine mode that this machine uses
   for the index in the tablejump instruction.
  (shell-command "rm expr.o stmt.o")*/

#define CASE_VECTOR_MODE PMmode

/*#define JUMP_TABLES_IN_TEXT_SECTION  (shell-command "rm final.o")*/
#ifdef JUMP_TABLES_IN_TEXT_SECTION
#define DEFAULT_TABLE_Pmode PMmode
#else /*  JUMP_TABLES_IN_TEXT_SECTION */
#define DEFAULT_TABLE_Pmode DMmode
#endif /*  JUMP_TABLES_IN_TEXT_SECTION */

#define DEFAULT_CODE_Pmode PMmode
#define DEFAULT_DATA_Pmode DMmode
#define Extract_mode_Pmode(rtx,mode) extract_mode_pmode(rtx,mode)
#define Extract_tree_Pmode(tree)     extract_tree_pmode(tree)
#define Extract_rtx_Pmode(rtx)       extract_rtx_pmode(rtx)
#define Pmode1(X) (get_pmode(X))
#define PMODE_P(X) (((X)==PMmode)||((X)==DMmode))

/*(shell-command "rm calls.o expr.o tree.o varasm.o")*/
/* A function address in a call instruction
   is a byte address (for indexing purposes)
   so give the MEM rtx a byte's mode.  */
#define FUNCTION_MODE PMmode

#else /* MEMSEG */
#define Pmode1(X) SImode
#define FUNCTION_MODE SImode
#define CASE_VECTOR_MODE SImode
#endif /* MEMSEG */

#define CHECK_CONSTRAINTS (!(reload_in_progress||reload_completed)||check_constraints())

/* STANDARD REGISTER USAGE. */

/* Number of actual hardware registers.
   The hardware registers are assigned numbers for the compiler
   from 0 to just below FIRST_PSEUDO_REGISTER.
   All registers that the compiler knows about must be given numbers,
   even those that are not normally considered general registers. */


#define REG_R0  0
#define REG_R1  1
#define REG_R2  2
#define REG_R3  3
#define REG_R4  4
#define REG_R5  5
#define REG_R6  6
#define REG_R7  7
#define REG_R8  8
#define REG_R9  9
#define REG_R10 10
#define REG_R11 11
#define REG_R12 12
#define REG_R13 13
#define REG_R14 14
#define REG_R15 15

#define REG_I0  16
#define REG_B0  17
#define REG_L0  18
#define REG_M0  19
#define REG_I1  20
#define REG_B1  21
#define REG_L1  22
#define REG_M1  23
#define REG_I2  24
#define REG_B2  25
#define REG_L2  26
#define REG_M2  27
#define REG_I3  28
#define REG_B3  29
#define REG_L3  30
#define REG_M3  31
#define REG_I4  32
#define REG_B4  33
#define REG_L4  34
#define REG_M4  35
#define REG_I5  36
#define REG_B5  37
#define REG_L5  38
#define REG_M5  39
#define REG_I6  40
#define REG_B6  41
#define REG_L6  42
#define REG_M6  43
#define REG_I7  44
#define REG_B7  45
#define REG_L7  46
#define REG_M7  47
#define REG_I8  48
#define REG_B8  49
#define REG_L8  50
#define REG_M8  51
#define REG_I9  52
#define REG_B9  53
#define REG_L9  54
#define REG_M9  55
#define REG_I10 56
#define REG_B10 57
#define REG_L10 58
#define REG_M10 59
#define REG_I11 60
#define REG_B11 61
#define REG_L11 62
#define REG_M11 63
#define REG_I12 64
#define REG_B12 65
#define REG_L12 66
#define REG_M12 67
#define REG_I13 68
#define REG_B13 69
#define REG_L13 70
#define REG_M13 71
#define REG_I14 72
#define REG_B14 73
#define REG_L14 74
#define REG_M14 75
#define REG_I15 76
#define REG_B15 77
#define REG_L15 78
#define REG_M15 79

#define REG_PC       80
#define REG_PCSTK    81
#define REG_PCSTKP   82
#define REG_FADDR    83
#define REG_DADDR    84
#define REG_LADDR    85
#define REG_CURLCNTR 86
#define REG_LCNTR    87
#define REG_MODE1    88
#define REG_MODE2    89
#define REG_IRPTL    90
#define REG_IMASK    91
#define REG_IMASKP   92
#define REG_ASTAT    93
#define REG_STKY     94

#define REG_USTAT1   95
#define REG_USTAT2   96

#define REG_DMWAIT   97
#define REG_DMBANK1  98
#define REG_DMBANK2  99
#define REG_DMBANK3  100
#define REG_DMADR    101
#define REG_PMWAIT   102
#define REG_PMBANK1  103
#define REG_PMADR    104
#define REG_PX       105
#define REG_PX1      106
#define REG_PX2      107
#define REG_TPERIOD  108
#define REG_TCOUNT   109

#define REG_MRF 110
#define REG_MRB 111

#define LAST_REG 111
#define LAST_REG_NAMES LAST_REG+10
#define FIRST_PSEUDO_REGISTER (LAST_REG+1)

/* First floating point register 'f0' */
#define FIRST_FLOAT_REG      REG_R0

/* these registers are hardwired! */

#define MINUS1_MREG(M) ((M==DMmode)?REG_M7:\
			(M==PMmode)?REG_M15:999)
#define PLUS1_MREG(M)  ((M==DMmode)?REG_M6:\
			(M==PMmode)?REG_M14:999)
#define ZERO_MREG(M)   ((M==DMmode)?REG_M5:\
			(M==PMmode)?REG_M13:999)

#define SCRATCH_DM_MREG REG_M4
#define SCRATCH_PM_MREG REG_M12
#define SCRATCH_DREG    REG_R2

/*(shell-command "rm regclass.o" nil)*/
/* 1 for registers that have pervasive standard uses
   and are not available for the register allocator.
   On the 21k,
     'i6'  frame pointer
     'i7'  stack pointer
     'm7'  hardwired -1
     'm6'             1


Note: THERE ARE NO "SCRATCH" REGISTERS.  Do not make any
 without first consulting with JAK
*/

#define FIXED_REGISTERS  {\
 /* r0-r7 */   0, 0, 0, 0, 0, 0, 0, 0,       \
 /* r8-r15 */  0, 0, 0, 0, 0, 0, 0, 0,       \
 /* iblm0-1 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /* iblm2-3 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /* iblm4-5 */ 0, 0, 0, 0, 0, 0, 0, 1,       \
 /* iblm6-7 */ 1, 1, 1, 1, 1, 1, 1, 1,       \
 /* iblm8-9 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*iblm10-11*/ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*iblm12-13*/ 0, 0, 0, 0, 0, 0, 0, 1,       \
 /*iblm14-15*/ 1, 1, 1, 1, 1, 1, 1, 1,       \
 /*pc-lcntr*/  0, 0, 0, 0, 0, 0, 0, 0,       \
 /*mode-ustat*/0, 0, 0, 0, 0, 0, 0, 0,       \
 /*usta-pmbk*/ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*pmad-mrb*/  0, 0, 0, 0, 0, 0, 0, 0,       \
  }

/* 1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.
   On the 21k r0
   r2 is used to save fp on function call.
     these registers are not available across function calls!
     Scratch registers:
      r2, r4, r8, r12

      b4 l4 m4 i4
      b12 l12 m12 i12
and, of course
      i13 l13 b13 (the return value!)
(shell-command "rm regclass.o ")
*/
#define CALL_USED_REGISTERS  {\
 /* r0-r7 */   1, 1, 1, 0, 1, 0, 0, 0,       \
 /* r8-r15 */  1, 0, 0, 0, 1, 0, 0, 0,       \
 /* iblm0-1 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /* iblm2-3 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /* iblm4-5 */ 1, 1, 1, 1, 0, 0, 0, 1,       \
 /* iblm6-7 */ 1, 1, 1, 1, 1, 1, 1, 1,       \
 /* iblm8-9 */ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*iblm10-11*/ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*iblm12-13*/ 1, 1, 1, 1, 1, 1, 1, 1,       \
 /*iblm14-15*/ 1, 1, 1, 1, 1, 1, 1, 1,       \
 /*pc-lcntr*/  0, 0, 0, 0, 0, 0, 0, 0,       \
 /*mode-ustat*/0, 0, 0, 0, 0, 0, 0, 0,       \
 /*usta-pmbk*/ 0, 0, 0, 0, 0, 0, 0, 0,       \
 /*pmad-mrb*/  0, 0, 0, 0, 0, 0, 0, 0,       \
  }


#define IS_DREG(REG)     ((REG)>=REG_R0 && (REG)<=REG_R15)
#define IS_DAG_REG(REG)  ((REG)>=REG_I0 && (REG)<=REG_M15)
#define IS_DAG1_REG(REG) ((REG)>=REG_I0 && (REG)<=REG_M7)
#define IS_DAG2_REG(REG) ((REG)>=REG_I8 && (REG)<=REG_M15)
#define IS_IREG(REG)     (IS_DAG_REG(REG) && ((REG)&0x3)==0)
#define IS_BREG(REG)     (IS_DAG_REG(REG) && ((REG)&0x3)==1)
#define IS_LREG(REG)     (IS_DAG_REG(REG) && ((REG)&0x3)==2)
#define IS_MREG(REG)     (IS_DAG_REG(REG) && ((REG)&0x3)==3)
#define IS_PSEUDO(REG)   ((REG)>=FIRST_PSEUDO_REGISTER)
#define IS_DM_IREG(REG)  (IS_DAG1_REG(REG) && ((REG)&0x3)==0)
#define IS_DM_BREG(REG)  (IS_DAG1_REG(REG) && ((REG)&0x3)==1)
#define IS_DM_LREG(REG)  (IS_DAG1_REG(REG) && ((REG)&0x3)==2)
#define IS_DM_MREG(REG)  (IS_DAG1_REG(REG) && ((REG)&0x3)==3)
#define IS_PM_IREG(REG)  (IS_DAG2_REG(REG) && ((REG)&0x3)==0)
#define IS_PM_BREG(REG)  (IS_DAG2_REG(REG) && ((REG)&0x3)==1)
#define IS_PM_LREG(REG)  (IS_DAG2_REG(REG) && ((REG)&0x3)==2)
#define IS_PM_MREG(REG)  (IS_DAG2_REG(REG) && ((REG)&0x3)==3)
#define IS_MULT_ACC_REG(REG) ((REG)==REG_MRF||(REG)==REG_MRB)
#define IREGS_DIFFERENT_DAG(reg1,reg2)  iregs_different_dag(reg1,reg2)
#define IS_SYS(REG)      ((REG)>=REG_PC && (REG)<=REG_TCOUNT)



/* CONDITIONAL_REGISTER_USAGE
     Zero or more C statements that may conditionally modify two variables
     `fixed_regs' and `call_used_regs' (both of type `char
     []') after they have been initialized from the two preceding macros.

     This is necessary in case the fixed or call-clobbered registers depend
     on target flags.

     You need not define this macro if it has no work to do.

     If the usage of an entire class of registers depends on the target
     flags, you may indicate this to GCC by using this macro to modify
     `fixed_regs' and `call_used_regs' to 1 for each of the
     registers in the classes which should not be used by GCC.  Also define
     the macro `REG_CLASS_FROM_LETTER' to return `NO_REGS' if it
     is called with a letter for a class that shouldn't be used.

     (However, if this class is not included in `GENERAL_REGS' and all
     of the insn patterns whose constraints permit this class are
     controlled by target switches, then GCC will automatically avoid using
     these registers when the target switches are opposed to them.) */
#define CONDITIONAL_REGISTER_USAGE conditional_register_usage()

/* The macros REG_OK_FOR..._P assume that the arg is a REG rtx
   and check its validity for a certain class.
   We have two alternate definitions for each of them.
   The usual definition accepts all pseudo regs; the other rejects
   them unless they have been allocated suitable hard regs.
   The symbol REG_OK_STRICT causes the latter definition to be used.

   Most source files want to accept pseudo regs in the hope that
   they will get allocated to the class that the insn wants them to be in.
   Source files for reload pass need to be strict.
   After reload, it makes no difference, since pseudo regs have
   been eliminated by then.  */

#define REG_OK_FOR_INDEX_STRICT_P(X,STRICT_P,MODE)\
    ((((MODE)==DMmode) && IS_DM_MREG(REGNO(X))) \
     || (((MODE)==PMmode) && IS_PM_MREG(REGNO(X))) \
     || (!(STRICT_P) && IS_PSEUDO (REGNO (X)) \
	 && !REGNO_POINTER_FLAG(REGNO (X)) ))

#if 0
#define REG_OK_FOR_BASE_STRICT_P(X,STRICT_P,MODE)\
     ((!ADSP210Z3u \
 	|| RTX_ISPOINTER_P (X)) \
      && ((((MODE)==DMmode) && IS_DM_IREG (REGNO (X))) \
	  ||(((MODE)==PMmode)&&IS_PM_IREG (REGNO (X))) \
	  ||(!(STRICT_P) && IS_PSEUDO (REGNO (X)))))
#endif

#define REG_OK_FOR_BASE_STRICT_P(X,STRICT_P,MODE)\
     ((((MODE)==DMmode) && IS_DM_IREG (REGNO (X))) \
      ||(((MODE)==PMmode)&&IS_PM_IREG (REGNO (X))) \
      ||(!(STRICT_P) && IS_PSEUDO (REGNO (X))))

/* Nonzero if X is a hard reg that can be used as an index
   or if it is a pseudo reg.  */
#define REG_OK_FOR_INDEX_P(X,MODE) \
REG_OK_FOR_INDEX_STRICT_P(X,STRICTNESS,MODE)

/* Nonzero if X is a hard reg that can be used as a base reg
   or if it is a pseudo reg.  */
#define REG_OK_FOR_BASE_P(X,MODE)  \
    REG_OK_FOR_BASE_STRICT_P(X,STRICTNESS,MODE)


/*
 * This awesome table is JAK's letters.
 * Register allocation Constraints.
 *  a -- pm b
 *  b -- r4-r7
 *  c -- r8-r11
 *  d -- r0-r15
 *  e -- pm l
 *  f -- mrf/mrb
 *  g -- builtin: any general register, memory or immediate integer
 *  h -- dm b
 *  i -- builtin: immediate integer
 *  j -- dm l
 *  k -- r0-r3
 *  l -- r12-r15
 *  m -- builtin: memory
 *  n -- builtin: known immediate integer
 *  o -- builtin: offsetable memory
 *  p -- builtin: any valid memory address
 *  q --
 *  r -- builtin: general register
 *  s -- builtin: integer, non-explicit
 *  t -- system-regs
 *  u -- ustat
 *  v --
 *  w -- dm i reg
 *  x -- dm m reg
 *  y -- pm i reg
 *  z -- pm m reg
 *  A -- pm(i,m)
 *  B -- pm(m,i)
 *  C -- pm(i,6) or pm(6,i)
 *  D -- pm(32,i)
 *  E -- builtin: immediate floating point
 *  F -- builtin: immediate floating point
 *  G -- floating point zero
 *  H -- unimplemented floating point
 *  I -- integer 1
 *  J -- 6 bit immediate
 *  K -- 24 bit immediate
 *  L -- 8 bit immediate
 *  M -- 32 bit immediate
 *  N -- integer -1
 *  O -- integet 0
 *  P -- -1,0,1 (in conjunction with %P
 *  Q -- dm(i,m)
 *  R -- dm(m,i)
 *  S -- dm(i,6) or dm(6,i)
 *  T -- pm(32) or dm(32)
 *  U -- dm(32,i)
 *  V -- builtin: non-offsettable memory
 *  W --
 *  X -- builtin: any operand
 *  Y --
 *  Z --
 *  < -- builtin: autodecrement
 *  > -- builtin: autoincrement
 *  ? -- builtin: disparage slightly this alternative
 *  ! -- builtin: disparage greatly this alternative
 *  = -- builtin: operand is write only
 *  + -- builtin: operand both input and output
 *  & -- builtin: early clobber
 *  % -- builtin: commutative
 *  # -- builtin: ignore as constraint, only use for register preferencing
 *  * -- builtin: ignore fore register preferencing
 */

/* Define the classes of registers for register constraints in the
   machine description.  Also define ranges of constants.

   One of the classes must always be named ALL_REGS and include all hard regs.
   If there is more than one class, another class must be named NO_REGS
   and contain no registers.

   The name GENERAL_REGS must be the name of a class (or an alias for
   another name such as ALL_REGS).  This is the class of registers
   that is allowed by "g" or "r" in a register constraint.
   Also, registers outside this class are allocated only when
   instructions express preferences for them.

   The classes must be numbered in nondecreasing order; that is,
   a larger-numbered class must never be contained completely
   in a smaller-numbered class.

   For any two classes, it is very desirable that there be another
   class that represents their union.
     Classes d_dmi_regs and d_dmm_regs are unions of D_REGS with
     DM_IREGS and DM_MREGS respectively. They are used only
     for finer computation of register class subunions

     This is helpful if you add a new register class!
   $ egrep -l 'REG_CLASS_CONTENTS|REG_CLASS_NAME|enum reg_class' *.c
   (shell-command "rm flow.o global-alloc.o local-alloc.o recog.o reg-stack.o regclass.o reload.o reload1.o stupid.o")  <-C-xC-e
   $ */
enum reg_class {
    NO_REGS,
    r0r3DREGS,
    r4r7DREGS,
    r8r11DREGS,
    r12r15DREGS,
    r0r7DREGS,
    r8r15DREGS,
    DM_IREGS,
    DM_MREGS,
    DM_BREGS,
    DM_LREGS,
    PM_IREGS,
    PM_MREGS,
    PM_BREGS,
    PM_LREGS,
    IREGS_REGS,
    MREGS_REGS,
    DAG1_REGS,
    DAG2_REGS,
    DAG_REGS,
    D_REGS,
    USTAT_REGS,
    SYSTEM_REGS,
    d_DMI_REGS,
    d_DMM_REGS,
    d_DAG1_REGS,
    d_PMI_REGS,
    d_PMM_REGS,
    d_M_REGS,
    d_I_REGS,
    d_M_DMI_REGS,
    d_DAG2_REGS,
    d_DAG1_PMI_REGS,
    d_DAG1_DAG2_REGS,
    MULT_ACC_REGS,
    d_mult_regs,
    D_DAG1_MULT_REGS,
    D_DAG2_MULT_REGS,
    DREGS_SYSTEM_REGS,
    GENERAL_REGS,
    ALL_REGS, LIM_REG_CLASSES
    };


#define N_REG_CLASSES ((int)LIM_REG_CLASSES)

/* Give names of register classes as strings for dump file.   */

#define REG_CLASS_NAMES     \
{ "NO_REGS", 		    \
  "r0r3DREGS",		    \
  "r4r7DREGS",		    \
  "r8r11DREGS",		    \
  "r12r15DREGS",	    \
  "r0r7DREGS", 		    \
  "r8r15DREGS",		    \
  "DM_IREGS", 		    \
  "DM_MREGS", 		    \
  "DM_BREGS", 		    \
  "DM_LREGS", 		    \
  "PM_IREGS",		    \
  "PM_MREGS",		    \
  "PM_BREGS",		    \
  "PM_LREGS",		    \
  "IREGS_REGS",		    \
  "MREGS_REGS",		    \
  "DAG1_REGS", 		    \
  "DAG2_REGS", 		    \
  "DAG_REGS",		    \
  "D_REGS",    		    \
  "USTAT_REGS",    	    \
  "SYSTEM_REGS",	    \
  "d_DMI_REGS", 	    \
  "d_DMM_REGS",  	    \
  "d_DAG1_REGS",  	    \
  "d_PMI_REGS", 	    \
  "d_PMM_REGS",  	    \
  "d_M_REGS", 		    \
  "d_I_REGS", 		    \
  "d_M_DMI_REGS", 	    \
  "d_DAG2_REGS",  	    \
  "d_DAG1_PMI_REGS",	    \
  "d_DAG1_DAG2_REGS",	    \
  "MULT_ACC_REGS", 	    \
  "d_mult_regs",	    \
  "D_DAG1_MULT_REGS",	    \
  "D_DAG2_MULT_REGS",	    \
  "DREGS_SYSTEM_REGS",	    \
  "GENERAL_REGS", 	    \
  "ALL_REGS" }

/* Define which registers fit in which classes.
   This is an initializer for a vector of HARD_REG_SET
   of length N_REG_CLASSES.  */

/*(shell-command "rm regclass.o")*/
#define REG_CLASS_CONTENTS  \
{ { 0x00000000 , 0x00000000 , 0x00000000 , 0x0000 } , /** NO_REGS      **/\
  { 0x0000000f , 0x00000000 , 0x00000000 , 0x0000 } , /** r0r3DREGS    **/\
  { 0x000000f0 , 0x00000000 , 0x00000000 , 0x0000 } , /** r4r7DREGS    **/\
  { 0x00000f00 , 0x00000000 , 0x00000000 , 0x0000 } , /** r8r11DREGS   **/\
  { 0x0000f000 , 0x00000000 , 0x00000000 , 0x0000 } , /** r12r15DREGS  **/\
  { 0x000000ff , 0x00000000 , 0x00000000 , 0x0000 } , /** r0r7DREGS    **/\
  { 0x0000ff00 , 0x00000000 , 0x00000000 , 0x0000 } , /** r8r15DREGS   **/\
  { 0x11110000 , 0x00001111 , 0x00000000 , 0x0000 } , /** DM_IREGS     **/\
  { 0x88880000 , 0x00008888 , 0x00000000 , 0x0000 } , /** DM_MREGS     **/\
  { 0x22220000 , 0x00002222 , 0x00000000 , 0x0000 } , /** DM_BREGS     **/\
  { 0x44440000 , 0x00004444 , 0x00000000 , 0x0000 } , /** DM_LREGS     **/\
  { 0x00000000 , 0x11110000 , 0x00001111 , 0x0000 } , /** PM_IREGS     **/\
  { 0x00000000 , 0x88880000 , 0x00008888 , 0x0000 } , /** PM_MREGS     **/\
  { 0x00000000 , 0x22220000 , 0x00002222 , 0x0000 } , /** PM_BREGS     **/\
  { 0x00000000 , 0x44440000 , 0x00004444 , 0x0000 } , /** PM_LREGS     **/\
  { 0x11110000 , 0x11111111 , 0x00001111 , 0x0000 } , /** IREGS_REGS   **/\
  { 0x88880000 , 0x88888888 , 0x00008888 , 0x0000 } , /** MREGS_REGS   **/\
  { 0x99990000 , 0x00009999 , 0x00000000 , 0x0000 } , /** DAG1_REGS    **/\
  { 0x00000000 , 0x99990000 , 0x00009999 , 0x0000 } , /** DAG2_REGS    **/\
  { 0x99990000 , 0x99999999 , 0x00009999 , 0x0000 } , /** DAG_REGS     **/\
  { 0x0000ffff , 0x00000000 , 0x00000000 , 0x0000 } , /** D_REGS       **/\
  { 0x00000000 , 0x00000000 , 0x80000000 , 0x0001 } , /** USTAT_REGS   **/\
  { 0x00000000 , 0x00000000 , 0x7fff0000 , 0x3ffe } , /** SYSTEM_REGS  **/\
  { 0x1111ffff , 0x00001111 , 0x00000000 , 0x0000 } , /** d_DMI_REGS   **/\
  { 0x8888ffff , 0x00008888 , 0x00000000 , 0x0000 } , /** d_DMM_REGS   **/\
  { 0x9999ffff , 0x00009999 , 0x00000000 , 0x0000 } , /** d_DAG1_REGS  **/\
  { 0x0000ffff , 0x11110000 , 0x00001111 , 0x0000 } , /** d_PMI_REGS   **/\
  { 0x0000ffff , 0x88880000 , 0x00008888 , 0x0000 } , /** d_PMM_REGS   **/\
  { 0x8888ffff , 0x88888888 , 0x00008888 , 0x0000 } , /** d_M_REGS     **/\
  { 0x1111ffff , 0x11111111 , 0x00001111 , 0x0000 } , /** d_I_REGS     **/\
  { 0x9999ffff , 0x88889999 , 0x00008888 , 0x0000 } , /** d_M_DMI_REGS **/\
  { 0x0000ffff , 0x99990000 , 0x00009999 , 0x0000 } , /** d_DAG2_REGS  **/\
  { 0x9999ffff , 0x11119999 , 0x00001111 , 0x0000 } , /** d_DAG1_PMI_REGS**/\
  { 0x9999ffff , 0x99999999 , 0x00009999 , 0x0000 } , /** d_DAG1_DAG2_REGS**/\
  { 0x00000000 , 0x00000000 , 0x00000000 , 0xc000 } , /** MULT_ACC_REGS**/\
  { 0x0000ffff , 0x00000000 , 0x00000000 , 0xc000 } , /** d_mult_regs  **/\
  { 0x9999ffff , 0x00009999 , 0x00000000 , 0xc000 } , /** D_DAG1_MULT_REGS  **/\
  { 0x0000ffff , 0x99990000 , 0x00009999 , 0xc000 } , /** D_DAG2_MULT_REGS  **/\
  { 0x0000ffff , 0x00000000 , 0x7fff0000 , 0x3ffe } , /** DREGS_SYSTEM_REGS **/\
  { 0xffffffff , 0xffffffff , 0x8000ffff , 0x0001 } , /** GENERAL_REGS **/\
  { 0xffffffff , 0xffffffff , 0xffffffff , 0xffff } , /** ALL_REGS     **/ }


/* The same information, inverted:
   Return the class number of the smallest class containing
   reg number REGNO.  This could be a conditional expression
   or could index an array.  */

extern enum reg_class regno_reg_class[];

extern enum reg_class regno_reg_class[]; /* declared in aux-output.c */
#define REGNO_REG_CLASS(REGNO) regno_reg_class[REGNO]


/* Get reg_class from a letter such as appears in the machine description.
   We do a trick here to modify the effective constraints on the
   machine description; we zorch the constraint letters that aren't
   appropriate for a specific target.  This allows us to guarantee
   that a specific kind of register will not be used for a given target
   without fiddling with the register classes above.
   only letter left is 'v'.

(shell-command "rm local-alloc.o reg-stack.o recog.o regclass.o reload.o reload1.o")*/

#define REG_CLASS_FROM_LETTER(C)  \
    ((C) == 'd' ? D_REGS :	  \
     (C) == 'k' ? r0r3DREGS :	  \
     (C) == 'b' ? r4r7DREGS :	  \
     (C) == 'c' ? r8r11DREGS :	  \
     (C) == 'l' ? r12r15DREGS :	  \
     (C) == 'w' ? DM_IREGS :	  \
     (C) == 'x' ? DM_MREGS :	  \
     (C) == 'y' ? PM_IREGS :	  \
     (C) == 'z' ? PM_MREGS :	  \
     (C) == 'a' ? PM_BREGS :      \
     (C) == 'e' ? PM_LREGS :      \
     (C) == 'h' ? DM_BREGS :      \
     (C) == 'j' ? DM_LREGS :      \
     (C) == 'u' ? USTAT_REGS :    \
     (C) == 't' ? SYSTEM_REGS :   \
     (C) == 'f' ? MULT_ACC_REGS : \
     NO_REGS)

/*  `HARD_REGNO_NREGS (REGNO, MODE)'
     A C expression for the number of consecutive hard registers,
     starting at register number REGNO, required to hold a value of
     mode MODE.

     On a machine where all registers are exactly one word, a suitable
     definition of this macro is

          #define HARD_REGNO_NREGS(REGNO, MODE)            \
             ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1)  \
              / UNITS_PER_WORD))

   Return number of consecutive hard regs needed starting at reg REGNO
   to hold something of mode MODE.
   This is ordinarily the length in words of a value of mode MODE
   but can be less for certain modes in special long registers.
   On the 21k, ordinary registers hold 32 bits worth;
(shell-command "rm caller-save.o combine.o flow.o function.o global-alloc.o global.o integrate.o jump.o local-alloc.o parallel.o recog.o reload.o reload1.o recog.o rtlanal.o sched.o stupid.o varasm.o") */

#define HARD_REGNO_NREGS(REGNO, MODE)		\
    ((((MODE)==DImode) || (MODE)==DFmode || (MODE) == SCmode || (MODE) == CSImode)\
     ? 2 : 1)

/* Some classes are VERY small */
#define CLASS_LIKELY_SPILLED_P(CLASS)	((CLASS) == USTAT_REGS)

/* Return the maximum number of consecutive registers
   needed to represent mode MODE in a register of class CLASS.  */
#define CLASS_MAX_NREGS(CLASS, MODE)	\
    (((CLASS)==NO_REGS)?0		\
      :((((MODE)==DImode) || (MODE)==DFmode || (MODE) == SCmode || (MODE) == CSImode) \
	     ? 2 : 1))

/* `HARD_REGNO_MODE_OK (REGNO, MODE)'
     A C expression that is nonzero if it is permissible to store a
     value of mode MODE in hard register number REGNO (or in several
     registers starting with that one).  For a machine where all
     registers are equivalent, a suitable definition is

          #define HARD_REGNO_MODE_OK(REGNO, MODE) 1

     It is not necessary for this macro to check for the numbers of
     fixed registers, because the allocation mechanism considers them
     to be always occupied.

     On some machines, double-precision values must be kept in even/odd
     register pairs.  The way to implement that is to define this macro
     to reject odd register numbers for such modes.

     The minimum requirement for a mode to be OK in a register is that
     the `movMODE' instruction pattern support moves between the
     register and any other hard register for which the mode is OK;
     and that moving a value into the register and back out not alter
     it.

     Since the same instruction used to move `SImode' will work for all
     narrower integer modes, it is not necessary on any machine for
     `HARD_REGNO_MODE_OK' to distinguish between these modes, provided
     you define patterns `movhi', etc., to take advantage of this.
     This is useful because of the interaction between
     `HARD_REGNO_MODE_OK' and `MODES_TIEABLE_P'; it is very desirable
     for all integer modes to be tieable.

     Many machines have special registers for floating point
     arithmetic.  Often people assume that floating point machine
     modes are allowed only in floating point registers.  This is not
     true.  Any registers that can hold integers can safely *hold* a
     floating point machine mode, whether or not floating arithmetic
     can be done on it in those registers.  Integer move instructions
     can be used to move the values.

     On some machines, though, the converse is true: fixed-point
     machine modes may not go in floating registers.  This is true if
     the floating registers normalize any value stored in them,
     because storing a non-floating value there would garble it.  In
     this case, `HARD_REGNO_MODE_OK' should reject fixed-point machine
     modes in floating registers.  But if the floating registers do
     not automatically normalize, if you can store any bit pattern in
     one and retrieve it unchanged without a trap, then any machine
     mode may go in a floating register and this macro should say so.

     The primary significance of special floating registers is rather
     that they are the registers acceptable in floating point
     arithmetic instructions.  However, this is of no concern to
     `HARD_REGNO_MODE_OK'.  You handle it by writing the proper
     constraints for those instructions.

     On some machines, the floating registers are especially slow to
     access, so that it is better to store a value in a stack frame
     than in such a register if floating point arithmetic is not being
     done.  As long as the floating registers are not in class
     `GENERAL_REGS', they will not be used unless some pattern's
     constraint asks for one.

   Value is 1 if hard register REGNO can hold a value of machine-mode MODE.
   On the 21k, */
/*(shell-command "rm caller-save.o combine.o emit-rtl.o global-alloc.o local-alloc.o reload.o reload1.o stupid.o")*/

#define HARD_REGNO_MODE_OK(REGNO, MODE) hard_regno_mode_ok(REGNO,MODE)

/* Value is 1 if it is a good idea to tie two pseudo registers
   when one has mode MODE1 and one has mode MODE2.
   If HARD_REGNO_MODE_OK could produce different values for MODE1 and MODE2,
   for any hard reg, then this must be 0 for correct output.
   (shell-command "rm combine.o local-alloc.o regclass.o") <-C-xC-e  */

#define MODES_TIEABLE_P(MODE1, MODE2) ((MODE1==MODE2)||modes_tieable_p(MODE1,MODE2))

/*(shell-command "rm reload.o")*/
#define PREFERRED_RELOAD_CLASS(X,CLASS) preferred_reload_class(X,CLASS)
#define PREFERRED_OUTPUT_RELOAD_CLASS(X,CLASS) \
        preferred_output_reload_class(X,CLASS)

/*(shell-command "rm loop.o regclass.o reload.o")*/
#define REGISTER_MOVE_COST(FROM,TO) register_move_cost(FROM,TO)

/* Any of the 21k index registers m0-m7 */
#define REGNO_OK_FOR_INDEX_P(R,MODE) \
    ((((MODE)==DMmode)&& \
      (IS_DM_MREG(R)||IS_DM_MREG((unsigned)reg_renumber[R])))|| \
     (((MODE)==PMmode)&& \
      (IS_PM_MREG(R)||IS_PM_MREG((unsigned)reg_renumber[R]))))


/* The 21k can use i0-i7 for base registers for cau instructions
 * but only i0-i7 for dau instructions */

#define REGNO_OK_FOR_BASE_P(R,MODE) \
    ((((MODE)==DMmode)&& \
      (IS_DM_IREG(R)||IS_DM_IREG((unsigned)reg_renumber[R])))|| \
     (((MODE)==PMmode)&& \
      (IS_PM_IREG(R)||IS_PM_IREG((unsigned)reg_renumber[R]))))

/* Registers r0-r15 on the 21k */
#define REGNO_OK_FOR_DATA_P(REGNO) \
    (IS_DREG(REGNO) ||\
     IS_DREG((unsigned) reg_renumber[REGNO]))

/* Registers f0-f15 on the 21k are r0 - r15 */
#define REGNO_OK_FOR_FP_P(REGNO) REGNO_OK_FOR_DATA_P(REGNO)

/* Now macros that check whether X is a register and also,
   strictly, whether it is in a specified class.

   These macros are specific to the 21k, and may be used only
   in code for printing assembler insns and in conditions for
   define_optimization.  */
#define DATA_REG_P(X) \
  (REG_P (X) && REGNO_OK_FOR_DATA_P (REGNO (X)))

#define FP_REG_P(X) \
  (REG_P (X) && REGNO_OK_FOR_FP_P (REGNO (X)))

/* Specify the registers used for certain standard purposes.
   The values of these macros are register numbers.  */
/*(shell-command "rm reload.o reload1.o caller-save.o") <-C-xC-e*/
#define  SECONDARY_OUTPUT_RELOAD_CLASS(class,mode,x) \
      secondary_output_reload_class(class,mode,x)
#define  SECONDARY_INPUT_RELOAD_CLASS(class,mode,x) \
      secondary_input_reload_class(class,mode,x)

/* The letters I, J, K, L and M, P in a register constraint string
   can be used to stand for particular ranges of immediate operands.
   This macro defines what the ranges are.
   C is the letter, and VALUE is a constant value.
   Return 1 if VALUE is in the range specified by C.

   For the 21k
   'L' is 8-bit  immediate.
   'J' is 6-bit  immediate.
   'K' is 24-bit immediate.
   'P' is -1,0,1
*/

#define CONST_OK_FOR_LETTER_P(VALUE, C) const_ok_for_letter_p(VALUE, C)

#define CONST_DOUBLE_OK_FOR_LETTER_P(VALUE, C)\
  (((C) == 'G')?                              \
   (GET_CODE (VALUE) == CONST_DOUBLE          \
    && CONST_DOUBLE_LOW (VALUE) == 0x0        \
    && CONST_DOUBLE_HIGH (VALUE) == 0x0)      \
   : ((C) == 'H')? 0 : (abort (),0))

/* Given an rtx X being reloaded into a reg required to be
   in class CLASS, return the class of reg to actually use.
   In general this is just CLASS; but on some machines
   in some cases it is preferable to use a more restrictive class.
   On the 21k, prohibit floating point constants from getting reloaded into
   any registers. Force it to be a memory constant.
 */

#define EXTRA_CONSTRAINT(OP,C) extra_constraint (OP,C)
#define TARGET_EXTRA_CONSTRAINT_LETTERS   case 'A': case 'B': case 'C': case 'D':

#define PREDICATE_CODES \
{"post_modified_operand",	  {POST_DEC,POST_INC}}, 	\
{"reg_or_imm_operand",		  {CONST_INT, SYMBOL_REF, CONST, SUBREG, REG}}, \
{"mr_operand",			  {SUBREG, REG}},		\
{"datreg_or_mr_operand",	  {SUBREG, REG}},		\
{"datreg_or_mr_or_zero_operand",  {SUBREG,REG,CONST_INT}}, 	\
{"datareg_operand",		  {SUBREG, REG}},		\
{"doloop_cc0_p",	  	  {UNSPEC_VOLATILE, REG}}, 	\
{"dreg_or_const_operand", 	  {CONST_INT, SUBREG, REG}}, 	\
{"postmodifiable_memory_operand", {MEM}},			\
{"signed_comparison_operator", 	  {NE,EQ,GE,GT,LE,LT}},		\
{"parallel_move_or_register_operand", {SUBREG, REG, MEM}}, 	\
{"ureg_operand", 		  {SUBREG,REG}},

#define CBUGSUPPORT
#define PUT_SDB_STATIC_DEF(a)			\
( fputs ("\t.def\t", asm_out_file),		\
  output_addr_const (asm_out_file, (a)),	\
  fprintf (asm_out_file, SDB_DELIM))


#define SDB_GENERATE_FAKE(BUFFER, NUMBER) \
  sprintf ((BUFFER), "_%dfake", (NUMBER));

#define SDB_DEBUGGING_INFO
#define PUT_SDB_ENDEF fputs("\t.endef;\n", asm_out_file)
#define PUT_SDB_BLOCK_START(LINE)		\
  fprintf (asm_out_file,			\
	   "\t.def\t.bb%s\t.val\t.%s\t.scl\t100%s\t.line\t%d%s\t.endef;\n", \
	   SDB_DELIM, SDB_DELIM, SDB_DELIM, (LINE), SDB_DELIM)
#define PUT_SDB_BLOCK_END(LINE)			\
  fprintf (asm_out_file,			\
	   "\t.def\t.eb%s\t.val\t.%s\t.scl\t100%s\t.line\t%d%s\t.endef;\n",  \
	   SDB_DELIM, SDB_DELIM, SDB_DELIM, (LINE), SDB_DELIM)
#define PUT_SDB_FUNCTION_START(LINE)		\
  fprintf (asm_out_file,			\
	   "\t.def\t.bf%s\t.val\t.%s\t.scl\t101%s\t.line\t%d%s\t.endef;\n", \
	   SDB_DELIM, SDB_DELIM, SDB_DELIM, (LINE), SDB_DELIM)
#define PUT_SDB_FUNCTION_END(LINE)		\
  fprintf (asm_out_file,			\
	   "\t.def\t.ef%s\t.val\t.%s\t.scl\t101%s\t.line\t%d%s\t.endef;\n", \
	   SDB_DELIM, SDB_DELIM, SDB_DELIM, (LINE), SDB_DELIM)
#define PUT_SDB_EPILOGUE_END(NAME)			\
do { fprintf (asm_out_file, "\t.def\t");		\
     ASM_OUTPUT_LABELREF (asm_out_file, NAME);		\
     fprintf (asm_out_file,				\
	      "%s\t.val\t.%s\t.scl\t-1%s\t.endef;\n",	\
	      SDB_DELIM, SDB_DELIM, SDB_DELIM); } while (0)

#define ASM_OUTPUT_SOURCE_LINE(file,last_linenum)\
	  fprintf (file, "\t.ln\t%d;\n",last_linenum);

#define PUT_SDB_PROLOGUE_END(file, name)	\
  fprintf (file,				\
	   "\t.def end_prologue;\t.val\t.;\t.scl\t109;\t.endef;\n", name)

/* Stack layout; function entry, exit and calling.  */

/* Define this if pushing a word on the stack
   makes the stack pointer a smaller address.  */
#define STACK_GROWS_DOWNWARD

/* Defined to describe what operation is used to perform a stack push. */

#define STACK_PUSH_CODE POST_DEC

/* Define this if the nominal address of the stack frame
   is at the high-address end of the local variables;
   that is, each additional local variable allocated
   goes at a more negative offset in the frame.  */
#define FRAME_GROWS_DOWNWARD

/* Offset within stack frame to start allocating local variables at.
   If FRAME_GROWS_DOWNWARD, this is the offset to the END of the
   first local allocated.  Otherwise, it is the offset to the BEGINNING
   of the first local allocated.  */
#define STARTING_FRAME_OFFSET (CALLING_PROTOCALL_RTRTS?-UNITS_PER_WORD:0)

/* If we generate an insn to push BYTES bytes,
   this says how many the stack pointer really advances by.
   On the 21k , a push MUST BE a long transfer.
   Therefore round to the next word.  */
/*(shell-command "rm calls.o combine.o expr.o function.o jump.o reorg.o") <-C-xC-e*/
#define PUSH_ROUNDING(BYTES) ((((BYTES)+UNITS_PER_WORD-1)/UNITS_PER_WORD ) * UNITS_PER_WORD)


#define STACK_POINTER_OFFSET UNITS_PER_WORD

/* Offset of first parameter from the argument pointer register value.
   NOTE: Normally this would be 8 (to skip the return address and frame
         pointer on the stack. */

#define FIRST_PARM_OFFSET(FNDECL)   UNITS_PER_WORD

/* Value is 1 if returning from a function call automatically
   pops the arguments described by the number-of-args field in the call.
   FUNTYPE is the data type of the function (as a tree),
   or for a library call it is an identifier node for the subroutine name. */

#define RETURN_POPS_ARGS(FUNTYPE, SIZE) 0

/* Define how to find the value returned by a function.
   VALTYPE is the data type of the value (as a tree).
   If the precise function being called is known, FUNC is its FUNCTION_DECL;
   otherwise, FUNC is 0. On the 21k return in 'r0' */

#define VALUE_REGNO(MODE) REG_R0


#define FUNCTION_VALUE(VALTYPE, FUNC)  \
  gen_rtx (REG, TYPE_MODE (VALTYPE), \
           VALUE_REGNO(TYPE_MODE(VALTYPE)))

/* Define how to find the value returned by a library function
   assuming the value has mode MODE.  */

/* On the 21k return floats in 'r0' and fixed point in 'f0' which is one in the same.  */

#define LIBCALL_VALUE(MODE)  gen_rtx (REG, MODE, VALUE_REGNO(MODE))

/* 21k returns everything in r0 */

#define FUNCTION_VALUE_REGNO_P(N) ((N) == REG_R0)


/* 1 if N is a possible register number for function argument passing.
   On the 21k, no registers are used in this way.  */

extern int function_arg_regno();

#define FUNCTION_ARG_REGNO_P(N) function_arg_regno_p(N)

/* Define a data type for recording info about an argument list
   during the scan of that argument list.  This data type should
   hold all necessary information about the function itself
   and about the args processed so far, enough to enable macros
   such as FUNCTION_ARG to determine where the next arg should go.

   On the 21k, this is a single integer, which is is the number
   of arguments scanned so far.  */

#define CUMULATIVE_ARGS int

/* Initialize a variable CUM of type CUMULATIVE_ARGS
   for a call to a function whose data type is FNTYPE.
   For a library call, FNTYPE is 0.

   On the 21k, the offset starts at 0.  */

#define INIT_CUMULATIVE_ARGS(CUM,FNTYPE,LIBNAME)  ((CUM) = 0)

/* Update the data in CUM to advance over an argument
   of mode MODE and data type TYPE.
   (TYPE is null for libcalls where that information may not be available.)  */

#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED)	\
 ((CUM) += ((MODE) != BLKmode			\
	    ? GET_MODE_SIZE (MODE) \
	    : int_size_in_bytes (TYPE) )) /* bytes are long words */


/*
 * It matters whether named parameters are passed correctly in varargs functions
 * e.g., with
 *	extern func (int, int, char *, ...);
 * the first THREE arguments are passed in registers.
 * BUT, there's too much existing object code that expects it on the stack
 * so we can't turn this on now...
 */

   /*#define STRICT_ARGUMENT_NAMING */


/* Define where to put the arguments to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).  */

extern void * function_arg();

#define FUNCTION_ARG(CUM, MODE, TYPE, NAMED) \
	 function_arg(CUM,MODE,TYPE,NAMED)
	   

/* For an arg passed partly in registers and partly in memory,
	 FUNCTION_ARG_PARTIAL_NREGS(CUM, MODE, TYPE, NAMED)
   is the number of registers used.
   We do not define this because gcc puts the stuff in the consecutive
   registers instead of the scratch registers we want, i.e. instead
   of using r4,r8,r12 gcc would use r4,r5,r6 */

/* This macro generates the assembly code for function entry.
   FILE is a stdio stream to output the code to.
   SIZE is an int: how many units of temporary storage to allocate.
   Refer to the array `regs_ever_live' to determine which registers
   to save; `regs_ever_live[I]' is nonzero if register number I
   is ever used in the function.  This macro is responsible for
   knowing which registers should not be saved even if used.
   Defined in out-dsp21k.c  */

#define FUNCTION_PROLOGUE(FILE, SIZE)     function_prologue(FILE, SIZE)

/* Output assembler code to FILE to increment profiler label # LABELNO
   for profiling a function entry.  Ignore on the  for the moment!!! */

#define FUNCTION_PROFILER(FILE, LABELNO)  \
  fprintf (FILE, "\tr0=LP%d;\n\tcall mcount;\n", (LABELNO))

/* Output assembler code to FILE to initialize this source file's
   basic block profiling info, if that has not already been done.
   Ignore on the 21k for the moment !!!!!  */

/*
#define FUNCTION_BLOCK_PROFILER(FILE, LABELNO)  \
  fprintf (FILE, "\ttstl LPBX0\n\tif ne jump LPI%d;\n\t%s(%s,-1)=LPBX0;\n\tjump ___bb_init_func\n\taddql #4,sp\nLPI%d:\n",  \
	   LABELNO, STACK_REF, STACK_POINTER_STRING, LABELNO);    */

/* Output assembler code to FILE to increment the entry-count for
   the BLOCKNO'th basic block in this source file. Ignore on
   the 21k for the moment !!!!!  */

/*
#define BLOCK_PROFILER(FILE, BLOCKNO)	\
  fprintf (FILE, "\taddql #1,LPBX2+%d\n", 4 * BLOCKNO)  */

/* EXIT_IGNORE_STACK should be nonzero if, when returning from a function,
   the stack pointer does not matter.  The value is tested only in
   functions that have frame pointers.
   No definition is equivalent to always zero.  */

#define EXIT_IGNORE_STACK 1

/* This macro generates the assembly code for function exit,
   on machines that need it.  If FUNCTION_EPILOGUE is not defined
   then individual return instructions are generated for each
   return statement.  Args are same as for FUNCTION_PROLOGUE.

   The function epilogue should not depend on the current stack pointer!
   It should use the frame pointer only.  This is mandatory because
   of alloca; we also take advantage of it to omit stack adjustments
   before returning. Defined in out-dsp21k.c */

#define FUNCTION_EPILOGUE(FILE, SIZE)   function_epilogue(FILE, SIZE)

/* This is a hook for other tm files to change.  */
#define FUNCTION_EXTRA_EPILOGUE(FILE, SIZE)

/* If the memory address ADDR is relative to the frame pointer,
   correct it to be relative to the stack pointer instead.
   This is for when we don't use a frame pointer.
   ADDR should be a variable name.  */

#define FIX_FRAME_POINTER_ADDRESS(ADDR,DEPTH)  abort();


/* Register to use for pushing function arguments.  */
#define STACK_POINTER_REGNUM (PMSTACK?REG_I15:REG_I7)

/* Base register for access to local variables of the function.  */

#define FRAME_POINTER_REGNUM (PMSTACK?REG_I14:REG_I6)

/* We have to define two pairs of frame/stack regnums because */
/* ELIMINABLE_REGS is used to initalize static data, and thus */
/* can't use conditionals in STACK_POINTER_REGNUM and */
/* FRAME_POINTER_REGNUM: */

#define ELIMINABLE_REGS {{REG_I6,  REG_I7}, {REG_I14, REG_I15}}

/** INITIAL_ELIMINATION_POINTER_OFFSET and CAN_ELIMINATE **/
/** must be defined if ELIMINABLE_REGS. At this stage we  */
/** define them to disallow register elimination: **/

#define CAN_ELIMINATE(FROM,TO) 0
#define INITIAL_ELIMINATION_OFFSET(FROM,TO,RESULT) 0

/* Base register for access to arguments of the function.  */
#define ARG_POINTER_REGNUM FRAME_POINTER_REGNUM

/* Register in which static-chain is passed to a function.  */
/* Not currently used for C, define it to be anything */
#define STATIC_CHAIN_REGNUM 0

/* Register in which address to store a structure value
   is passed to a function.  */
#define STRUCT_VALUE_REGNUM REG_R1

/* Define this if PCC uses the nonreentrant convention for returning
   structure and union values.
#define PCC_STATIC_STRUCT_RETURN */

#define INITIAL_FRAME_POINTER_OFFSET(DEPTH) {(DEPTH) = 0; }



/* Value should be nonzero if functions must have frame pointers.
   Zero means the frame pointer need not be set up (and parms
   may be accessed via the stack pointer) in functions that seem suitable.
   This is computed in `reload', in reload1.c.  */
#define FRAME_POINTER_REQUIRED 0



/*     STORAGE LAYOUT            */
/* target machine storage layout */

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.
   On the 21k bit field operations are packed from right most bit
   to left most.
 */
#define BITS_BIG_ENDIAN  1


/* Define this if most significant byte of a word is the lowest numbered.
 * We can't access bytes but if we could we would in the Big Endian order.
 */
#define BYTES_BIG_ENDIAN 1

/* Define this if most significant word of a multiword number is numbered.
 * For the 21k there are no machine instructions for them.
 */
#define WORDS_BIG_ENDIAN 1

/* number of bits in an addressable storage unit */

#define BITS_PER_UNIT 32

/* Width in bits of a "word", which is the contents of a machine register.
   Note that this is not necessarily the width of data type `int';
   if using 16-bit ints on a 68000, this would still be 32.
   But on a machine with 16-bit registers, this would be 16.  */
#define BITS_PER_WORD 32

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD 1

/* Width in bits of a pointer.
   See also the macro `Pmode1' defined below.  */
#define POINTER_SIZE 32

/* Allocation boundary (in *bits*) for storing pointers in memory.  */
#define POINTER_BOUNDARY 32

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY 32

/* Boundary (in *bits*) on which stack pointer should be aligned.  */
#define STACK_BOUNDARY 32

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY 32

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY BITS_PER_WORD

/* No data type wants to be aligned rounder than this.  */
#define BIGGEST_ALIGNMENT 32

/* Define this if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT 0

/*(shell-command "rm c-decl.o stor-layout.o")*/
#define PCC_BITFIELD_TYPE_MATTERS 1
/* Define number of bits in most basic integer type.
   (If undefined, default is BITS_PER_WORD).  */



#define CHAR_TYPE_SIZE   (BITS_PER_WORD/UNITS_PER_WORD)
#define SHORT_TYPE_SIZE	 (UNITS_PER_WORD==1?32:16)
#define INT_TYPE_SIZE    32
#define LONG_TYPE_SIZE   32
#define FLOAT_TYPE_SIZE  32
#define DOUBLE_TYPE_SIZE (DOUBLE_IS_32?32:64)
#define LONG_DOUBLE_TYPE_SIZE (DOUBLE_IS_32?32:64)
#define LONG_LONG_TYPE_SIZE 64

#define UNITS_PER_FLOAT  ((FLOAT_TYPE_SIZE  + BITS_PER_UNIT - 1) / \
			   BITS_PER_UNIT)

#define UNITS_PER_DOUBLE ((DOUBLE_TYPE_SIZE + BITS_PER_UNIT - 1) / \
 			   BITS_PER_UNIT)


/* what is the 'type' of size_t */
#define SIZE_TYPE "long unsigned int"

/* Define this as 1 if `char' should by default be signed; else as 0.  */
#define DEFAULT_SIGNED_CHAR 1

#define DOUBLES_ARE_FLOATS 1

#define REAL_VALUE_QUANTIZE(x0,x1) real_value_quantize(&x0,&x1)


/******* KLUDGES: the following 4 macros are there just to get 2.00 compile ***/
/*******  I (az) have no idea at this point (12/11/91) what should really ****/
/*******  be in their place ***/

#define TRAMPOLINE_SIZE 10
#define INITIALIZE_TRAMPOLINE(a,b,c)

#define TRAMPOLINE_TEMPLATE(file) \
  fprintf(file, "! -------- Trampoline Template (TBD) ---------\n");


#ifdef MEMSEG

#define MEMSEG_TO_MODE(MS) (((MS)==PROG_MEM)?PMmode:\
			    ((MS)==DATA_MEM)?DMmode:VOIDmode())

#define MEMSEG_NAME(MS) \
  ((MS)==DATA_MEM    ? "DM" : \
   (MS)==PROG_MEM    ? "PM" : \
   (MS)==UNKNOWN_MEM ? "UNKNOWN_MEM" : \
   "!!! ILLEGAL_MEM !!!")

#define LEGAL_MEMSEG(MS) (((MS)==DATA_MEM) || ((MS)==PROG_MEM) || \
			  ((MS)==UNKNOWN_MEM))

#define CHECK_MEMSEG(MS) {if(!LEGAL_MEMSEG(MS)abort());}


typedef enum {UNKNOWN_MEM, DATA_MEM, PROG_MEM} memory_segment;

#endif /* MEMSEG */

#define SELECT_SECTION(T,R) select_section(T,R)

/*** Memory segment where automatic and undeclared static variables go **/

extern memory_segment default_memory_segment;

/*** Default machine mode for pointers -- must correspond **/
/***  to default_memory_segment ***/

extern enum machine_mode default_pointer_mode;
extern memory_segment code_memory_segment;

#define DEFAULT_STRENGTH_REDUCTION_BENEFIT 1

/*  (shell-command "rm -f c-lex.o")  */
#define HANDLE_PRAGMA(stream) handle_pragma(stream)

/*(shell-command "rm -f varasm.o")*/
#define POLYMORPH_LIBRARY_CALLS(FUNEXP,DECL)\
{ rtx polymorph_library_calls();            \
    FUNEXP = polymorph_library_calls (FUNEXP,DECL); }

/*  (shell-command "rm -f global-alloc.o local-alloc.o regclass.o reload1.o stupid.o aux-output.o caller-save.o cse.o debug-prints.o explow.o final.o flow.o function.o genoutput.o  insn-output.o jump.o loop.o recog.o reg-stack.o reload.o reorg.o sched.o stmt.o varasm.o")  */
#define REG_ALLOC_ORDER {                        \
    REG_R2, REG_R4, REG_R8, REG_R12, 		 \
    REG_I4, REG_M4, REG_I12, REG_M12,		 \
    REG_R1, REG_R5, REG_R9, REG_R13, 		 \
    REG_R3, REG_R7, REG_R11, REG_R15,		 \
    REG_R0, REG_R6, REG_R10, REG_R14,		 \
    REG_I0, REG_M0, REG_I1, REG_M1, 		 \
    REG_I2, REG_M2, REG_I3, REG_M3, REG_I5,	 \
    REG_I8, REG_M8, REG_I9, REG_M9, 		 \
    REG_I10, REG_M10, REG_I11, REG_M11, REG_I13, \
    REG_I14, REG_I15, 				 \
    REG_MRF, REG_MRB, REG_USTAT1, REG_USTAT2,	 \
    FIRST_PSEUDO_REGISTER }

#define PEEP_PARALLEL 1

/* (shell-command "rm -f flow.o") */
#define CONST1_OK_FOR_AUTO_MODIFY(N) ((-32<=(N))&&((N)<=31))

#define CONST_OK_FOR_AUTO_MODIFY(X)   CONST1_OK_FOR_AUTO_MODIFY(\
				     GET_CODE (X) == CONST\
				      ? INTVAL (XEXP (X, 0))\
				      : INTVAL (X))

#endif /* ACH_PARSE */

extern enum machine_mode stack_pmode;


#endif /* ADSP21K_H */

#define ADI_PROFILER

#define IS_MAC_OPERATOR(insn) (is_mac_operator(insn))
