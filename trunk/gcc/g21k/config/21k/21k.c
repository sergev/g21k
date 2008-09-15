/*
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

/* Identification String */ 
static char *__ident__ = "@(#) aux-output21k.c 2.26@(#)";

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "machmode.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "output.h"
#include "tree.h"
#include "obstack.h"
#include "insn-attr.h"
#include "reload.h"

#if defined (__WATCOMC__) || defined (__EMX__)
int
strncasecmp(char *x,char *y, int z) 
{
  int i, ret_val=0;

  for (i=0; i<z; i++)
    {
      ret_val=x[i]-y[i];
      if ((x[i] == NULL) || (y[i] == NULL) || (ret_val != 0))
	break;
    }
  return (ret_val);
}
#endif

enum machine_mode stack_pmode;

#include "expr.h"	/** need decls of some functions like negate_rtx **/



typedef enum {  DONT_DO_MODIFY, DO_MODIFY_OKAY  } memory_access_t;

void complete_reg_alloc_order();
static void print_operand_address_1 ( FILE *, rtx, memory_access_t);

char           *arch_reserved_registers = 0;
char           *arch_scratch_registers  = 0;
char           *pmcode_segment = "seg_pmco";
char           *pmdata_segment = "seg_pmda";
char           *dmdata_segment = "seg_dmda";
char	       *temp_segment_name = 0;

extern int      optimize;
extern int      flag_delayed_branch;
extern int      flag_short_double;
extern int	flag_no_builtin;

static char    *fp;
static char    *sp;
static int      Register_save_offset;

#ifdef ADI_PROFILER
static rtx emuclock_rtx;
#endif 


char *alt_reg_names[] = ALT_REGISTER_NAMES;

#define RET  return ""				 /* Used in machine
						  * description */
#define IS_REG_SAVETYPE(i) 1
#define IS_REG_RESERVED(i) 0

static int save_order[] = {
  REG_R1, REG_R3, REG_R5, REG_R6, REG_R7, REG_R9, REG_R10, REG_R11,
  REG_R13, REG_R14, REG_R15, REG_I0, REG_I1, REG_I2, REG_I3, REG_I5,
  REG_I8, REG_I9, REG_I10, REG_I11, REG_I13, REG_I14, REG_I15, REG_M0,
  REG_M1, REG_M2, REG_M3, REG_M8, REG_M9, REG_M10, REG_M11, REG_M12,
  REG_MRF, REG_MRB, REG_USTAT1, REG_USTAT2,
  -1};

enum reg_class
reg_class_intersection(c1, c2)
    enum reg_class c1,c2;
{
    enum reg_class result = NO_REGS, *p;
    if (reg_class_subset_p(c1, c2)) return c1;
    if (reg_class_subset_p(c2, c1)) return c2;

    for (p = &reg_class_subclasses[c1][0];
	 *p != LIM_REG_CLASSES;
	 p++)
	if (reg_class_subset_p(*p, c2))
	    result = *p;
    return result;
}

/**** dag1_register_p returns TRUE if register number "regno" ***/
/*** belongs to DAG number dagno (1 or 2) ***/

int
dag_register_p(regno, dagno)
     int regno, dagno;
{
  int result;
  
  switch (dagno)
    {
    case 1:
      result = IS_DAG1_REG(regno);
      break;
    case 2:
      result = IS_DAG2_REG(regno);
      break;
    default:
      fprintf(stderr, "'dagno' must be 1 or 2, not %d\n", dagno);
      abort();
    }
  return (result);
}

int
is_dag_register (rtx pat)
{
  rtx dest;

  dest = SET_DEST(pat);
  SWITCH_SUBREG(dest);
  if (GET_CODE(dest) == REG)
    if (IS_DAG_REG(REGNO(dest))) 
      return 0;
  return 1;
}

int
is_dag_reg (rtx insn)
{
    int regnum;

    if (GET_CODE (insn) != REG)
	return 0;

    regnum = REGNO (insn);

    if (IS_DAG_REG (regnum))
	return 1;
    else 
	return 0;
}

int
is_small_constant (rtx op)
{
    return (op == const0_rtx || op == const1_rtx || op == constm1_rtx);
}

/*
 * RUNTIME ENVIRONMENT:
 */
void restore_registers(file, size, extra)
    FILE           *file;
    int             size,
                    extra;
{
  if (MAIN_JUST_RETURNS || strcmp("main", current_function_name) != 0)
    {
      int regno, ind, sofar = 0, frame_offset;
      
      int return_in_r1
	=  current_function_return_rtx
	  && (current_function_return_rtx->mode == PDImode
	      || current_function_return_rtx->mode == DImode
	      || (DOUBLE_IS_64 && current_function_return_rtx->mode == DFmode));
      
      for (ind = 0; (regno = save_order[ind]) >= 0; ind++)
	if (regs_ever_live[regno]
	    && !call_used_regs[regno]
	    && IS_REG_SAVETYPE(regno)
	    && !IS_REG_RESERVED(regno)
	    && !(return_in_r1 && regno == REG_R1))
	  {
	    frame_offset = ++sofar + size + extra;
	    
	    if (IS_MULT_ACC_REG(regno))
	      {
		fprintf(file, "\t%s=%s(%d,%s);\n",
			reg_names[SCRATCH_DREG], STACK_REF,
			- frame_offset, fp);
		fprintf(file, "\t%s=%s;\n",
			regno == REG_MRF ? "MR0F" : "MR0B",
			reg_names[SCRATCH_DREG]);
	      }
	    else
	      fprintf(file, "\t%s=%s(%d,%s);\n",
		      reg_names[regno], STACK_REF,
		      -frame_offset, fp);
	  }
    }
}

int
iregs_different_dag(reg1, reg2)
     int reg1, reg2;
{
  if(IS_DM_IREG(reg1) && IS_PM_IREG(reg2))
    return 1;
  if(IS_PM_IREG(reg1) && IS_DM_IREG(reg2))
    return 1;
  return 0;
}

void 
save_registers(file, size, extra)
     FILE *file;
     int  size, extra;
{
  if (MAIN_JUST_RETURNS || strcmp("_main", current_function_name) != 0)
    {
      int regno,ind, sofar = 0, frame_offset;
      int return_in_r1
	=  current_function_return_rtx
	  && (current_function_return_rtx->mode == PDImode
	      || current_function_return_rtx->mode == DImode
	      || (DOUBLE_IS_64 && current_function_return_rtx->mode == DFmode));
      
      for (ind = 0; (regno = save_order[ind]) >= 0; ind++) 
	{
	  if (regs_ever_live[regno]
	      && !call_used_regs[regno]
	      && IS_REG_SAVETYPE(regno)
	      && !IS_REG_RESERVED(regno)
	      && !(return_in_r1 && regno == REG_R1))
	    {
	      frame_offset = (++sofar) + size + extra;
	      
	      if (IS_MULT_ACC_REG (regno))
		{
		  fprintf(file, "\t%s=%s;\n", reg_names[SCRATCH_DREG], 
			  regno == REG_MRF ?
			  "MR0F" : "MR0B");
		  fprintf(file, "\t%s(%d,%s)=%s;\n", STACK_REF, 
			  -(frame_offset), 
			  fp, reg_names[SCRATCH_DREG]);
		}
	      else if (IS_DREG(regno) 
		       || IREGS_DIFFERENT_DAG(STACK_POINTER_REGNUM, regno))
		{
		  fprintf(file, "\t%s(%d,%s)=%s;\n", STACK_REF,
			  -(frame_offset), fp, reg_names[regno]);
		}
	      else  
		{
		  fprintf(file, "\t%s=%s;\n", reg_names[SCRATCH_DREG], 
			  reg_names[regno]);
		  fprintf(file, "\t%s(%d,%s)=%s;\n", STACK_REF,
			  -(frame_offset), fp, reg_names[SCRATCH_DREG]);
		}
	    }
	}
    }
}

void
function_prologue(file, size)
     FILE *file;
     int  size;
{
  int regno, ind, size_plus_save_regs = size;
  int return_in_r1;
  extern int flag_short_double; /* c-decl.c */

  fprintf(file, "!\tFUNCTION PROLOGUE: %s\n\
!\t%s protocol, params %s, %s stack, doubles are %s\n",
	  current_function_name,
	  CALLING_PROTOCALL_RTRTS ? "rtrts" : "pcrts",
	  TARGET_REGPARM ? "in registers" : "on the stack",
	  PMSTACK ? "PM" : "DM",
	  flag_short_double ? "floats" : "doubles");
  
  if (!MAIN_JUST_RETURNS 
      && strcmp("main", current_function_name) == 0)
    fprintf(file, ".extern\t_exit;\n");
  
#if MAIN_SHOULD_SET_TRUNCATION_MODE
  if (strcmp("main", current_function_name) == 0)
    fprintf(file, "\tbit set mode1 0x8000; !set TRUNC mode\n");
#endif
  /*
   * For matt, we've got to do the prologue a little differently than we
   * might otherwise.  We've GUARANTEED him that all non-leaf functions
   * will begin with a push of the parent frame pointer. This can take a
   * several forms.  First, if we've got PCRTS and locals are fewere than
   * 32, we can carve out space for locals with the parent frame pointer
   * push.  This looks like "dm(i7,-29)=r2" (for example).  The other
   * option (which happens whenever PCRTS && locals > 32 or when RTRTS) is
   * just a parent frame pointer push "dm(i7,m7)=r2". Then, if we've got
   * RTRTS, we push the return address.  Then, for either PCRTS or RTRTS we
   * carve out stack space for automatics.
   */
  return_in_r1
    =  current_function_return_rtx
      && (current_function_return_rtx->mode == PDImode
	  || current_function_return_rtx->mode == DImode
	  || (DOUBLE_IS_64 && current_function_return_rtx->mode == DFmode));
  
  for (ind = 0; (regno = save_order[ind]) >= 0; ind++)
    if (regs_ever_live[regno]
	&& !call_used_regs[regno]
	&& IS_REG_SAVETYPE(regno)
	&& !IS_REG_RESERVED(regno)
	&& !(return_in_r1 && regno == REG_R1))
      size_plus_save_regs++;

  if (CALLING_PROTOCALL_PCRTS)
    if (size_plus_save_regs < 32)
      fprintf(file, "\t%s(%s,%d)=%s;\n", STACK_REF,
	      sp, -size_plus_save_regs - 1, reg_names[SCRATCH_DREG]);
    else
      {
	fprintf(file, "\t%s(%s,%s)=%s;\n", STACK_REF,
		sp, reg_names[MINUS1_MREG(stack_pmode)],
		reg_names[SCRATCH_DREG]);
	fprintf(file, "\tmodify(%s,%d);\n", sp, -size_plus_save_regs);
      }
  else
    {
      /* Conforms to new calling protocol  6/94*/
      if (size_plus_save_regs > 0)
	fprintf(file, "\tmodify(%s,%d);\n", sp, -size_plus_save_regs);
    }

  /*
   * Save any registers this function uses, unless they are used in a call,
   * in which case we don't need to
   */
  if (size_plus_save_regs > 0)
    {
      fprintf (file, "!\tsaving registers: \n");
      save_registers (file, size, CALLING_PROTOCALL_RTRTS ? 1 : 0);
    }
  PUT_SDB_PROLOGUE_END (file, current_function_name);
}

/*
 * Tests if INSN is safe in the N'th delay slot of this function
 * Basically, we can do anything in the delay slot that doesn't
 * use one of the preserved registers - as we will have restored them
 * by this time!
 */
int
eligible_for_epilogue_p(insn,n)
  rtx	insn;
  int	n;
{
  HARD_REG_SET	saved_regs;
  int reg,i;

  if (GET_CODE (insn) != INSN)
    return 0;

  if (get_attr_length(insn) != 1)
      return 0;

  /* Build list of saved registers */
  for (i=0; (reg = save_order[i]) >= 0; i++)
      if (regs_ever_live[reg] && !call_used_regs[reg]
	  && IS_REG_SAVETYPE(reg)
	  && !IS_REG_RESERVED(reg))
	  SET_HARD_REG_BIT (saved_regs, reg);

  return ! rtx_mentions_hard_regs_p (PATTERN(insn), saved_regs);
}

/*
 * Function is coded.  Tack on the epilogue code to restore registers
 * and return to our caller.
 */
void
function_epilogue(file, size)
    FILE           *file;
    int             size;
{
  int in_main = !MAIN_JUST_RETURNS && strcmp(current_function_name, "main") == 0;
  fprintf(file, "!\tFUNCTION EPILOGUE: \n");

  fprintf(file, "\ti12=%s(-1,%s);\n", STACK_REF, fp);

  restore_registers(file, size, CALLING_PROTOCALL_RTRTS ? 1 : 0);
  
  if (in_main)
    fprintf (file, "\tjump (PC, _exit) (DB);!.return_main\n");

  else if (CALLING_PROTOCALL_PCRTS)
    fprintf (file, "\trts (DB);\n");

  else
    fprintf (file, "\tjump (m14,i12) (DB);!.return\n",
	     STACK_REF, reg_names[MINUS1_MREG(stack_pmode)], fp);
  
  if (ADSP210Z3u && !in_main)
   {
     if (current_function_epilogue_delay_list)
	 final_scan_insn (XEXP (current_function_epilogue_delay_list, 0), file, optimize, 0, 1);
     else
	 fputs ("\tnop;\n", file);
     fputs ("\tRFRAME;\n", file);
   }
  else
    fprintf (file, "\t%s=%s;\n\t%s=%s(0,%s);\n", sp, fp, fp, STACK_REF, fp);
}


void
jjb_output_indirect_call (indirect_reg, insn)
     rtx indirect_reg, insn;
{
  rtx lab = gen_label_rtx();
  rtx ops[6];

  ops[0] = indirect_reg;
  ops[1] = lab;
  ops[2] = frame_pointer_rtx;
  ops[3] = stack_pointer_rtx;
  ops[4] = gen_rtx (REG, PMmode, PCREL_ONLY ? SCRATCH_PM_MREG: ZERO_MREG(PMmode));
  ops[5] = PCREL_ONLY ? gen_rtx (REG, PMmode, REG_I12): indirect_reg;
  output_asm_insn("r2=%2;", ops);

  if (PCREL_ONLY)
    output_asm_insn ("r0=pc;\n\tr0=%0-r0, %4=r0;\n\t%2=%3;\n\t%5=r0;",ops);
  else 
    output_asm_insn ("%2=%3;", ops);
  
  if (KEEP_I13)
    output_asm_insn ("i13=%1-1;\n\tjump (%4,%5) (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;\n%L1", ops);
  else
    output_asm_insn ("jump (%4,%5) (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;", ops);
}

void
jjb_output_call(what, insn)
     rtx what, insn;
{
  rtx lab = gen_label_rtx();
  rtx ops[5];
  
  ops[0] = what;
  ops[1] = lab;
  ops[2] = frame_pointer_rtx;
  ops[3] = stack_pointer_rtx;
  
  if (ADSP210Z4u)
    {
      output_asm_insn ("cjump (pc, %s0) (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;", ops);
    }
  else if (ADSP210Z3u)
    {
      output_asm_insn ("cjump %s0 (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;", ops);
    }
  else 
    if (KEEP_I13)
      output_asm_insn 
	("i13=%1-1;\n\tr2=%2;\n\t%2=%3;\n\tjump (pc, %s0) (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;\n%L1",
	 ops);
    else
      output_asm_insn ("r2=%2;\n\t%2=%3;\n\tjump (pc, %s0) (DB);!.call\n\tdm(%3,m7)=r2;\n\tdm(%3,m7)=pc;", ops);
}

int
strcommalen (char *s)
{
  int len = 0;
  while (s && *s && *s != ',') s++,len++;
  return len;
}

void
conditional_register_usage()
{
  char *cp;
  int  done, i;


  static int first_time = 1;

  if (! first_time)
      return;

  first_time = 0;

  complete_reg_alloc_order();

  for (cp=arch_reserved_registers; arch_reserved_registers && *cp; )
    {
      if(*cp==',')
	cp++;
      else
	{
	  for (i = 0,done=0; i < FIRST_PSEUDO_REGISTER; i++)
	    if (reg_names[i][0] 
	       && !strncasecmp (reg_names[i], cp, strcommalen (cp)))
	      {
		if(fixed_regs[i])
		  error("Can't reserve fixed register \"%s\"",reg_names[i]);
		else if(call_used_regs[i])
		  error("Can't reserve call-used register \"%s\"",reg_names[i]);
		else {
		  fixed_regs[i] = 1;
		  call_used_regs[i] = 1;
		}
		cp+=strlen(reg_names[i]);
		done = 1;
		break;
	      }
	  if(!done)
	    {
	      error("Unrecognized reserved register \"%s\"",cp);
	      return;
	    }
	}
    }

  for (cp = arch_scratch_registers; arch_scratch_registers && *cp ;)
    {
      if(*cp==',')
	cp++;
      else
	{
	  for (i = 0, done=0; i < FIRST_PSEUDO_REGISTER; i++)
	    if (reg_names[i][0] 
		&& !strncasecmp (reg_names[i], cp, strcommalen(cp)))
	      {
		call_used_regs[i] = 1;
		cp+=strlen (reg_names[i]);
		done = 1;
		break;
	      }
	  if (! done)
	    {
	      warning ("Unrecognized call_used register \"%s\"",cp);
	      return;
	    }
	}
    }
}

/*** The following is called only wheh doubles-are-doubles ***/

#define IEEE_DOUBLE_BIAS 1023
#define IEEE_DOUBLE_EXP_PREC 11
#define IEEE_DOUBLE_PREC1 20
#define IEEE_DOUBLE_PREC2 32

typedef struct ieeedouble
{
    unsigned        sgn:1;
    unsigned        exp:11;
    unsigned        mant1:IEEE_DOUBLE_PREC1;
    unsigned        mant2;
}               ieeedouble;

void
ieee_double_comps(d, ostr)
     double          d;
     ieeedouble     *ostr;
     
