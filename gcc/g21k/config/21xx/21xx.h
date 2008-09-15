/* Definitions of target machine for GNU compiler.  

   Analog Devices
   ADSP 21XX
   Dev. Tools Team


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

#ifndef ADSP21_H
#define ADSP21_H

#include <stdio.h>


#define DSP21XX
#define ADI
#define MEMSEG
#define QImode SImode
#define ADI_ARG(x) ,x


#define SIGNED_POINTERS 0

/*   A number, the maximum number of registers that can appear in a
     valid memory address.  Note that it is up to you to specify a
     value equal to the maximum number that `GO_IF_LEGITIMATE_ADDRESS'
     would ever accept. 
*/
#define MAX_REGS_PER_ADDRESS 2

/* Macros to check register numbers against specific register classes.  */

/* These assume that REGNO is a hard or pseudo reg number.
   They give nonzero only if REGNO is a hard reg of the suitable class
   or a pseudo reg currently allocated to a suitable hard reg.
   Since they use reg_renumber, they are safe only once reg_renumber
   has been allocated, which happens in local-alloc.c.  */


/* Recognize any constant value that is a valid address.  */

#define CONSTANT_ADDRESS_P(X)  (CONSTANT_P (X) ||\
	((GET_CODE(X)==PLUS || GET_CODE(X) == MINUS) &&\
	(CONSTANT_P(XEXP(X,0))) && (CONSTANT_P(XEXP(X,1)))))

/* Nonzero if the constant value X is a legitimate general operand.
   It is given that X satisfies CONSTANT_P or is a CONST_DOUBLE.  */

#define LEGITIMATE_CONSTANT_P(X) 1

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

#define REG_OK_FOR_INDEX_STRICT_P(X)     (0)
#define REG_OK_FOR_INDEX_NON_STRICT_P(X) (0)

/* (shell-command "rm  aux-output.o reload.o") */
#define REG_OK_FOR_BASE_STRICT_P(x,mode) \
  ((IS_IREG(REGNO(x))&&mode==DMmode)||(IS_DAG2_IREG(REGNO(x))&&mode==PMmode))
#define REG_OK_FOR_BASE_NON_STRICT_P(x,mode) \
  (IS_PSEUDO(REGNO(x))||(IS_IREG(REGNO(x))&&mode==DMmode)||(IS_DAG2_IREG(REGNO(x))&&mode==PMmode))


#define REG_OK_FOR_MODIFY_STRICT_P(X)    (IS_MREG(REGNO(X)))
#define REG_OK_FOR_MODIFY_NON_STRICT_P(X)    (IS_MREG(REGNO(X))||IS_PSEUDO(REGNO(X)))

#ifndef REG_OK_STRICT

/* Nonzero if X is a hard reg that can be used as an index
   or if it is a pseudo reg.  */
#define REG_OK_FOR_INDEX_P(X,MODE) REG_OK_FOR_INDEX_NON_STRICT_P(X)

/* Nonzero if X is a hard reg that can be used as a base reg
   or if it is a pseudo reg.  */
#define REG_OK_FOR_BASE_P(X,MODE)  REG_OK_FOR_BASE_NON_STRICT_P(X,MODE)

#define REG_OK_FOR_MODIFY_P(X,MODE)  REG_OK_FOR_MODIFY_NON_STRICT_P(X)

#else

/* Nonzero if X is a hard reg that can be used as an index.  */
#define REG_OK_FOR_INDEX_P(X,MODE) REG_OK_FOR_INDEX_STRICT_P(X) 

/* Nonzero if X is a hard reg that can be used as a base reg.  */
#define REG_OK_FOR_BASE_P(X,MODE)  REG_OK_FOR_BASE_STRICT_P(X,MODE)

#define REG_OK_FOR_MODIFY_P(X,MODE)  REG_OK_FOR_MODIFY_STRICT_P(X)

#endif

#define SWITCH_SUBREG(X) {while(GET_CODE(X)==SUBREG) X=SUBREG_REG(X);}

/* GO_IF_LEGITIMATE_ADDRESS recognizes an RTL expression
   that is a valid memory address for an instruction.
   The MODE argument is the machine mode for the MEM expression
   that wants to use this address.
  
        any constant including a symbol_ref
	ireg
	(post_inc ireg)
	(post_dec ireg)
*/

#define CONST_INT_P(X) (GET_CODE(X)==CONST_INT)

/* (shell-command "rm  aux-output.o") */

#define POST_MODIFY_P(X) (GET_CODE (X) == POST_INC || \
			  GET_CODE (X) == POST_DEC || \
			  GET_CODE (X) == POST_MODIFY)

/*
`GO_IF_LEGITIMATE_ADDRESS (MODE, X, LABEL)'
     A C compound statement with a conditional `goto LABEL;' executed
     if X (an RTX) is a legitimate memory address on the target
     machine for a memory operand of mode MODE.

     It usually pays to define several simpler macros to serve as
     subroutines for this one.  Otherwise it may be too complicated to
     understand.

     This macro must exist in two variants: a strict variant and a
     non-strict one.  The strict variant is used in the reload pass. 
     It must be defined so that any pseudo-register that has not been
     allocated a hard register is considered a memory reference.  In
     contexts where some kind of register is required, a
     pseudo-register with no hard register must be rejected.

     The non-strict variant is used in other passes.  It must be
     defined to accept all pseudo-registers in every context where
     some kind of register is required.

     Compiler source files that want to use the strict variant of this
     macro define the macro `REG_OK_STRICT'.  You should use an
     `#ifdef REG_OK_STRICT' conditional to define the strict variant
     in that case and the non-strict variant otherwise.

     Typically among the subroutines used to define
     `GO_IF_LEGITIMATE_ADDRESS' are subroutines to check for
     acceptable registers for various purposes (one for base
     registers, one for index registers, and so on).  Then only these
     subroutine macros need have two variants; the higher levels of
     macros may be the same whether strict or not.

     Normally, constant addresses which are the sum of a `symbol_ref'
     and an integer are stored inside a `const' RTX to mark them as
     constant.  Therefore, there is no need to recognize such sums
     specifically as legitimate addresses.  Normally you would simply
     recognize any `const' as legitimate.

     Usually `PRINT_OPERAND_ADDRESS' is not prepared to handle constant
     sums that are not marked with  `const'.  It assumes that a naked
     `plus' indicates indexing.  If so, then you *must* reject such
     naked constant sums as illegitimate addresses, so that none of
     them will be given to `PRINT_OPERAND_ADDRESS'.

     On some machines, whether a symbolic address is legitimate
     depends on the section that the address refers to.  On these
     machines, define the macro `ENCODE_SECTION_INFO' to store the
     information into the `symbol_ref', and then check for it here. 
     When you see a `const', you will have to look inside it to find
     the `symbol_ref' in order to determine the section.  *Note
     Assembler Format::.

     The best way to modify the name string is by adding text to the
     beginning, with suitable punctuation to prevent any ambiguity. 
     Allocate the new name in `saveable_obstack'.  You will have to
     modify `ASM_OUTPUT_LABELREF' to remove and decode the added text
     and output the name accordingly.

     You can check the information stored here into the `symbol_ref' in
     the definitions of `GO_IF_LEGITIMATE_ADDRESS' and
     `PRINT_OPERAND_ADDRESS'.*/

/* if changed recompile: explow.c recog.c reload.c */
#ifdef REG_OK_STRICT
#define GO_IF_LEGITIMATE_ADDRESS(MODE, X, WIN) \
	if(go_if_legitimate_address_strict(MODE,X)) goto WIN
#else
#define GO_IF_LEGITIMATE_ADDRESS(MODE, X, WIN) \
	if(go_if_legitimate_address_non_strict(MODE,X)) goto WIN
#endif