{
  int             sign = 0,
  exponent = IEEE_DOUBLE_BIAS,
  m1 = 0,
  m2 = 0;
  int             i,
  dig;
  double          mantissa;

#ifdef sun
  /* Seems only the sun has a HUGE_VAL inf. pc /go32 doesn't. 
     This is only a marker to remind us what to do next time. */
  if (d == HUGE_VAL  /* This should be an Inf on the sun it is. */)
    {  /* make this an inf */
      ostr->sgn   = 0;
      ostr->exp   = 0x7ff;
      ostr->mant1 = 0;
      ostr->mant2 = 0;
      return;
    }
#endif
  if (d != 0) 
    {
      
      if (d < 0)
	{
	  sign = 1;
	  d = -d;
	}
      
      while (d < 1)
	{
	  d = 2. * d;
	  exponent--;
	}
      while (d >= 2.)
	{
	  d = d / 2.;
	  exponent++;
	}
      mantissa = d - 1.;
      
      for (i = 0;
	   i < IEEE_DOUBLE_PREC1 + IEEE_DOUBLE_PREC2;
	   i++)	
	{
	  
	  mantissa = mantissa * 2;
	  
	  if (mantissa >= 1)
	    {
	      mantissa = mantissa - 1;
	      dig = 1;
	    }
	  else
	    dig = 0;
	  
	  if (i < IEEE_DOUBLE_PREC1)
	    m1 = 2 * m1 + dig;
	  else
	    m2 = 2 * m2 + dig;
	}
    }
  else
    {
      exponent=0;
    }
  
  ostr->sgn = sign;
  ostr->exp = exponent;
  ostr->mant1 = m1;
  ostr->mant2 = m2;
}

/*** returns bits in array AR. The goal is to be able  **/
/**  to dump AR in hex and get something suitable for a machine  **/
/**  with non-swapped bytes, words. Thus, the code below is NOT  ***/
/*** completely target independent, but it is host-independt as long as ***/
/*** host integers are at least 32 bits. ***/

void
ieee_double_hex(d, ar)
    double          d;
    int             ar[];

{
    ieeedouble      s;

    ieee_double_comps(d, &s);
    ar[0] = (s.sgn << IEEE_DOUBLE_EXP_PREC) | s.exp;
    ar[0] = (ar[0] << IEEE_DOUBLE_PREC1) | s.mant1;
    ar[1] = s.mant2;
}

/************ End of output for doubles **********************************/


 /********************************************/
 /*** Implementation of Jump Tables **********/
 /********************************************/


/* The jump_table_separator variable is used to insert separator     */
/* characters (',' in our case) in front of each byt the very        */
/* first element of the jump table. This is achieved by              */
/* by initializing it to empty string (jt_first_separator), and then */
/* then switching it to the real thing (jt_non_first_separator).     */

char           *jump_table_separator;
char           *jt_first_separator = "";
char           *jt_non_first_separator = ",\n";


void
asm_output_addr_vec_elt(file, value)
    FILE           *file;
    int             value;
{
    fprintf(file, "%s_L$%d", jump_table_separator, value);
    jump_table_separator = jt_non_first_separator;
}

void
asm_output_addr_diff_elt(file, value, rel)
    FILE           *file;
    int             value;
    int             rel;
{
    fprintf(file, "\t.word L%d-L%d\n", value, rel);
}

void
asm_output_case_label(file, prefix, num, table)
    FILE           *file;
    char           *prefix;
    int             num;
    rtx             table;
{
    rtx             pattern;
    int             table_len;

    pattern = PATTERN(table);
    table_len = XVECLEN(pattern, 0);
    fprintf(file, "\t.var _%s$%d[%d]=\n", prefix, num, table_len);
    jump_table_separator = jt_first_separator;
}


void
asm_output_reg_push (file, regno)
     FILE* file;
     int regno;
{
  fprintf (file, "\t%s(%s,%s)=%s;\n",
	   STACK_REF,                
	   sp,                       
	   reg_names [MINUS1_MREG(stack_pmode)],
	   reg_names [regno]);
};

void
asm_output_reg_pop (file, regno)
     FILE* file;
     int regno;
{
  fprintf (file, "\t%s=%s(%s,%s);\n",
	   reg_names[regno],
	   STACK_REF,
	   sp,
	   reg_names[PLUS1_MREG(stack_pmode)]);
}

/* If there is a jump in the last three slots of a loop then we need 
   to pad with NOP.

   Given that we are at the end of the loop determined by final_insn
   (final.c) we need to see if there is a branch in the last 3 slots of
   the doloop.

   This function was done in conjunction with Alan Lehotsky from Mercury

   Scan forward to see if we can find the insn that acts as the loop end, and
   then look to see that it is branching back to a special HW loop insn
   indicated by the UNSPEC_VOLATILE
*/
static int
atendof_adoloop ()
{
  int insn_seen=0;
  rtx p1;
  rtx set;
  rtx label,top;
  extern rtx final_insn;

  for ( p1=next_active_insn (final_insn);
       p1 != NULL_RTX;
       p1 = next_active_insn (p1) )
      {
	switch (GET_CODE (p1)) {
      case JUMP_INSN:
	  label = JUMP_LABEL (p1);
	  if ( !label ) continue;  /* Something weird */
	  top = next_active_insn (label);

	  if (top && GET_CODE (PATTERN (top)) == UNSPEC_VOLATILE &&
	      XINT (PATTERN (top),1) == 3)
	      return 1;

	  /* FALL-THRU */
      case CALL_INSN:
      case INSN:
	  insn_seen += get_attr_length (p1);
	  break;
	}

	if (insn_seen >= 3)
	    return 0;
      }
  return 0;
}

/*******************************************************************
  Punctuations  in templates:

  %!	==> "(DB)" or "(DB);nop;nop;" depending on opt. flags
  %?	==> "(DB); r2=fp; fp=sp"
  %^        %^ -- is the number of instructions to skip for an unsigned branch
            it is used to skip the delayed branch fields.
  %&    ==> nop; nop; if target is not Z3
  %-	SZ or EQ flag for relational branches
  %~    NOT SZ or NE flag
  %<	LT or MS
  %>	GE or NOT MS

  %J#	Complement cond code of # (EQ->NE, GT->LE)
  %M#	==> "pm(A)"/"dm(A)" with MODIFY
  %Q#	==> "pm(A)"/"dm(A)" without MODIFY (A = addr(#)).
  %a# 	(final) print reference to memory address #
  %b#	Upper word of double # (#
  %c#	(final) constant expression # without punctuation
  %f#	Register # as floating point reg (overrides #'s mode)
  %j#	Cond code of the operand # (EQ, GT, etc)	
  %l#	(final) label name of #
  %n#	(final) negated constant #
  %r#	Register N as fixed point reg (overrides #'s mode)
  %s#	output address (looks identical to %a# ???)
  %u#	Lower word of double # (#
  %D#   infix notation for , or ; for output of lcntr and do loop.

  %m#   use mode of operator and operator itself to devise how to 
        output ssi
  %L#   code a label _ln: 
  %p#   pm or dm is genrated based on the operand.
*******************************************************************/

int
print_operand_punct_valid_p(code)
     char code;
{
  static char    *legal_codes = "?!^&-~<>";
  char           *p = legal_codes;
  
  while (*p)
    if (code == *p++)
      return 1;
  return 0;
}

/* Print operand X (an rtx) in assembler syntax to file FILE.
   CODE is a letter or dot (`z' in `%z0') or 0 if no letter was specified.
   For `%' followed by punctuation, CODE is the punctuation and X is null.

   DSP21K extensions for operand codes:
*/
void
print_mult_acc_operand(file, x, part)
     FILE           *file;
     rtx             x;
     int             part;
{
  int             regno = REGNO(x);
  
  switch (regno)
    {
    case REG_MRF:      fprintf (file, "MR%dF", part);      break;
    case REG_MRB:      fprintf (file, "MR%dB", part);      break;
    default:           abort();
    }
}

void
print_operand (file, x, code)
     FILE *file;
     rtx x;
     char code;
{
  rtx arg0;
  int part = 0; /** 0 for 'b', 1 for 'u' modifier */

  while(x && GET_CODE (x) == CONST)
      x = XEXP (x, 0);

  switch (code)
    {
    case '-':			/* shifter or alu equality */
      fputs ( (cc_prev_status.flags & CC_IN_SHIFT)? "sz" : "eq", file);
      break;

    case '~':			/* shifter or alu inequality */
      fputs ( (cc_prev_status.flags & CC_IN_SHIFT)? "not sz" : "ne", file);
      break;

    case '<':			/* multiplier or alu negative */
      fputs ( (cc_prev_status.flags & CC_IN_MULT)? "ms":"lt", file);
      break;

    case '>':			/* multiplier or alu positive */
      fputs ( (cc_prev_status.flags & CC_IN_MULT)? "not ms":"ge", file);
      break;

    case 's':
      arg0 = XEXP(x, 0);
      output_address (arg0);
      break;
      
    case '?':
      fprintf(file, "(DB); r2=%s; %s=%s", fp, fp, sp);
      break;

    case '&':
      if (!ADSP210Z3u)
	fprintf (file, "nop;nop;");
      break;

    case '!':
      if (optimize > 0
	  && flag_delayed_branch
	  && dbr_sequence_length())
	fprintf(file, "(DB)");
      else if (!ADSP210Z3u || atendof_adoloop ())
	fprintf(file, "(DB); nop; nop");
      break;

      /* This is how many instructions to skip for unsigned comparisons */
    case '^':
      if (ADSP210Z3u)
	fprintf (file, "2");
      else
	fprintf (file, "4");
      break;

    case 'p':
      {
	enum machine_mode m = GET_MODE (x);
	fprintf (file, "%s", m == DMmode ? "dm" : "pm");
	break;
      }
    case 'z':
      {
	enum rtx_code op = GET_CODE (x);
	char textop = 0;

	switch (op)
	  {
	  case PLUS:
	    textop = '+';
	    break;
	  case MULT:
	    textop = '*';
	    break;
	  case MINUS:
	    textop = '-';
	    break;
	  default:
	    abort ();
	  }
	if (textop)
	  fputc (textop, file);
      }
      break;

    case 'm':
      if (GET_CODE (x) == MULT
	  && GET_MODE (x) == SImode)
	fprintf (file, "(SSI)");
      break;

    case 'D':
      if (REG_P (x) || (CONSTANT_P (x) && (INTVAL (x) < (1<<16)-1)))
	fputc (',', file),	fputc (' ', file);
      else 
	fputc (';', file),      fputc ('\n', file),     fputc ('\t', file);
      break;
      
    case 'L':
      fputc ('\n', file);
      output_asm_label (x);
      fputc (':', file);
      fputc ('\t', file);
      break;

    case 'j':
      switch (GET_CODE (x))
	{
	case EQ:
	  fprintf(file, cc_prev_status.flags&CC_IN_SHIFT?"sz":"eq");
	  break;
	case NE:
	  fprintf(file, cc_prev_status.flags&CC_IN_SHIFT?"not sz":"ne");
	  break;
	case GT:
	  fprintf(file, "gt");
	  break;
	case LT:
	  fprintf(file, cc_prev_status.flags&CC_IN_MULT?"ms":"lt");
	  break;
	case GE:
	  fprintf(file, cc_prev_status.flags&CC_IN_MULT?"not ms":"ge");
	  break;
	case LE:
	  fprintf(file, "le");
	  break;
	default:
	  output_operand_lossage("invalid %%j value");
	}
      break;
      
    case 'J':					 /* reverse logic */
      switch (GET_CODE(x))
	{
	case EQ:
	  fprintf(file, cc_prev_status.flags&CC_IN_SHIFT?"not sz":"ne");
	  break;
	case NE:
	  fprintf(file, cc_prev_status.flags&CC_IN_SHIFT?"sz":"eq");
	  break;
	case GT:
	  fprintf(file, "le");
	  break;
	case LT:
	  fprintf(file, "ge");
	  break;
	case GE:
	  fprintf(file, cc_prev_status.flags&CC_IN_MULT?"ms":"lt");
	  break;
	case LE:
	  fprintf(file, cc_prev_status.flags&CC_IN_MULT?"not ms":"gt");
	  break;
	default:
	  output_operand_lossage("invalid %%J value");
	}
      break;
    case 'M':
      {
	rtx addr = XEXP(x,0);
	fprintf (file,
		 (GET_MODE(addr) == PMmode) ? "pm(" : 
		 (GET_MODE(addr) == DMmode) ? "dm(" :
		 "<Unknown Segment!!>");
	print_operand_address_1(file, addr, DO_MODIFY_OKAY);
	fprintf (file,")");
      }
      break;	
      
    case 'P':
      if (CONSTANT_P (x))
      {
	switch (INTVAL (x))
	    {
	  case -1:	fputs ("m7", file);	break;
	  case 0:	fputs ("m5", file);	break;
	  case 1:	fputs ("m6", file);	break;
	  default:	abort();
	    }
	break;
      } else
	  goto dflt;

    case 'R':
      if (CONSTANT_P (x))
      { 
	switch (INTVAL (x))
	    {
	  case -1:	fputs ("m15", file);	break;
	  case 0:	fputs ("m13", file);	break;
	  case 1:	fputs ("m14", file);	break;
	  default:	abort();
	    }
	break;
      } else
	  goto dflt;

    case 'Q':
      {
	rtx addr = XEXP(x,0);
	fprintf (file,
		 (GET_MODE(addr) == PMmode) ? "pm(" : 
		 (GET_MODE(addr) == DMmode) ? "dm(" :
		 "<Unknown Segment!!>");
	print_operand_address_1(file, addr, DONT_DO_MODIFY);
	fprintf (file,")");
      }
      
      break;
      
    case 'u':					 /*** "Bottom" part of double mode ***/
      if (GET_CODE(x) == CONST_DOUBLE)	 /* (const_double:M ADDR I0
					  * I1 ...) */
	{
	  fprintf(file, "0x%0.8x", CONST_DOUBLE_LOW(x));
	}
      else if ((GET_CODE(x) == REG) &&
	       (IS_MULT_ACC_REG(REGNO(x))))
	print_mult_acc_operand(file, x, 1);
      else
	print_operand(file, x, 0);
      break;
      
    case 'b':					 /* "Upper"  part of double
						  * register */
      
      part = 1;
      if (GET_CODE(x) == CONST_DOUBLE)	 /* (const_double:M ADDR I0
					  * I1 ...) */
	{
	  fprintf(file, "0x%0.8x", CONST_DOUBLE_HIGH(x));
	  break;
	}
      if (GET_CODE(x) != REG)
	abort();
      {
	int             regno = REGNO(x);
	
	if (IS_MULT_ACC_REG(regno))
	  print_mult_acc_operand(file, x, 0);
	else
	  {
	    regno++;
	    switch (GET_MODE(x))
	      {
	      case DFmode:
		if (!DOUBLE_IS_32) goto def;  /* Avoid float ALU */
		/* fall thru */
		    
	      case SCmode:
		fprintf(file, "%s", alt_reg_names[regno]);
		break;

	      case CSImode:
	      case DImode:
def:
		fprintf(file, "%s", reg_names[regno]);
		break;

	      default:
		fprintf(file,
			"<< Only double mode can be printed with '%%u'");
	      }
	  }
      }
      break;
      
    default:
dflt:
      switch (GET_CODE (x))
	{
	case SUBREG:
	  if ((GET_MODE (x) == SFmode || (DOUBLE_IS_32 && GET_MODE (x) == DFmode))
	      && REGNO (SUBREG_REG (x)) < (sizeof (alt_reg_names)/sizeof (char *)))
	    fprintf (file, alt_reg_names [REGNO (SUBREG_REG (x))]);
	  else
	    fprintf (file, reg_names [REGNO (SUBREG_REG (x))]);
	  break;
	case REG:
	  {
	    int regno = REGNO(x);
	    enum machine_mode mode = (code == 'f')
	      ? SFmode
		: (code == 'r')
		  ? SImode
		    : GET_MODE(x);
	    if((mode==SFmode || (mode==DFmode && DOUBLE_IS_32) || mode==SCmode) 
	       && regno < (sizeof (alt_reg_names)/sizeof (char *)))
	      fprintf(file,alt_reg_names[regno]);
	    else
	      fprintf(file,reg_names[regno]);
	  }
	  break;
	case MEM:
	{
	  rtx addr = XEXP(x,0);
	  switch(GET_MODE(addr))
	    {
	    case PMmode:
	      fprintf(file, "pm");
	      break;
	    case DMmode:
	      fprintf(file, "dm");
	      break;
	    default:
	      abort();
	    }
	  fprintf (file,"(");
	  output_address (addr);
	  fprintf (file,")");
	}
      break;

    case CONST_INT:
      output_addr_const(file, x);
      break;

    case CONST_DOUBLE:
      {
	union { double d;
		int    i[2];
	      } u;

	u.i[0] = CONST_DOUBLE_LOW(x);
	u.i[1] = CONST_DOUBLE_HIGH(x);

	ASM_OUTPUT_FLOAT (file, u.d);
      }
      break;
    default:
      output_addr_const(file, x);
    }
  }
}


int
hardwired_mregister(oper, mode)
    rtx             oper;
    enum machine_mode mode;

{
    switch (INTVAL(oper))
    {
    case 0:
	return ZERO_MREG(mode);
    case 1:
	return PLUS1_MREG(mode);
    case -1:
	return MINUS1_MREG(mode);
    }
return (0);
}

/** Note that MODE is not that of the resulting RTX, but of the  **/
/** whole address in which it will be used. The mode servers for **/
/** determining which DAG the hardwired register must come from  **/

rtx
hardwired_mreg_rtx(x, mode)
    rtx             x;
    enum machine_mode mode;

{
    return gen_rtx(REG, SImode, hardwired_mregister(x, mode));
}

static void
print_operand_address_1(file, addr, modify)
    FILE           *file;
    register rtx    addr;
    memory_access_t modify;
{

    switch (GET_CODE(addr))
    {
    case SUBREG:
	if (modify == DONT_DO_MODIFY)
	    fprintf(file, "%s,%s", reg_names[ZERO_MREG(GET_MODE(addr))],
		    reg_names[REGNO(SUBREG_REG(addr))]);
	else
	    fprintf(file, "%s,%s", reg_names[REGNO(SUBREG_REG(addr))],
		    reg_names[ZERO_MREG(GET_MODE(addr))]);
	break;
    case REG:
	if (modify == DONT_DO_MODIFY)
	    fprintf(file, "%s,%s", reg_names[ZERO_MREG(GET_MODE(addr))],
		    reg_names[REGNO(addr)]);
	else
	    fprintf(file, "%s,%s", reg_names[REGNO(addr)],
		    reg_names[ZERO_MREG(GET_MODE(addr))]);
	break;

    case POST_DEC:
	fprintf(file, "%s,%s", reg_names[SUBREGNO(XEXP(addr, 0))],
		reg_names[MINUS1_MREG(GET_MODE(addr))]);
	break;
    case POST_INC:
	fprintf(file, "%s,%s", reg_names[SUBREGNO(XEXP(addr, 0))],
		reg_names[PLUS1_MREG(GET_MODE(addr))]);
	break;

    case POST_MODIFY:
	fprintf(file, "%s,", reg_names[SUBREGNO(XEXP(addr, 0))]);
	if (CONSTANT_P (XEXP (addr, 1)))
	  {
	    rtx constant = XEXP (addr, 1);
	    
	    if (GET_CODE (constant) == CONST
		&& XEXP (constant, 0) == const1_rtx)

	      fprintf (file, "%s", reg_names [PLUS1_MREG (GET_MODE (addr))]);

	    else if (GET_CODE (constant) == CONST
		     && XEXP (constant, 0) == constm1_rtx)
	      fprintf (file, "%s", reg_names [MINUS1_MREG (GET_MODE (addr))]);
	    else
	      output_addr_const(file, XEXP(addr, 1));
	  }
	else
	    fprintf(file, "%s", reg_names[SUBREGNO(XEXP(addr, 1))]);
	break;

    case PLUS:
    case MINUS:
	{
	    rtx op0 = XEXP(addr, 0);
	    rtx op1 = XEXP(addr, 1);
	    rtx temp;
	    int swap_operands = (REG_P (op0) || GET_CODE (op0) == SUBREG)
	                         && (IS_IREG (SUBREGNO (op0)));
	    if (swap_operands)
		temp = op0, op0 = op1, op1 = temp;
	    
	    print_operand(file, op0, 0);
	    if(CONSTANT_P(op0) && CONSTANT_P(op1))
	      fputc('+',file);
	    else
	      fputc(',', file);
	    print_operand(file, op1, 0);
	}
	break;
    default:
	output_addr_const(file, addr);
    }
}


void
print_operand_address(file, addr)
     FILE           *file;
     rtx    addr;
{
  print_operand_address_1(file, addr, DO_MODIFY_OKAY);
}

struct asm_option		/* Replication from toplev.c */
{
  char *string;
  int *variable;
  int on_value;
};

#define MAX_LINE 79

static int
output_option (file, type, name, pos)
     FILE *file;
     char *type;
     char *name;
     int pos;
{
  int type_len = strlen (type);
  int name_len = strlen (name);

  if (1 + type_len + name_len + pos > MAX_LINE)
    {
      fprintf (file, "\n" ASM_COMMENT_START " %s%s", type, name);
      return 3 + type_len + name_len;
    }
  fprintf (file, " %s%s", type, name);
  return pos + 1 + type_len + name_len;
}

static struct { char *name; int value; } m_options[] = TARGET_SWITCHES;

void
output_options (file, f_options, f_len, W_options, W_len)
     FILE *file;
     struct asm_option *f_options;
     int f_len;
     struct asm_option *W_options;
     int W_len;
{
  int j;
  int flags = target_flags;
  int pos = 32767;
  extern char *version_string;

  fprintf (file, ASM_COMMENT_START "GNU CC %s", version_string);

  if (optimize)
    {
      char opt_string[20];
      sprintf (opt_string, "%d", optimize);
      pos = output_option (file, "-O", opt_string, pos);
    }

  for (j = 0; j < f_len; j++)
    {
      if (*f_options[j].variable == f_options[j].on_value)
	pos = output_option (file, "-f", f_options[j].string, pos);
    }

  for (j = 0; j < W_len; j++)
    {
      if (*W_options[j].variable == W_options[j].on_value)
	pos = output_option (file, "-W", W_options[j].string, pos);
    }

  for (j = 0; j < sizeof m_options / sizeof m_options[0]; j++)
    {
      if (m_options[j].name[0] != '\0'
	  && m_options[j].value > 0
	  && ((m_options[j].value & flags) == m_options[j].value))
	{
	  pos = output_option (file, "-m", m_options[j].name, pos);
	  flags &= ~ m_options[j].value;
	}
    }

  fputs ("\n\n", file);
}

/******** Functions to print the header and the tail of ********/
/******** the assembler file **********************************/

void
asm_file_start(file)
     FILE *file;
{
  extern char    *input_filename;

  fp = reg_names[FRAME_POINTER_REGNUM];
  sp = reg_names[STACK_POINTER_REGNUM];
  
  fprintf(file, "!\t\tAnalog Devices ADSP210x0\n");
  fprintf(file, ".segment /pm %s; .file \"%s\"; .endseg;\n",
	  pmcode_segment, input_filename);
  fprintf(file, ".segment /dm %s;\n", dmdata_segment);
#ifdef ADI_PROFILER
    if (PROFILE) {
	emuclock_rtx = 
	    gen_rtx(ASM_OPERANDS, VOIDmode,
		    "%0=emuclk;", "=d", 0,
		    rtvec_alloc(0), rtvec_alloc(0), "???", 1);
	MEM_VOLATILE_P(emuclock_rtx) = 1;
    }
#endif
}

void
asm_file_end (file)
     FILE *file;
{
    fprintf(file, ".endseg;\n");

#if defined(KEEP_STAT) && defined(ADSP_PROFILE_STATUS)
    if (KEEP_STAT)
	write_stat_file();
#endif
}


/*
 * BEGIN: Operand Predicates --->   (END)
 *   For subregs, switch to the reg (SUBREG_REG)
 */

#define SWITCH_SUBREG_AND_MODE(X,M) {if(GET_CODE(X)==SUBREG) \
{X=SUBREG_REG(X); M=GET_MODE(X);}}


/** This is a kludge, but it seems to work. ***/
int 
really_nonmemory(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int result;

    SWITCH_SUBREG_AND_MODE(op, mode);
    result = nonmemory_operand(op, mode);
    return (result);
}

int 
mreg_or_6bit_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int result;
  
  if (GET_MODE(op) != mode && mode != VOIDmode)
    return 0;
  SWITCH_SUBREG(op);
  
  if (REG_P (op))
      result = (IS_PSEUDO (REGNO (op)) && !REGNO_POINTER_FLAG (REGNO (op)) ) ||
	  IS_MREG (REGNO (op));
  else if ( CONST_INT_P (op))
    result = const_ok_for_letter_p(INTVAL(op), 'J');
  else
    result = 0;
  
  return (result);
}

int 
indexreg_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int result;
  
  if (GET_MODE(op) != mode && mode != VOIDmode)
    return 0;
  SWITCH_SUBREG(op);
  
  result = REG_P (op) && 
	    (IS_PSEUDO (REGNO (op)) || (IS_IREG (REGNO (op))));
  return (result);
}


int
modifyreg_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  SWITCH_SUBREG(op);
  if (GET_MODE(op) != mode && mode != VOIDmode)
    return 0;

  if (REG_P (op))
      {
	  int reg = REGNO (op);

	  return (IS_PSEUDO (reg) && !REGNO_POINTER_FLAG (reg))
	      || IS_MREG (reg);
      }

  return 0;
}

int
modifyreg_or_zm1p1_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int             result;
  
  if (GET_MODE(op) != mode && mode != VOIDmode)
    return 0;
  SWITCH_SUBREG(op);
  if (REG_P(op))
    result = ( IS_PSEUDO(REGNO(op)) && !REGNO_POINTER_FLAG (REGNO(op)))
	      || IS_MREG(REGNO(op));
  else if (GET_CODE(op) == CONST_INT)
    switch (INTVAL(op))
      {
      case 1:
      case -1:
      case 0:
	result = 1;
	break;
      default:
	result = 0;
      }
  return (result);
}

/**** returns true for  Modifier Registers and 6 bit constants **/

int 
mod6_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int result, bit6_p;
  bit6_p = CONSTANT_6BIT_P(op);
  result = bit6_p || modifyreg_operand(op, mode);
  return (result);
}

/**** returns true for  Modifier Registers and integer constants **/

int 
mod32_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int result, const_p;
  
  const_p = CONST_INT_P(op);
  result = const_p || modifyreg_operand(op, mode);
  return (result);
}

/**** returns true for  Modifier Registers and integer constants **/

int 
mod_or_const_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int result, const_p, code = GET_CODE(op);

  /*
   * Should we be worried about preindexed addressing here?
   */
  const_p = (code == CONST_INT || code == SYMBOL_REF || code == LABEL_REF ||
	     (code == CONST && mode == GET_MODE (op)));
  result = const_p || modifyreg_operand(op, mode);
  return (result);
}

int 
datareg_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;
    SWITCH_SUBREG(op);
    result = (GET_CODE(op) == REG
	      && (IS_PSEUDO (REGNO(op)) || (IS_DREG (REGNO(op)))));
    return (result);
}

int 
dreg_or_6bit_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    if (CONST_INT_P (op))
	return const_ok_for_letter_p(INTVAL(op), 'J');

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;

    SWITCH_SUBREG(op);

    if (GET_CODE(op) == REG)
	return (IS_PSEUDO (REGNO(op)) || (IS_DREG (REGNO(op))));

    return 0;
}

int 
dreg_or_8bit_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    if (GET_CODE(op) == CONST_INT)
	return const_ok_for_letter_p(INTVAL(op), 'L');

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;

    SWITCH_SUBREG(op);

    if (GET_CODE(op) == REG)
	return (IS_PSEUDO (REGNO(op)) || (IS_DREG (REGNO(op))));

    return 0;
}

int 
dreg_or_const_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    if (GET_CODE(op) == CONST_INT)
      return 1;

    if (GET_MODE(op) != mode && mode != VOIDmode)
      return 0;
    SWITCH_SUBREG(op);
    if (GET_CODE(op) == REG)
      return (IS_PSEUDO (REGNO(op)) || (IS_DREG (REGNO(op))));
    return 0;
}


int 
datareg_si_or_sf_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result = 0;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;

    SWITCH_SUBREG(op);

    if (GET_MODE(op) == SImode || GET_MODE(op) == SFmode)
    {
	result = (GET_CODE(op) == REG
		  && (IS_PSEUDO (REGNO(op)) || (IS_DREG (REGNO(op)))));
    }

    return (result);
}

int 
datareg_si_or_sf_or_const1_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result = 0;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;
    SWITCH_SUBREG(op);

    if (GET_MODE(op) == SImode || GET_MODE(op) == SFmode)
    {
	result = ((GET_CODE(op) == REG
		   && (IS_PSEUDO (REGNO(op))
		       || (IS_DREG (REGNO(op)))))
		  || (op == const1_rtx) || (op == constm1_rtx));
    }
    return (result);
}

int 
datareg_si_or_sf_or_const_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result = 0;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;
    SWITCH_SUBREG(op);

    if (GET_MODE(op) == SImode || GET_MODE(op) == SFmode)
    {
	result = ((GET_CODE(op) == REG
		   && (IS_PSEUDO (REGNO(op))
		       || (IS_DREG (REGNO(op)))))
		  || (GET_CODE(op) == CONST_INT));
    }
    return (result);
}

int 
mr_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;
    SWITCH_SUBREG(op);
    result = (GET_CODE(op) == REG
	      && (IS_PSEUDO (REGNO(op))
		  || IS_MULT_ACC_REG (REGNO (op))));

    return (result);
}


int 
datreg_or_mr_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;
    SWITCH_SUBREG(op);
    result = (GET_CODE(op) == REG
	      && (IS_PSEUDO(REGNO(op))
		  || IS_DREG(REGNO(op))
		  || IS_MULT_ACC_REG (REGNO (op)) ));
    return (result);
}

int 
datreg_or_mr_or_zero_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    int             result;

    if (GET_MODE(op) != mode && mode != VOIDmode)
	return 0;

    if (op == const0_rtx)
      return 1;

    SWITCH_SUBREG(op);
    result = (GET_CODE(op) == REG
	      && (IS_PSEUDO(REGNO(op))
		  || IS_DREG(REGNO(op))
		  || IS_MULT_ACC_REG (REGNO (op)) ));
    return (result);
}

int
indexed_by_lit6 (rtx op, enum machine_mode mode)
{
  rtx mem;
  rtx op0,op1;

  if (GET_CODE (op) != MEM)
    return 0;

  mem = XEXP(op, 0);

  switch (GET_CODE(mem))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
      return 0;
    case POST_MODIFY:
      if (REG_P(XEXP(mem, 1))
	  || (CONSTANT_P(XEXP(mem, 1))
	      && const_ok_for_letter_p(INTVAL(XEXP(mem, 1)), 'J')))
	return 1;
      return 0;

    case PLUS:
    case MINUS:
      op0 = XEXP(mem, 0), op1 = XEXP(mem, 1);

      if (GET_CODE(op1) == CONST_INT
	  && const_ok_for_letter_p(INTVAL(op1), 'J'))
	return 1;

      if (GET_CODE(op0) == CONST_INT
	  && const_ok_for_letter_p(INTVAL(op0), 'J'))
	return 1;
    }
  return 0;
}

int 
parallel_move_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
    rtx             mem,
                    op0,
                    op1;

    if (!(mode == SImode || mode == SFmode || mode == DMmode || mode == PMmode))
	return 0;
    if (!(GET_MODE(op) == SImode || GET_MODE(op) == SFmode 
	|| GET_MODE(op) == DMmode || GET_MODE(op) == PMmode))
	return 0;

    SWITCH_SUBREG(op);

    if (REG_P(op))
	return ! IS_MULT_ACC_REG (REGNO(op));

    else if (GET_CODE(op) == MEM)
    {
	mem = XEXP(op, 0);

	switch (GET_CODE(mem))
	{
	case REG:
	case POST_INC:
	case POST_DEC:
	    return 1;
	case POST_MODIFY:
	    if (REG_P(XEXP(mem, 1))
		|| (CONSTANT_P(XEXP(mem, 1))
		    && const_ok_for_letter_p(INTVAL(XEXP(mem, 1)), 'J')))
		return 1;
	    return 0;

	case PLUS:
	case MINUS:
	    op0 = XEXP(mem, 0), op1 = XEXP(mem, 1);

	    if ((REG_P(op0) && IS_IREG(SUBREGNO(op0)))
		&& ((REG_P(op1) && IS_MREG(SUBREGNO(op1)))
		    || (GET_CODE(op1) == CONST_INT
			&& const_ok_for_letter_p(INTVAL(op1), 'J'))))
		return 1;

	    if ((REG_P(op1) && IS_IREG(REGNO(op1)))
		&& ((REG_P(op0) && IS_MREG(REGNO(op0)))
		    || (GET_CODE(op0) == CONST_INT
			&& const_ok_for_letter_p(INTVAL(op0), 'J'))))

		return 1;

	default:
	    break;
	}
    }
    return 0;
}

int
parallel_move_or_register_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (mode == VOIDmode) mode = SImode;
  return REG_P (op) || parallel_move_operand (op, mode);
}

int
parallel_move_or_register_or_lit_operand (op, mode)
  rtx op;
  enum machine_mode mode;
{
  return (is_small_constant (op)
	  || parallel_move_or_register_operand (op, mode));
}



int
legit_parallel_move_insn (insn, forinsn)
     rtx insn, forinsn;
{
  rtx pat, src, dest;

  pat = PATTERN (insn);
  src = SET_SRC (pat);
  dest = SET_DEST (pat);

  return ((REG_P (dest) && (REG_P (src) || is_small_constant (src)))
	  || ((GET_CODE (src) == MEM) && REG_P (dest)
	      && (DATA_REG_P (dest) || !indexed_by_lit6 (src, VOIDmode)))
	  || ((GET_CODE (dest) == MEM)
	      && (REG_P (src) || is_small_constant (src))
	      && (DATA_REG_P (src) || !indexed_by_lit6 (dest, VOIDmode))));
}

/*** post_modified_operand(op, mode) returns 1 if OP is a memory operand **/
/*** with POST_INC or POST_DEC ***/

int 
post_incremented_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  if (GET_CODE(op) == MEM)
    {
      rtx             addr = XEXP(op, 0);
      enum rtx_code   code = GET_CODE(addr);
      
      return (code == POST_INC);
    }
  return 0;
}

int 
post_decremented_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  if (GET_CODE(op) == MEM)
    {
      rtx             addr = XEXP(op, 0);
      enum rtx_code   code = GET_CODE(addr);
      
      return (code == POST_DEC);
    }
  return 0;
}

int 
post_modified_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  enum rtx_code   code = GET_CODE(op);
  
  return (code == POST_DEC
	  || code == POST_INC);
  /*	  || code == POST_MODIFY);*/
}

int
immediate_or_indexreg_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
     
{
  return immediate_operand(op, mode) 
    || indexreg_operand(op, mode);
}

int
immediate_or_datareg_operand(op, mode)
     register rtx    op;
     enum machine_mode mode;
     
{
  return immediate_operand(op, mode) 
    || datareg_operand(op, mode);
}

int
dag_immediate_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  switch (GET_CODE (op)) 
  {
    case CONST:	return dag_immediate_operand ( XEXP (op, 0), mode);
    case PLUS:	return dag_immediate_operand ( XEXP (op, 0), mode)
	            && dag_immediate_operand ( XEXP (op, 1), mode);

    case CONST_INT:
    case SYMBOL_REF:
    case LABEL_REF:	return 1;

    default:	return 0;
  }
}

int
reg_or_imm_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  return dag_immediate_operand(op, mode) 
    || register_operand(op, mode);
}

int 
always_true(op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  return 1;
}

int 
compute_shift_immediate_p(op)
     rtx op;
{
  return compute_shiftop(op, SImode) 
    && GET_CODE(XEXP(op, 1)) == CONST_INT;
}


/*
 * Operator predicates.
 */
int
plus_or_minus_operator(op, mode)
    rtx             op;
    enum machine_mode mode;
{
  return (GET_CODE(op) == PLUS || GET_CODE(op) == MINUS);
}

int
compute_binaryop (op, mode)
    rtx             op;
    enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case PLUS:
    case MINUS:
    case MULT:
    case SMAX:
    case SMIN:
      return 1;
    case AND:
    case IOR:
    case XOR:
    case LSHIFT:
    case ASHIFT:
    case ROTATE:
      return mode == SImode;
    default:
      return 0;
    }
}

/*
 * Match only SIGNED relational operators
 */

int
signed_comparison_operator (rtx op, enum machine_mode mode)
{
  switch (GET_CODE(op))
    {
    case LEU:
    case LTU:
    case GTU:
    case GEU:
      return 0;
    default:
      return comparison_operator (op, mode);
    }
}

int
compute_shiftop (op, mode)
     rtx             op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case LSHIFT:
    case ASHIFT:
    case ROTATE:
      return mode == SImode;
    default:
      return 0;
    }
}

int
compute_unaryop (op, mode)
     rtx op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case NOT:
      return mode == SImode;
    case ABS:
    case NEG:
      return 1;
    default:
      return 0;
    }
}

int
infix_21k_operatorp (op, mode)
     rtx op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case PLUS:
    case MINUS:
    case MULT:
      return 1;
    default:
      return 0;
    }
}