/* Try machine-dependent ways of modifying an illegitimate address
   to be legitimate.  If we find one, return the new, valid address.
   This macro is used in only one place: `memory_address' in explow.c.

   OLDX is the address as it was before break_out_memory_refs was called.
   In some cases it is useful to look at this to decide what needs to be done.

   MODE and WIN are passed so that this macro can use
   GO_IF_LEGITIMATE_ADDRESS.

   It is always safe for this macro to do nothing.  It exists to recognize
   opportunities to optimize the output.
	*/

#define LEGITIMIZE_ADDRESS(X,OLDX,MODE,WIN) if(legitimize_address(&X,MODE)) goto WIN


/* Go to LABEL if ADDR (a legitimate address expression)
   has an effect that depends on the machine mode it is used for. */

/* (shell-command "rm -f cse.o expr.o flow.o") */

#if 0
#define HAVE_POST_INCREMENT
#define HAVE_POST_DECREMENT 
#endif

#define HAVE_POST_MODIFY

#define GO_IF_MODE_DEPENDENT_ADDRESS(ADDR,LABEL)  \
 if (GET_CODE (ADDR) == POST_INC || GET_CODE (ADDR) == POST_DEC) goto LABEL 


/* Control the predefined macros to the preprocessor */

#define CPP_PREDEFINES " "
#define ASM_SPEC "-c -s"

/* Control the assembler format that we output.  */

/* Output at beginning of assembler file.  */
#define ASM_FILE_START(file) asm_file_start(file)

#define ASM_FILE_END(file) asm_file_end(file)


#define ASM_IDENTIFY_GCC(FILE)   \
  fprintf (FILE, "!gcc_compiled\n");
/* Output to assembler file text saying following lines
   may contain character constants, extra white space, comments, etc.  */

#define ASM_APP_ON "!APP\n"

/* Output to assembler file text saying following lines
   no longer contain unusual constructs.  */

#define ASM_APP_OFF "!NO_APP\n"

/* Output before read-only data.  */

#define TEXT_SECTION_ASM_OP "\n"

/* Output before writable data.  */

#define DATA_SECTION_ASM_OP "\n"

#define EXTRA_SECTIONS in_pmda_section

extern char *pick_segment_name();
extern char *pick_section();

#define SELECT_SECTION(T,R) select_section(T,R)

#define READONLY_DATA_SECTION data_section

/* How to renumber registers for dbx and gdb. */

#define DBX_REGISTER_NUMBER(REGNO) ((REGNO))

/* This is how to output the definition of a user-level label named NAME,
   such as the label on a static function or variable NAME.  */

#define ASM_OUTPUT_LABEL(FILE,NAME)    asm_output_label(FILE,NAME)


/* This is how to output a command to make the user-level label named NAME
   defined for reference from other files.  */

#define ASM_GLOBALIZE_LABEL(FILE,NAME)	asm_globalize_label(FILE,NAME)


/* This is how to output a reference to a user-level label named NAME.
   `assemble_name' uses this.  */

#define ASM_OUTPUT_LABELREF(FILE,NAME)	asm_output_labelref(FILE,NAME)

/* This is how to output an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.  */

#define ASM_OUTPUT_INTERNAL_LABEL(FILE,PREFIX,NUM) asm_output_internal_label(FILE,PREFIX,NUM)


/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#define ASM_GENERATE_INTERNAL_LABEL(LABEL,PREFIX,NUM) asm_generate_internal_label(LABEL,PREFIX,NUM)

/* This is how to output an assembler line defining a `double' constant.  */

#define ASM_OUTPUT_DOUBLE(FILE,VALUE)  asm_output_double(FILE,VALUE)


/* This is how to output an assembler line defining a `float' constant.  */

#define ASM_OUTPUT_FLOAT(FILE,VALUE)  asm_output_float(FILE,VALUE)


/* Likewise for `char' and `short' constants.  */

#define ASM_OUTPUT_SHORT(FILE,VALUE) asm_output_short(FILE,VALUE)


#define ASM_OUTPUT_CHAR(FILE,VALUE)  asm_output_char(FILE,VALUE)

#define ASM_OUTPUT_BYTE(FILE,VALUE)  asm_output_byte(FILE,VALUE)

#define ASM_OUTPUT_ASCII(FILE,PTR,LEN) asm_output_ascii(FILE,PTR,LEN)

/* This is how to output an insn to push a register on the stack.
   It need not be very fast code since it is used only for profiling  */

#define ASM_OUTPUT_REG_PUSH(FILE,REGNO)  abort()

/* This is how to output an insn to pop a register from the stack.
   It need not be very fast code since it is used only for profiling  */

#define ASM_OUTPUT_REG_POP(FILE,REGNO)  abort()

/* This is how to output an element of a case-vector that is absolute. */

#define ASM_OUTPUT_ADDR_VEC_ELT(FILE, VALUE)  asm_output_addr_vec_elt(FILE,VALUE)


/* This is how to output an element of a case-vector that is relative.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, VALUE, REL) asm_output_addr_diff_elt(FILE, VALUE, REL)

/* This is a "prologue" for "switch" */

#define ASM_OUTPUT_CASE_LABEL(file,pr,num,tab) asm_output_case_label(file,pr,num,tab)

/* This is an "epiloge" for "switch" */

#define ASM_OUTPUT_CASE_END(file,num,tab) asm_output_case_end(file,num,tab)

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */

#define ASM_OUTPUT_ALIGN(FILE,LOG)  asm_output_align(FILE,LOG)

#define ASM_OUTPUT_SKIP(FILE,SIZE) asm_output_skip(FILE,SIZE) 


/* This says how to output an assembler line
   to define a global common symbol.  */

#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)  generate_global_varibale(FILE,NAME,SIZE,ROUNDED)
	
/* This says how to output an assembler line
   to define a local common symbol.  */

#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE, ROUNDED)   generate_local_varibale(FILE,NAME,SIZE,ROUNDED)


#define ASM_OUTPUT_POOL_PROLOGUE(FILE,FUNNAME,FUNDECL,SIZE) \
  if(SIZE>0) fprintf(FILE,"! Constant pool for function \"%s\", size: %d\n",FUNNAME,SIZE)

/* This one is specific to 21xx, it outputs a constant (as in .CONST) to
the output file.  Checkout varasm.c to see where it is used! */

#define ASM_OUTPUT_POOL_LABEL(FILE,LABEL_PREFIX,LABEL_NO) \
  fprintf(FILE,".var/DM/ROM\t%s%d_;\n\t.init\t%s%d_:",LABEL_PREFIX,LABEL_NO,LABEL_PREFIX,LABEL_NO)




/* Store in OUTPUT a string (made with alloca) containing
   an assembler-name for a local static variable named NAME.
   LABELNO is an integer which is different for each call.  */