int program_memory_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  register enum rtx_code code = GET_CODE(op);
  int             mode_altering_drug = 0;
  extern int      volatile_ok;
  
  if (mode != PMmode &&
      GET_MODE(op) != PMmode &&
      GET_MODE(op) != VOIDmode)
    return 0;
  
  if (CONSTANT_P(op))
    return ((GET_MODE(op) == VOIDmode || GET_MODE(op) == mode)

#ifdef LEGITIMATE_PIC_OPERAND_P
	    && (!flag_pic || LEGITIMATE_PIC_OPERAND_P(op))
#endif
	    && LEGITIMATE_CONSTANT_P(op));
  if (code == SUBREG)
    {
      
#ifdef INSN_SCHEDULING
      
	/*
	 * On machines that have insn scheduling, we want all memory
	 * reference to be explicit, so outlaw paradoxical SUBREGs.
	 */
	if (GET_CODE(SUBREG_REG(op)) == MEM
	    && GET_MODE_SIZE(mode) > GET_MODE_SIZE(GET_MODE(SUBREG_REG(op))))
	    return 0;
#endif

	op = SUBREG_REG(op);
	code = GET_CODE(op);

#if 0

	/*
	 * No longer needed, since (SUBREG (MEM...)) will load the MEM into a
	 * reload reg in the MEM's own mode.
	 */
	mode_altering_drug = 1;
#endif
    }

    if (code == REG)
	/* A register whose class is NO_REGS is not a general operand.  */
	return (IS_PSEUDO (REGNO(op))
		|| REGNO_REG_CLASS (REGNO(op)) != NO_REGS);

    if (code == MEM)
    {
	register rtx    y = XEXP(op, 0);

	if (!volatile_ok && MEM_VOLATILE_P(op))
	    return 0;
	/* Use the mem's mode, since it will be reloaded thus.  */
	mode = GET_MODE(op);
	GO_IF_LEGITIMATE_ADDRESS(mode, y, win);
    }
    return 0;

win:
    if (mode_altering_drug)
	return !mode_dependent_address_p(XEXP(op, 0));
    return 1;
}

int 
no_mr_operand(rtx op, enum machine_mode mode)
{
  if (REG_P(op) && IS_MULT_ACC_REG (REGNO(op)))
    return 0;
  return general_operand (op, mode);
}

/* Allow anything BUT a MR register */
int
ureg_operand (rtx op, enum machine_mode mode)
{
  return (REG_P(op) && !IS_MULT_ACC_REG (REGNO(op)));
}

int
xxmr_operand(op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (REG_P(op) && IS_MULT_ACC_REG (REGNO (op)))
    return 1;
  return 0;
}

int 
dreg_or_zero_operand(op, mode)
     rtx             op;
     enum machine_mode mode;
{
  SWITCH_SUBREG(op);
  if ((REG_P(op) && IS_DREG(REGNO(op))) 
      || op == const0_rtx)
    return 1;
  return 0;
}

/*
 * END: Predicates
 */


/*
  ; compute,  dm(Ia,Mb) = ureg		M = d
  ; compute,  dm(Mb,Ia) = ureg		T = d
  ; compute,  ureg = dm(Ia,Mb)		dwx = M
  ; compute,  ureg = dm(Mb,Ia)		dwx = T
  ;
  ; compute,  dm(Ia,<6bit>) = dreg	Q =
  ; compute,  dm(<6bit),Ia) = dreg
  ; compute,  dreg = dm(Ia,<6bit>)
  ; compute,  dreg = dm(<6bit>,Ia)
  ;
  ; compute,  ureg = ureg
  ;
  ;
  ; shiftimm, dm(Ia,Mb) = dreg
  ; shiftimm, dreg = dm(Ia,Mb)
  ; compute,  modify(Ia,Mb)
  ;
  ; (post_inc ...)   (post_dec ...)   ireg
  ; (plus ireg 6bit) (plus ireg mreg)
  ; reg = reg
  ;
  */
int 
parallel_move_okay_p(lhs, rhs)
     rtx             lhs, rhs;
{
  rtx mem, reg, op0, op1;

  if (REG_P(lhs) && REG_P(rhs))
    return 1;
  else if (GET_CODE(lhs) == MEM && REG_P(rhs))
    mem = XEXP(lhs, 0), reg = rhs;
  else if (REG_P(lhs) && GET_CODE(rhs) == MEM)
    mem = XEXP(rhs, 0), reg = lhs;
  else
    return 0;
  
  if (!IS_DREG (REGNO(reg)))
    return 0;
  
  switch (GET_CODE(mem))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
      return 1;
      
    case POST_MODIFY:
      if (REG_P(XEXP(mem, 1))
	  || (CONSTANT_P(XEXP(mem, 1))
	      && const_ok_for_letter_p(INTVAL(XEXP(mem, 1)), 'J')))
	return 1;
      return 0;
      
    case PLUS:
    case MINUS:
      op0 = XEXP (mem, 0), op1 = XEXP(mem, 1);
      if ((REG_P (op0) && IS_IREG (REGNO (op0)))
	  && ((REG_P (op1) && IS_MREG (REGNO (op1)))
	      || (CONST_INT_P (op1)
		  && const_ok_for_letter_p (INTVAL (op1), 'J'))))
	return 1;
      if ((REG_P (op1) && IS_IREG (REGNO (op1)))
	  && ((REG_P (op0) && IS_MREG (REGNO (op0)))
	      || (CONST_INT_P (op0)
		  && const_ok_for_letter_p (INTVAL (op0), 'J'))))
	return 1;
      
    default:
      break;
    }
  return 0;
}

int 
parallel_move_okay_for_shiftimm_p(lhs, rhs)
    rtx             lhs, rhs;
{
  rtx mem, reg, op0, op1;
  
  if (REG_P (lhs) && REG_P (rhs))
    return 0;
  else if (GET_CODE(lhs) == MEM && REG_P(rhs))
    mem = XEXP(lhs, 0), reg = rhs;
  else if (REG_P(lhs) && GET_CODE(rhs) == MEM)
    mem = XEXP(rhs, 0), reg = lhs;
  else
    return 0;
  
  if (!IS_DREG (REGNO (reg)))
    return 0;
  
  switch (GET_CODE(mem))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
    case POST_MODIFY:
      return 1;
    default:
      break;
    }
  return 0;
}

int 
const_ok_for_letter_p(value, letter)
     int             value;
     char            letter;
{
  int             low,  high = -1,  result;

  switch (letter)
    {
    case 'I':
      return value == 1;
    case 'N':
      return value == -1;
    case 'O':
      return value == 0;
    case 'J':		/* Six bits, sign extended */
      return (value >= -32) && (value <= 31);
    case 'K':
      return (unsigned int) value <= 0x00ffffff;
    case 'L':
      high = 127;
      break;
    case 'M':
      return 1;
    case 'P':
      return (value >= -1 && value <= 1);
    default:
      break;
    }
  low = -(high + 1);
  result = (low <= value) && (value <= high);
  return (result);
}

/*(doc "Register Classes" "`SECONDARY_INPUT_RELOAD_CLASS (CLASS, MODE, X)'")*/

/*  X <- (CLASS,MODE) */
enum reg_class
secondary_output_reload_class(class, mode, x)
     enum reg_class  class;
     enum machine_mode mode;
     rtx x;
{
    enum reg_class result = NO_REGS;

    SWITCH_SUBREG(x);

    if (reg_classes_intersect_p (class, MULT_ACC_REGS) &&
	     ! (datareg_operand(x, mode)))
	result = D_REGS;

    else if (memory_const_address_p(x)) 
	result = NO_REGS;

    else if ( ! (reg_class_subset_p(class, d_DAG1_REGS)) &&
	     (memory_ref_p(x, PMmode)) &&
	     ! memory_const_address_p(x))
	result = d_DAG1_REGS;
    
    else if ( ! (reg_class_subset_p(class, d_DAG2_REGS)) &&
	     (memory_ref_p(x, DMmode)) &&
	     ! memory_const_address_p(x))
	result = mode==PMmode ? d_DAG2_REGS : D_REGS;
    else if(REG_P(x) && 
	    TEST_HARD_REG_BIT(reg_class_contents[(int)MULT_ACC_REGS],REGNO (x)) &&
	    !reg_class_subset_p(class,D_REGS))
      return D_REGS;

    return(result);
}

/**** Operations on registers classes ***/

enum reg_class
reg_class_complement(c1, c2)
    enum reg_class c1, c2;
{
    enum reg_class *scp, result = NO_REGS;
    for (scp = &reg_class_subclasses[c1][0];
	 *scp == LIM_REG_CLASSES && ! reg_classes_intersect_p (*scp, c2);
	 scp++) {
	result = *scp;
    }
    return (result);
}


/*   (CLASS,MODE) <- X  */
enum reg_class 
secondary_input_reload_class(class,mode,x)
    enum reg_class class;
    enum machine_mode mode;
    rtx x;
{
    enum reg_class result = NO_REGS;

    SWITCH_SUBREG(x);

    if (reg_classes_intersect_p (class, MULT_ACC_REGS) &&
	! (datareg_operand(x, mode)))
      result = D_REGS;
    else if ( ! (reg_class_subset_p(class, d_DAG1_REGS)) &&
	     (memory_ref_modified_p(x, PMmode)))
      result = d_DAG1_REGS;
    
    else if ( ! (reg_class_subset_p(class, d_DAG2_REGS)) &&
	     (memory_ref_modified_p(x, DMmode)))
      result = mode == PMmode ? d_DAG2_REGS : D_REGS;
    else if(REG_P(x) && 
	    TEST_HARD_REG_BIT(reg_class_contents[(int)MULT_ACC_REGS], REGNO (x)) &&
	    !reg_class_subset_p(class,D_REGS))
      return D_REGS;
    else
      switch(GET_CODE(x))
	{
	case POST_INC:
	  {
	    /* we're reloading a post_inc operation
	       check to see what we're post-inc'ing */
	    enum reg_class c1, c2;
	    rtx sub = XEXP(x,0);
	    /* it had better be a register */
	    if( ! REG_P(sub))
	      abort();
	    
	    /* if it is a register, perhap's its a pseudo 
	       if it is a pseudo, get the memory location */
	    if (IS_PSEUDO (REGNO (sub)))
	      sub=reg_equiv_mem[REGNO (sub)];
	    if(!sub)
	      abort();
	    /* 
	      okay, now sub is what we're REALLY trying to reload.
	      so see if we can go directly from sub to class and class to sub
	      we've got to do some funky stuff here to get the intersection.  
	      If one of the secondary_*_reload_class() returns NO_REGS, and
	      the other returns XYZ, we want XYZ.  If one returns XY and the
	      other YZ, we want Y.  If both return NO_REGS, we want NO_REGS,
	      and if one returns X and the other Z, we're in trouble.
	      */
	    c1 = secondary_input_reload_class(class,GET_MODE(sub),sub);
	    c1 = (c1==NO_REGS)?GENERAL_REGS:c1;
	    c2 = secondary_output_reload_class(class,GET_MODE(sub),sub);
	    c2 = (c2==NO_REGS)?GENERAL_REGS:c2;
	    result = reg_class_intersection(c1,c2);
	    if(result==NO_REGS)
	      abort();
	    result = (result==GENERAL_REGS)?NO_REGS:result;
	    break;
	  }
	}
    return(result);
}

enum reg_class 
nonempty_complement(c1, c2)
    enum reg_class c1, c2;
{
    int comp = reg_class_complement(c1, c2);
    return (comp == NO_REGS) ? c1 : comp;
}

enum reg_class
preferred_output_reload_class(x, class)
    rtx x;
    enum reg_class class;
{
    enum reg_class result = 
	reg_class_intersection
	    (class,
	     secondary_output_reload_class(class, GET_MODE(x), x));
    
    return result == NO_REGS ? class : result;
}

/*(doc "Register Classes" "`PREFERRED_RELOAD_CLASS (X, CLASS)'")*/
enum reg_class
preferred_reload_class(x,class)
    rtx x;
    enum reg_class class;
{
    enum reg_class result = 
	reg_class_intersection
	    (class,
	     secondary_input_reload_class(class, GET_MODE(x), x));
    
    return result == NO_REGS ? class : result;
}



/*** return true if X is memory reference to DAG specified by MODE **/
/** and with post-modification **/


int
memory_ref_modified_p (x, mode)
    rtx x;
    enum machine_mode mode;
{
    rtx addr;
    enum rtx_code addr_code;

    if (GET_CODE (x) != MEM) return 0;
    addr = XEXP(x,0);
    if (mode != GET_MODE(addr)) return 0;
    addr_code = GET_CODE(addr);
    return ((addr_code == POST_MODIFY) ||
	    (addr_code == POST_INC) ||
	    (addr_code == POST_DEC));
}


int
memory_ref_non_modified_p (x, mode)
    rtx x;
    enum machine_mode mode;
{
    rtx addr;
    enum rtx_code addr_code;

    if (GET_CODE (x) != MEM) return 0;
    addr = XEXP(x,0);
    if (mode != GET_MODE(addr)) return 0;

    SWITCH_SUBREG(addr);

    addr_code = GET_CODE(addr);
    return ((addr_code == PLUS) ||
	    (addr_code == MINUS) ||
	    (CONSTANT_P (addr)) ||
	    (REG_P (addr)));
}

/** In memory_ref_p MODE refers to that of the address (!), not MEM. ***/

int 
memory_ref_p(x, mode)
    rtx x;
    enum machine_mode mode;
{
    return (memory_ref_modified_p(x,mode) ||
	    memory_ref_non_modified_p(x,mode));
}

int
memory_const_address_p (x)
    rtx x;
{
    rtx addr;

    if (GET_CODE (x) != MEM) return 0;
    addr = XEXP(x,0);
    return (CONSTANT_P(addr));
}

extern int somewhere_in_reload; /* this is defined in reload1.c to be true during
				   almost the entire reload pass */

int
reloaded_reg_ok_for_base_p(reg, mode, rindex)
    rtx reg;
    enum machine_mode mode;
    int rindex[];
{
  if(rindex[0] >= 0 && reload_in_reg[rindex[0]] == reg)
    return reload_reg_class[rindex[0]] == BASE_REG_CLASS(mode);
  else if(rindex[1] >= 0 && reload_in_reg[rindex[1]] == reg)
    return reload_reg_class[rindex[1]] == BASE_REG_CLASS(mode);
  else
    {
      SWITCH_SUBREG(reg);

      if (REG_P(reg) && REG_OK_FOR_BASE_P(reg, mode))
	  if (somewhere_in_reload)
	      return (!IS_PSEUDO (REGNO (reg)));
          else
	      return 1;
      else
	  return 0;
    }
}

int
reloaded_reg_ok_for_index_p(reg, mode, rindex)
    rtx reg;
    enum machine_mode mode;
    int rindex[];
{
  if(rindex[0] >= 0 && reload_in_reg[rindex[0]] == reg)
    return reload_reg_class[rindex[0]] == INDEX_REG_CLASS(mode);
  else if(rindex[1] >= 0 && reload_in_reg[rindex[1]] == reg)
    return reload_reg_class[rindex[1]] == INDEX_REG_CLASS(mode);
  else
    {
      SWITCH_SUBREG (reg);
      if (REG_P (reg) && REG_OK_FOR_INDEX_P (reg, mode))
	  if (somewhere_in_reload)
	      return (!IS_PSEUDO (REGNO (reg)));
          else
	      return 1;
      else
	  return 0;
    }
}


int
extra_constraint_AQ(addr, addr_mode, rindex)
    rtx addr;
    enum machine_mode addr_mode;
    int rindex[];
{
    rtx op0, op1;
    switch (GET_CODE(addr))
    {
      case POST_MODIFY:
	op0 = XEXP(addr, 0);
	op1 = XEXP(addr, 1);
	if (reloaded_reg_ok_for_base_p(op1, addr_mode, rindex))
	{
	    rtx tmp = op0;
	    op0 = op1;
	    op1 = tmp;
	}

	if (! reloaded_reg_ok_for_base_p(op0, addr_mode, rindex)) return 0;

	switch(GET_CODE(op1)) 
	  {
	    
	  case REG:
	  case SUBREG:
	    return reloaded_reg_ok_for_index_p(op1, addr_mode, rindex);

	  case CONST_INT:
	    return (-1 <= INTVAL(op1) && INTVAL(op1) <= 1);
	    
 	  case CONST:
 	    if (GET_CODE (XEXP (op1, 0)) == CONST_INT)
 	      return (-1 <= INTVAL (XEXP (op1, 0)) 
		      && INTVAL (XEXP (op1, 0)) <= 1);
	    
	  default: return 0;
	  }
	break;

      case POST_INC:
      case POST_DEC:
	return reloaded_reg_ok_for_base_p(XEXP(addr, 0), addr_mode, rindex);

	/** (mem (reg)) is still (I,M) because we can use M5/M13 **/

      case REG:	return reloaded_reg_ok_for_base_p(addr,addr_mode, rindex);
      default: return 0;
    }
}

int
extra_constraint_BR(addr, addr_mode, rindex)
    rtx addr;
    enum machine_mode addr_mode;
    int rindex[];
{
    switch (GET_CODE(addr)) {
	
      case REG:	/** single reg is OK -- can use (m5,i)**/
      case SUBREG:
	return (reloaded_reg_ok_for_base_p(addr, addr_mode, rindex));
	break;
	
      case PLUS:
      {
	  rtx op0 = XEXP(addr, 0);
	  rtx op1 = XEXP(addr, 1);
	  return ((reloaded_reg_ok_for_base_p (op0, addr_mode, rindex) &&
		   reloaded_reg_ok_for_index_p(op1, addr_mode, rindex)) ||
		  (reloaded_reg_ok_for_base_p (op1, addr_mode, rindex) &&
		   reloaded_reg_ok_for_index_p(op0, addr_mode, rindex)));
      }
	break;
      default: return 0;
    }
}

int
extra_constraint_CS(addr, addr_mode, rindex)
     rtx addr;
     enum machine_mode addr_mode;
     int rindex[];
{
    rtx base, offset;
    int oval;	/** integer value of offset **/

    switch (GET_CODE(addr))
    {
      case PLUS:
	base = XEXP(addr, 0);
/*	offset = reduce_addr_operand(XEXP(addr, 1));*/
	offset = XEXP(addr, 1);
	if (CONSTANT_P(base)) 
	  {
	    rtx tmp = base;
	    base = offset;
	    offset = tmp;
	  }
	if (GET_CODE(offset) != CONST_INT)
	  return 0;
	oval = INTVAL(offset);
	goto plus_minus;

      case MINUS:
	base = (XEXP(addr, 0));
/*	offset = reduce_addr_operand(XEXP(addr, 1)); */
	offset = XEXP(addr, 1);
	if (GET_CODE(offset) != CONST_INT)
	  return 0;
	oval = - INTVAL(offset);

      plus_minus:
	if (oval > 31 || oval < -32) 
	  return 0;
	return reloaded_reg_ok_for_base_p(base, addr_mode, rindex);
	
      case POST_MODIFY:
	base = XEXP(addr, 0);
/*	offset = reduce_addr_operand(XEXP(addr, 1));  */

	offset = XEXP(addr, 1); 

	if (GET_CODE (offset) == CONST)
	  offset = XEXP (offset, 0);
	  
	if (GET_CODE(offset) != CONST_INT)
	  return 0;

	oval = INTVAL(offset);

	if (oval > 31 || oval < -32) 
	  return 0;

	return reloaded_reg_ok_for_base_p(base, addr_mode, rindex);

      case POST_INC:
      case POST_DEC:
	base = XEXP(addr, 0);
	return reloaded_reg_ok_for_base_p(base, addr_mode, rindex);
      default:
	return 0;
    }
}

int
extra_constraint_DU(addr, addr_mode, rindex)
    rtx addr;
    enum machine_mode addr_mode;
    int rindex[];
{
    rtx base, offset;
    
    switch(GET_CODE(addr)) 
      {
      case PLUS:
	base = XEXP(addr, 0);
/*	offset = reduce_addr_operand(XEXP(addr, 1)); */
	offset = XEXP(addr, 1);
	if (CONSTANT_P(base)) 
	  {
	    rtx tmp = base;
	    base = offset;
	    offset = tmp;
	  }

	return (CONSTANT_ADDRESS_P(offset) &&
		reloaded_reg_ok_for_base_p(base, addr_mode, rindex));

      case MINUS:
	base = XEXP(addr, 0);
/*	offset = reduce_addr_operand(XEXP(addr, 1)); */
	offset = XEXP(addr, 1);

	return (CONSTANT_ADDRESS_P(offset) &&
		reloaded_reg_ok_for_base_p(base, addr_mode, rindex));
	
      case SUBREG:
      case REG: 
	return 1;  /* I noticed this and it worries me */
	
      default: 
	return 0;
    }
}

int
extra_constraint_T(addr)
    rtx addr;
{
  if(CONSTANT_ADDRESS_P(addr))
    return 1;
  else if(GET_CODE(addr)==PLUS 
	  && CONSTANT_ADDRESS_P (XEXP (addr, 0)) 
	  && CONSTANT_ADDRESS_P (XEXP (addr, 1)))
    return 1;
  else
    return 0;
}

int 
extra_constraint (op, c)
    rtx op;
    char c;
{
    
    rtx addr, op0, op1;
    enum machine_mode addr_mode;
    int i, rindex[2]={-1,-1};		/** reload index **/
    
    if (GET_CODE(op) != MEM) return 0;

    addr = XEXP(op,0);
    addr_mode = GET_MODE(addr);
    switch(c)
    {
      case 'Q': case 'R': case 'S': case 'U':
	if (addr_mode != DMmode) return 0;
	break;
      case 'A': case 'B': case 'C': case 'D':
	if (addr_mode != PMmode) return 0;
	break;
    }

    if(somewhere_in_reload)
	for (i=0; i<n_reloads && rindex[1]==-1; i++)
	    if(reload_needed_for[i]==op && 
	       !reload_optional[i] && 
	       !reload_secondary_p[i])
	      if(rindex[0]==-1)
		rindex[0]=i;
	      else
		rindex[1]=i;
    
    if (rindex[0] < 0)
    {
	GO_IF_LEGITIMATE_ADDRESS(addr_mode, addr, ok_so_far);
	return 0;
    }
    
  ok_so_far:
    
    switch (c)
      {
      case 'A':
      case 'Q':	
	return extra_constraint_AQ(addr, addr_mode, rindex);
      case 'B':
      case 'R':	
	return extra_constraint_BR(addr, addr_mode, rindex);
      case 'C':
      case 'S': 
	return extra_constraint_CS(addr, addr_mode, rindex);
      case 'D':
      case 'U':	
	return extra_constraint_DU(addr, addr_mode, rindex);
      case 'T':  
	return extra_constraint_T(addr);
      default: 	
	return 0;
      }
  }


void
dbr_output_seqend (file)
     FILE           *file;
{
  extern rtx final_sequence;
  if (optimize > 0 && flag_delayed_branch)
    {
      int len = dbr_sequence_length();
      if (GET_CODE (XVECEXP (final_sequence, 0, 0)) == CALL_INSN)
	{
	  if (!len) 
	    fputs("\tnop;\n", file);
	}
      else
	switch (len)
	  {
	  case 1:
	    fputs("\tnop;\n", file);
	    break;
	  case 0:
	    fputs("\tnop;nop;\n", file);
	    break;
	  default:
	    fputc('\n', file);
	    break;
	  }
    }
}



/****** Checking constraints: some instructions, like "movsi", "addsi", etc,
  can be emitted during or after reload, in which case constraints must
  be matched precisely. The following functions does exactly that

int 
check_constraints ()
{

    if (! reload_in_progress && ! reload_completed) return(1);
    else
    {
	int save_which_alternative = which_alternative, result;
	result = constrain_operands(insn_code_num,1);
	which_alternative =  save_which_alternative;
	return(result);
    }
}
 ****/



/*** return true if both op1 and op2 are two differrent register operands **/

int 
different_registers(op1, op2)
     rtx op1, op2;
{
  int op1_reg_p, op2_reg_p, result;

  op1_reg_p = GET_CODE(op1);
  op2_reg_p = GET_CODE(op2);
  if ( !REG_P (op1) )
    return 1;
  if ( !REG_P (op2) )
    return 1;
  result = (REGNO (op1) != REGNO (op2));
  return result;
}


/***********************************************************************
  ARGUMENT PASSING  : pass first 3 arguments in registers 4,8 and 12.
   except when these are varargs.
***********************************************************************/

int             max_arg_regs = 3;
static          arg_regs[] = {4, 8, 12, 999};


void *
function_arg(cum, mode, type, named)
     CUMULATIVE_ARGS cum;
     enum machine_mode mode;
     tree            type;
     int             named;
{
  rtx             arg = 0;
  static already_on_stack=0;
  
  if(cum==0)
    already_on_stack = 0;
  
  if (!already_on_stack &&
      named &&
      TARGET_REGPARM && 
      cum < max_arg_regs &&
      GET_MODE_SIZE(mode) == 1)
    arg = gen_rtx(REG, mode, arg_regs[cum]);
  else
    already_on_stack = 1;
  
  return arg;
}

int 
function_arg_regno_p(n)
    int n;
{
    int i;

    for (i = 0; i < max_arg_regs; i++)
	if (n == arg_regs[i])
	    return 1;
    return 0;
}


/*  r -> m           +----------------+ high
 *                   | 	       	      |
 *      	     | 		      |
 *      	     | 		      |
 *      	     | 		      |
 *      	     +----------------+
 *                   |	  b LS        |
 *                   +----------------+
 *            ptr->  |	  u MS        |
 *      	     +----------------+
 *      	     |	              |
 */
void
copy_double_reg_to_memory(operands)
     rtx operands[];
{
    rtx             xoperands[4],
                    addr;
    int pm_memory;
    addr = xoperands[0] = XEXP(operands[0], 0);

    xoperands[1] = operands[1];
    pm_memory = (GET_MODE(addr) == PMmode);

  switch (GET_CODE(addr))
    {

    case POST_INC:
	output_asm_insn (pm_memory ?
			 "pm(%0,m14)=%u1; pm(%0,m14)=%b1;" :
			 "dm(%0,m6)=%u1; dm(%0,m6)=%b1;", xoperands);
	break;

    case POST_DEC:
	xoperands[0] = XEXP(xoperands[0], 0);
	output_asm_insn(pm_memory ?
			"pm(%0,m14)=%u1; pm(%0,-3)=%b1;" :
			"dm(%0,m6)=%u1; dm(%0,-3)=%b1;", xoperands);
	break;

    case POST_MODIFY:
        /* This is short circuted in go_if_legitimate_address */
	xoperands[2] = XEXP(operands[0],1);
	output_asm_insn(pm_memory ? 
			"pm(1,%0)=%b1; pm(%0,%2)=%u1;" :
			"dm(1,%0)=%b1; dm(%0,%2)=%u1;", xoperands);
	break;

    case CONST:
	xoperands[2] = plus_constant(addr, 1);
	output_asm_insn(pm_memory ?
			"pm(%0)=%u1; pm(%2)=%b1;" :
			"dm(%0)=%u1; dm(%2)=%b1;", xoperands);
	break;

    case PLUS:
      if (GET_CODE(XEXP(addr, 1)) == CONST_INT)
	{
	    xoperands[0] = XEXP(addr, 0);	 /* iregister */
	    xoperands[1] = XEXP(addr, 1);	 /* const-int */
	    xoperands[2] = gen_rtx(CONST_INT, SImode,INTVAL(xoperands[1]) + 1);
	    xoperands[3] = operands[1];
	    output_asm_insn(pm_memory ?
			    "pm(%1,%0)=%u3; pm(%2,%0)=%b3;" :
			    "dm(%1,%0)=%u3; dm(%2,%0)=%b3;", xoperands);
	}
	else
	    abort();
	break;

    case REG:
	output_asm_insn(pm_memory ? 
			"pm(%0,m14)=%u1; pm(%0,m15)=%b1;" :
			"dm(%0,m6)=%u1; dm(%0,m7)=%b1;", xoperands);
	break;

    case SYMBOL_REF:
	output_asm_insn(pm_memory ?
			"pm(%0)=%u1; pm(%0+1)=%b1;" :
			"dm(%0)=%u1; dm(%0+1)=%b1;", xoperands);
	break;

    default:
      abort();
    }
}


/*  m -> r           +----------------+ high
 *                   | 	       	      |
 *      	     | 		      |
 *      	     | 		      |
 *      	     | 		      |
 *      	     +----------------+
 *                  0|	  b LS        |
 *                   +----------------+
 *            ptr-> 1|	  u MS        |          r[0] = ptr
 *      	     +----------------+          r[1] = ptr+1
 *      	    2|	              |
 *      	     +----------------+
 *      	    3| 		      |
 *      	     +----------------+
 */
void
copy_double_memory_to_reg(operands)
    rtx operands[];
{
    rtx xoperands[4], addr,x;
    int pm_memory;

    xoperands[0] = operands[0];
    addr = xoperands[1] = XEXP(operands[1], 0);
    pm_memory = (GET_MODE(addr) == PMmode);

    switch (GET_CODE(addr))
    {
    case POST_INC:
	output_asm_insn(pm_memory ? 
			"%u0=pm(%1,m14); %b0=pm(%1,m14);" :
			"%u0=dm(%1,m6); %b0=dm(%1,m6);", xoperands);
	break;

    case POST_DEC:
	output_asm_insn(pm_memory ?
			"%u0=pm(%1,m14); %b0=pm(%1,-3);" :
			"%u0=dm(%1,m6); %b0=dm(%1,-3);", xoperands);
	break;

    case POST_MODIFY:
        /* This is short circuited in go_if_legitimate_address */
	x = xoperands[1];
	xoperands[1] = XEXP(x,0);
	xoperands[2] = XEXP(x,1);
	output_asm_insn(pm_memory ?
			"%b0=pm(1,%1); %u0=pm(%1,%2);" :
			"%b0=dm(1,%1); %u0=dm(%1,%2);", xoperands);
	break;

    case CONST:
	xoperands[2] = plus_constant(addr, 1);
	output_asm_insn(pm_memory ? 
			"%u0=pm(%1); %b0=pm(%2);" :
			"%u0=dm(%1); %b0=dm(%2);", xoperands);
    break;

    case PLUS:
      if (GET_CODE(XEXP(xoperands[1], 1)) == CONST_INT)
	{
	    xoperands[1] = XEXP(addr, 0);	 /* iregister */
	    xoperands[2] = XEXP(addr, 1);	 /* const-int */
	    xoperands[3] = gen_rtx(CONST_INT, SImode, INTVAL(xoperands[2])+1);
	    output_asm_insn(pm_memory ?
			    "%u0=pm(%2,%1); %b0=pm(%3,%1);" :
			    "%u0=dm(%2,%1); %b0=dm(%3,%1);", xoperands);
	}
      else
	abort();
      break;

    case REG:
	output_asm_insn(pm_memory ? 
			"%u0=pm(%1,m14); %b0=pm(%1,m15);" :
			"%u0=dm(%1,m6); %b0=dm(%1,m7);", xoperands);
    break;

    case SYMBOL_REF:
	output_asm_insn(pm_memory ? 
			"%u0=pm(%1); %b0=pm(%1+1);" :
			"%u0=dm(%1); %b0=dm(%1+1);", xoperands);
	break;

    default:
      abort();
    }
}


int
is_constant_1p(x, insn)
    rtx             x;
    rtx             insn;

{
    rtx             l;

#if 0
    /** This part causes problems when the initialization of x is moved **/
    /** out of the loop. This predicate then returns 1 before loop and **/
    /** 0 after. Thus "unrecognized insn" in later phases. **/
    /** See /usr/az/tests/gccs/reconecon.c **/
    if (REG_P(x))
    {
	l = reg_set_last(x, insn);
	if (l && GET_CODE(l) == CONST_INT && INTVAL(l) == 1)
	    return 1;
	else
	    return 0;
    }
    else
#endif	
	if (GET_CODE(x) == CONST_INT && INTVAL(x) == 1)
	return 1;
    return 0;
}

int 
is_constant_m1p(x, insn)
     rtx x, insn;
{
  rtx l;

  if (REG_P(x))
    {
      l = reg_set_last(x, insn);
      if (l && GET_CODE(l) == CONST_INT && INTVAL(l) == -1)
	return 1;
      else
	return 0;
    }
  else if (GET_CODE(x) == CONST_INT && INTVAL(x) == -1)
    return 1;
  return 0;
}


int 
address_cost(x)
     rtx x;
{
#define ABSOLUTE_VALUE(X) ((X)<0?-(X):(X))
  switch (GET_CODE(x))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
      return 2;
    case PLUS:
      return 10;
    case POST_MODIFY:
      if (GET_CODE (XEXP (x, 0)) == REG
	  && (GET_CODE (XEXP (x, 1)) == REG
	      || (CONSTANT_P (XEXP (x,1))
		  && is_small_constant (XEXP (XEXP (x, 0), 1)))))
	return 2;
      
      else if (CONSTANT_P(XEXP(x, 1)) && ABSOLUTE_VALUE(INTVAL(XEXP(x, 1))) < 32)
	return 4;
      else
	return 6;
    default:
      return 2;
    }
}

enum machine_mode
extract_mode_pmode(x, mode)
     rtx             x;
     enum machine_mode mode;
{

#ifdef MEMSEG
  if (GET_MODE(x) == VOIDmode)
    return mode;
  else
    return GET_MODE(x);
#else
  return (GET_MODE(x) == SImode ? SImode : mode);
#endif
}

enum machine_mode
extract_segment_pmode(x)
     memory_segment x;
{
  switch(x) {
  case PROG_MEM: return PMmode;
  case DATA_MEM: return DMmode;
  default: abort();
  }
}

/*** Note that Pmode of decl is not the same as DECL_MODE: ***/
/*** "int pm * dm x" makes x with DECL_MODE==PMmode, while ***/
/*** extract_tree_pmode will return DMmode  -- the former is for the ***/
/*** value, the latter -- for the address ***/

enum machine_mode
extract_tree_pmode (tr)
    tree tr;
{
  if (! tr) 
    abort();
  switch (TREE_CODE(tr))
    {
    case VAR_DECL:
    case CONSTRUCTOR:
      return extract_segment_pmode(TREE_MEMSEG(tr));
    case FUNCTION_DECL:
    case FUNCTION_TYPE:
      return PMmode;
      /** Note that Pmode of a pointer (POINTER_TYPE and REFERENCE_TYPE) **/
      /**  must be deduced from the segment of the thing it points to, ***/
      /*** which is given by TREE_TYPE ***/
    case POINTER_TYPE:
    case REFERENCE_TYPE:
    case ARRAY_TYPE:
      if (TREE_CODE (TREE_TYPE (tr)) == FUNCTION_TYPE)
	return PMmode;
      else return extract_segment_pmode(TREE_MEMSEG(TREE_TYPE(tr)));
      
    case INTEGER_TYPE:
    case REAL_TYPE:
    case COMPLEX_TYPE:
    case ENUMERAL_TYPE:
    case BOOLEAN_TYPE:
    case CHAR_TYPE:
      return extract_segment_pmode (TREE_MEMSEG (tr));
      
    default:
      return extract_tree_pmode(TREE_TYPE(tr));
      abort();
    }
}

enum machine_mode
extract_rtx_pmode(x)
     rtx x;
     
{
  if (x) 
    {
      if (GET_CODE(x) == CONST_INT && 
	  GET_MODE(x) == VOIDmode) 
	return SImode;
      else 
	return GET_MODE(x);
    }
  else
    return SImode;
}

enum machine_mode
get_pmode(x)
    enum machine_mode x;
{
  return x;
}

int 
register_move_cost(from, to)
     enum reg_class  from;
     enum reg_class  to;
{
  if (from == MULT_ACC_REGS && to != D_REGS && to != GENERAL_REGS)
    return 100;
  else if (to == MULT_ACC_REGS && from != D_REGS & from != GENERAL_REGS)
    return 100;
  else
    return 2;
}

/** Returns true if OP is a memory reference to the memory space
    indicated by MODE (either PMmode or DMmode).
    This is used to check which DAG to get a small constant when
    using certain patterns. **/

int 
dag_source(op, mode)
     rtx op;
     enum machine_mode mode;
{
  if (GET_CODE(op) == MEM) {
    rtx addr = XEXP(op,0);
    enum machine_mode addr_mode = GET_MODE(addr);
    enum rtx_code     addr_code = GET_CODE(addr);
    if (addr_mode != PMmode && addr_mode != DMmode) abort();
    return (mode == addr_mode);
  } else
      return 0;
}

#define RTX_INDEX_P(X,STRICT,M) \
    (CONSTANT_P(X) ||\
     (REG_P(X) && REG_OK_FOR_INDEX_STRICT_P (X,STRICT,M)))

#define RTX_BASE_P(X,STRICT,M) \
     (REG_P(X) && REG_OK_FOR_BASE_STRICT_P (X,STRICT,M))

/*
 * Macro to catch two CONSTANT_P things that could be likely to generate
 * a indexed address that would cross a memory space..
 */
#define XMEMSPACE(x) (GET_CODE (x) == CONST || \
		       GET_CODE (x) == SYMBOL_REF || \
		       GET_CODE (x) == LABEL_REF)

rtx
legitimize_address(x, oldx, mode)
     rtx             x, oldx;
     enum machine_mode mode;
{
  extern rtx force_operand(),copy_to_reg();
  rtx offset, frame, index, y;