#define ASM_FORMAT_PRIVATE_NAME(OUTPUT, NAME, LABELNO) \
    ( (OUTPUT) = (char *) alloca (strlen ((NAME)) + 10),	\
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

#define PRINT_OPERAND(FILE, X, CODE)  print_operand(FILE,X,CODE);

/* Print a memory address as an operand to reference that memory location.
   Defined in out-dsp21k.c */

#define PRINT_OPERAND_ADDRESS(FILE, ADDR) print_operand_address(FILE,ADDR)

	/*** Print a symbol separating elements of an array or ***/
	/*** a record ***/

#define ASM_OUTPUT_ELT_SEP(file) asm_output_elt_sep(file)

	/*** Print out the size of an initialized array. ***/
	/*** It normaly follows the name of the array    ***/

#define ASM_OUTPUT_INIT_ASSIGN(file) asm_output_init_assign(file) 


	/**** For emiting ".var" statements: ********/

#define ASM_OUTPUT_VAR_LABEL(file,name) asm_output_var_label(file,name)

#define ASM_OUTPUT_DECL_END(file) fprintf(file,";\n");

#define ASM_OUTPUT_EXTERNAL(stream,decl,name) asm_output_external(stream,decl,name)

#define ASM_OUTPUT_EXTERNAL_LIBCALL(stream,symref) asm_output_external_libcall(stream,symref)

#define RET  return ""        /* Used in machine description */

#define ASM_DECLARE_OBJECT_NAME(file,name,decl) asm_declare_object_name(file,name,decl)

/* How to refer to registers in assembler output.
   This sequence is indexed by compiler's hard-register-number (see above).  */

/* (shell-command "rm print_rtl.o regclass.o rtl.o varasm.o") */

#define REGISTER_NAMES \
{ "ax0", "ax1", "ay0", "ay1", "mx0", "mx1", "my0", "my1", \
  "mr2", "mr1", "mr0", "mf",  "si",  "se",  "sr1", "sr0", \
  "ar" , "sb",  "px",  "cntr","astat","mstat","stat","imask", \
  "icntl", "tx0", "tx1", "rx0", "rx1", "ifc", "mr", "sr", \
 "i0", "i1", "i2", "i3",  "i4",  "i5",  "i6",  "i7",    \
 "m0", "m1", "m2", "m3",  "m4",  "m5",  "m6",  "m7",    \
 "l0", "l1", "l2", "l3",  "l4",  "l5",  "l6",  "l7", \
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

#define CC_IN_FPCOND  04000

#define NOTICE_UPDATE_CC(EXP, INSN) notice_update_cc( (EXP))


/* Compute the cost of computing a constant rtl expression RTX
   whose rtx-code is CODE.  The body of this macro is a portion
   of a switch statement.  If the code is computed here,
   return it with a return statement.  Otherwise, break from the switch. */


#if 1

/* Try to discourage cse to allocate registers for constants */
/* (az 1/24/94) */

#define CONST_COSTS(RTX,CODE,OUTER_CODE) \
  case CONST_INT:						\
  case CONST:							\
  case LABEL_REF:						\
  case SYMBOL_REF:						\
  case CONST_DOUBLE:						\
    return 0;

#else

#define CONST_COSTS(RTX,CODE,OUTER_CODE) \
  case CONST_INT:						\
    return 1;     \
  case CONST:							\
  case LABEL_REF:						\
  case SYMBOL_REF:						\
    return 1;							\
  case CONST_DOUBLE:						\
    return 2;

#endif

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

#define RTX_COSTS(x, code,outer_code) 				\
  case MULT: total = 2; break; 				\
  case DIV: if (GET_MODE(x) == SImode 			\
		&& GET_CODE (XEXP(x,1)) == CONST_INT 	\
		&& INTVAL (XEXP(x,1))==2) total =2;     \
  break;\
 case ASHIFT: case ASHIFTRT: case LSHIFT: case LSHIFTRT: \
  if (CONST_INT_P(x)) total=2; else total =4; \
  break;
    

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

#define DOUBLE_IS_32     1

/* Run-time compilation parameters selecting different hardware subsets.  */

extern int target_flags;

/* Macros used in the machine description to test the flags.  */

/* Compile passing first two args in regs 0 and 1.
   This exists only to test compiler features that will
   be needed for RISC chips.  It is not usable
   and is not intended to be usable on this cpu.  */

#define DEBUGING       		(target_flags &   0x1)
#define KEEP_STAT       	(target_flags &   0x2)
#define PMSTACK                 (target_flags &   0x4)
#define DMSTACK                 (!(target_flags & 0x4))
#define LISTING_WITH_SOURCE     (target_flags &   0x8)

#define NODOLOOPS		(target_flags &  0x10)

#define RE_RECOG_FLOW		 (target_flags  &    0x20)
#define RE_RECOG_RELOAD		 (target_flags  &    0x40)
#define PROLOG_INLINE          (!(target_flags  &   0x100))
#define NOCSE			 (target_flags  &   0x200)
#define NOSCHEDULER		 (target_flags  &   0x400)
#define NOINIT                   (target_flags  &   0x800)
#define KILL_REGS		(!(target_flags &  0x1000))
#define NOCOMBINE		(!(target_flags &  0x2000))
#define STATIC_SPILL		 (target_flags  &  0x4000)
#define ROM                      (target_flags  &  0x8000)
#define DIV_INLINE              (!(target_flags & 0x10000))
#define INLINE_DOUBLE             (target_flags & 0x20000)
#define TARGET_2100               (target_flags & 0x40000)

/* Macro to define tables used to set the flags.
   This is a list in braces of pairs in braces,
   each pair being { "NAME", VALUE }
   where VALUE is the bits to set or minus the bits to clear.
   An empty string NAME is used to identify the default VALUE.  */

/* (shell-command "rm -f toplev.o") */

#define TARGET_DEFAULT   0x0

#define TARGET_SWITCHES {\
  	{ "debug" ,         1},    \
	{ "stat" ,          2},    \
	{ "pmstack", 	    4},    \
	{ "listm",          8},    \
	{ "no-doloops",    16},    \
	{ "called-prolog", 0x100}, \
	{ "no-cse",        0x200}, \
	{ "no-scheduler",  0x400}, \
        { "no-init",       0x800},    \
	{ "no-kill-regs", 0x1000}, \
	{ "full-combine", 0x2000}, \
	{ "static-spill", 0x4000}, \
	{ "rom",          0x8000}, \
	{ "called-div",  0x10000}, \
	{ "small-code",  0x10100},    /* collection of options \
					 to gen smaller (slower) code */ \
        { "inline-double", 0x20000}, \
        { "2100", 0x40000}, \
        { "", TARGET_DEFAULT}}

extern char *arch_reserved_registers,
	    *boot_page_number,
	    *pmcode_segment,
	    *pmdata_segment,
	    *dmdata_segment,
            *reg_cost_string,
            *runhdr_string;

extern int register_cost;

#define TARGET_OPTIONS {\
	{ "reserved=", &arch_reserved_registers }, \
        { "boot-page=",&boot_page_number }, \
	{ "pmcode=",   &pmcode_segment   }, \
	{ "pmdata=",   &pmdata_segment   }, \
	{ "dmdata=",   &dmdata_segment   }, \
	{ "reg-cost=", &reg_cost_string  }, \
	{ "runhdr=",   &runhdr_string    }} /* runhdr not really used by compiler */

#define SWITCH_TAKES_ARG(CHAR)      \
  ((CHAR) == 'D' || (CHAR) == 'U' || (CHAR) == 'o' \
   || (CHAR) == 'e' || (CHAR) == 'T' || (CHAR) == 'u' \
   || (CHAR) == 'I' || (CHAR) == 'm' \
   || (CHAR) == 'L' || (CHAR) == 'A')

#define WORD_SWITCH_TAKES_ARG(STR)			\
 (!strcmp (STR, "Tdata") || !strcmp (STR, "include")	\
  || !strcmp (STR, "arch")                              \
  || !strcmp (STR, "imacros"))


#define TARGET_VERSION fprintf (stderr, " (adsp21xx 5.01):");

#define DEFAULT_TARGET_MACHINE "21xx"

#define CROSS_COMPILER

#define ADII                   "ADII_21"
#define USE_ADI_DSP            "ADI_DSP"

#ifndef __MSDOS__
#define INCLUDE_FILES_LOCATION DEFAULT_TARGET_MACHINE ## "/include"
#define CC1_LOCATION           DEFAULT_TARGET_MACHINE ## "/etc/"
#define LIB_LOCATION           DEFAULT_TARGET_MACHINE ## "/lib/"
#else /* __MSDOS__ */
#define INCLUDE_FILES_LOCATION DEFAULT_TARGET_MACHINE  "\\include"
#define CC1_LOCATION           DEFAULT_TARGET_MACHINE  "\\etc\\"
#define LIB_LOCATION           DEFAULT_TARGET_MACHINE  "\\lib\\"
#endif /* __MSDOS__ */

#define STANDARD_EXEC_PREFIX   0
#define STANDARD_STARTFILE_PREFIX 0
#define STANDARD_STARTFILE_PREFIX_1 0
#define STANDARD_STARTFILE_PREFIX_2 0
#define STANDARD_EXEC_PREFIX_1 0

#define INCLUDE_DEFAULTS { 0 , 0 };
#ifdef __MSDOS__
#define EXECUTABLE_SUFFIX ".exe"
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR  '\\'
#endif /* __MSDOS__ */

#define TARGET_MEM_FUNCTIONS

/* Specify the machine mode that this machine uses
   for the index in the tablejump instruction.  */

#define CASE_VECTOR_MODE PMmode

#define JUMP_TABLES_IN_TEXT_SECTION


/* Define this if the tablejump instruction expects the table
   to contain offsets from the address of the table.
   Do not define this if the table should contain absolute addresses.  */
/* #define CASE_VECTOR_PC_RELATIVE  */

/* Specify the tree operation to be used to convert reals to integers.  */
#define IMPLICIT_FIX_EXPR FIX_ROUND_EXPR

/* This is the kind of divide that is easiest to do in the general case.  */
#define EASY_DIV_EXPR TRUNC_DIV_EXPR

/* Max number of bytes we can move from memory to memory
   in one reasonably fast instruction.  */
#define MOVE_MAX 1

/* Define if shifts truncate the shift count
   which implies one can omit a sign-extension or zero-extension
   of a shift count.  */
#define SHIFT_COUNT_TRUNCATED

/* Value is 1 if truncating an integer of INPREC bits to OUTPREC bits
   is done just by pretending it is already truncated.  */
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1

/* We assume that the store-condition-codes instructions store 0 for false
   and some other value for true.  This is the value stored for true.  */
#define STORE_FLAG_VALUE  1

/* When a prototype says `char' or `short', really pass an `int'.  */
#define PROMOTE_PROTOTYPES 

/* Specify the machine mode that pointers have.
   After generation of rtl, the compiler makes no further distinction
   between pointers and any other objects of this machine mode.  */


typedef enum {UNKNOWN_MEM, DATA_MEM, PROG_MEM} memory_segment;


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
extern memory_segment default_memory_segment;

#define DEFAULT_CODE_Pmode PMmode
#define DEFAULT_DATA_Pmode DMmode

#define Extract_mode_Pmode(rtx,mode) extract_mode_pmode(rtx,mode)
#define Extract_tree_Pmode(tree)     extract_tree_pmode(tree)
#define Extract_rtx_Pmode(rtx)       extract_rtx_pmode(rtx)
#define Pmode1(X) (X)
#define PMODE_P(X) (((X)==PMmode)||((X)==DMmode))


/*(shell-command "rm calls.o expr.o tree.o varasm.o")*/
/* A function address in a call instruction
   is a byte address (for indexing purposes)
   so give the MEM rtx a byte's mode.  */
#define FUNCTION_MODE PMmode


#define DEFAULT_TABLE_Pmode DMmode


/* STANDARD REGISTER USAGE.  */

/* Number of actual hardware registers.
   The hardware registers are assigned numbers for the compiler
   from 0 to just below FIRST_PSEUDO_REGISTER.
   All registers that the compiler knows about must be given numbers,
   even those that are not normally considered general registers. */


#define REG_AX0 0
#define REG_AX1 1
#define REG_AY0 2
#define REG_AY1 3
#define REG_MX0 4
#define REG_MX1 5
#define REG_MY0 6
#define REG_MY1 7
#define REG_MR2 8
#define REG_MR1 9
#define REG_MR0 10
#define REG_MF  11
#define REG_SI	12
#define REG_SE	13
#define REG_SR1	14
#define REG_SR0 15
#define REG_AR  16
#define REG_SB  17
#define REG_PX  18
#define REG_CNTR 19
#define REG_ASTAT 20
#define REG_MSTAT 21
#define REG_SSTAT 22
#define REG_IMASK 23
#define REG_ICNTL 24
#define REG_TX0   25
#define REG_TX1   26
#define REG_RX0   27
#define REG_RX1   28
#define REG_IFC   29
#define REG_MR	  30
#define REG_SR	  31
#define REG_I0  32
#define REG_I1  33
#define REG_I2  34
#define REG_I3  35
#define REG_I4  36
#define REG_I5  37
#define REG_I6  38
#define REG_I7  39

#define REG_M0  40
#define REG_M1  41
#define REG_M2  42 
#define REG_M3  43 
#define REG_M4  44 
#define REG_M5  45
#define REG_M6  46 
#define REG_M7  47 

#define REG_L0  48
#define REG_L1  49 
#define REG_L2  50 
#define REG_L3  51 
#define REG_L4  52 
#define REG_L5  53
#define REG_L6  54 
#define REG_L7  55 

#if 0
do not use it yet.
/* Carry bit as a separate registers. Even so we consider it to be
 an SImode register, it should not be a part of any class except of its own,
so it is not allocated any values. */

#define REG_CARRY 57
#endif

#define LAST_REG 56
#define FIRST_PSEUDO_REGISTER (LAST_REG+1)


/* these registers are hardwired! */
/* #define PM_MINUS1_MREG   REG_M7*/
#define PM_ZERO_MREG     REG_M6
#define DM_ZERO_MREG     REG_M2
/* #define DM_MINUS1_MREG   REG_M3
#define DM_PLUS1_MREG    REG_M1 */
#define SCRATCH_DM_MREG  REG_M0
#define SCRATCH_PM_MREG	 REG_M5
#define SCRATCH_PM_IREG  REG_I6
#define SCRATCH_DREG     REG_SI

/*

	m4 	frame pointer
	m1 	+1
	m7, m3 	-1
	m0	0
	l0-l7   0
	i4	stack pointer

   (shell-command "rm regclass.o") <-C-xC-e

 */
#define FIXED_REGISTERS                     	\
  {					    	\
/* ax0, ax1, ay0, ay1, mx0, mx1, my0, my1 */	\
     0,   0,   0,   0,   0,   0,   0,   0,	\
/* mr2, mr1, mr0,  mf,  si,  se, sr1, sr0 */	\
     0,   0,   0,   0,   0,   0,   0,   0,	\
/*  ar,  sb,  px,cntr,astat,mstat,sstat,imask */\
     0,   0,   1,   1,   1,   1,   1,   1,	\
/*icntl,tx0, tx1, rx0, rx1, ifc,  mr,  sr,*/	\
     1,   1,   1,   1,   1,   1,   0,	0,	\
/*  i0,  i1,  i2,  i3,  i4,  i5,  i6,  i7 */	\
     0,   0,   0,   0,   1,   0,   0,   0,	\
/*  m0,  m1,  m2,  m3,  m4,  m5,  m6,  m7 */	\
     0,   1,   1,   0,   1,   0,   1,   0,	\
/*  l0,  l1,  l2,  l3,  l4,  l5,  l6,  l7 */	\
     1,   1,   1,   1,   1,   1,   1,   1, 	\
  }




#define IS_PSEUDO(REG) ((REG)>=FIRST_PSEUDO_REGISTER)

/* Testing hard-reg-set with pseudo-register numbers is generally */
/* unpredictable */

#define IS_DREG(REG)	(((REG)<FIRST_PSEUDO_REGISTER)&& \
			 TEST_HARD_REG_BIT(reg_class_contents[D_REGS],(REG)))
#define IS_IREG(REG)   		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DM_IREGS],(REG)))
#define IS_MREG(REG)   		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DM_MREGS],(REG)))
#define IS_DAG1(REG)   		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG1_REGS],(REG)))
#define IS_DAG1_IREG(REG)   	(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG1_IREGS],(REG)))
#define IS_DAG1_MREG(REG)   	(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG1_MREGS],(REG)))
#define IS_DAG2(REG)   		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG2_REGS],(REG)))
#define IS_DAG2_IREG(REG)   	(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG2_IREGS],(REG)))
#define IS_DAG2_MREG(REG)  	(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DAG2_MREGS],(REG)))
#define IS_PM_MREG(REG)     	(IS_DAG2_MREG(REG))
#define IS_PM_IREG(REG)		(IS_DAG2_IREG(REG))
#define IS_DM_IREG(REG)		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DM_IREGS],(REG)))
#define IS_DM_MREG(REG)		(((REG)<FIRST_PSEUDO_REGISTER)&& TEST_HARD_REG_BIT(reg_class_contents[DM_MREGS],(REG)))