  /*
   * (plus:SI (plus:SI (reg:SI 51) (reg/v:SI 54)) (const_int -200))
   * 
   * ===> (set base (+ fp offset))
   * 
   * (mem (+ base index))  easier to SR
   * 
   * (plus:SI (plus:SI (mult:SI (reg/v:SI 56) (const_int 10)) (reg:SI 51))
   * (const_int -200)) ==> (set base (+ fp offset)) (set x (* index k))
   * (mem (+ base x))  easier to SR
   */
  
  if (ADSP210Z3u && GET_CODE (x) == PLUS)
    if ((CONSTANT_P (XEXP (x, 0)) && CONSTANT_P (XEXP (x, 1)))
        || (REG_P (XEXP (x, 0)) && XMEMSPACE (XEXP (x, 1)))
	|| (REG_P (XEXP (x, 1)) && XMEMSPACE (XEXP (x, 0)))
	|| (!(RTX_BASE_P (XEXP (x,0),0,mode) 
	      && RTX_INDEX_P (XEXP (x,1),0,mode))
	    && !(RTX_BASE_P (XEXP (x,1),0,mode) 
		 && RTX_INDEX_P (XEXP (x,0),0,mode))))
      {
	rtx reg;
	if (XMEMSPACE (XEXP (x, 0))) {
	  rtx r = copy_to_reg (XEXP(x,0));
	  REGNO_POINTER_FLAG (REGNO (r)) = 1;
	  reg = gen_rtx (PLUS, GET_MODE (x), XEXP (x, 1), r);
	}
	else if (XMEMSPACE (XEXP (x, 1))) {
	  rtx r = copy_to_reg (XEXP(x,1));
	  REGNO_POINTER_FLAG (REGNO (r)) = 1;
	  reg = gen_rtx (PLUS, GET_MODE (x), XEXP (x, 0), r);
	}
	else
	  {
	    reg = copy_to_reg (x);
	    REGNO_POINTER_FLAG (REGNO (reg)) = 1;
	  }
      }
  
  if (GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 0)) == PLUS)
    {
      frame = XEXP (XEXP (x, 0), 0);

      if (frame == virtual_stack_vars_rtx
	  || frame == frame_pointer_rtx)

	index = XEXP(XEXP(x, 0), 1);

      else
	{
	  index = frame;
	  frame = XEXP(XEXP(x, 0), 1);
	}

      if (frame != virtual_stack_vars_rtx
	  && frame != frame_pointer_rtx)
	return 0;

      offset = XEXP (x, 1);
      if (CONSTANT_P(offset)
	  && REG_P(index))
	{
	  rtx r;
	  y = gen_rtx (PLUS,
		      Pmode1 (Extract_rtx_Pmode (frame)),
		       r = copy_to_reg (gen_rtx (PLUS,
						 Pmode1 (Extract_rtx_Pmode (frame)),
						 frame, offset)),
		       index);
	  REGNO_POINTER_FLAG (REGNO (r)) = 1;
	  if (memory_address_p (mode, y))
	    return y;
	}
      else if (CONSTANT_P (offset)
	       && GET_CODE (index) == MULT)
	{
	  rtx r;
	  y = copy_to_reg 
	    (gen_rtx (PLUS,
		      Pmode1 (Extract_rtx_Pmode (frame)),
		      copy_to_reg (index),
		      r = copy_to_reg (gen_rtx
				       (PLUS, Pmode1 (Extract_rtx_Pmode (frame)),
					frame, offset))
		      ));
	  REGNO_POINTER_FLAG (REGNO (r)) = 1;
	  return y;
	}
      else if (!REG_P (x))
	{ 
	  /* Well lets just copy to a register to legitimize the expression */
	  return (copy_to_reg (x));
	}
    }

  /* for dealing with complexes:
    (plus:DM (post_dec:DM (reg:DM 44 i7))
             (const_int 1)) */

  else if (GET_CODE (x) == PLUS
	   && GET_CODE (XEXP (x, 0)) == POST_DEC
	   && CONSTANT_P (XEXP (x, 1)))
    {
      enum machine_mode mode1 = GET_MODE (XEXP (x, 0));
      rtx new = emit_insn (gen_rtx (SET, mode1,
				    XEXP (XEXP (x, 0), 0),
				    gen_rtx (PLUS, mode1, 
					     XEXP (XEXP (x, 0), 0),
					     constm1_rtx)));
      rtx plus = gen_rtx (PLUS, mode1,
			  XEXP (XEXP (x, 0), 0),
			  XEXP (x, 1));
			  
      return plus;
    }
  return 0;
}

/* Note that MODE is that of MEM, not of X -- we need
   ADDR_MODE instead. */

int 
go_if_legitimate_address (mode, x, strict_p)
     enum machine_mode mode;
     rtx             x;
     int		    strict_p;
{
  rtx op0, op1;
  int okadr = 0;
  int base_ok, index_ok;

  extern long loop_stack;
  enum machine_mode addr_mode = GET_MODE(x);
  
  switch(GET_CODE(x)) 
    {
    case REG:
      CHECK_PREMODIFY_MEMORY_ACCESS (x, strict_p);
      return (REG_OK_FOR_BASE_STRICT_P (x, strict_p, addr_mode));
      
    case SUBREG:
      if (REG_P (XEXP (x, 0))) {
	CHECK_PREMODIFY_MEMORY_ACCESS (XEXP (x, 0), strict_p);
	return (REG_OK_FOR_BASE_STRICT_P (SUBREG_REG (x), strict_p, addr_mode));
      } else
	return 0;
      
    case SYMBOL_REF:
    case LABEL_REF:
    case CONST_INT:
      return 1;
      
    case CONST:
      return (mode == addr_mode) || GET_MODE_SIZE (mode) == GET_MODE_SIZE (addr_mode);
      
    case POST_INC:
    case POST_DEC:
      CHECK_PREMODIFY_MEMORY_ACCESS (XEXP (x, 0), strict_p);
      return (RTX_BASE_P (XEXP (x, 0), strict_p, addr_mode));
      
    case POST_MODIFY:
      {
	op0 = XEXP(x,0);
	op1 = XEXP(x,1);
	
	base_ok = 0;
	index_ok = 0;
	
	SWITCH_SUBREG(op0);
	
	SWITCH_SUBREG(op1);
	
	base_ok  = RTX_BASE_P (op0, strict_p, addr_mode);
	
	switch (GET_CODE (op1))
	  {
	  case REG:
	    index_ok = RTX_INDEX_P (op1, strict_p, addr_mode);
	    break;
	    
	  case CONST_INT:
	    index_ok = CONST_OK_FOR_AUTO_MODIFY (op1);
	    break;
	    
	  case CONST:
	    if (GET_CODE (XEXP (op1, 0)) == CONST_INT)
	      index_ok = CONST_OK_FOR_AUTO_MODIFY (XEXP (op1, 0));
	    break;
	    
	  default:
	    index_ok = 0;
	  }
	
	return (base_ok && index_ok);
      }
    
    case PLUS:
      {
	op0 = XEXP (x,0); 
	op1 = XEXP (x,1);

	SWITCH_SUBREG (op0);
	SWITCH_SUBREG (op1);
	
	if (CONSTANT_P (op0) && CONSTANT_P (op1))
	  /* Link|Compile time constant address? */
	  return 1;
	
	/* dm(const+const) */
	if (!ADSP210Z3u 
	    && CONSTANT_P(op0) && CONSTANT_P(op1))
	  return 1;
	
	/* dm(ireg,const) | dm(const,ireg) */
	
	if (ADSP210Z3u) {
	  if ((REG_P (op0) && XMEMSPACE (op1))) {
	    CHECK_PREMODIFY_MEMORY_ACCESS (op0, strict_p);
	    return 0;
	  }
	  if ((REG_P (op1) && XMEMSPACE (op0))) {
	    CHECK_PREMODIFY_MEMORY_ACCESS (op1, strict_p);
	    return 0;
	  }
	}

	/* Well if reload has completed then we need to
	   -- ignore this stuff because it is too late.
	   look at the reload_completed ....
	   */
	if (((CONST_INT_P (op1)) 
	     && RTX_BASE_P (op0, strict_p, addr_mode))) {
	  CHECK_PREMODIFY_MEMORY_ACCESS (op0, strict_p);
	  return (!ADSP210Z3u || reload_completed || RTX_ISPOINTER_P (op0));
	}
	
	if ((CONST_INT_P (op0))
	    && (!ADSP210Z3u || RTX_ISPOINTER_P (op1))
	    && RTX_BASE_P (op1, strict_p, addr_mode)) {
	  CHECK_PREMODIFY_MEMORY_ACCESS (op1, strict_p);
	  return (!ADSP210Z3u || reload_completed || RTX_ISPOINTER_P (op1));
	}
	
	/* dm(ireg,mreg) | dm(mreg,ireg) */
	
	else if (mode == DImode 
		 || (DOUBLE_IS_64 
		     && mode == DFmode))
	  return 0;
	
	if ((RTX_BASE_P (op0, strict_p, addr_mode) 
	     && RTX_INDEX_P (op1, strict_p, addr_mode))) {
	  CHECK_PREMODIFY_MEMORY_ACCESS (op0, strict_p);
	  return 1;
	}

	if ((RTX_BASE_P (op1, strict_p, addr_mode) 
	     && RTX_INDEX_P (op0, strict_p, addr_mode))) {
	  CHECK_PREMODIFY_MEMORY_ACCESS (op1, strict_p);
	  return 1;
	}
	return 0;
      }

    default: return 0;
    }
}

int 
modes_tieable_p(m1, m2)
     enum machine_mode m1, m2;
{
  int regno, tieable_p;
  tieable_p = ((GET_MODE_CLASS(m1) == MODE_FLOAT)
	       == (GET_MODE_CLASS(m2) == MODE_FLOAT));

  for (regno = 0; regno < FIRST_PSEUDO_REGISTER && tieable_p; regno++)
    tieable_p 
      &= (HARD_REGNO_MODE_OK(regno, m1) == HARD_REGNO_MODE_OK(regno, m2));
  return tieable_p;
}

int 
unsigned_compare_p(insn)
     rtx insn;
{
  rtx pat = PATTERN(insn);

  if (GET_CODE(pat) == SET
      && GET_CODE(XEXP(pat, 0)) == PC
      && GET_CODE(XEXP(pat, 1)) == IF_THEN_ELSE)
    {
      rtx             test = XEXP(XEXP(pat, 1), 0);
      enum rtx_code   code = GET_CODE(test);

      if (code == LTU
	  || code == GTU
	  || code == LEU
	  || code == GEU)
	return 1;
    }
  return 0;
}

#ifdef MEMSEG
void
select_section(tr, reloc)
     tree            tr;
     int             reloc;
{
  if (TREE_MEMSEG(tr) == PROG_MEM)
    pmda_section();
  else
    data_section();
}

memory_segment    default_memory_segment = DATA_MEM;
memory_segment  code_memory_segment = PROG_MEM;
#endif


/*
 * The following is used to decide how to call a function.
 */

static int      This_function_passes_args_in_registers = 0;

enum call_modifier_tag
{
  MODIFY_BY_NILL = 0,
  MODIFY_BY_020 = 1,
  MODIFY_D,
  MODIFY_P, MODIFY_DD, MODIFY_DP,
  MODIFY_PD, MODIFY_PP,
  MODIFY_F,			/* FP function */
  MODIFY_FD,
  MODIFY_FP, MODIFY_DDD, MAX_DO_UNTIL,
};

#include "21klibs.h"		/* Generated from 21klibs.gperf */

int 
is_double_math_library (name)
     char *name;
{
  struct lib_func *lf = is_library_function (name, strlen(name));
  if (lf && lf->fname == MODIFY_F )
    return 1;
  else
    return 0;
}


int 
is_function_a_library_routine(name)
     char *name;
{
    if (is_library_function(name,strlen(name)))
	return 1;
    return 0;
}

int 
does_function_have_polymorphic_property(name)
     char *name;
{
  struct lib_func *lf = is_library_function(name,strlen(name));
  if (lf)
    return lf->num_pptrs;
  return 0;
}

int
hard_regno_mode_ok (regno, mode)
    int regno;
    enum machine_mode mode;
{
  switch (mode) 
    {
    case PMmode: 
      return IS_DAG1_REG (regno) || IS_DAG2_REG (regno) || IS_DREG (regno);

    case SImode: 
      if (IS_SYS (regno))
	  return TRUE;
      /* fall thru */

    case DMmode: 
      return IS_DAG1_REG (regno) || IS_MULT_ACC_REG (regno) || IS_DREG (regno);

    case DImode: 
      return IS_MULT_ACC_REG (regno) || (IS_DREG (regno) && IS_DREG (regno+1));

    case SFmode:
      return (IS_DREG (regno)) || (IS_DAG1_REG (regno));

    case SCmode:
    case CSImode:
    case DFmode:
      return (IS_DREG (regno) && IS_DREG (regno+1));
      
    case VOIDmode:
      return 1;

    default:
      return 0;
      /*** This should be abort(), but this kills reloader ***/
      /*** in some strange cases, so we make it return FALSE  instead **/
    }
}

void
put_char_in_string (char **b, int *len,char **p, char c)
{
  int offset = *p - *b;

  if (*p+2 > *b+*len) {
    *b = (char *)xrealloc( *b , *len += 512 );
    *p = *b + offset;
  }
  *(*p)++ = c;
  *(*p) = 0;
}

void
handle_pragma(stream)
     FILE *stream;
{
  char *cp, *cp2;
  char buffer[256];

  fgets(buffer,(sizeof(buffer)/sizeof(char)),stream);
  if(feof(stream))
    {
      error("EOF encountered while parsing #pragma");
      return;
    }
  if(strlen(buffer)==(sizeof(buffer)/sizeof(char))-1)
    {
      error("#pragma line too long");
      return;
    }
  cp=buffer;
  while(isspace(*cp))
    cp++;
  
  if((cp2=strchr(cp,' '))!=NULL)
     *cp2 = '\0';
  if((cp2=strchr(cp,'\n'))!=NULL)
     *cp2 = '\0';

  if(!strcmp(cp,"inline_data"))
    error("#pragma inline_data is obsolete, use asm()");
  else if(!strcmp(cp,"inline"))
    error("#pragma inline is obsolete, use asm()");
  else
    {
      cp2=alloca(strlen(cp)+30);
      sprintf(cp2,"#pragma %s is not supported",cp);
      warning(cp2);
    }
  ungetc('\n',stream);
  return;
}

char*
mangle_library_function_name (name, plist, nptrs)
     char *name;
     tree plist;
     int  nptrs;
{
  extern struct obstack *saveable_obstack;
  char augment[10];
  int i = 0;
  tree p;
  char *real_library_name;

  for (p = plist; p ; p = TREE_CHAIN (p))
    {
      tree x = TREE_VALUE (p);
      
      if (x && TREE_CODE (TREE_TYPE (x)) == POINTER_TYPE)
	{
	  char c;
	  x = TREE_TYPE (TREE_TYPE (x));
	  
	  switch (TREE_MEMSEG (x)) {
	  case PROG_MEM: c = 'P'; break;
	  case DATA_MEM: c = 'D'; break;
	  default:
	    debug_tree (plist);
	    abort ();
	  }
	  augment[i++] = c;
	}
    }

  if (i != nptrs) {
    return name;
  }
  augment[i]=0;
  real_library_name = alloca (strlen (name) + 3 + i );
  strcpy (real_library_name, "__");
  strcat (real_library_name, name);
  strcat (real_library_name, augment);
  name = obstack_copy0 (saveable_obstack, real_library_name, strlen (real_library_name));

  return name;
}


/*
  Mangle the name when arguments are given by 1 or 2 rtx's 
  When one rtx is given a2 is 0 
*/

char*
mangle_libfun_name_rtx (name, a1, a2)
    char *name;
    rtx a1, a2;
{
    extern struct obstack *saveable_obstack;
    char augment[3], *pa=augment;
    int i = 0;
    char *real_library_name;

    if (GET_MODE(a1) == PMmode)
	*pa++ = 'P';
    else if  (GET_MODE(a1) == DMmode)
	*pa++ = 'D';
    else
	abort();
    if (a2) {
	if (GET_MODE(a2) == PMmode)
	    *pa++ = 'P';
	else if  (GET_MODE(a2) == DMmode)
	    *pa++ = 'D';
	else
	    abort();
    }
    *pa=0;
    real_library_name = alloca (strlen (name) + 6);
    strcpy (real_library_name, "__");
    strcat (real_library_name, name);
    strcat (real_library_name, augment);
    name = obstack_copy0 (saveable_obstack, real_library_name,
			  strlen (real_library_name));
    return name;
}

rtx
polymorph_library_calls (funexp, decl)
     rtx funexp;
     tree decl;
{
  int n;
  char *name = XSTR (funexp,0);

  if (GET_CODE (funexp) != SYMBOL_REF)
    return funexp;
  
  if (flag_short_double && !flag_no_builtin)
    {
      struct lib_func *lf = is_library_function (name, strlen(name));

      if (lf && lf->fname == MODIFY_F )
	{
	  extern struct obstack *saveable_obstack;
	  int i = 0;
	  char *real_library_name;

	  real_library_name = alloca (strlen (name) + 2);
	  strcpy (real_library_name, name);
	  strcat (real_library_name, "f");
	  name = obstack_copy0 (saveable_obstack, real_library_name,
				strlen (real_library_name));
	  funexp = gen_rtx (SYMBOL_REF, GET_MODE (funexp), name);
	}
    }

  if (n = does_function_have_polymorphic_property (name))
    {
      name = mangle_library_function_name (name,
					   TREE_OPERAND (decl,1), n);
      return gen_rtx (SYMBOL_REF, GET_MODE (funexp), name);
    }

  return funexp;
}

/*
  Polymorph library call with a single argument given by ARG1
*/

rtx
polymorph_library_calls_1 (funexp, arg1)
     rtx funexp, arg1;
{
  int n;
  char *name;
  if (GET_CODE (funexp) == SYMBOL_REF
      && (n = does_function_have_polymorphic_property (XSTR (funexp,0))))
    {
      name = mangle_libfun_name_rtx (XSTR (funexp,0), arg1, 0);
      return gen_rtx (SYMBOL_REF, GET_MODE (funexp), name);
    }
  return funexp;
}

/*
  Polymorph library call with a single argument given by ARG1
*/

rtx
polymorph_library_calls_2 (funexp, arg1, arg2)
     rtx funexp, arg1, arg2;
{
  int n;
  char *name;
  if (GET_CODE (funexp) == SYMBOL_REF
      && (n = does_function_have_polymorphic_property (XSTR (funexp,0))))
    {
	name = mangle_libfun_name_rtx (XSTR (funexp,0), arg1, arg2);
	return gen_rtx (SYMBOL_REF, GET_MODE (funexp), name);
    }
  return funexp;
}


/*
 * Generate variations on the name of a function
 *
 * file    output stream to write to
 * name    generic function name
 * n       number of parameters to vary
 *
 * NOTE: since strtod() is polymorphic in PM/DM and F/D need extra complexity here
 */
void
generate_polymorphic_library_extern (file,name,n)
     FILE *file;
     char *name;
     int n;
{
  int i,j;
  char *pmdm = (char *)xmalloc (n+1);
  char *f = (flag_short_double && is_double_math_library (name))? "f":"";

  for(i=0;i<n;i++) pmdm[i]='D';
  pmdm[i]=0;

  fprintf(file, ".extern\t_%s%s,___%s%s%s", name,f,name,f,pmdm);

  for ( i = 0; i < n; i++) {
    for ( j = i; j < n; j++) {
      pmdm[j] = 'P';
      fprintf(file, ",___%s%s%s",name,f,pmdm);
      pmdm[j] = 'D';
    }
    pmdm[i] = 'P';
  }
  fprintf (file,";\n");
  free(pmdm);
}

void
notice_update_cc ( rtx BODY, rtx INSN )
{
    extern CC_STATUS cc_status;
	
    switch (get_attr_cc (INSN)) {
    case CC_SHIFT:
        CC_STATUS_INIT;
	cc_status.value1 = SET_DEST (BODY);
	cc_status.flags |= CC_NO_OVERFLOW | CC_IN_SHIFT;
	break;

    case CC_MULT:
	CC_STATUS_INIT;
	cc_status.value1 = SET_DEST (BODY);
	cc_status.flags |= CC_NO_OVERFLOW | CC_IN_MULT;
	break;

    case CC_SETS:
	CC_STATUS_INIT;
	cc_status.value1 = SET_DEST (BODY);
	cc_status.flags |= CC_NO_OVERFLOW;
	break;
    case CC_UNCHANGED:
	break;
    case CC_COPY:
	if (cc_status.value1 != 0
	    && reg_overlap_mentioned_p (SET_DEST (BODY), cc_status.value1))
	    cc_status.value1 = 0;
	if (cc_status.value2 != 0
	    && reg_overlap_mentioned_p (SET_DEST (BODY), cc_status.value2))
	    cc_status.value2 = 0;
	break;
    case CC_CLOBBER:
	CC_STATUS_INIT;
	break;
    case CC_COMPARE:
	/* Insn is a compare which sets the CC fully.  Update CC_STATUS for
	 * this compare and mark whether the test will be signed or unsigned. */
	{
	    register rtx    _p = PATTERN (INSN);

	    CC_STATUS_INIT;
	    if (GET_CODE (_p) == PARALLEL)	break;
	    cc_status.value1 = SET_SRC (_p);
	    if (GET_CODE (SET_SRC (_p)) == REG)
		cc_status.flags |= CC_NO_OVERFLOW;
	}
	break;

    default:
	abort();
    }
}

/*
 * Look for cases where the conditional branch we're about to generate would
 * have eliminated the test instruction that just set the condition codes
 * that we actually need.  Shift instructions can only set the SZ flag, so
 * any other relational tests must use the ALU ccs.  The multiplier only sets
 * the MS, so any tests other than LT or GE will need ALU flags.
 *
 * RETURNS TRUE if the available flags can be used to achieve the condition
 */
int
shift_or_mult_cc_p ( enum mode_class relop )
{
  switch (relop)
      {
    case EQ:
    case NE:	if (cc_prev_status.flags&CC_IN_MULT)
		  return 0;	/* Only have MS */
		return 1;

    case  LT:
    case GE:	if (cc_prev_status.flags&CC_IN_SHIFT)
		  return 0;     /* Only have SZ */
		return 1;

    default:	if (( cc_prev_status.flags & CC_IN_SHIFT)
		   || (cc_prev_status.flags & CC_IN_MULT))
		  return 0;	/* Have either SZ or MS, but need more */
		return 1;
      }
}

/**========= Peephole parallelizer =======================================**/

extern rtx peep_insn[];

#include "recog.h"

/*** Delete ';' from the copy of TEMPLATE and return the copy ***/

char *
fix_template(template)
    char *template;
{
    int len = strlen(template);
    char *p;
    static char new_template[111];

    if (*template == '*') abort();

    strcpy(new_template, template);
    for(p = new_template + len;
	p > new_template && *p != ';';
	p--);
    if (p != new_template) *p = 0;
    return(new_template);
}

char *peep_templates[6];
char *peep_note;
int peep_n_operands[6];
static struct 
{
  int found;
  int where;
  char *what;
  char *note;
} special;

rtx peep_old_operands[6][10];

void
collect_templates_and_operands(n, perm)
    int n;
    int perm[];
{
    int ii, i, insn_code_number;
    char* template; 
    peep_note=0;
    for(ii=0; ii<n; ii++) {
	rtx insn = peep_insn[perm[ii]];

	insn_code_number = recog_memoized (insn);
	insn_extract (insn);
	constrain_operands(insn_code_number, 1);
	if(special.found && special.where==ii)
	  {
	    template=special.what;
	    peep_note=special.note;
	  }
	else
	  {
	    template = insn_template[insn_code_number];
	    if (template == 0)
	      template = (*insn_outfun[insn_code_number]) (recog_operand, insn);
	  }
	peep_templates[ii] = template;
	peep_n_operands[ii] = insn_n_operands[insn_code_number];
	for(i=0; i<peep_n_operands[ii]; i++)
	    peep_old_operands[ii][i] = recog_operand[i];
    }
    
}

static char * new_par_template;

#define IS_DM_MOVE(T)  ((T)==TYPE_DM_MOVE)
#define IS_PM_MOVE(T)  ((T)==TYPE_PM_MOVE)
#define IS_MULT_FIX(T) ((T)==TYPE_COMP_MULT_FIX)
#define IS_ALU_FIX(T)  ((T)==TYPE_COMP_ALU_FIX)
#define IS_MULT_FLOAT(T) ((T)==TYPE_COMP_MULT_FLOAT)
#define IS_ALU_FLOAT(T)  ((T)==TYPE_COMP_ALU_FLOAT)

#define IS_COMPUTE(T)  ((T)==TYPE_COMP_ALU_FIX || \
			(T)==TYPE_COMP_ALU_FLOAT || \
			(T)==TYPE_COMP_MULT_FIX || \
			(T)==TYPE_COMP_MULT_FLOAT || \
			(T)==TYPE_COMPUTE)

#define IS_MOVE(T) (((T) == TYPE_PM_MOVE) || \
		    ((T) == TYPE_DM_MOVE) || \
		    ((T) == TYPE_DREG_ONE)|| \
		    ((T) == TYPE_MOVE))


#define IS_POST_MOD(T) ((T)==TYPE_PM_MOVE||(T)==TYPE_DM_MOVE)

int 
is_dual_add_sub(rtx1,rtx2)
  rtx rtx1, rtx2;
{
  if(GET_CODE(PATTERN(rtx1))==SET && GET_CODE(PATTERN(rtx2))==SET &&
     GET_CODE(XEXP(PATTERN(rtx1),1))==PLUS && GET_CODE(XEXP(PATTERN(rtx2),1))==MINUS &&
     XEXP(XEXP(PATTERN(rtx1),1),0) == XEXP(XEXP(PATTERN(rtx2),1),0) &&
     XEXP(XEXP(PATTERN(rtx1),1),1) == XEXP(XEXP(PATTERN(rtx2),1),1))
    return 1;
  return 0;
}
    
int
is_mac_operator(insn)
    rtx insn;
{
    return (GET_RTX_CLASS(GET_CODE(insn)) == 'i' 
	&& GET_CODE(PATTERN(insn)) == SET
	&& OPERATOR_MAC==get_attr_operator(insn));
}

int
parallel_5_p(p)
rtx *p;
{

  if(is_dual_add_sub(p[1],p[2]) && 
     IS_MULT_FIX(get_attr_type(p[0])) && 
     IS_ALU_FIX(get_attr_type(p[1])) && 
     IS_DM_MOVE(get_attr_type(p[3])) && 
     IS_PM_MOVE(get_attr_type(p[4])))
    return 1;
  if(is_dual_add_sub(p[1],p[2]) &&
     IS_MULT_FLOAT(get_attr_type(p[0])) && 
     IS_ALU_FLOAT(get_attr_type(p[1])) && 
     IS_DM_MOVE(get_attr_type(p[3])) && 
     IS_PM_MOVE(get_attr_type(p[4])))
    return 1;
  return 0;
}

int 
parallel_4_p(p)
  rtx *p;
{

  if(IS_MULT_FIX(get_attr_type(p[0])) && 
     IS_ALU_FIX(get_attr_type(p[1])) && 
     IS_DM_MOVE(get_attr_type(p[2])) && 
     IS_PM_MOVE(get_attr_type(p[3])))
    return 1;
  if(IS_MULT_FLOAT(get_attr_type(p[0])) && 
     IS_ALU_FLOAT(get_attr_type(p[1])) && 
     IS_DM_MOVE(get_attr_type(p[2])) && 
     IS_PM_MOVE(get_attr_type(p[3])))
    return 1;
  if(is_dual_add_sub(p[1],p[2]) &&
     IS_MULT_FIX(get_attr_type(p[0])) && 
     IS_ALU_FIX(get_attr_type(p[1])) && 
     IS_MOVE(get_attr_type(p[3])))
    return 1;
  if(is_dual_add_sub(p[1],p[2]) &&
     IS_MULT_FLOAT(get_attr_type(p[0])) && 
     IS_ALU_FLOAT(get_attr_type(p[1])) && 
     IS_MOVE(get_attr_type(p[2])))
    return 1;
  if(is_dual_add_sub(p[0],p[1]) &&
     IS_DM_MOVE(get_attr_type(p[2])) && 
     IS_PM_MOVE(get_attr_type(p[3])))
    return 1;
  return 0;
}

int 
parallel_3_p(p)
  rtx *p;
{
  if(is_dual_add_sub(p[0],p[1]) && IS_MOVE(get_attr_type(p[2])))
    return 1;
  if(IS_MULT_FIX(get_attr_type(p[0])) && 
     IS_ALU_FIX(get_attr_type(p[1])) && 
     IS_MOVE(get_attr_type(p[2])))
    return 1;
  if(IS_MULT_FLOAT(get_attr_type(p[0])) && 
     IS_ALU_FLOAT(get_attr_type(p[1])) && 
     IS_MOVE(get_attr_type(p[2])))
    return 1;
  if(IS_COMPUTE(get_attr_type(p[0])) && 
     IS_DM_MOVE(get_attr_type(p[1])) && 
     IS_PM_MOVE(get_attr_type(p[2])))
    return 1;
  if(get_attr_type(p[0])==TYPE_DREG_ZERO && 
     IS_DM_MOVE(get_attr_type(p[1])) &&
     IS_PM_MOVE(get_attr_type(p[2])))
    {
      special.found=1;
      special.where=0;
      special.what="%r0=%r0-%r0";
      special.note="zero";
      return 1;
    }
  if(((IS_MULT_FIX(get_attr_type(p[0])) && IS_ALU_FIX(get_attr_type(p[1]))) ||
      (IS_MULT_FLOAT(get_attr_type(p[0])) && IS_ALU_FLOAT(get_attr_type(p[1])))) &&
     get_attr_type(p[2])==TYPE_DREG_ZERO)
    {
      special.found=1;
      special.where=2;
      special.what="%0=m5;";
      special.note="zero";
      return 1;
    }
  if (get_attr_type(p[0])==TYPE_DREG_MOVE && 
      IS_DM_MOVE(get_attr_type(p[1])) &&
      IS_PM_MOVE(get_attr_type(p[2])))
    {
      special.found=1;
      special.where=0;
      special.what="%0=pass %1;";
      special.note="move";
      return 1;
    }
  if(((IS_MULT_FIX(get_attr_type(p[0])) && IS_ALU_FIX(get_attr_type(p[1]))) ||
      (IS_MULT_FLOAT(get_attr_type(p[0])) && IS_ALU_FLOAT(get_attr_type(p[1])))) &&
     get_attr_type(p[2])==TYPE_DREG_MOVE)
    {
      special.found=1;
      special.where=2;
      special.what="%0=%1;";
      special.note="move";
      return 1;
    }
  return 0;
}

int 
parallel_2_p(p)
  rtx *p;
{
  if(IS_MULT_FIX(get_attr_type(p[0])) && IS_ALU_FIX(get_attr_type(p[1])))
    return 1;

  if(IS_MULT_FLOAT(get_attr_type(p[0])) && IS_ALU_FLOAT(get_attr_type(p[1])))
    return 1;
  if (IS_COMPUTE(get_attr_type(p[0])) && IS_MOVE(get_attr_type(p[1])))
    return 1;

  if(TYPE_SHIFTIMM==get_attr_type(p[0]) 
     && IS_POST_MOD(get_attr_type(p[1])))
    {
      rtx body = single_set(p[1]);

      if (body && 
	  parallel_move_okay_for_shiftimm_p (SET_DEST(body), SET_SRC(body)))
	  return 1;
    }

  if(IS_DM_MOVE(get_attr_type(p[0])) && IS_PM_MOVE(get_attr_type(p[1])))
    return 1;
  if(get_attr_type(p[0])==TYPE_DREG_ZERO && IS_MOVE(get_attr_type(p[1])))
    {
      special.found=1;
      special.where=0;
      special.what="%r0=%r0-%r0";
      special.note="zero";
      return 1;
    }
  if(IS_COMPUTE(get_attr_type(p[0])) && get_attr_type(p[1])==TYPE_DREG_ZERO)
    {
      special.found=1;
      special.where=1;
      special.what="%0=m5;";
      special.note="zero";
      return 1;
    }
  if (get_attr_type(p[0])==TYPE_DREG_MOVE && IS_MOVE(get_attr_type(p[1])))
    {
      special.found=1;
      special.where=0;
      special.what="%0=pass %1;";
      special.note="move";
      return 1;
    }
  if(IS_COMPUTE(get_attr_type(p[0])) && get_attr_type(p[1])==TYPE_DREG_MOVE)
    {
      special.found=1;
      special.where=1;
      special.what="%0=%1;";
      special.note="move";
      return 1;
    }
  if(get_attr_type(p[0])==TYPE_DREG_ZERO && get_attr_type(p[1])==TYPE_DREG_MOVE)
    {
      special.found=1;
      special.where=0;
      special.what="%r0=%r0-%r0;";
      special.note="zero and move";
      return 1;
    }
  if(get_attr_type(p[0])==TYPE_DREG_MOVE && get_attr_type(p[1])==TYPE_DREG_MOVE)
    {
      special.found=1;
      special.where=0;
      special.what="%0=pass %1;";
      special.note="move and move";
      return 1;
    }
  return 0;
}

int 
reorder(n,perm,s,f)
  int n, *perm, f();
  rtx *s;
{
  int i;
  
  rtx *tmp=alloca(n*sizeof(rtx)); /* b'cse WATCOM can't handle - rtx tmp[n]; */

  special.found=0;
  for(i=0;i<n;i++)
    tmp[i]=NULL;
  return re_x(n,n,perm,s,tmp,f);
}

int 
re_x(n,x,perm,in,try,f)
  rtx in[],try[];
  int *perm;
  int n,x,f();
{
  int i;
  if(n==0)
    return (*f)(try);
  for(i=0;i<x;i++)
    if(try[i]==NULL)
      {
	try[i]=in[n-1];
	if(re_x(n-1,x,perm,in,try,f))
	  {
	    perm[i]=n-1;
	    return 1;
	  }
	try[i]=NULL;
      }
  return 0;
}
     
int
peep_parallelizeable_2(active)
    int active;
{
    int result = 0;
    int perm[2];
    
    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;

    if (! set_use_independent_p(peep_insn, 2))
	return 0;
    
    if(reorder(2,perm,peep_insn,parallel_2_p))
      {
	  if (active)
	      collect_templates_and_operands(2, perm);
	return 1;
      }

    return 0;
}

int
peep_parallelizeable_3(active)
    int active;
{
    int result = 0;
    int perm[3];
    

    if (INSN_CODE(peep_insn[0]) < 0) return 0;
    if (INSN_CODE(peep_insn[1]) < 0) return 0;
    if (INSN_CODE(peep_insn[2]) < 0) return 0;

    if (! set_use_independent_p(peep_insn, 3))
      return 0;

    if(reorder(3,perm,peep_insn,parallel_3_p))
      {
	  if (active)
	      collect_templates_and_operands(3, perm);
	  return 1;
      }
    return 0;
}

int
peep_parallelizeable_4(active)
    int active;
{
    int result = 0;
    int perm[4];
    

    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[2]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[3]) < 0) 
      return 0;

    if (! set_use_independent_p(peep_insn, 4)) 
      return 0;

    if(reorder(4,perm,peep_insn,parallel_4_p))
      {
	  if (active)
	      collect_templates_and_operands(4, perm);
	  return 1;
      }
    return 0;
}

int
peep_parallelizeable_5(active)
    int active;
{
    int result = 0;
    int perm[4];
    

    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[2]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[3]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[4]) < 0) 
      return 0;

    if (! set_use_independent_p (peep_insn, 5)) 
      return 0;

    
    if (reorder (5, perm, peep_insn,parallel_5_p))
      {
	  if (active)
	      collect_templates_and_operands(5, perm);
	  return 1;
      }
    return 0;
}

#include "recog.h"

int short_hand_p;

char *
dump_parallel (n)
  int n;
{
  int ii;
  short_hand_p = 1;
  putc('\t', asm_out_file);
  for (ii=0; ii<n; ii++) 
    {
      if(ii>0) 
	putc(',', asm_out_file);
      output_asm_insn (fix_template(peep_templates[ii]),
		       peep_old_operands[ii]);
    }
  putc(';', asm_out_file);
  if(peep_note)
    {
      fputs("   !",asm_out_file);
      fputs(peep_note,asm_out_file);
    }
  short_hand_p = 0;
  return "";
}

static HARD_REG_SET set_regs;

static void
mark_reg_as_set(r, something)
    rtx r, something;
{
    if (REG_P(r))
      SET_HARD_REG_BIT(set_regs, REGNO(r));
    else 
      if(GET_CODE(r)==SUBREG)
	{
	  int i;
	  for(i=0;i<HARD_REGNO_NREGS(SUBREGNO(r),GET_MODE(XEXP(r,0))); i++)
	    SET_HARD_REG_BIT(set_regs,SUBREGNO(r)+i);
	}
}