/* 1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.
   On the 21k r0  

   (shell-command "rm regclass.o") <-C-xC-e

*/

#define CALL_USED_REGISTERS                 \
  {					    	\
/* ax0, ax1, ay0, ay1, mx0, mx1, my0, my1 */	\
     0,   0,   0,   1,   0,   0,   0,   1,	\
/* mr2, mr1, mr0,  mf,  si,  se, sr1, sr0 */	\
     1,   1,   1,   1,   1,   1,   1,   1,	\
/*  ar,  sb,  px,cntr,astat,mstat,sstat,imask */\
     1,   0,   0,   1,   0,   0,   0,   0,	\
/*icntl,tx0, tx1, rx0, rx1, ifc,  mr,  sr,*/	\
     0,   0,   0,   0,   0,   0,   0,	0,	\
/*  i0,  i1,  i2,  i3,  i4,  i5,  i6,  i7 */	\
     0,   1,   0,   0,   1,   0,   1,   0,	\
/*  m0,  m1,  m2,  m3,  m4,  m5,  m6,  m7 */	\
     0,   1,   1,   1,   1,   1,   1,   0,	\
/*  l0,  l1,  l2,  l3,  l4,  l5,  l6,  l7 */	\
     1,   1,   1,   1,   1,   1,   1,   1,	\
  }

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

   
   reg classes:

   class1: A_R_REGS
	ar
   class2: M_R_REGS
	mr0
   class3: SR1_REG
	sr1
   class4: A_Y_REGS
	ay0 ay1
   class5: AY_F_REGS
        ay (for float)
   class6: AX_F_REGS
        ax (for float)
   class7: FLOAT_REGS
        ax0:ax1 ay0:ay1
   class8: M_Y_REGS
	my0 my1 mf
   class9: AXNR_REGS
	ax0 ax1  mr0 mr1  sr0 sr1
   class10: A_X_REGS
	ax0 ax1 ar mr0 mr1  sr0 sr1 (same as A_X_REGS w/o ar)
   class11: M_X_REGS
+-	mx0 mx1 ar mr0 mr1  sr0 sr1
   class12: S_E_REGS
	se
   class13: S_I_REGS
	si ar mr0 mr1 mr2 sr0 sr1
   class14: ALU_REGS
	ax0 ax1 ar mr0 mr1  sr0 sr1
	ay0 ay1
   class15: DNAR_REGS
	all of the above except ar
   class16: D_REGS
	all of the above (dregs)
   class17: DAG1_IREGS	
	i0-i3
   class18: DAG1_MREGS
	m0-m3
   class19: PM_IREGS/ DAG2_IREGS
	i4-i7
   class20: PM_MREGS/ DAG2_MREGS
	m4-m7
   class21: DM_IREGS
	i0-i7
   class22: DM_MREGS
	m0-m7
   class23: DAG1_REGS
	i0-i3 m0-m3
   class24: DAG2_REGS
	i4-i7 m4-m7
   class25 M_OR_DAG1_I
        m0-m7, i0-i3
   class26: DAG
	i0-i7 m0-m7
   class27: CNTR
        cntr
*/

/* Classes d_dmi_regs and d_dmm_regs are unions of D_REGS with 
  DM_IREGS and DM_MREGS respectively. They are used only 
 for finer computation of register class subunions 

   (shell-command "rm flow.o global-alloc.o local-alloc.o recog.o reg-stack.o regclass.o reload.o reload1.o stupid.o") <-C-xC-e

*/

enum reg_class { 
    NO_REGS,		/*  0 */
    A_R_REGS,		/*  1 */
    MR0_REG,		/*  2 */
    MR1_REG,		/*  3 */
    SR1_REG,    	/*  4 */
    A_Y_REGS,		/*  5 */
    AY_F_REGS,		/*  6 */
    AX_F_REGS,		/*  7 */
    FLOAT_REGS,		/*  8 */
    M_Y_REGS,		/*  9 */
    AXNR_REGS,		/* 10 */
    A_X_REGS,		/* 11 */
    M_X_REGS,		/* 12 */
    S_E_REGS,		/* 13 */
    S_I_REGS,		/* 14 */
    ALU_REGS,		/* 15 */
    DNAR_REGS,		/* 16 */
    D_REGS,		/* 17 */
    DAG1_IREGS,		/* 18 */
    DAG1_MREGS,		/* 19 */
    DAG2_IREGS,		/* 20 */
    DAG2_MREGS,		/* 21 */
    DM_IREGS,		/* 22 */
    DM_MREGS,		/* 23 */
    DAG1_REGS,		/* 24 */
    DAG2_REGS,		/* 25 */
    M_OR_DAG1_IREGS,	/* 26 */
    DAG_REGS,		/* 27 */
    CNTR_REGS,		/* 28 */
    CNTR_D_REGS,	/* 29 */
    GENERAL_REGS, 	/* 30 */

  RC_AUX_32,
  RC_AUX_33,
  RC_AUX_34,
  RC_AUX_35,

  RC_AUX_36,	/* RC_AUX_33 | AY_F_REGS */
  RC_AUX_37,	/* D_REGS | DAG2_IREGS */
  RC_AUX_38,	/* D_REGS | DAG_REGS */
  RC_AUX_39,	/* D_REGS | DM_MREGS */
  RC_AUX_40,	/* D_REGS | M_OR_DAG1_IREGS */

  RC_AUX_41,  /* 659 */
  RC_AUX_42,  /* 692 */
  RC_AUX_43,  /* 699 */
  RC_AUX_44,  /* 699 */
  RC_AUX_45,  /* 699 */
  RC_AUX_46,  /* 776 */
  RC_AUX_47,  /* 1212 */
  RC_AUX_48,  /* 1365 */

  RC_AUX_49,  /* 2466 */
  RC_AUX_50,  /* 2466 */
  RC_AUX_51,  /* 2466 */
  RC_AUX_52,  /* 2466 */
  RC_AUX_53,  /* 2466 */
  RC_AUX_54,  /* 2466 */
  RC_AUX_55,  /* 2466 */
  RC_AUX_56,  /* 3532 */

  RC_AUX_57,  /* 155 */
  RC_AUX_58,  /* 200 */
  RC_AUX_59,  /* 200 */
  RC_AUX_60,  /* 364 */
  RC_AUX_61,  /* 364 */

  RC_AUX_62,  /* 528 */
  RC_AUX_63,  /* 652 */

  RC_AUX_64,  /* 7758 */
  RC_AUX_65,  /* 13111 */

    ALL_REGS,
    LIM_REG_CLASSES 
    };

#define PM_IREGS DAG2_IREGS
#define PM_MREGS DAG2_MREGS
#define PM_REGS  DAG2_REGS


#define N_REG_CLASSES ((int)LIM_REG_CLASSES)

/* Give names of register classes as strings for dump file.   */