/*** Returns true if insns in INSN_ARR (of length N_INSNS) ***/
/*** are set/use independent, i.e. no register set by any of ***/
/*** insns is used by any  of the following ones. ***/

/*** This is NOT a complete independency checking -- for example ***/
/*** it does not check the memory access. For 21k this is enought ***/
/*** because only accesses to different memories can be parallelized ***/
/*** This will not work for chips with 2 or more buses to the same memory ***/


int
set_use_independent_p(insn_arr, n_insns)
    rtx insn_arr[];
    int  n_insns;
{
    int inum;
    void mark_reg_as_set();

    CLEAR_HARD_REG_SET(set_regs);
    for(inum=0; inum<n_insns-1; inum++) {
	rtx insn = insn_arr[inum], next_insn;
	if (GET_CODE(insn) != INSN) return 0;
	note_stores(PATTERN(insn_arr[inum]), mark_reg_as_set);
	next_insn = insn_arr[inum+1];
	if (rtx_mentions_hard_regs_p(PATTERN(next_insn), set_regs))
	    return 0;
    }
    /*** If we got out of the loop, we are fine ***/
    return 1;
}


/*** True if X refers to any register from REGSET ***/

int
rtx_mentions_hard_regs_p(x, regset)
    rtx x;
    HARD_REG_SET regset;
{
    char *format;
    enum rtx_code xcode = GET_CODE(x);
    int i;
    int regno=0;

    while (xcode == SUBREG)
      {
	regno += SUBREG_WORD (x);
	x = SUBREG_REG (x);
	xcode = GET_CODE (x);
      }

    if ((xcode == REG) && (TEST_HARD_REG_BIT(regset, REGNO(x)+regno)))
	return 1;

    format = GET_RTX_FORMAT(xcode);

    for (i = GET_RTX_LENGTH (xcode) - 1; i >= 0; i--)
    {
	if (format[i] == 'E')
	{
	    register int j;
	    for (j = XVECLEN (x, i) - 1; j >= 0; j--)
		if (rtx_mentions_hard_regs_p (XVECEXP (x, i, j), regset))
		    return 1;
	}
	else if (format[i] == 'e'
		 && rtx_mentions_hard_regs_p(XEXP (x, i), regset))
	    return 1;
    }
    return 0;
}

/* double -> float */
void real_value_quantize (x0,x1)
     int *x0, *x1;
{
  double value;
  union real_extract u;
  int exp;
  double fr;
return;

  u.i[0]=*x0;
  u.i[1]=*x1;

  value = u.d;

#ifdef __EMX__
#define HUGE_VAL 1.79769313486231570e+308
#endif

#ifndef HUGE_VAL
#define HUGE_VAL HUGE
#endif

  /* This is broken if we get Infinity */
  if (value == HUGE_VAL)
    return;

  fr = frexp (value, &exp); /* checkout X3.159-1989 (X3J11) section 4.5.4.2 */

/* quantize the mantissa */
  fr = ldexp (fr,24);

#if defined (__EMX__) || (defined(__GNUDOS__) || defined (__WATCOMC__))
  modf(fr+.5*((fr<0)?-1:1),&fr);
#else
  modf(fr+copysign(0.5,fr),&fr);
#endif

  fr = ldexp(fr, -24);
  
/* limit the exponent to 8 bits */
  if((exp-1)>127)
    {
      /* floating point number out of range, we've got a problem */
      exp=0;
      error("float constant out of range");
    }
  else if((exp-1) < -128)
    {
      fr=ldexp(fr,exp+128);
      exp=-128;
      pedwarn("float constant under flow");
    }

  u.d = ldexp(fr, exp);  /* this can be found in section 4.5.4.3 */

  *x0=u.i[0];
  *x1=u.i[1];

/* There!  And now how many people do you know comment their C code with
   citations from ANSI spec's? 
*/
   
}
  
  

enum reg_class regno_reg_class[] = 
{

/* First the register file 0-15*/
  r0r3DREGS,        r0r3DREGS,        r0r3DREGS,        r0r3DREGS,
  r4r7DREGS,        r4r7DREGS,        r4r7DREGS,        r4r7DREGS,
  r8r11DREGS,       r8r11DREGS,       r8r11DREGS,       r8r11DREGS,
  r12r15DREGS,      r12r15DREGS,      r12r15DREGS,      r12r15DREGS,
/* Now DAG 1 16-47*/
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
  DM_IREGS,         ALL_REGS,         ALL_REGS,         DM_MREGS,
/* Now DAG 2 48-79*/
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
  PM_IREGS,         ALL_REGS,         ALL_REGS,         PM_MREGS,
/* Now weird stuff 80-109 */
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         USTAT_REGS,
  USTAT_REGS,          SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,         SYSTEM_REGS,
  SYSTEM_REGS,         SYSTEM_REGS, 
/* And even MULT_ACC_REGS 110-111*/
  MULT_ACC_REGS,    MULT_ACC_REGS     };

#if 0
/* Now weird stuff 80-109 */
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS,         ALL_REGS,         ALL_REGS,
  ALL_REGS,         ALL_REGS, 
#endif
int
postmodifiable_memory_operand (op,mode)
     register rtx    op;
     enum machine_mode mode;
{
  if (GET_CODE(op) == MEM)   
    {
      enum rtx_code   code = GET_CODE (XEXP (op,0));
      return (code == POST_INC || code == POST_MODIFY
	      || code == PRE_INC || code == REG);
    }
  return 0;
}

int register30_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int regno = REGNO (op);
  return register_operand (op, mode) 
    && (regno >= REG_R0 && regno <= REG_R3);
}

int register74_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int regno = REGNO (op);
  return register_operand (op, mode) 
    && (regno >= REG_R4 && regno <= REG_R7);
}

int register118_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int regno = REGNO (op);
  return register_operand (op, mode) 
    && (regno >= REG_R8 && regno <= REG_R11);
}

int register1512_operand (op, mode)
     register rtx    op;
     enum machine_mode mode;
{
  int regno = REGNO (op);
  return register_operand (op, mode) 
    && (regno >= REG_R12 && regno <= REG_R15);
}

int
opposite_banks (op1,op2)
     rtx op1,op2;
{
  enum machine_mode m1 = GET_MODE (XEXP (op1,0));
  enum machine_mode m2 = GET_MODE (XEXP (op2,0));
  return ((m1 == DMmode && m2 == PMmode) || (m1 == PMmode && m2 == DMmode));
}



/*
  Fix of the chip problem: write to M or L reg from DAG2 followed by 
  a read from an I-reg from DAG2 requires  a one-cycle stall.

  1. For sequential instructions we just insert a nop between them.
     
  2. If ML-write is the last insn of a do-loop and I-read is the first,
     we insert a nop at the beginning of the loop.

  3. If ML-write is the last insn the delay branch slot and I-read is 
     the target of the jump, we add a NOP in front of the target but after
     the preceding label.

  4. If both ML-write and I-read are inside delay branch slots, we 
     move the insn in the first slot (ML-write) in front of
     the jump, move the second slot (I-read) to the first, and fill
     the second slot with a NOP. This way we do not have to
     check the second slot against the jump target.

*/

static void check_straight_code();
static void check_db_jump();
static void check_doloop_head();
static int  dag2_ml_write_reg();
static int  dag2_i_read_reg();
static int  dag2_i_read_reg_par();


void
fix_dag2_ml_write_i_read (first)
    rtx first;
{
    rtx insn;
    int regnum;
    for (insn=first; insn; insn = next_real_insn(insn))
    {
	check_straight_code(insn);
	check_db_jump(insn);
	check_doloop_head(insn);
	if ((regnum = dag2_ml_write_reg(insn)) >= 0)
	{
	    rtx next      = next_active_insn(insn);
	    if (dag2_i_read_reg_par(next) == regnum)
		emit_insn_before(gen_nop(), next);
	}
    }
}

/*
  Check whether consecutive parallelizable blocks of insns
  contain conflicting instructions.
*/

void
check_straight_code(insn)
    rtx insn;
{
    rtx next;
    int regnum;

    if ((regnum = dag2_ml_write_reg(insn)) < 0)
	return;
    
    /* no more than 4 insns can be parallelized if one of them */
    /* is assignment to M-reg. */

    peep_insn[0] = insn;
    
    next = next_active_insn(peep_insn[0]);
    peep_insn[1] = next;
    if (peep_parallelizeable_2(0))
    {
	next = next_active_insn(peep_insn[1]);
	peep_insn[2] = next;
	if (peep_parallelizeable_3(0))
	{
	    next = next_active_insn(peep_insn[2]);
	    peep_insn[3] = next;
	    if (peep_parallelizeable_4(0))
	    {
		next = next_active_insn(peep_insn[3]);
	    }
	}
    }
    
    if (dag2_i_read_reg_par(next) == regnum)
	emit_insn_before(gen_nop(), next);
}

/* 
  Check the doloop head to see if the last and the first insns of the 
  loop form a bad sequence.
*/

#include "doloop.h"

static void
check_doloop_head(insn)
    rtx insn;
{
    if (doloop_start_p(insn))
    {    
	int regnum;
	rtx 
	    pat = PATTERN(insn),
	    end_label  = XEXP(DOLOOP1_END_LABEL(pat),0),
	    last_insn  = prev_active_insn(end_label),
	    first_insn = next_active_insn(insn);
	if ((regnum = dag2_ml_write_reg(last_insn)) >= 0 &&
	    dag2_i_read_reg_par(first_insn) == regnum)
	    emit_insn_before(gen_nop(), first_insn);
    }
}

/*
  Check wether INSN is a sequence if jump and one or two delay slots.
  If it is, check two slots for being bad, and check the second slot against 
  the jump target, but not against the next sequential insn. If INSN is
  a delay slot, return last insn in the last slot.
*/

static void
check_db_jump(insn)
    rtx insn;
{
    rtx pat, jump, db_slot1=0, db_slot2=0;
    int regnum;

    if (GET_CODE (insn) != INSN)
	return;
    pat = PATTERN(insn);
    if (GET_CODE(pat) != SEQUENCE)
	return;
    /* If only one slot is filled, the second one is NOP, and */
    /* everything is ok */
    if ( XVECLEN(pat, 0) < 3)
	return;
    jump = XVECEXP(pat,0,0);
    if (GET_CODE(jump) != JUMP_INSN)
	return;
    db_slot1 = XVECEXP(pat, 0, 1);
    db_slot2 = XVECEXP(pat, 0, 2);
	
    /* Note that we do not have to check this stuff if */
    /* one delay slot has been shifted out: the second one */
    /* becomes a NOP and can't form bad sequences with either */
    /* the jump target or next sequential insn */

    if ((regnum = dag2_ml_write_reg(db_slot1)) >= 0 &&
	dag2_i_read_reg(db_slot2) == regnum)
    {
	rtx
	    pat1 = PATTERN(db_slot1),
	    pat2 = PATTERN(db_slot2);
	emit_insn_before(pat1, insn);
	PATTERN(db_slot1) = pat2;
	PATTERN(db_slot2) = gen_nop();
    }
    
    /* Check for problems with both jump target and next sequential */

    else if ((regnum = dag2_ml_write_reg(db_slot2)) >= 0)
    {
	rtx 
	    next        = next_active_insn(db_slot2),
	    jump_target = next_active_insn(JUMP_LABEL(jump));
       
	if (jump_target 
	    && dag2_i_read_reg_par(jump_target) == regnum)
	    emit_insn_before(gen_nop(), jump_target);
	if (next
	    && dag2_i_read_reg_par(next) == regnum)
	    emit_insn_before(gen_nop(), next);
    }
}


/* Next two functions return "DAG2-relative" register numbers: */
/* M12, L12, I12 all should return 4, and M8,L8 AND I8 should */
/* all return 0, while non-DAG2  register should return -1 */

static int
dag2_ml_write_reg(insn)
    rtx insn;
{
    rtx pat, dest;
    int regnum;

    if (GET_CODE(insn) != INSN)
	return -1;
    pat = PATTERN(insn);
    if (GET_CODE(pat) != SET)
	return -1;
    dest = SET_DEST(pat);
    SWITCH_SUBREG(dest);
    if (GET_CODE(dest) != REG)
	return -1;
    regnum = REGNO(dest);
    if (IS_PM_MREG(regnum))
	return (regnum - REG_M8);
    if (IS_PM_LREG(regnum))
	return (regnum - REG_L8);
    else return -1;
}

static int
dag2_i_read_reg(insn)
    rtx insn;
{
    rtx pat, src;
    int regnum;

    if (GET_CODE(insn) != INSN)
	return -1;
    pat = PATTERN(insn);
    if (GET_CODE(pat) != SET)
	return -1;
    src = SET_SRC(pat);
    SWITCH_SUBREG(src);
    if (GET_CODE(src) != REG)
	return -1;
    regnum = REGNO(src);
    if (IS_PM_IREG(regnum))
	return (regnum - REG_I8);
    else return -1;
}


/*
  Check whether there are any i-reads in parallelizeable sequence 
  starting with insn. If n are not parallelizeable, then n+1 should 
  not be either. 
*/

static int
dag2_i_read_reg_par(insn)
    rtx insn;
{
    rtx pat, src;
    int regnum;

    if ((regnum = dag2_i_read_reg(insn)) >= 0)
	return regnum;

    peep_insn[0] = insn;

    /* No more than 4 insns can be parallelized if one of them */
    /* is an assignment to I-reg */

    if (!(peep_insn[1] = next_active_insn(peep_insn[0])))
	return -1;

    if (! peep_parallelizeable_2(0)) 
	return -1;
    if ((regnum = dag2_i_read_reg(peep_insn[1])) >= 0)
	return regnum;

    if (!(peep_insn[2] = next_active_insn(peep_insn[1])))
	return -1;

    if (! peep_parallelizeable_3(0)) 
	return -1;
    if ((regnum = dag2_i_read_reg(peep_insn[2])) >= 0)
	return regnum;

    if (!(peep_insn[3] = next_active_insn(peep_insn[2])))
	return -1;

    if (! peep_parallelizeable_4(0)) 
	return -1;
    if ((regnum = dag2_i_read_reg(peep_insn[3])) >= 0)
	return regnum;

    return -1;
}

int
user_defined_segment(decl)
    tree decl;
{
    tree segment_node = GET_SEG_ATTR(decl);
    if (segment_node)
    {
	char *segname = IDENTIFIER_POINTER(segment_node);
	memory_segment ms = TREE_MEMSEG (decl);

	fprintf(asm_out_file, ".endseg;\n");
	fprintf(asm_out_file, ".segment /%s %s;\n",
		MEMSEG_NAME(ms), segname);
    }
    return (segment_node != 0);
}

int
reg_equal_p (x,y)
     rtx x,y;
{
  return GET_CODE (x) == REG && GET_CODE (y) == REG 
    && REGNO (x) == REGNO (y);
}

void
complete_reg_alloc_order()
{
    int i,j;
    int na;  /* Number of Allocated regs, or first Non-Allocated */
    short allocated[FIRST_PSEUDO_REGISTER];

    for(i=0; i<FIRST_PSEUDO_REGISTER; i++)
	allocated[i]=0;
    for(i=0; 
	i<FIRST_PSEUDO_REGISTER && reg_alloc_order[i] < FIRST_PSEUDO_REGISTER; 
	i++)
	allocated[reg_alloc_order[i]] = 1;
    na = i;
    for(i=na, j=0; i < FIRST_PSEUDO_REGISTER; j++) {
	if (! allocated[j]) {
	    reg_alloc_order[i] = j;
	    i++;
	}
    }
}

/*============================================ Profiler ===*/

#ifdef ADI_PROFILER

/* rtx for profile counter */
static rtx function_profcntr_rtx;
static rtx function_total_profcntr_rtx;
static rtx function_call_counter_rtx;

/* rtx for emulator clock */
static rtx emuclock_rtx;

/* Decrement function counter by EMUCLK */

static void
adjust_clock_counter(enum rtx_code op, rtx counter)
{
    rtx
	tmp1 = gen_reg_rtx (SImode),
	tmp2 = gen_reg_rtx (SImode);
    
    emit_insn (gen_rtx (SET, SImode, tmp1, counter));
    emit_insn (gen_rtx (SET, SImode, tmp2, emuclock_rtx));
    emit_insn (gen_rtx (SET, SImode, tmp1, gen_rtx (op, SImode, tmp1, tmp2)));
    emit_insn (gen_rtx (SET, SImode, counter, tmp1));
}



static void
adjust_call_counter()
{
    rtx	tmp1 = gen_reg_rtx (SImode);
    
    emit_insn (gen_rtx (SET, SImode, tmp1, function_call_counter_rtx));
    emit_insn (gen_rtx (SET, SImode, tmp1,
			gen_rtx(PLUS, SImode, tmp1, const1_rtx)));
    emit_insn (gen_rtx (SET, SImode, function_call_counter_rtx, tmp1));
}

void
emit_prologue_profiler(int fstartp)
{
    if (fstartp) {
	adjust_call_counter();
	adjust_clock_counter(MINUS, function_total_profcntr_rtx);
    }
    adjust_clock_counter(MINUS, function_profcntr_rtx);
}

/* Increment function counter by EMUCLK */

void
emit_epilogue_profiler(int fendp)
{
    if (fendp)
	adjust_clock_counter(PLUS, function_total_profcntr_rtx);
    adjust_clock_counter(PLUS, function_profcntr_rtx);
}

/*
  Create the slot for the counter and 
  initialize function_cntr_address.
  */

void
init_function_profiler()
{
    const char *prefix =   "_profcntr_";
    const int plen = 12;
    extern int var_labelno;
    extern struct obstack *saveable_obstack;
    int fname_len = strlen(current_function_name);
    char *profcntr_name =
	obstack_alloc(saveable_obstack, fname_len + plen + 10);

    sprintf(profcntr_name,
	    "%s%s_%d", prefix, current_function_name, var_labelno++);
    function_profcntr_rtx = 
	gen_rtx(MEM, SImode,
		gen_rtx(SYMBOL_REF, DMmode, profcntr_name));
    function_call_counter_rtx = 
	gen_rtx(MEM, SImode, plus_constant (gen_rtx (SYMBOL_REF, DMmode, profcntr_name), 1));

    function_total_profcntr_rtx =
	gen_rtx(MEM, SImode, plus_constant (gen_rtx (SYMBOL_REF, DMmode, profcntr_name), 2));

    data_section();
    fprintf(asm_out_file, "\t.global _%s;\n", profcntr_name);
    fprintf(asm_out_file, "\t.var    _%s[3] = 0,0,0;\n", profcntr_name);
}

#endif   /** ADI_PROFILER **/