#define REG_CLASS_NAMES \
{ 		\
    "NO_REGS", \
    "A_R_REGS", \
    "MR0_REG", \
    "MR1_REG", \
    "SR1_REG",  \
    "A_Y_REGS", \
    "AY_F_REGS", \
    "AX_F_REGS", \
    "FLOAT_REGS", \
    "M_Y_REGS", \
    "AXNR_REGS", \
    "A_X_REGS", \
    "M_X_REGS", \
    "S_E_REGS", \
    "S_I_REGS", \
    "ALU_REGS", \
    "DNAR_REGS", \
    "D_REGS", \
    "DAG1_IREGS", \
    "DAG1_MREGS", \
    "DAG2_IREGS", \
    "DAG2_MREGS", \
    "DM_IREGS",  \
    "DM_MREGS",  \
    "DAG1_REGS",  \
    "DAG2_REGS",  \
    "M_OR_DAG1_IREGS", \
    "DAG_REGS",  \
    "CNTR_REGS",  \
    "CNTR_D_REGS",  \
    "GENERAL_REGS",  \
  "RC_AUX_32", \
  "RC_AUX_33", \
  "RC_AUX_34", \
  "RC_AUX_35", \
\
  "RC_AUX_36",  \
  "RC_AUX_37",  \
  "RC_AUX_38",  \
  "RC_AUX_39",  \
  "RC_AUX_40",  \
\
  "RC_AUX_41", \
  "RC_AUX_42", \
  "RC_AUX_43", \
  "RC_AUX_44", \
  "RC_AUX_45", \
  "RC_AUX_46", \
  "RC_AUX_47", \
  "RC_AUX_48", \
\
  "RC_AUX_49", \
  "RC_AUX_50", \
  "RC_AUX_51", \
  "RC_AUX_52", \
  "RC_AUX_53", \
  "RC_AUX_54", \
  "RC_AUX_55", \
  "RC_AUX_56", \
\
  "RC_AUX_57", \
  "RC_AUX_58", \
  "RC_AUX_59", \
  "RC_AUX_60", \
  "RC_AUX_61", \
\
  "RC_AUX_62", \
  "RC_AUX_63", \
\
  "RC_AUX_64", \
\
  "RC_AUX_65", \
\
\
    "ALL_REGS" }

/* Define which registers fit in which classes.
   This is an initializer for a vector of HARD_REG_SET
   of length N_REG_CLASSES.  
   (shell-command "rm regclass.o") <-C-xC-e
*/

#define REG_CLASS_CONTENTS  \
{ {0x00000000, 0x00000000},     /* NO_REGS 	*/ \
  {0x00010000, 0x00000000},     /* A_R_REG 	*/ \
  {0x00000400, 0x00000000},     /* MR0_REG 	*/ \
  {0x00000200, 0x00000000},     /* MR1_REG 	*/ \
  {0x00004000, 0x00000000},     /* SR1_REG 	*/ \
  {0x0000000c, 0x00000000},     /* A_Y_REG 	*/ \
  {0x0000000c, 0x00000000},     /* AY_F_REG 	*/ \
  {0x00000003, 0x00000000},     /* AX_F_REG 	*/ \
  {0x0000000f, 0x00000000},     /* FLOAT_REG	*/ \
  {0x000000c0, 0x00000000},     /* M_Y_REG 	*/ \
  {0x0000c603, 0x00000000},     /* AXNR_REG 	*/ \
  {0x0001c603, 0x00000000},     /* A_X_REG 	*/ \
  {0x0001c630, 0x00000000},     /* M_X_REG 	*/ \
  {0x00002000, 0x00000000},     /* S_E_REG 	*/ \
  {0x0001d600, 0x00000000},     /* S_I_REG 	*/ \
  {0x0001c60f, 0x00000000},     /* ALU_REGS 	*/ \
  {0x0000d6ff, 0x00000000},     /* DNAR_REGS 	*/ \
  {0x0001d6ff, 0x00000000},     /* D_REGS 	*/ \
  {0x00000000, 0x0000000f},     /* DAG1_IREGS, 	*/ \
  {0x00000000, 0x00000f00},     /* DAG1_MREGS, 	*/ \
  {0x00000000, 0x000000f0},     /* DAG2_IREGS, 	*/ \
  {0x00000000, 0x0000f000},     /* DAG2_MREGS, 	*/ \
  {0x00000000, 0x000000ff},     /* DM_IREGS, 	*/ \
  {0x00000000, 0x0000ff00},     /* DM_MREGS, 	*/ \
  {0x00000000, 0x00000f0f},     /* DAG1_REGS, 	*/ \
  {0x00000000, 0x0000f0f0},     /* DAG2_REGS, 	*/ \
  {0x00000000, 0x0000ff0f},     /* M_OR_DM_DAG1_IREGS */ \
  {0x00000000, 0x0000ffff},     /* DAG_REGS, 	*/ \
  {0x00080000, 0x00000000},     /* CNTR_REGS, 	*/ \
  {0x0009d6ff, 0x00000000},     /* CNTR_D_REGS 	*/ \
  {0x0001f6ff, 0x0000ffff},     /* GENERAL 	*/ \
\
   {0x0009f6ff, 0x0000ffff},	/* RC_AUX_32 */ \
   {0x00010400, 0x00000000},	/* RC_AUX_33 */ \
   {0x0001c6cf, 0x00000000},	/* RC_AUX_34 */ \
   {0x00014000, 0x00000000},	/* RC_AUX_35 */ \
\
   {0x00010600,  0x00000000}, 	/* RC_AUX_36 */ \
   {0x0001d6ff,  0x0000ffff}, 	/* RC_AUX_37 */ \
   {0x0001d6ff,  0x0000ff0f}, 	/* RC_AUX_38 */ \
   {0x00014400,  0x00000000}, 	/* RC_AUX_39 */ \
   {0x0001d603,  0x00000000}, 	/* RC_AUX_40 */ \
\
 {0x0001000c, 0x00000000}, \
 {0x0001060c, 0x00000000}, \
 {0x0001c633, 0x00000000}, \
 {0x0001060f, 0x00000000}, \
 {0x000106c0, 0x00000000}, \
 {0x0001440c, 0x00000000}, \
 {0x0001d603, 0x000000f0}, \
 {0x00004000, 0x0000000f}, \
\
 {0x0001d603, 0x0000000f}, \
 {0x0001400c, 0x0000000f}, \
 {0x0001460c, 0x0000000f}, \
 {0x0001c633, 0x0000000f}, \
 {0x0001460f, 0x0000000f}, \
 {0x000146c0, 0x0000000f}, \
 {0x00014400, 0x0000000f}, \
 {0x0001440c, 0x0000000f}, \
\
 {0x000146cc, 0x00000000}, /* RC_AUX_57 */\
 {0x000146cf, 0x0000000f}, /* RC_AUX_58 */\
 {0x0001c63f, 0x0000000f}, /* RC_AUX_59 */\
 {0x0001d60f, 0x000000ff}, /* RC_AUX_60 */\
 {0x0001d60f, 0x0000000f}, /* RC_AUX_61 */\
\
 {0x0001d6cf, 0x000000ff}, /* RC_AUX_62 */\
 {0x000146cf, 0x00000000}, /* RC_AUX_63 */\
\
 {0x0001c6ff, 0x00000000}, /* RC_AUX_64 */\
\
 {0x0001d6ff, 0x000000f0}, /* RC_AUX_65 */\
\
  {0x3fffffff, 0x00ffffff}}     /* ALL_REG	*/

/* The same information, inverted:
   Return the class number of the smallest class containing
   reg number REGNO.  This could be a conditional expression
   or could index an array.  */

extern enum reg_class regno_reg_class[];


#if 1
#define REGNO_REG_CLASS(REGNO) ((REGNO)<FIRST_PSEUDO_REGISTER ? \
				regno_reg_class[REGNO] : 0)
#else
#define REGNO_REG_CLASS(REGNO) \
	(((REGNO) <= REG_AX1) ? AX_F_REGS	\
	:((REGNO) <= REG_AY1) ? A_Y_REGS	\
	:((REGNO) <= REG_MX1) ? M_X_REGS	\
	:((REGNO) <= REG_MY1) ? M_Y_REGS	\
	:((REGNO) == REG_MR2) ? ALL_REGS	\
	:((REGNO) == REG_MR1) ? A_X_REGS	\
	:((REGNO) == REG_MR0) ? MR0_REG	        \
	:((REGNO) <= REG_MF)  ? ALL_REGS	\
	:((REGNO) == REG_SI)  ? S_I_REGS	\
	:((REGNO) == REG_SE)  ? S_E_REGS	\
	:((REGNO) == REG_SR1) ? SR1_REG	        \
	:((REGNO) == REG_SR0) ? A_X_REGS	\
	:((REGNO) == REG_AR)  ? A_R_REGS	\
	:((REGNO) == REG_CNTR) ? CNTR_REGS      \
	:((REGNO) <= REG_IFC) ? ALL_REGS	\
	:((REGNO) <  REG_I0)  ? ALL_REGS	\
	:((REGNO) <= REG_I3)  ? DAG1_IREGS	\
	:((REGNO) <= REG_I7)  ? DAG2_IREGS	\
	:((REGNO) <= REG_M3)  ? DAG1_MREGS	\
	:((REGNO) <= REG_M7)  ? DAG2_MREGS	\
	:((REGNO) <= REG_L7)  ? GENERAL_REGS	\
	:0)	
#endif


/* The class value for index registers, and the one for base regs.  */

#define INDEX_REG_CLASS(MODE) NO_REGS

#define MODIFY_REG_CLASS(MODE) \
    (((MODE)==DMmode)?DAG1_MREGS:((MODE)==PMmode)?PM_MREGS:NO_REGS)


/* 
  Strictly speaking, the base reg class for dm is all i regs,
  however, let's preference dag1 regs because otherwise all hell
  breaks loose.
*/
/* (shell-command "rm  regclass.o reload.o reload1.o") */
#define BASE_REG_CLASS(MODE) \
    (((MODE)==DMmode)?DM_IREGS:((MODE)==PMmode)?PM_IREGS:NO_REGS)

#define ALL_BASE_REG_CLASS  DM_IREGS

#define BASE_REG_CLASS1(m)  BASE_REG_CLASS(m)
#define INDEX_REG_CLASS1(m) INDEX_REG_CLASS(m)


/* Get reg_class from a letter such as appears in the machine description.
   We do a trick here to modify the effective constraints on the
   machine description; we zorch the constraint letters that aren't
   appropriate for a specific target.  This allows us to guarantee
   that a specific kind of register will not be used for a given target
   without fiddling with the register classes above. */

#define REG_CLASS_FROM_LETTER(C) (\
     (C) == 'A' ? A_Y_REGS :	\
     (C) == 'B' ? M_Y_REGS :	\
     (C) == 'C' ? MR0_REG :	\
     (C) == 'D' ? SR1_REG :   	\
     (C) == 'W' ? MR1_REG :	\
     (C) == 'a' ? A_X_REGS :	\
     (C) == 'b' ? M_X_REGS :	\
     (C) == 'c' ? A_R_REGS :	\
     (C) == 'd' ? S_I_REGS :	\
     (C) == 'e' ? D_REGS :	\
     (C) == 'f' ? S_E_REGS :	\
     (C) == 'h' ? DNAR_REGS:	\
     (C) == 'j' ? AXNR_REGS:	\
     (C) == 'k' ? AX_F_REGS:	\
     (C) == 'l' ? AY_F_REGS:	\
     (C) == 'q' ? CNTR_REGS :   \
     (C) == 'w' ? DAG1_IREGS :	\
     (C) == 'x' ? DAG1_MREGS :	\
     (C) == 'y' ? DAG2_IREGS :	\
     (C) == 'z' ? DAG2_MREGS :	\
     NO_REGS)

/*
  `HARD_REGNO_NREGS (REGNO, MODE)'
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
*/


#define HARD_REGNO_NREGS(REGNO, MODE) (((MODE)==DImode||(MODE==SFmode)) ? 2  : 1)


/* Return the maximum number of consecutive registers
   needed to represent mode MODE in a register of class CLASS.  */

#define CLASS_MAX_NREGS(CLASS, MODE)	class_max_nregs(CLASS,MODE)

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

	*/

#define HARD_REGNO_MODE_OK(REGNO, MODE) hard_regno_mode_ok(REGNO,MODE)


/* Value is 1 if it is a good idea to tie two pseudo registers
   when one has mode MODE1 and one has mode MODE2.
   If HARD_REGNO_MODE_OK could produce different values for MODE1 and MODE2,
   for any hard reg, then this must be 0 for correct output.  */

#define MODES_TIEABLE_P(MODE1, MODE2)	(0)

#define PREFERRED_RELOAD_CLASS(X,CLASS) preferred_reload_class(X,CLASS)

#define PREFERRED_OUTPUT_RELOAD_CLASS(X,CLASS) preferred_output_reload_class(X,CLASS)

#define REGNO_OK_FOR_INDEX_P(REGNO,MODE) (0)

#define REGNO_OK_FOR_BASE_P(REGNO,MODE) regno_ok_for_base_p(REGNO,MODE)

#define REGNO_OK_FOR_MODIFY_P(REGNO,MODE) regno_ok_for_modify_p(REGNO,MODE)

#define CONDITIONAL_REGISTER_USAGE conditional_register_usage()

/*(shell-command "rm reload.o reload1.o caller-save.o") <-C-xC-e*/
enum reg_class secondary_output_reload_class();
enum reg_class secondary_input_reload_class();

#define  SECONDARY_OUTPUT_RELOAD_CLASS(class,mode,x) secondary_output_reload_class(class,mode,x)
#define  SECONDARY_INPUT_RELOAD_CLASS(class,mode,x) secondary_input_reload_class(class,mode,x)

#define SMALL_REGISTER_CLASSES

#define CONST_OK_FOR_LETTER_P(VALUE, C) const_ok_for_letter_p(VALUE, C)

#define CONST_DOUBLE_OK_FOR_LETTER_P(VALUE, C)  0

#define EXTRA_CONSTRAINT(OP,C) extra_constraint (OP,C)

/* (shell-command "rm regclass.o") */

#define REG_ALLOC_ORDER  {	\
	REG_AX1, 	REG_AY1,	REG_MX1,	REG_MY1,	\
	REG_MR0,	REG_MR1,	REG_M5,		REG_SI,		\
	REG_AR,		REG_I2,		REG_I6,		                \
	REG_M0,		REG_AX0,	REG_AY0,	REG_MX0,	\
	REG_MY0,	REG_SR0,	REG_SR1,	REG_I0,		\
	REG_I1,		REG_I3,		REG_I5,		REG_I7,		\
	REG_MF,		REG_SB,		REG_PX,			        \
	REG_M7,		REG_M1,		REG_M3,                         \
        FIRST_PSEUDO_REGISTER}




#define SDB_DEBUGGING_INFO
#define ADI_MERGED_LISTING
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
	  fprintf (file, "\t.ln\t%d;\n",\
		   ((sdb_begin_function_line > -1)\
		    ? last_linenum - sdb_begin_function_line : 1))



/* Stack layout; function entry, exit and calling.  */

/* Define this if pushing a word on the stack
   makes the stack pointer a smaller address.  */
#define STACK_GROWS_DOWNWARD

/* Define this if the nominal address of the stack frame
   is at the high-address end of the local variables;
   that is, each additional local variable allocated
   goes at a more negative offset in the frame.  */
#define FRAME_GROWS_DOWNWARD

/* Offset within stack frame to start allocating local variables at.
   If FRAME_GROWS_DOWNWARD, this is the offset to the END of the
   first local allocated.  Otherwise, it is the offset to the BEGINNING
   of the first local allocated.  */
/* (shell-command "rm function.o") */
#define STARTING_FRAME_OFFSET -1

/* If we generate an insn to push BYTES bytes,
   this says how many the stack pointer really advances by.
   On the 21xx, a push MUST BE a 16-bit transfer.
   Therefore round to the next multiple of 2   */


#define PUSH_ROUNDING(BYTES) (BYTES)

/* (shell-command "rm  function.o explow.o") */
#define STACK_POINTER_OFFSET 1

/* Offset of first parameter from the argument pointer register value.
   NOTE: Normally this would be 8 (to skip the return address and frame
         pointer on the stack. */

#define FIRST_PARM_OFFSET(FNDECL)   1

/* Value is 1 if returning from a function call automatically
   pops the arguments described by the number-of-args field in the call.
   FUNTYPE is the data type of the function (as a tree),
   or for a library call it is an identifier node for the subroutine name. */

#define RETURN_POPS_ARGS(FUNTYPE, SIZE) 0

/* Define how to find the value returned by a function.
   VALTYPE is the data type of the value (as a tree).
   If the precise function being called is known, FUNC is its FUNCTION_DECL;
   otherwise, FUNC is 0. 

On the 21xx return in ar */

/*(shell-command "rm regclass.o") */
#define VALUE_REGNO(MODE) (((MODE)==SFmode||(MODE)==DImode)?(REG_SR1):(REG_AR))
#define FUNCTION_VALUE(VALTYPE, FUNC)  \
  gen_rtx (REG, TYPE_MODE (VALTYPE), \
           VALUE_REGNO(TYPE_MODE(VALTYPE)))

/* Define how to find the value returned by a library function
   assuming the value has mode MODE.  */

#define LIBCALL_VALUE(MODE)  gen_rtx (REG, MODE, VALUE_REGNO(MODE))

/*(shell-command "rm regclass.o") */
#define FUNCTION_VALUE_REGNO_P(N) ((N) == REG_AR)

/* Define this if PCC uses the nonreentrant convention for returning
   structure and union values.  */

#define PCC_STATIC_STRUCT_RETURN

/* 1 if N is a possible register number for function argument passing.
   On the 21xx, no registers are used in this way.  */

extern int function_arg_regno();

#define FUNCTION_ARG_REGNO_P(N) function_arg_regno_p(N)

/*(shell-command "rm calls.o cp-decl.o cp-init.o cp-search.o cp-tree.o expr.o \
  function.o integrate.o")*/
extern void * function_arg();
#define FUNCTION_ARG(CUM, MODE, TYPE, NAMED) \
	 function_arg(CUM,MODE,TYPE,NAMED)

/* Define a data type for recording info about an argument list
   during the scan of that argument list.  This data type should
   hold all necessary information about the function itself
   and about the args processed so far, enough to enable macros
   such as FUNCTION_ARG to determine where the next arg should go.

   On the 21xx, this is a single integer, which is a number of bytes
   of arguments scanned so far.  */

#define CUMULATIVE_ARGS int

/* Initialize a variable CUM of type CUMULATIVE_ARGS
   for a call to a function whose data type is FNTYPE.
   For a library call, FNTYPE is 0.

   On the 21xx, the offset starts at 0.  */

#define INIT_CUMULATIVE_ARGS(CUM,FNTYPE,LIBNAME)  ((CUM) = 0)

/* Update the data in CUM to advance over an argument
   of mode MODE and data type TYPE.
   (TYPE is null for libcalls where that information may not be available.)  */

#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED)	\
 ((CUM) += ((MODE) != BLKmode			\
	    ? GET_MODE_SIZE (MODE) \
	    : int_size_in_bytes (TYPE) )) 

/* Define where to put the arguments to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
v   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).  */

/* On the 21xx all args are pushed */


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
  fprintf (FILE, "\ttstl LPBX0\n\tif ne jump LPI%d;\n\tdm(i7,-1)=LPBX0;\n\tjump ___bb_init_func\n\taddql #4,sp\nLPI%d:\n",  \
	   LABELNO, LABELNO);    */

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
#define STACK_POINTER_REGNUM REG_I4

/* Base register for access to local variables of the function.  */
#define FRAME_POINTER_REGNUM REG_M4

/* Base register for access to arguments of the function.  */
#define ARG_POINTER_REGNUM FRAME_POINTER_REGNUM

/* Register in which static-chain is passed to a function.  */
/* Not currently used for C, define it to be anything */
#define STATIC_CHAIN_REGNUM 0

/* Register in which address to store a structure value
   is passed to a function.  */
#define STRUCT_VALUE_REGNUM REG_AX0

#define INITIAL_FRAME_POINTER_OFFSET(DEPTH) {(DEPTH)=initial_frame_pointer_offset();}


/* Value should be nonzero if functions must have frame pointers.
   Zero means the frame pointer need not be set up (and parms
   may be accessed via the stack pointer) in functions that seem suitable.
   This is computed in `reload', in reload1.c.  */
#define FRAME_POINTER_REQUIRED 0


/*     STORAGE LAYOUT            */
/* target machine storage layout */

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.
   On the 21xx bit field operations are packed from right most bit
   to left most.
 */
/* (shell-command "rm  combine.o cse.o expmed.o function.o recog.o") */
#define BITS_BIG_ENDIAN  0

/* Define this if most significant byte of a word is the lowest numbered.
 * We can't access bytes but if we could we would in the Big Endian order.
 */
#define BYTES_BIG_ENDIAN 1

/* Define this if most significant word of a multiword number is numbered.
 */
#define WORDS_BIG_ENDIAN 1

/* number of bits in an addressible storage unit */

#define BITS_PER_UNIT 16

/* Width in bits of a "word", which is the contents of a machine register.
   Note that this is not necessarily the width of data type `int';
   if using 16-bit ints on a 68000, this would still be 32.
   But on a machine with 16-bit registers, this would be 16.  */
#define BITS_PER_WORD 16

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD 1

/* Width in bits of a pointer.
   See also the macro `Pmode' defined below.  */
#define POINTER_SIZE 16

/* Allocation boundary (in *bits*) for storing pointers in memory.  */
#define POINTER_BOUNDARY 16

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY 16

/* Boundary (in *bits*) on which stack pointer should be aligned.  */
#define STACK_BOUNDARY 16

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY 16

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY 16

/* No data type wants to be aligned rounder than this.  */
#define BIGGEST_ALIGNMENT 16

/* Define this if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT 0

/* Define number of bits in most basic integer type.
   (If undefined, default is BITS_PER_WORD).  */


#define CHAR_TYPE_SIZE   16
#define SHORT_TYPE_SIZE  16
#define INT_TYPE_SIZE    16
#define LONG_TYPE_SIZE   32
#define FLOAT_TYPE_SIZE  32
#ifdef DOUBLE_IS_32
#define DOUBLE_TYPE_SIZE    32
#else
#define DOUBLE_TYPE_SIZE 32
#endif
#define LONG_DOUBLE_TYPE_SIZE 32
#define LONG_LONG_TYPE_SIZE 32

#define UNITS_PER_FLOAT  ((FLOAT_TYPE_SIZE  + BITS_PER_UNIT - 1) / \
			   BITS_PER_UNIT)

#define UNITS_PER_DOUBLE ((DOUBLE_TYPE_SIZE + BITS_PER_UNIT - 1) / \
 			   BITS_PER_UNIT)


/* Define this as 1 if `char' should by default be signed; else as 0.  */
#define DEFAULT_SIGNED_CHAR 1

#define DOUBLES_ARE_FLOATS 1

#define TRAMPOLINE_SIZE 10

#define INITIALIZE_TRAMPOLINE(a,b,c)

#define TRAMPOLINE_TEMPLATE(file) trampoline_template(file)

/* We do not want address arithmetic to be performed in DImode, which */
/* would happen since default for SIZE_TYPE is 'long unsigned int' */

#define   SIZE_TYPE    "unsigned int"
#define   PTRDIFF_TYPE "int"

extern  int *mod_reg_equiv_const;
extern  int mod_reg_equiv_size;

extern int class_ok_for_mode();
/** Used by machine description code */

extern void double_const_to_singles();

#define PEEP_PARALLEL

extern void prereload_calls();

#ifdef RTX_CODE
enum machine_mode extract_tree_pmode();
enum machine_mode extract_rtx_pmode();
#endif

#define HANDLE_PRAGMA handle_pragma

void clear_better_rtx();
struct rtx_def *find_better_rtx ();
void check_invalid_change_reloads();
void change_reloads();
void fix_param_bug();

#define REG_CLASS_SUBUNION_debug(X,Y) ({check_subunion(X,Y); \
				  reg_class_subunion[X][Y];})

#define REG_CLASS_SUBUNION(X,Y) reg_class_subunion[X][Y]

#endif
