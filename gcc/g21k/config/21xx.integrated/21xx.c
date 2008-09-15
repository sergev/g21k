/*
 * Definitions of target machine for GNU compiler.
 *
 * Analog Devices ADSP 21XX Dev. Tools Team
 *
 *
 * Copyright (C) 1990 Free Software Foundation, Inc.
 *
 * This file is part of GNU CC.
 *
 * GNU CC is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 1, or (at your option) any later version.
 *
 * GNU CC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * GNU CC; see the file COPYING.  If not, write to the Free Software
 * Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include "config.h"
#include "rtl.h"
#include "expr.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "output.h"
#include "tree.h"
#include "insn-attr.h"
#include "input.h"
#include "reload.h"
#include "flags.h"

extern int      optimize;
extern int      flag_delayed_branch;


static char    *fp;
static char    *sp;
static int      Number_of_registers_saved;
static char    *varname;


int  *mod_reg_equiv_const;
int  mod_reg_equiv_size;

/* Identification String */ static char *__ident__ = "@(#) aux-output21xx.c 1.5@(#)";

#define RET  return ""                           /* Used in machine
						  * description */
#define IS_REG_SAVETYPE(i) 1
#define IS_REG_RESERVED(i) 0
    
  memory_segment  default_memory_segment = DATA_MEM;
memory_segment  code_memory_segment = PROG_MEM;
enum machine_mode stack_pmode;

output_int(FILE *file, long x)
{
  fprintf(file, "%d", (x<<16)>>16); /* sign extend this puppy ! */
}

int             max_arg_regs = 2;
static          arg_regs[] = {16, 3, 999};


void           *
  function_arg(cum, mode, type, named)
CUMULATIVE_ARGS cum;
enum machine_mode mode;
tree            type;
int             named;

{
  rtx             arg = 0;
  static          already_on_stack = 0;
    
  if (cum == 0)
    already_on_stack = 0;
    
  if (!already_on_stack &&
      named &&
      cum < max_arg_regs &&
      GET_MODE_SIZE(mode) == 1)
    arg = gen_rtx(REG, mode, arg_regs[cum]);
  else
    already_on_stack = 1;
    
  return arg;
}

int
  function_arg_regno_p(n)
int             n;
{
  int             i;
    
  for (i = 0; i < max_arg_regs; i++)
    if (n == arg_regs[i])
      return 1;
  return 0;
}



enum reg_class
reg_class_intersection(c1, c2)
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

notice_update_cc(exp)
     rtx             exp;
{
  CC_STATUS_INIT;
  return;
    
  switch (GET_CODE(exp))
    {
    case SET:
      switch (GET_CODE(SET_DEST(exp)))
	{
	case MEM:
	  switch (GET_CODE(SET_SRC(exp)))
	    {
	    case REG:                            /* if doing register to
						  * memory tranfer, don't
						  * update the CC, but might
						  * invalidate the RTX's from
						  * which the CC came from */
	      if (cc_status.value1
		  && reg_overlap_mentioned_p(SET_DEST(exp), cc_status.value1))
		cc_status.value1 = 0;
	      if (cc_status.value2
		  && reg_overlap_mentioned_p(SET_DEST(exp), cc_status.value2))
		cc_status.value2 = 0;
	      return;                            /* transfer to memory, no CC
						  * update */
	    default:
	      abort();
	    }
	case REG:
	  switch (GET_CODE(SET_SRC(exp)))
	    {
	    case REG:
	      return;                            /* register to register
						  * transfer, no CC update */
	    case PLUS:
	    case MINUS:
	    case MULT:
	      cc_status.value1 = SET_SRC(exp);
	      cc_status.value2 = SET_DEST(exp);
	      return;
	    case MEM:                            /* same as up above, doing
						  * mem to reg xfer doesn't
						  * change CC, but might
						  * change what CC came from */
	      if (cc_status.value1 && GET_CODE(cc_status.value1) == MEM)
		cc_status.value1 = 0;
	      if (cc_status.value2 && GET_CODE(cc_status.value2) == MEM)
		cc_status.value2 = 0;
	      return;
	    case CONST_INT:                      /* loading a constant
						  * doesn't set it */
	      return;
	    default:
	      abort();
	    }
	default:
	  abort();
	}
    case CALL:
      CC_STATUS_INIT;                            /* Function calls clobber
						  * the cc's.  */
      return;
    default:
      abort();
    }
    
}




/*
 * RUNTIME ENVIRONMENT:
 *
 *
 *
 */
void
  restore_registers(file, size)
FILE           *file;
int             size;
{
  int             regno;
    
  if (Number_of_registers_saved &&
      (strcmp("main_", current_function_name) != 0))
    {
      for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
	if (regs_ever_live[regno]
	    && !call_used_regs[regno]
	    && IS_REG_SAVETYPE(regno)
	    && !IS_REG_RESERVED(regno)
	    && regno != FRAME_POINTER_REGNUM
	    && regno != STACK_POINTER_REGNUM)
	  {
	    if (IS_DREG(regno))
	      {
#ifdef PM_MINUS1_MREG
		  fprintf(file, "\t%s=dm(%s,%s);\n", reg_names[regno],
			  reg_names[SCRATCH_PM_IREG],
			  reg_names[PM_MINUS1_MREG]);
#else
		  fprintf(file, "\t%s=dm(%s,%s);\n", reg_names[regno],
			  reg_names[SCRATCH_PM_IREG],
			  reg_names[SCRATCH_PM_MREG]);
#endif

	      }
	    else
	      {
#ifdef PM_MINUS1_MREG
		  fprintf(file, "\t%s=dm(%s,%s);\n", reg_names[SCRATCH_DREG],
			  reg_names[SCRATCH_PM_IREG],
			  reg_names[PM_MINUS1_MREG]);
#else
		  fprintf(file, "\t%s=dm(%s,%s);\n", reg_names[SCRATCH_DREG],
			  reg_names[SCRATCH_PM_IREG],
			  reg_names[SCRATCH_PM_MREG]);
#endif
		fprintf(file, "\t%s=%s;\n", reg_names[regno], reg_names[SCRATCH_DREG]);
	      }
	  }
    }
}



void
  save_registers(file, size)
FILE           *file;
int             size;
{
  int             regno;
    
  if (Number_of_registers_saved &&
      strcmp("_main", current_function_name) != 0)
    {
      for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
	if (regs_ever_live[regno]
	    && !call_used_regs[regno]
	    && IS_REG_SAVETYPE(regno)
	    && !IS_REG_RESERVED(regno)
	    && regno != FRAME_POINTER_REGNUM)
	  {
	    if (IS_DREG(regno))
	      {
		fprintf(file, "\tdm(%s,%s)=%s;\n", sp,
#ifdef PM_MINUS1_MREG
			reg_names[PM_MINUS1_MREG],
#else
			reg_names[SCRATCH_PM_MREG],
#endif
			reg_names[regno]);
	      }
	    else
	      {
		fprintf(file, "\t%s=%s;\n", reg_names[SCRATCH_DREG], reg_names[regno]);
		fprintf(file, "\tdm(%s,%s)=%s;\n", sp,
#ifdef PM_MINUS1_MREG
			reg_names[PM_MINUS1_MREG],
#else
			reg_names[SCRATCH_PM_MREG],
#endif
			reg_names[SCRATCH_DREG]);
	      }
	  }
    }
}


/*
  want:
    
  * if(size>0)
    
  scratch=fp;
  fp=sp;
  m=-(size+1)
  dm(sp,m)=scratch
    
  dm(sp,-1)=save xn
    
  * if(size==0)
    
  scratch=fp
  fp=sp
  dm(sp,-1)=scratch
    
  dm(sp,-1)=save xn
    
  */
void
  function_prologue(file, size)
FILE           *file;
int             size;
{
  int             regno;
  int             numregs_used = 0;
  int             size_plus_save_regs;
    
  fprintf(file,"\tmr1=toppcstack;\n");
  fprintf(file,"\tmr0=%d;\n", -(size+1));
  fprintf(file,"\tcall ___lib_save_large_frame;\n");

#if 0  

  fprintf(file, "!\tFUNCTION PROLOGUE: %s\n",
	  current_function_name);
    
  for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
    if (regs_ever_live[regno] && !call_used_regs[regno])
      numregs_used++;
    
  Number_of_registers_saved = numregs_used;
    
  fprintf(file, "\t%s=%s;\n", reg_names[SCRATCH_DREG], fp);
  fprintf(file, "\t%s=%s;\n", fp, sp);

#ifndef PM_MINUS1_MREG
  if (Number_of_registers_saved || size==0)
    fprintf(file, "\t%s=-1;\n",reg_names[SCRATCH_PM_MREG]);
#endif

  if (size > 0)
    {
      fprintf(file, "\t%s=%d;\n", reg_names[SCRATCH_PM_MREG], -(size + 1));
      fprintf(file, "\tdm(%s,%s)=%s;\n", sp, reg_names[SCRATCH_PM_MREG],
	      reg_names[SCRATCH_DREG]);
        
    }
  else
    {
#ifdef PM_MINUS1_MREG
      fprintf(file, "\tdm(%s,%s)=%s;\n", sp, reg_names[PM_MINUS1_MREG], reg_names[SCRATCH_DREG]);
#else
      fprintf(file, "\tdm(%s,%s)=%s;\n", sp, reg_names[SCRATCH_PM_MREG], reg_names[SCRATCH_DREG]);
#endif
    }
  fprintf(file, "!\tsaving registers: \n");
  save_registers(file, size);
    
  fprintf(file, "!\tEND FUNCTION PROLOGUE: \n");
#endif
}


/*
    
  want:
    
  * if (size >0)
    
  i=fp
  m=-(size)
  scratch=dm(i,m)
    
  restore=dm(i,-1)  xn
  sp=fp
  fp=scratch
    
  * if(size==0)
    
  i=fp
  scratch=dm(i,-1)
    
  restore=dm(i,-1) xn
  sp=fp
  fp=scratch
    
    
    
  * but if no registers were saved:
    
  sp=fp
  scratch=dm(sp,0)
  fp=scratch;
    
  */


void
  function_epilogue(file, size)
FILE           *file;
int             size;
{
  fprintf(file,"\tmr0=%d;\n",-(size+1));
  fprintf(file,"\tjump ___lib_restore_large_frame;\n");

#if 0

  fprintf(file, "!\tFUNCTION EPILOGUE: \n");
    

  if (Number_of_registers_saved > 0)
    {
#ifndef PM_MINUS1_MREG
      if(size==0 || Number_of_registers_saved)
	fprintf(file, "\t%s=-1;\n",reg_names[SCRATCH_PM_MREG]);
#endif
      fprintf(file, "\t%s=%s;\n", reg_names[SCRATCH_PM_IREG], fp);
      if (size > 0)
	{
	  fprintf(file, "\t%s=%d;\n", reg_names[SCRATCH_PM_MREG], -size);
	  fprintf(file, "\t%s=dm(%s,%s);\n",
		  reg_names[SCRATCH_DREG],
		  reg_names[SCRATCH_PM_IREG],
		  reg_names[SCRATCH_PM_MREG]);
	}
      else
	fprintf(file, "\t%s=dm(%s,%s);\n",
		reg_names[SCRATCH_DREG],
		reg_names[SCRATCH_PM_IREG],
#ifdef PM_MINUS1_MREG
		reg_names[PM_MINUS1_MREG]);
#else
		reg_names[SCRATCH_PM_MREG]);
#endif
        
      restore_registers(file, size);
        
      fprintf(file, "\t%s=%s;\n", sp, fp);
    }
  else
    {
      fprintf(file, "\t%s=%s;\n", sp, fp);
      fprintf(file, "\t%s=dm(%s,%s);\n",
	      reg_names[SCRATCH_DREG],
	      sp,
	      reg_names[PM_ZERO_MREG]);
    }
  fprintf(file, "\t%s=%s;\n",
	  fp,
	  reg_names[SCRATCH_DREG]);
    
  fprintf(file, "\trts;\n");
#endif
    
}



output_asm_return_insn()
{
  fprintf(asm_out_file, "\tjump __%s_end;!stupid\n", current_function_name);
}


char           *arch_reserved_registers = 0;

void
  conditional_register_usage()
{
  char           *cp;
  int             done,
  i;
    
#ifdef  PM_MINUS1_MREG  
  fixed_regs[PM_MINUS1_MREG] = 1;
#endif

#ifdef DM_MINUS1_MREG   
  fixed_regs[DM_MINUS1_MREG] = 1;
#endif

#ifdef DM_PLUS1_MREG    
  fixed_regs[DM_PLUS1_MREG] = 1;
#endif


  cp = arch_reserved_registers;
  if (!arch_reserved_registers)
    return;
  while (*cp)
    {
      if (*cp == ',')
	cp++;
      else
	{
	  for (i = 0, done = 0; i < FIRST_PSEUDO_REGISTER && !done; i++)
	    if (reg_names[i][0] && !strncasecmp(reg_names[i], cp, strlen(reg_names[i])))
	      {
		if (fixed_regs[i])
		  error("Can't reserve fixed register \"%s\"", reg_names[i]);
		else if (call_used_regs[i])
		  error("Can't reserve call-used register \"%s\"", reg_names[i]);
		else
		  fixed_regs[i] = 1;
		cp += strlen(reg_names[i]);
		done = 1;
	      }
	  if (!done)
	    {
	      error("Unrecognized reserved register \"%s\"", cp);
	      return;
	    }
	}
    }
}

asm_make_name(file, name)
     FILE           *file;
     char           *name;
{
/* remap '-' in names to '_' (2100 likes it that way! )
   remap '.' to '_' too! */
  char *cp=name;
  while(*++cp)
    if(*cp=='-' | *cp=='.')
      *cp='_';
  fprintf(file, "%s_", name);
}

asm_output_label(file, name)
     FILE           *file;
     char           *name;
{
  asm_make_name(file, name);
  fputs(":\n", file);
}

asm_globalize_label(file, name)
     FILE           *file;
     char           *name;
{
  fputs(".entry\t", file);
  asm_make_name(file, name);
  fputs(";\n", file);
}

asm_output_labelref(file, name)
     FILE           *file;
     char           *name;
{
  fputc('^', file);
  asm_make_name(file, name);
}

asm_output_internal_label(file, prefix, num)
     FILE           *file;
     char           *prefix;
     int             num;
{
  char            s[100];
    
  asm_generate_internal_label(s, prefix, num);
  asm_make_name(file, s);
  fputc(':', file);
}

asm_generate_internal_label(label, prefix, num)
     char           *label;
     char           *prefix;
     int             num;
{
  sprintf(label, "%s%d", prefix, num);
}


asm_output_reg_push(file, regno)
     FILE           *file;
     int             regno;
{
#ifdef PM_MINUS1_MREG
  fprintf(file, "\tdm(%s,%s)=%s;\n",
	  sp,
	  reg_names[PM_MINUS1_MREG],
	  reg_names[regno]);
#else
  abort();
#endif
}

asm_output_reg_pop(file, regno)
     FILE           *file;
     int             regno;
{
  fprintf(file, "\t%s=dm(%s,POP!);\n",
	  reg_names[regno],
	  sp);
}

asm_output_int(file, value)
     FILE           *file;
     rtx             value;
{
  output_addr_const(file, value);
}

asm_output_short(file, value)
     FILE           *file;
     rtx             value;
{
  output_addr_const(file, value);
}

asm_output_char(file, value)
     FILE           *file;
     rtx             value;
{
  output_addr_const(file, value);
}

asm_output_byte(file, value)
     FILE           *file;
     int             value;
{
  output_int(file,value);
}

asm_output_float(file, value)
     FILE           *file;
     int             value;
{
  output_int(file,value);
}

asm_output_double(file, value)
     FILE           *file;
     int             value;
{
  output_int(file,value);
}

asm_output_ascii(file, ptr, len)
     FILE           *file;
     char           *ptr;
     int             len;
{
  int             i;
  int             lcount = 0;
    
  for (i = 0; i < len; i++)
    {
      output_int(file,ptr[i]);
      if (i + 1 < len)
	fputc(',',file);
      if ((++lcount % 20) == 0)
	fprintf(file, "\n");
    }
}

generate_global_varibale(file, name, size, rounded)
     FILE           *file;
     char           *name;
     int             size,
       rounded;
{
  varname = name;
  data_section();
    
  fputs(".global\t", file);
  asm_make_name(file, name);
  fprintf(file, ";\n");
    
  fputs(".var\t", file);
  asm_make_name(file, name);
  if (rounded > 1)
    fprintf(file, "[%d]", rounded);
    
  fprintf(file, ";\n");
}



generate_local_varibale(file, name, size, rounded)
     FILE           *file;
     char           *name;
     int             size,
       rounded;
{
  varname = name;
  data_section();
  fputs(".var\t", file);
  asm_make_name(file, name);
  if (rounded > 1)
    fprintf(file, "[%d]", rounded);
  fprintf(file, ";\n");
    
}


/********************************************/
/*** Implementation of Jump Tables **********/
/********************************************/


/* The jump_table_separator variable is used to insert separator     */
/* characters (',' in our case) in front of each byt the very        */
/* first element of the jump table. This is achieved by              */
/* by initializing it to empty string (jt_first_separator), and then */
/* then switching it to the real thing (jt_non_first_separator).     */

static char     jump_table_separator[256];


asm_output_addr_vec_elt(file, value)
     FILE           *file;
     int             value;
{
  char            s[100];
    
  sprintf(s, "L%d", value);
  fprintf(file, "%s^", jump_table_separator);
  asm_make_name(file, s);
  strcpy(jump_table_separator, ",\n\t\t");
}

asm_output_addr_diff_elt(file, value, rel)
     FILE           *file;
     int             value;
     int             rel;
{
  fprintf(file, "\t.word L%d-L%d\n", value, rel);
}

asm_output_case_label(file, prefix, num, table)
     FILE           *file;
     char           *prefix;
     int             num;
     rtx             table;
{
  rtx             pattern;
  int             table_len;
  char            name[256];
    
  pattern = PATTERN(table);
  table_len = XVECLEN(pattern, 0);
  asm_generate_internal_label(name, prefix, num);
  fprintf(file, ".var %s_[%d];\n", name, table_len);
  sprintf(jump_table_separator, ".init %s_:\t", name);
}

asm_output_case_end(file, num, table)
     FILE           *file;
     int             num;
     rtx             table;
{
  fprintf(file, ";\n");
}


asm_output_align(file, log)
     FILE           *file;
     int             log;
{
  if ((log) != 0)
    fprintf(file, "ERROR (align %d)\n", log);
}

asm_output_skip(file, size)
     FILE           *file;
     int             size;
{
  while(--size)
    {
      fprintf(file, "0,");
    }
  fprintf(file,"0");
}

int
  print_operand_punct_valid_p(code)
char            code;
{
  static char    *legal_codes = "?_.!#LMjJubD";
  char           *p = legal_codes;
    
  while (*p)
    if (code == *p++)
      return 1;
  return 0;
}


/* Print operand X (an rtx) in assembler syntax to file FILE.
   CODE is a letter or dot (`z' in `%z0') or 0 if no letter was specified.
   For `%' followed by punctuation, CODE is the punctuation and X is null.
     
   */



print_operand(file, x, code)
     FILE           *file;
     rtx             x;
     char            code;
{
    
  switch (code)
    {
    case 'A':
      output_address(x);
      break;
    case 'S':
      fprintf(file, "%s", reg_names[STACK_POINTER_REGNUM]);
      break;
    case 's':
      fprintf(file, "dm");
      break;
    case 'M':
#ifdef PM_MINUS1_MREG
      fprintf(file, "%s", reg_names[PM_MINUS1_MREG]);
#else
      abort();
#endif
      break;
    case 'm':
#ifdef DM_MINUS1_MREG
      fprintf(file, "%s", reg_names[DM_MINUS1_MREG]);
#else
      abort();
#endif
      break;
    case 'Z':
      fprintf(file, "%s", reg_names[PM_ZERO_MREG]);
      break;
    case 'z':
      fprintf(file, "%s", reg_names[DM_ZERO_MREG]);
      break;
    case 'p':
#ifdef DM_PLUS1_MREG
      fprintf(file, "%s", reg_names[DM_PLUS1_MREG]);
#else
      abort();
#endif
      break;
    case 'k':
      switch (GET_CODE(x))
	{
	case REG:
	  fprintf(file, "%s", reg_names[REGNO(x)]);
	  break;
	case CONST_INT:
	  switch (INTVAL(x))
	    {
	    case 0:
	      fprintf(file, "%s", reg_names[DM_ZERO_MREG]);
	      break;
#ifdef DM_MINUS1_MREG
	    case -1:
	      fprintf(file, "%s", reg_names[DM_MINUS1_MREG]);
	      break;
#endif
#ifdef DM_PLUS1_MREG
	    case 1:
	      fprintf(file, "%s", reg_names[DM_PLUS1_MREG]);
	      break;
#endif
	    default:
	      output_operand_lossage("invalid %%c value");
	    }
	  break;
	default:
	  output_operand_lossage("invalid %%c value");
	}
      break;
    case 'K':
      switch (GET_CODE(x))
	{
	case REG:
	  fprintf(file, "%s", reg_names[REGNO(x)]);
	  break;
	case CONST_INT:
	  switch (INTVAL(x))
	    {
#if PM_MINUS1_MREG
	    case -1:
	      fprintf(file, "%s", reg_names[PM_MINUS1_MREG]);
	      break;
#endif
	    case 0:
	      fprintf(file, "%s", reg_names[PM_ZERO_MREG]);
	      break;
	    default:
	      output_operand_lossage("invalid %%c value");
	    }
	  break;
	default:
	  output_operand_lossage("invalid %%c value");
	}
      break;
    case '?':
      break;
        
    case '!':
      break;
        
    case '_':
      fprintf(file, "%s", reg_names[SCRATCH_DREG]);
      break;
        
    case '.':
      fprintf(file, "%s", reg_names[DM_ZERO_MREG]);
      break;
        
    case 'D':
      fprintf(file, "l%d", REGNO(x) - REG_I0);
      break;
        
    case 'j':
      switch (GET_CODE(x))
	{
	case EQ:
	  fprintf(file, "eq");
	  break;
	case NE:
	  fprintf(file, "ne");
	  break;
	    
	case GT:
	case GTU:
	  fprintf(file, "gt");
	  break;
	    
	case LT:
	case LTU:
	  fprintf(file, "lt");
	  break;
	    
	case GE:
	case GEU:
	  fprintf(file, "ge");
	  break;
	    
	case LE:
	case LEU:
	  fprintf(file, "le");
	  break;
	    
	default:
	  output_operand_lossage("invalid %%j value");
	}
      break;
        
    case 'J':                                    /* reverse logic */
      switch (GET_CODE(x))
	{
	case EQ:
	  fprintf(file, "ne");
	  break;
	case NE:
	  fprintf(file, "eq");
	  break;
	    
	case GT:
	case GTU:
	  fprintf(file, "le");
	  break;
	    
	case LT:
	case LTU:
	  fprintf(file, "ge");
	  break;
	    
	case GE:
	case GEU:
	  fprintf(file, "lt");
	  break;
	    
	case LE:
	case LEU:
	  fprintf(file, "gt");
	  break;
	default:
	  output_operand_lossage("invalid %%j value");
	}
      break;
    case 'Q':
      fprintf(file, "dm(");
      print_operand_address_1(file, XEXP(x, 0));
      fprintf(file, ")");
      break;
    case 'F':
      fprintf(file, "%s", reg_names[REGNO(x)]);
      break;
    case 'f':
      fprintf(file, "%s", reg_names[REGNO(x) + 1]);
      break;
        
    case 'O':
      if (GET_CODE(x) == LABEL_REF)
	{
	  char            buf[256];
	    
	  ASM_GENERATE_INTERNAL_LABEL(buf, "L", CODE_LABEL_NUMBER(XEXP(x, 0)));
	  assemble_name(file, buf);
	  break;
	}
      /* else fall thru to default */
    default:
      switch (GET_CODE(x))
	{
	case REG:
	  fprintf(file, "%s", reg_names[REGNO(x) + ((code == 'b') ? 1 : 0)]);
	  break;
	case MEM:
	  fprintf(file, "dm(");
	  output_address(XEXP(x, 0));
	  fprintf(file, ")");
	  break;
	case CONST_INT:
	  output_int(file,INTVAL(x));
	  break;
	case SYMBOL_REF:
	  assemble_name(file, XSTR(x, 0));
	  break;
	case CONST:
	  print_operand(file,XEXP(x,0),0);
	  break;
	default:
	  output_addr_const(file, x);
	}
    }
}



print_operand_address_1(file, addr)
     FILE           *file;
     register rtx    addr;
{
    
  switch (GET_CODE(addr))
    {
    case REG:
      if (IS_DAG2(REGNO(addr)))
	fprintf(file, "%s,%s", reg_names[REGNO(addr)], reg_names[PM_ZERO_MREG]);
      else
	fprintf(file, "%s,%s", reg_names[REGNO(addr)], reg_names[DM_ZERO_MREG]);
      break;
#if defined PM_MINUS1_MREG && defined DM_MINUS1_MREG
    case POST_DEC:
      if (IS_DAG2(REGNO(XEXP(addr, 0))))
	fprintf(file, "%s,%s", reg_names[REGNO(XEXP(addr, 0))], reg_names[PM_MINUS1_MREG]);
      else
	fprintf(file, "%s,%s", reg_names[REGNO(XEXP(addr, 0))], reg_names[DM_MINUS1_MREG]);
      break;
#endif
#if defined DM_MINUS1_MREG
    case POST_INC:
      if (IS_DAG2(REGNO(XEXP(addr, 0))))
	fprintf(file, "%s,m5", reg_names[REGNO(XEXP(addr, 0))]);
      else
	fprintf(file, "%s,%s", reg_names[REGNO(XEXP(addr, 0))], reg_names[DM_PLUS1_MREG]);
      break;
#endif
    case POST_MODIFY:
    {
	rtx
	    base     = XEXP(addr,0),
	    modifier = XEXP(addr,1);
	SWITCH_SUBREG(base);
	SWITCH_SUBREG(modifier);
	fprintf(file, "%s,%s", reg_names[REGNO(base)],
		reg_names[REGNO(modifier)]);
    }
      break;
    case PLUS:
      if (GET_CODE(XEXP(addr, 0)) == CONST_INT)
	{
	  print_operand_address(file, XEXP(addr, 1));
	  if (INTVAL(XEXP(addr, 0)) >= 0)
	    fprintf(file, "+");
	  print_operand_address(file, XEXP(addr, 0));
	}
      else
	{
	  print_operand_address(file, XEXP(addr, 0));
	  if (INTVAL(XEXP(addr, 1)) >= 0)
	    fprintf(file, "+");
	  print_operand_address(file, XEXP(addr, 1));
	}
      break;
    case MINUS:
      print_operand_address(file, XEXP(addr, 0));
      fprintf(file, "-");
      print_operand_address(file, XEXP(addr, 1));
      break;
    case SYMBOL_REF:
      asm_make_name(file, XSTR(addr, 0));
      break;
    case LABEL_REF:
      {
	char            s[100];
	
	sprintf(s, "L%d", CODE_LABEL_NUMBER(XEXP(addr, 0)));
	asm_make_name(file, s);
      }
      break;
    case CODE_LABEL:
      {
	char            s[100];
	
	sprintf(s, "L%d", CODE_LABEL_NUMBER(addr));
	asm_make_name(file, s);
      }
      break;
    case CONST:
      print_operand_address(file, XEXP(addr, 0));
      break;
    default:
      output_addr_const(file, addr);
    }
}


print_operand_address(file, addr)
     FILE           *file;
     register rtx    addr;
{
  print_operand_address_1(file, addr);
}




/*** Print a symbol separating elements of an array or ***/
/*** a record ***/

void
  asm_output_elt_sep(file)
FILE           *file;
{
  fprintf(file, ",");
}

/*** Print out the size of an initialized array. ***/
/*** It normaly follows the name of the array    ***/

void
  asm_output_array_size(file, size)
FILE           *file;
int             size;
{
  fprintf(file, "[%d]", size);
}

/*** Print out the symbol specifying the assignment of ***/
/*** initial values to a static  variable (usualy "=") ***/

void
  asm_output_init_assign(file)
FILE           *file;
{
  fprintf(file, ".init\t%s_: ", varname);
}


/*** In case when variables have labels different ***/
/*** from those for functions */

void
  asm_output_var_label(file, name)
FILE           *file;
char           *name;
{
  varname = name;
  fprintf(file, ".var\t%s_;\n", name);
}

void
  asm_output_external(file, decl, name)
FILE           *file;
rtx             decl;
char           *name;
{
  fprintf(file, ".external\t%s_;\n", name);
}

void
  asm_output_external_libcall(file, symref)
FILE           *file;
rtx             symref;
{
  fprintf(file, ".external\t");
  asm_make_name(file, XSTR(symref, 0));
  fprintf(file, ";\n");
}


/******** Functions to print the header and the tail of ********/
/******** the assembler file **********************************/

void
  asm_file_start(file)
FILE           *file;
{
  register char  *module_name = (char *) xmalloc(strlen(main_input_filename) + 6);
  int             len = strlen(main_input_filename);
    
  strcpy(module_name, main_input_filename);
  strip_off_ending(module_name, len);
    
  fp = reg_names[FRAME_POINTER_REGNUM];
  sp = reg_names[STACK_POINTER_REGNUM];
    
  fprintf(file, "!\t\tAnalog Devices ADSP21XX\n");
  fprintf(file, ".MODULE/RAM\t_");
  asm_make_name(file, module_name);
  fprintf(file, ";\n");
  fprintf(file, ".external ___lib_save_large_frame;\n");
  fprintf(file, ".external ___lib_restore_large_frame;\n");
}

void
  asm_file_end(file)
FILE           *file;
{
  fprintf(file, ".ENDMOD;\n");
    
#if defined(KEEP_STAT) && defined(ADSP_PROFILE_STATUS)
  if (KEEP_STAT)
    write_stat_file();
#endif
}


/*
 * operand Predicates
 */

int
  indexreg_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  if (GET_MODE(op) != mode && mode != VOIDmode)
    return 0;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		(IS_IREG(REGNO(op)))));
  return (result);
}

int
  dag1_i_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		TEST_HARD_REG_BIT(reg_class_contents[DM_IREGS], REGNO(op))));
    
  return (result);
}

int
  dag1_m_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		TEST_HARD_REG_BIT(reg_class_contents[DM_MREGS], REGNO(op))));
  return (result);
}

int
  dag2_i_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		TEST_HARD_REG_BIT(reg_class_contents[PM_IREGS], REGNO(op))));
  return (result);
}
int
  dag2_m_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		TEST_HARD_REG_BIT(reg_class_contents[PM_MREGS], REGNO(op))));
  return (result);
}

int
  dag1_m_or_imm(op, mode)
register rtx    op;
enum machine_mode mode;
{
  return dag1_m_operand(op, mode) || immediate_operand(op, mode);
}
int
  dag2_m_or_imm(op, mode)
register rtx    op;
enum machine_mode mode;
{
  return dag2_m_operand(op, mode) || immediate_operand(op, mode);
}

int
  register_non_fp_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  return (GET_CODE(op) == REG && (REGNO(op) != FRAME_POINTER_REGNUM));
}

int
  a_r_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		(REGNO(op) == REG_AR)));
  return result;
    
}
int
  mr_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = mode == DImode && (GET_CODE(op) == REG
			      && (IS_PSEUDO(REGNO(op)) ||
				  (REGNO(op) == REG_MR0)));
  return result;
    
}
int
  alu_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
  int             result;
    
  result = (GET_CODE(op) == REG
	    && (IS_PSEUDO(REGNO(op)) ||
		TEST_HARD_REG_BIT(reg_class_contents[ALU_REGS], REGNO(op))));
  return result;
    
}

/* general_or_imm_operand is used because general_operand rejects
   partial integer immediates */
int
  general_or_imm_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
    
  if (CONSTANT_P(op))
    return 1;
    
  return general_operand(op, mode);
}

int
  sf_register_operand(op, mode)
rtx             op;
enum machine_mode mode;
{
  int             regno;
    
  if (!REG_P(op))
    return 0;
  regno = REGNO(op);
  if (reg_renumber && IS_PSEUDO(regno))
    regno = reg_renumber[regno];
  if (regno >= 0 && !IS_PSEUDO(regno) && !hard_regno_mode_ok(regno, mode))
    return 0;
  return 1;
}

int
  sf_operand(op, mode)
rtx             op;
enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case REG:
      return sf_register_operand(op, mode);
    case CONST_DOUBLE:
    case CONST:
      return 1;
    case MEM:
      return (GET_MODE(op) == mode && dag1_i_operand(XEXP(op, 0), 0));
    }
  return 0;
}

int
  register_or_imm_operand(op, mode)
register rtx    op;
enum machine_mode mode;
{
    
  return register_operand(op, mode) || immediate_operand(op, mode);
}

int register_or_one_operand(op,mode)
register rtx    op;
enum machine_mode mode;
{
  return register_operand(op,mode) ||
    (GET_CODE(op)==CONST_INT && (INTVAL(op)==1 || INTVAL(op)==-1));
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

/**** KLUDGE (az, 12/11/91) ****/

void
  trampoline_template(file)
FILE           *file;
{
  fprintf(file, "! -------- Trampoline Template (TBD) ---------\n");
}

/**** KLUDGE (az, 12/11/91) ****/

int
  initial_frame_pointer_offset()
{
  return (111);
}


/********* dsp21k ************/

asm_declare_object_name(file, name, decl)
     FILE           *file;
     char           *name;
     tree            decl;
{
  int             size = int_size_in_bytes(TREE_TYPE(decl));
    
  varname = name;
  if (size < 0)
    abort();
  fprintf(file, ".var\t");
  asm_make_name(file, name);
  if (size != 1)
    fprintf(file, "[%d]", size);
  fprintf(file, ";\n");
}


plus_or_minus_operator(op, mode)
     rtx             op;
     enum machine_mode mode;
{
  return (GET_CODE(op) == PLUS || GET_CODE(op) == MINUS);
}

compute_binaryop(op, mode)
     rtx             op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case PLUS:
    case MINUS:
    case MULT:
    case AND:
    case IOR:
    case XOR:
    case SMAX:
    case SMIN:
    case LSHIFT:
    case ASHIFT:
    case ROTATE:
      return 1;
        
    default:
      return 0;
    }
}

compute_shiftop(op, mode)
     rtx             op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case LSHIFT:
    case ASHIFT:
    case ROTATE:
      return 1;
    default:
      return 0;
    }
}

compute_unaryop(op, mode)
     rtx             op;
     enum machine_mode mode;
{
  switch (GET_CODE(op))
    {
    case NOT:
    case ABS:
    case NEG:
      return 1;
    default:
      return 0;
    }
}

/*
  ; compute,  dm(Ia,Mb) = ureg          M = d
  ; compute,  dm(Mb,Ia) = ureg          T = d
  ; compute,  ureg = dm(Ia,Mb)          dwx = M
  ; compute,  ureg = dm(Mb,Ia)          dwx = T
  ;
  ; compute,  dm(Ia,<6bit>) = dreg      Q =
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
rtx             lhs;
rtx             rhs;
{
  rtx             mem,
  reg,
  op0,
  op1;
    
  if (REG_P(lhs) && REG_P(rhs))
    return 1;
  else if (GET_CODE(lhs) == MEM && REG_P(rhs))
    mem = XEXP(lhs, 0), reg = rhs;
  else if (REG_P(lhs) && GET_CODE(rhs) == MEM)
    mem = XEXP(rhs, 0), reg = lhs;
  else
    return 0;
    
  if (!IS_DREG(REGNO(reg)))
    return 0;
    
  switch (GET_CODE(mem))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
      return 1;
    case PLUS:
    case MINUS:
      op0 = XEXP(mem, 0), op1 = XEXP(mem, 1);
      if ((REG_P(op0) && IS_IREG(REGNO(op0)))
	  && ((REG_P(op1) && IS_MREG(REGNO(op1)))
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
  return 0;
}

int
  parallel_move_okay_for_shiftimm_p(lhs, rhs)
rtx             lhs;
rtx             rhs;
{
  rtx             mem,
  reg,
  op0,
  op1;
    
  if (REG_P(lhs) && REG_P(rhs))
    return 0;
  else if (GET_CODE(lhs) == MEM && REG_P(rhs))
    mem = XEXP(lhs, 0), reg = rhs;
  else if (REG_P(lhs) && GET_CODE(rhs) == MEM)
    mem = XEXP(rhs, 0), reg = lhs;
  else
    return 0;
    
  if (!IS_DREG(REGNO(reg)))
    return 0;
    
  switch (GET_CODE(mem))
    {
    case REG:
    case POST_INC:
    case POST_DEC:
      return 1;
    default:
      break;
    }
  return 0;
}
int
  const_double_ok_for_letter_p(value, letter)
rtx             value;
char            letter;
{
  int             retval = 0;
    
  switch (letter)
    {
    case 'G':
      switch (GET_CODE(value))
	{
	case CONST_DOUBLE:
	  retval = 1;
	  break;
	case CONST:
	  retval = (GET_MODE(value) == SFmode);
	}
      break;
    default:
      abort();                           /* we only should have been
					  * called with a 'G', but
					  * leave room for expansion */
    }
  return retval;
}

/* The letters I, J, K, L and M in a register constraint string
   can be used to stand for particular ranges of immediate operands.
   This macro defines what the ranges are.
   C is the letter, and VALUE is a constant value.
   Return 1 if VALUE is in the range specified by C.

   For the 21xx
   'I'  16-bit  immediate.
   'K'  14-bit immediate.
   'J'  1
   'N'  -1
   'O'  O
*/

int
  const_ok_for_letter_p(value, letter)
int             value;
char            letter;
{
  int             low,
  high = -1,
  result;
    
  switch (letter)
    {
    case 'I':
      high = 0x00ffff;
      break;
    case 'K':
      high = 0x003fff;
      break;
    case 'J':
      return value == 1;
    case 'N':
      return value == -1;
    case 'O':
      return value == 0;
    default:
      break;
    }
  low = -(high + 1);
  result = (low <= value) && (value <= high);
  return (result);
}


static int
  reloaded_reg_in_class(reg, class, rindex)
rtx             reg;
enum reg_class  class;
int             rindex[];
{
  if (rindex[0] >= 0 && reload_in_reg[rindex[0]] == reg)
    return reg_class_subset_p(reload_reg_class[rindex[0]], class);
  else if (rindex[1] >= 0 && reload_in_reg[rindex[1]] == reg)
    return reg_class_subset_p(reload_reg_class[rindex[1]], class);
  else
    {
      SWITCH_SUBREG(reg);
      return (REG_P(reg) && reg_class_subset_p(REGNO_REG_CLASS(REGNO(reg)), class));;
    }
}


extern int      somewhere_in_reload;             /* this is defined in
						  * reload1.c to be true
						  * during almost the entire
						  * reload pass */

/* Q,t is for immediate memory addresses */
static int
  extra_constraint_QT(addr)
rtx             addr;
{
  if (CONSTANT_P(addr))
    return 1;
  else if (GET_CODE(addr) == PLUS &&
	   CONSTANT_P(XEXP(addr, 0)) &&
	   CONSTANT_P(XEXP(addr, 1)))
    return 1;
  else
    return 0;
}

/* S,U are for dag2 post modify memory addresses */
static int
  extra_constraint_SU(addr, rindex)
rtx             addr;
int             rindex[];
{
  rtx             op;

  if(rindex[0]!=-1 && 
     rtx_equal_p(reload_in[rindex[0]],addr) &&
     reload_reg_class[rindex[0]]==DAG2_IREGS)
    return 1;
    
  switch (GET_CODE(addr))
    {
    case POST_MODIFY:
      if (!reloaded_reg_in_class(XEXP(addr, 0), DAG2_IREGS, rindex))
	return 0;
      op = XEXP(addr, 1);
      switch (GET_CODE(op))
	{
	case REG:
	case SUBREG:
	  return reloaded_reg_in_class(op, DAG2_MREGS, rindex);
	case CONST_INT:

	  if (INTVAL(op) == 0)
	      return 1;
#ifdef PM_MINUS1_MREG
	  if (INTVAL(op) == -1)
	      return 1;
#endif
#ifdef PM_PLUS1_MREG
	  if (INTVAL(op) == 1)
	      return 1;
#endif
	  return 0;
	default:
	  return 0;
	}
      break;
#ifdef PM_MINUS1_MREG
    case POST_INC:
    case POST_DEC:
      return reloaded_reg_in_class(XEXP(addr, 0), DAG2_IREGS, rindex);
#endif
    case SUBREG:
    case REG:
      return reloaded_reg_in_class(addr, DAG2_IREGS, rindex);
    default:
      return 0;
    }
}

/* R is for dag1 post modify memory addresses */
static int
  extra_constraint_R(addr, rindex)
rtx             addr;
int             rindex[];
{
  rtx             op;

  if(rindex[0]!=-1 && 
     rtx_equal_p(reload_in[rindex[0]],addr) &&
     reload_reg_class[rindex[0]]==DAG1_IREGS)
    return 1;
    
  switch (GET_CODE(addr))
    {
    case POST_MODIFY:
      if (!reloaded_reg_in_class(XEXP(addr, 0), DAG1_IREGS, rindex))
	return 0;
      op = XEXP(addr, 1);
      switch (GET_CODE(op))
	{
	case REG:
	case SUBREG:
	  return reloaded_reg_in_class(op, DAG1_MREGS, rindex);
	case CONST_INT:
	  if (INTVAL(op) == 0)
	      return 1;
#ifdef DM_MINUS1_MREG
	  if (INTVAL(op) == -1)
	      return 1;
#endif
	  return 0;
	default:
	  return 0;
	}
#if DM_MINUS1_MREG
    case POST_DEC:
      return reloaded_reg_in_class(XEXP(addr, 0), DAG1_IREGS, rindex);
#endif
    case SUBREG:
    case REG:
      return reloaded_reg_in_class(addr, DAG1_IREGS, rindex);
    default:
      return 0;
    }
}

/*
  Q     dm(imm)
  R     dm(dag1i,m)
  S     dm(dag2i,m)
  T     pm(imm)  ( <-- this is a valid addressing mode only for calls/jumps )
  U     pm(dag2i,m)
  */
int
  extra_constraint(op, c)
rtx             op;
char            c;
{
    
  rtx             addr,
  op0,
  op1;
  enum machine_mode addr_mode;
  int             i,
  rindex[2] = {-1, -1};  /** reload index **/
    
    
  if (GET_CODE(op) != MEM)
    return 0;
  addr = XEXP(op, 0);
  addr_mode = GET_MODE(addr);
  switch (c)
    {
    case 'Q':
    case 'R':
    case 'S':
      if (addr_mode != DMmode)
	return 0;
      break;
    case 'T':
    case 'U':
      if (addr_mode != PMmode)
	return 0;
      break;
    }
    
  if (somewhere_in_reload)
    for (i = 0; i < n_reloads && rindex[1] == -1; i++)
      if (reload_needed_for[i] == op &&
	  !reload_optional[i] &&
	  !reload_secondary_p[i])
	if (rindex[0] == -1)
	  rindex[0] = i;
	else
	  rindex[1] = i;
    
  if (rindex[0] < 0)
    {
      GO_IF_LEGITIMATE_ADDRESS(addr_mode, addr, ok_so_far);
      return 0;
    }
    
 ok_so_far:
    
  switch (c)
    {
    case 'Q':
    case 'T':
      return extra_constraint_QT(addr);
    case 'R':
      return extra_constraint_R(addr, rindex);
    case 'S':
    case 'U':
      return extra_constraint_SU(addr, rindex);
    default:
      return 0;
    }
}


/*** return true if both op1 and op2 are two differrent register operands **/

int
  different_registers(op1, op2)
rtx             op1,
  op2;
{
  int             op1_reg_p,
  op2_reg_p,
  result;
    
  op1_reg_p = GET_CODE(op1);
  op2_reg_p = GET_CODE(op2);
  result = (op1_reg_p && op2_reg_p) &&
    (REGNO(op1) != REGNO(op2));
  return result;
}


/** Note that constant address is allowed only in DMmode) **/

int
  go_if_legitimate_address_strict(mode, x)
enum machine_mode mode;
rtx             x;
{
  int ok = 0;
  enum machine_mode addr_mode = GET_MODE(x);
    
  if ((mode == SFmode || mode == DImode) &&
      (addr_mode == DMmode))
  {
      ok = CONSTANT_ADDRESS_P(x);
  }
  else
  {
      if ((REG_P(x) && REG_OK_FOR_BASE_STRICT_P(x)) ||
	  ((addr_mode == DMmode) && CONSTANT_ADDRESS_P(x)))
	  ok = 1;
      else if (POST_MODIFY_P(x)
	       && REG_P(XEXP(x, 0))
	       && REG_OK_FOR_BASE_STRICT_P(XEXP(x, 0))
	       && REG_OK_FOR_INDEX_STRICT_P(XEXP(x,0)))
	  ok = 1;
  }
  return ok;
}

int
  go_if_legitimate_address_non_strict(mode, x)
enum machine_mode mode;
rtx             x;
{
  int ok = 0;
  enum machine_mode addr_mode = GET_MODE(x);
    
      if ((REG_P(x) && REG_OK_FOR_BASE_NON_STRICT_P(x) &&
	   (x != virtual_stack_vars_rtx)) ||
	  ((addr_mode != PMmode) && (CONSTANT_ADDRESS_P(x))))
	ok = 1;
      else if (POST_MODIFY_P(x))
      {
	  rtx base, modifier;
	  int base_ok, modifier_ok;

	  base = XEXP(x,0);
	  SWITCH_SUBREG(base);
	  base_ok = 
	      REG_P(base) && REG_OK_FOR_BASE_NON_STRICT_P(base);
	  if (! base_ok) 
	      return 0;
	  modifier = XEXP(x,1);
	  SWITCH_SUBREG(modifier);
	  modifier_ok =
	      REG_P(modifier) && REG_OK_FOR_MODIFY_NON_STRICT_P(modifier);
	  if (! modifier_ok)
	      return 0;
	  return 1;
      }
	{
#if 0
      if (dag1_i_operand(x, mode))
	ok = 1;
      else if (GET_CODE(x) == POST_DEC
	       && REG_P(XEXP(x, 0))
	       && REG_OK_FOR_BASE_NON_STRICT_P(XEXP(x, 0)))
	ok = 1;
#endif
    }
  return ok;
}



copy_double_reg_to_memory(operands)
     rtx             operands[];
{
  rtx             xoperands[4],
  plus;
    
  xoperands[0] = XEXP(operands[0], 0);
  xoperands[1] = operands[1];
    
  switch (GET_CODE(xoperands[0]))
    {
    case POST_INC:
      output_asm_insn("dm(%0,m6)=u1; dm(%0,m6)=%b1;", xoperands);
      break;
    case POST_DEC:
      output_asm_insn("dm(%0,m6)=%u1; dm(%0,-3)=%b1;", xoperands);
      break;
    case PLUS:
      if (GET_CODE(XEXP(xoperands[0], 1)) == CONST_INT)
	{
	  plus = xoperands[0];
	  xoperands[0] = XEXP(plus, 0);  /* iregister */
	  xoperands[1] = XEXP(plus, 1);  /* const-int */
	  xoperands[2] = gen_rtx(CONST_INT, SImode, INTVAL(xoperands[1]) + 1);
	  xoperands[3] = operands[1];
	  output_asm_insn("dm(%1,%0)=%u3; dm(%2,%0)=%b3;", xoperands);
	}
      break;
        
    default:
      output_asm_insn("dm(%0,m6)=%u1; dm(%0,m7)=%b1;", xoperands);
      break;
    }
}


copy_double_memory_to_reg(operands)
     rtx             operands[];
{
  rtx             xoperands[4],
  plus;
    
  xoperands[0] = operands[0];
  xoperands[1] = XEXP(operands[1], 0);
    
  switch (GET_CODE(xoperands[1]))
    {
    case POST_INC:
      output_asm_insn("%u0=dm(%1,m6); %b0=dm(%1,m6);", xoperands);
      break;
    case POST_DEC:
      output_asm_insn("%u0=dm(%1,m6); %b0=dm(%1,-3);", xoperands);
      break;
    case PLUS:
      if (GET_CODE(XEXP(xoperands[1], 1)) == CONST_INT)
	{
	  plus = xoperands[1];
	  xoperands[1] = XEXP(plus, 0);  /* iregister */
	  xoperands[2] = XEXP(plus, 1);  /* const-int */
	  xoperands[3] = gen_rtx(CONST_INT, SImode, INTVAL(xoperands[2]) + 1);
	  output_asm_insn("%u0=dm(%2,%1); %b0=dm(%3,%1);", xoperands);
	}
      break;
    default:
      output_asm_insn("%u0=dm(%1,m6); %b0=dm(%1,m7);", xoperands);
      break;
    }
}


int
  is_constant_1p(x, insn)
rtx             x;
rtx             insn;
{
  rtx             l;
    
  if (REG_P(x))
    {
      l = reg_set_last(x, insn);
      if (l && GET_CODE(l) == CONST_INT && INTVAL(l) == 1)
	return 1;
      else
	return 0;
    }
  else if (GET_CODE(x) == CONST_INT && INTVAL(x) == 1)
    return 1;
  return 0;
}

int
  hard_regno_mode_ok(regno, mode)
int             regno;
enum machine_mode mode;
{
  int             ok = 0;
    
  switch (mode)
    {
    case PMmode:
    case DMmode:
      ok = TEST_HARD_REG_BIT(reg_class_contents[GENERAL_REGS], regno);
      break;
    case SImode:
      ok = TEST_HARD_REG_BIT(reg_class_contents[CNTR_D_REGS], regno);
      break;
    case DImode:
      ok = (regno == REG_SR0 || regno == REG_MR0);
      break;
    case SFmode:
      ok = (regno == REG_AX0 || regno == REG_AY0 || regno == REG_SR0 || regno == REG_MR0);
      break;
    default:
      break;
    }
  return ok;
}

int
  legitimize_address(x, mode)
rtx            *x;
enum machine_mode mode;
{
  rtx             new1,
  new2;
    
  return 0;
}



int
  class_max_nregs(class, mode)
{
  if(reg_class_subset_p(class,DAG_REGS))
    return 1;
  else
      return HARD_REGNO_NREGS(REG_AX0, mode);
}

enum reg_class
preferred_output_reload_class(x, class)
    rtx x;
    enum reg_class class;
{
    enum reg_class result = 
	reg_class_intersection (class, secondary_output_reload_class(class, GET_MODE(x), x));
      
    return result == NO_REGS ? class : result;
}

/*(doc "Register Classes" "`PREFERRED_RELOAD_CLASS (X, CLASS)'")*/
enum reg_class
preferred_reload_class(x,class)
    rtx x;
    enum reg_class class;
{
    enum reg_class result = 
      reg_class_intersection (class, secondary_input_reload_class(class, GET_MODE(x), x));
      
    return result == NO_REGS ? class : result;
}


enum reg_class
secondary_output_reload_class(class,mode,x)
     enum reg_class class;
     enum machine_mode mode;
     rtx x;
{
  return secondary_input_reload_class(class,mode,x);
}



/* Return the register class of a scratch register needed to copy IN into
   or out of a register in CLASS in MODE.  If it can be done directly,
   NO_REGS is returned.  */

/* Strategy as follows:

   For single word modes:
   1.  If we're transferring to a constant pm address,
       we're in trouble, legitimize address should've said impossible.
   2.  If we're transferring to a non-constant address,
       if we haven't got a dreg, we'll need one.

   For double word modes:
   1.  If we're transferring to a constant dm address,
       again, we'll need it to be dregs.
   2.  If we're transferring to a direct dm address, we'll
       need to see which dag we've got.  We're going to need
       both a +1 and -1 m register.  In dag1, we've got them:
       all set.  In dag2, though, we're going to need a +1 mregister.
       Also, if we don't have a dreg, we'll need one.
   3.  If we're transferring to a non-direct dm address,
       punt.

       (CLASS,MODE) <- X  */
      

enum reg_class 
secondary_input_reload_class(class,mode,x)
    enum reg_class class;
    enum machine_mode mode;
rtx x;
{
    enum reg_class result = NO_REGS;
      
    SWITCH_SUBREG(x);
      
    switch(mode)
    {
      case SImode:
      case DMmode:
      case PMmode:
	switch(GET_CODE(x))
	{
	  case MEM:
	    switch(GET_MODE(XEXP(x,0)))
	    {
	      case PMmode:
		if(CONSTANT_ADDRESS_P(XEXP(x,0)))
		    abort(); /* we've got trouble -- shouldn't be transferring to constant adress */
		goto is_dm;
	      case DMmode:
	      is_dm:
		if(!reg_class_subset_p(class,D_REGS))
		    return D_REGS;
		return NO_REGS;
		break;
	      default:
		abort(); /* we only hve pm and dm memory modes */
	    }
	    break;
	  case REG:
	    /* Need Dreg because pseudos are usually in memory */
	    if(REGNO(x) >= FIRST_PSEUDO_REGISTER)
	    {
		GO_IF_HARD_REG_SUBSET (reg_class_contents [class],
				       reg_class_contents [D_REGS],
				       noregs);
		return D_REGS;
	      noregs: 
		return NO_REGS;
	    }
	    else 
		return NO_REGS; /* can go any register to any other register, right? */
	    break;
	  case PLUS:
	    /* general strategy: 
	       if have (plus (x) (y)), and goal is an i reg
	       if either is an m reg of the right dag, then don't need anything
	       otherwise, get an mreg of the right dag.
	       */
	    if(reg_class_subset_p(class,DAG1_IREGS))
		if((REG_P(XEXP(x,0)) && IS_DAG1_MREG(REGNO(XEXP(x,0)))) ||
		   (REG_P(XEXP(x,1)) && IS_DAG1_MREG(REGNO(XEXP(x,1)))))
		    return NO_REGS;
		else
		    return DAG1_MREGS;
	    else if(reg_class_subset_p(class,DAG2_IREGS))
		if((REG_P(XEXP(x,0)) && IS_DAG2_MREG(REGNO(XEXP(x,0)))) ||
		   (REG_P(XEXP(x,1)) && IS_DAG2_MREG(REGNO(XEXP(x,1)))))
		    return NO_REGS;
		else
		    return DAG2_MREGS;
	    else
	      if(CONSTANT_P(XEXP(x,0)) && CONSTANT_P(XEXP(x,1)))
		return NO_REGS;
	    else
	      abort();
	  case CONST:
	  case CONST_INT:
	  case SYMBOL_REF:
	  case LABEL_REF:
	    return NO_REGS;
	  default:
	    abort();
	}
	break;
      case DImode:
      case SFmode:
	switch(GET_CODE(x))
	{
	  case REG:
	  {
	      rtx temp=gen_rtx(REG,SImode,REGNO(x));
	      return secondary_input_reload_class(class,SImode,temp);
	  }
	case MEM:
	{
	    rtx addr = XEXP(x,0);
	    switch (GET_CODE(addr))
	    {
	      case PLUS:
		if (GET_CODE(XEXP(addr, 0))==REG &&
		    GET_CODE(XEXP(addr, 1))==CONST_INT)
		{
		    if(!reg_class_subset_p(class,D_REGS))
			return D_REGS;
		    else 
			return NO_REGS;
		}
		break;
	      case POST_MODIFY:
	      {
		  rtx
		      base   = XEXP(addr, 0),
		      offset = XEXP(addr, 1);
		  int same_dag, base_rno, offset_rno;
		  if (GET_CODE(base) != REG)               
		      return D_REGS;
		  if (!REG_OK_FOR_BASE_STRICT_P(base))     
		      return D_REGS;
		  if (GET_CODE(offset) != REG)             
		      return D_REGS;
		  if (!REG_OK_FOR_MODIFY_STRICT_P(offset)) 
		      return D_REGS;
		  base_rno   = REGNO(base);
		  offset_rno = REGNO(offset);
		  same_dag = 
		      ((IS_DAG1_IREG(base_rno) && IS_DAG1_MREG(offset_rno)) ||
		       (IS_DAG2_IREG(base_rno) && IS_DAG2_MREG(offset_rno)));
		  if (same_dag) return NO_REGS;
		  else          return D_REGS;
	      }
		break;
	      case REG:
		if (REG_OK_FOR_BASE_STRICT_P(addr))
		    if(!reg_class_subset_p(class,D_REGS))
			return D_REGS;
		    else 
			return NO_REGS;
		else 
		    return D_REGS;
		break;
	      case SYMBOL_REF:
		return NO_REGS;
	      default:
		abort();
	    }
	}
	  break;
	default:
	  if(!CONSTANT_P(x))
	      abort(); /* gotta have mode ! */
      }
	return NO_REGS;
    }
}

int use_secondary_reload_patterns=0;


dm_incoming_reload(op, mode)
     rtx             op;
     enum machine_mode mode;
{
    /* match (mem (plus (reg) (const))) */
    if (! use_secondary_reload_patterns)
	return 0;
      
    if (GET_CODE(op) == MEM &&
	GET_CODE(XEXP(op, 0)) == PLUS &&
	REG_P(XEXP(XEXP(op, 0), 0)) &&
	CONSTANT_P(XEXP(XEXP(op, 0), 1)))
	return 0;
    if (GET_CODE(op) == PLUS &&
	REG_P(XEXP(op, 0)))
	return 1;
      
}

pm_incoming_reload(op, mode)
     rtx op;
     enum machine_mode mode;
{
    if (! use_secondary_reload_patterns)
	return 0;
    return dm_incoming_reload(op, mode);
}

dm_outcoming_reload(op, mode)
     rtx op;
     enum machine_mode mode;
{
    if (! use_secondary_reload_patterns)
	return 0;
    return dm_incoming_reload(op, mode);
}

pm_outcoming_reload(op, mode)
     rtx op;
     enum machine_mode mode;
{
    if (! use_secondary_reload_patterns)
	return 0;
    return dm_incoming_reload(op, mode);
}

int
  always_true(op, mode)
register rtx    op;
enum machine_mode mode;

{
  return (1);
}

/*
  Reload FROM which is a frame reference, to TO: if FROM = @(FP,A),
  then emit:
  A --> scratch_reg (which is dag2 I-reg)
  scratch_reg + FP ---> scratch_reg
  @(scratch_reg) --> TO
*/


void sec_reload_in (reg, from, scratch_reg, mode)
    rtx reg, from, scratch_reg;
    enum machine_mode mode;
{
    
    RTX_CODE code = GET_CODE(from);
    int regno;
    switch(code) {
      case REG:
	regno = REGNO(from);
	if (! IS_PSEUDO(regno))
	    abort();
	from = reg_equiv_memory_loc[regno];
	if (! from)
	    abort();

	/* otherwise fall through to MEM: */

      case MEM:
	/* First do a lot of checking, since for now it is designed for */
	/* quite limited case */
	if (! REG_P(reg))
	    abort();
      {
	  rtx 
	      addr = XEXP(from,0),
	      base, offset;
	  if (GET_CODE(addr) != PLUS)
	      abort();
	  base   = XEXP(addr, 0);
	  offset = XEXP(addr, 1);
	  if (! REG_P(base))
	      abort();
	  if (REGNO(base) != FRAME_POINTER_REGNUM)
	      abort();
	    
	  emit_insn (gen_rtx (SET, PMmode, scratch_reg, offset));
	  emit_insn (gen_rtx (SET, PMmode,
			      scratch_reg,
			      gen_rtx(PLUS, PMmode, 
				      scratch_reg, base)));
	  emit_insn (gen_rtx (SET, mode, reg,
			      (MEM, mode, scratch_reg)));
      }

	break;

      case PLUS:
	abort();
	if(REG_P(reg) &&
	   IS_IREG(REGNO(reg)) &&
	   REG_P(XEXP(from,0)) && 
	   IS_MREG(REGNO(XEXP(from,0))))
	{
	    emit_insn(gen_rtx(SET,mode, reg, XEXP(from,1)));
	    emit_insn(gen_rtx(SET,mode, reg,
			      gen_rtx(PLUS, mode, reg, XEXP(from,0))));
	}
	else
	    abort();
      default:
	abort();
    }
}


/** TO -- rtx where the reload happens to **/
/** REG -- register of primary reload class **/
/** SCRATCH_REG -- register of secondary reload class */

void sec_reload_out (x, scratch_reg, interm_reg, mode)
    rtx x, interm_reg, scratch_reg;
    enum machine_mode mode;
{
    
    RTX_CODE code = GET_CODE(x);
    int regno;
    rtx eqx = x;

    switch(code) {
      case REG:
	regno = REGNO(x);
	if (! IS_PSEUDO(regno))
	    abort();
	eqx = reg_equiv_memory_loc[regno];
	if (! eqx)
	    abort();

	/* otherwise fall through to MEM: */

      case MEM:
      {
	  rtx 
	      addr = XEXP(eqx, 0),
	      base, offset;
	  if (GET_CODE(addr) != PLUS)
	      abort();
	  base   = XEXP(addr, 0);
	  offset = XEXP(addr, 1);
	  if (! REG_P(base))
	      abort();
	  if (REGNO(base) != FRAME_POINTER_REGNUM)
	      abort();
	  emit_insn (gen_rtx (SET, PMmode, scratch_reg, offset));
	  emit_insn (gen_rtx (SET, PMmode, scratch_reg,
			      gen_rtx(PLUS, PMmode, scratch_reg, base)));
	  emit_insn (gen_rtx (SET, mode, (MEM, mode, scratch_reg), 
			      interm_reg));
      }
	break;

      case PLUS:

	abort();
	if(REG_P(interm_reg) &&
	   IS_IREG(REGNO(interm_reg)) &&
	   REG_P(XEXP(x,0)) && 
	   IS_MREG(REGNO(XEXP(x,0))))
	{
	    emit_insn(gen_rtx(SET,mode, interm_reg, XEXP(x,1)));
	    emit_insn(gen_rtx(SET,mode, interm_reg,
			      gen_rtx(PLUS, mode, interm_reg, XEXP(x,0))));
	}
	else
	    abort();
      default:
	abort();
    }
}


#ifdef OLD_21XX_FLOAT

/* I got this idea from the spur machine description.
   Basically, we've got a real tough time moving these two
   word operands around
   */

char           *
  output_move_double(operands)
rtx            *operands;
{
  enum
    {
      ERROR,
      REG_REG,                           /* register to register */
      REG_MEM,                           /* register to memory */
      MEM_REG,                           /* memory to register */
      REG_DEC,                           /* register to memory, post
					  * decrement */
      DEC_REG,                           /* memory (post decrement)
					  * to register */
      IMM_REG,                           /* immediate to register */
      IMM_MEM,                           /* immediate to memory */
    }               type = ERROR;                /* there may be more! */
  rtx             addr = 0;
  rtx             regs[4];
    
  if (REG_P(operands[0]))                        /* what is the destination?
						  * register */
    {
      if (REG_P(operands[1]))
	{
	  type = REG_REG;
	}
      else if (GET_CODE(operands[1]) == MEM)
	{
	  type = MEM_REG;
	  addr = XEXP(operands[1], 0);
	}
      else if (CONSTANT_P(operands[1]))
	{
	  type = IMM_REG;
	}
    }
  else if (GET_CODE(operands[0]) == MEM)
    {
      if (REG_P(operands[1]))
	{
	  type = REG_MEM;
	  addr = XEXP(operands[0], 0);
	}
      else if (CONSTANT_P(operands[1]))
	{
	  type = IMM_MEM;
	  addr = XEXP(operands[0], 0);
	}
        
      else
	{
	  abort();                               /* Can only transfer a
						  * register to memory */
	}
    }
  else
    abort();                             /* Can only transfer to
					  * memory or a register */
    
    
  if (addr)
    switch (GET_CODE(addr))
      {
      case REG:
	break;
      case POST_DEC:
	if (type == REG_MEM)
	  {
	    type = REG_DEC;
	    addr = XEXP(addr, 0);
	  }
	else if (type == MEM_REG)
	  {
	    addr = XEXP(addr, 0);
	    type = DEC_REG;
	  }
	else
	  abort();
	break;
      default:
	abort();
      }
    
    
  switch (type)
    {
    case REG_REG:
      regs[0] = gen_rtx(SUBREG, SImode, operands[0], 0);
      regs[1] = gen_rtx(SUBREG, SImode, operands[1], 0);
      regs[2] = gen_rtx(SUBREG, SImode, operands[0], 1);
      regs[3] = gen_rtx(SUBREG, SImode, operands[1], 1);
      break;
    case REG_MEM:
      regs[0] = gen_rtx(MEM, SImode, gen_rtx(POST_INC, DMmode, addr));
      regs[1] = gen_rtx(SUBREG, SImode, operands[1], 0);
      regs[2] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      regs[3] = gen_rtx(SUBREG, SImode, operands[1], 1);
      break;
    case REG_DEC:
      /*
	This code is commented out because it isn't appropriate for when
	doing something like
	
	(set (mem (post_dec (reg))) (reg))
	
	since that is the push insn
	
	
	regs[0] = addr;
	regs[1] = CONST1_RTX(DMmode);
	if (IS_DAG1(REGNO(addr)))
	output_asm_insn("modify(%0,%k1);", regs);
	else
	output_asm_insn("modify(%0,%K1);", regs);
	*/
      regs[0] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      regs[1] = gen_rtx(SUBREG, SImode, operands[1], 1);
      regs[2] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      regs[3] = gen_rtx(SUBREG, SImode, operands[1], 0);
      break;
    case DEC_REG:
      regs[0] = addr;
      regs[1] = CONST1_RTX(DMmode);
      if (IS_DAG1(REGNO(addr)))
	output_asm_insn("modify(%0,%k1);", regs);
      else
	output_asm_insn("modify(%0,%K1);", regs);
      regs[0] = gen_rtx(SUBREG, SImode, operands[1], 1);
      regs[1] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      regs[2] = gen_rtx(SUBREG, SImode, operands[1], 0);
      regs[3] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      break;
    case MEM_REG:
      regs[0] = gen_rtx(SUBREG, SImode, operands[0], 0);
      regs[1] = gen_rtx(MEM, SImode, gen_rtx(POST_INC, DMmode, addr));
      regs[2] = gen_rtx(SUBREG, SImode, operands[0], 1);
      regs[3] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
      break;
    case IMM_MEM:
    case IMM_REG:
      if (GET_MODE(operands[1]) == VOIDmode)     /* in which case immediate
						  * is just a large integer
						  * constant */
	{
	  abort();
	}
      else
	{
	  if (CONST_DOUBLE_MEM(operands[1]) == const0_rtx)
	    {
	        
	      /*
	       * not allocated a memory location, on chain of const_double
	       * expressions
	       */
	      regs[1] = gen_rtx(CONST_INT, SImode, XEXP(operands[1], 2));
	      regs[3] = gen_rtx(CONST_INT, SImode, XEXP(operands[1], 3));
	    }
	  else if (CONST_DOUBLE_MEM(operands[1]) == cc0_rtx)    /* not even on chain!
								 * (where is it?) */
	    {
	      regs[1] = gen_rtx(CONST_INT, SImode, XEXP(operands[1], 2));
	      regs[3] = gen_rtx(CONST_INT, SImode, XEXP(operands[1], 3));
	    }
	  else
	    abort();
	}
      if (type == IMM_REG)
	{
	  regs[0] = gen_rtx(SUBREG, SImode, operands[0], 0);
	  regs[2] = gen_rtx(SUBREG, SImode, operands[0], 1);
	  break;
	}
      else if (type == IMM_MEM)
	{
	  regs[0] = gen_rtx(MEM, SImode, gen_rtx(POST_INC, DMmode, addr));
	  regs[2] = gen_rtx(MEM, SImode, gen_rtx(POST_DEC, DMmode, addr));
	  break;
	}
    default:
      abort();                           /* impossible to reach ! */
    }
  output_asm_insn("%0=%1;%2=%3; ! moving a double reg operand", regs);
}

#endif

char *
output_move_double_to_reg(reg, x)
	rtx reg, x;
{
    abort();
}

char *
output_move_double_from_reg(reg, x)
    rtx reg, x;
{
    abort();
}

char *
output_move_double(operands)
    rtx operands[];
{
    if (REG_P(operands[0]))
	return output_move_double_to_reg(operands[0], operands[1]);
    else if (REG_P(operands[1]))
	return output_move_double_to_reg(operands[1], operands[0]);
    else
	abort();
}


char           *
  output_move_float(operands)
rtx            *operands;

{
  return output_move_double(operands);   /* atleast so far, the way
					  * we move floats is the
					  * same way we move any two
					  * word registers */
}
char           *
  output_push_float(operands)
rtx            *operands;

{
  rtx             regs[3];
    
  regs[0] = operands[0];
  switch (GET_CODE(operands[1]))
    {
    case REG:
      regs[1] = gen_rtx(SUBREG, SImode, operands[1], 1);
      regs[2] = gen_rtx(SUBREG, SImode, operands[1], 0);
      break;
    case CONST_DOUBLE:
      regs[1] = gen_rtx(CONST_INT, SImode,
			operands[1]->fld[2].rtint);     /* I'm sure there's a
							 * better way, but, aw
							 * hell, I guess I'm
							 * lazy.
							 * 
							 * Also, there's probably a
							 * macro for this
							 * somewhere, too.
							 * 
							 * Actually this will have
							 * to be redone when we
							 * figure out how to
							 * really output floats! */
      regs[2] = gen_rtx(CONST_INT, SImode, operands[1]->fld[1].rtint);
      break;
    default:
      abort();
    }
    
  output_asm_insn("%s0(%S0,%M0)=%1; %s0(%S0,%M0)=%2; ! pushing floats", regs);
}

int
  sf_reload_operand(op, mode)
rtx             op;
enum machine_mode mode;

{
  return 1;
}

int
  reg_in_class(class, regno)
enum reg_class  class;
int             regno;
{
  return TEST_HARD_REG_BIT(reg_class_contents[class], regno);
}



does_function_have_polymorphic_property()
{
  return 0;
}

enum machine_mode
  extract_segment_pmode(x)
memory_segment  x;
{
  switch (x)
    {
    case PROG_MEM:
      return PMmode;
    case DATA_MEM:
      return DMmode;
    default:
      abort();
    }
}


enum machine_mode extract_tree_pmode (tr)
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
    default:
      return extract_tree_pmode(TREE_TYPE(tr));
      abort();
    }
}

enum machine_mode extract_rtx_pmode(x)
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
  extract_mode_pmode(x, mode)
rtx             x;
enum machine_mode mode;

{
    
  if (GET_MODE(x) == VOIDmode)
    return mode;
  else
    return GET_MODE(x);
    
}

int regno_ok_for_base_p(int regno, enum machine_mode mode)
{
  if(reg_renumber[regno]!=-1)
    regno=reg_renumber[regno];
  if(mode==DMmode)
    return IS_DM_IREG(regno);
  else if(mode==PMmode)
    return IS_PM_IREG(regno);
  else
    abort();
}
int regno_ok_for_modify_p(int regno, enum machine_mode mode)
{
  if(reg_renumber[regno]!=-1)
    regno=reg_renumber[regno];
  if(mode==DMmode)
    return IS_DM_MREG(regno);
  else if(mode==PMmode)
    return IS_PM_MREG(regno);
  else
    abort();
}

/*
  Expand a double word move  (SFmode and DImode) into a set of single
  word moves. We expect that at least one of the operands is a register
*/


expand_double_move(to, from)
    rtx to, from;
{
    rtx to0, to1, from0, from1;
    int reverse=0;
    static expand_move_double_mem_to_reg(rtx, rtx, rtx);

    get_singles(to,   &to0,   &to1,   &reverse);
    get_singles(from, &from0, &from1, &reverse);
      
    if (GET_CODE(to0) == SUBREG)
	to0   = gen_rtx (STRICT_LOW_PART, SImode, to0);

    if (reverse) 
    {
	emit_move_insn(to0, from0);
	emit_move_insn(to1, from1);
    }
    else   
    {
	emit_move_insn(to1, from1);
	emit_move_insn(to0, from0);
    }
}

/*
  Take "big" (DI,SF) mode double_rtx, get to SImode rtx's
  for its components and put then in srtx0p and srtx1p.
*/

get_singles(double_rtx, srtx0p, srtx1p, revp)
    rtx double_rtx, *srtx0p, *srtx1p, *revp;
{
     switch (GET_CODE(double_rtx))
    {
      case REG:
	*srtx0p = gen_rtx (SUBREG, SImode, double_rtx, 0);
	*srtx1p = gen_rtx (SUBREG, SImode, double_rtx, 1);
	break;
      case CONST_DOUBLE:
	double_const_to_singles(double_rtx, srtx0p, srtx1p);
	break;
      case MEM:
	get_single_mems(double_rtx, srtx0p, srtx1p, &revp);
	break;
      case CONST_INT:
	*srtx0p = double_rtx;
	*srtx1p = const0_rtx;
	break;
      case SUBREG:
	/* Granted, this looks pretty weird,
	   but if the source contains something like:
		   union { float f; int i; } u1;
	   then we'll end up generating (subreg:SF (reg:DI) 0)
	   and we need to handle it somehow, so...*/
	get_singles(SUBREG_REG(double_rtx), srtx0p, srtx1p, revp);
	break;
      default:
	abort();
    }
}

/*
  Eventualy  we will be forced to do something more sophisticated,
  but let's see how far it gets us. (az, 3/3/93)
*/

get_single_mems(double_rtx, srtx0p, srtx1p, revp)
    rtx double_rtx, *srtx0p, *srtx1p;
    int *revp;
{
    rtx
	addr = XEXP(double_rtx, 0);

    switch (GET_CODE(addr))
    {
      case REG:
	if (reload_in_progress || reload_completed)
	    abort();
      {
	  enum machine_mode mode = Extract_rtx_Pmode(addr);
	  rtx
	      temp=gen_reg_rtx (mode),
	      addr0, addr1;
	  emit_insn (gen_rtx (SET,mode,temp,const1_rtx));
	  addr0 = gen_rtx (POST_MODIFY, mode, addr, temp);
	  *srtx0p = gen_rtx (MEM, SImode, addr0);
	  *srtx1p = gen_rtx (MEM, SImode, addr);
      }
	break;
      case POST_MODIFY:
      {
	  rtx
	      base     = XEXP(addr, 0), 
	      modifier = XEXP(addr, 1);
	  *revp = mod_reg_equiv_const[REGNO(modifier)] == -1;
	  *srtx0p = *srtx1p = gen_rtx(MEM, SImode, addr);
      }
	break;
      case SYMBOL_REF:
      case CONST:
	*srtx0p = gen_rtx (MEM, SImode, addr);
	*srtx1p = gen_rtx (MEM, SImode, plus_constant(addr, 1));
	break;
      case PLUS:
      {
	  rtx
	      base   = XEXP(addr, 0),
	      offset = XEXP(addr, 1);
	  if (!CONSTANT_P(base) || ! CONSTANT_P(offset))
	      abort();
	  *srtx0p = gen_rtx(MEM, SImode, addr);
	  *srtx1p = gen_rtx(MEM, SImode, plus_constant(addr, 1));
      }
	break;
      default:
	abort();
    }
}

#include "math.h"

void double_const_to_singles(double_rtx, srtx0p, srtx1p)
    rtx double_rtx, *srtx0p, *srtx1p;    
{
    REAL_VALUE_TYPE value;
    double fr;
    int exp;
    unsigned int high_int, low_int;
    unsigned int fr_int, sign;

    REAL_VALUE_FROM_CONST_DOUBLE(value, double_rtx);
      
    /* checkout X3.159-1989 (X3J11) section 4.5.4.2 */
    fr = frexp (value, &exp);
    exp--;
    /* quantize the mantissa */
    fr = ldexp (fr,24);
#ifdef __GNUDOS__
    modf(fr+.5*((fr<0)?-1:1),&fr);
#else
    modf(fr+copysign(0.5,fr),&fr);
#endif
    
    /* Check the exponent range) */
    if (exp>126 || exp < -127)
	error("float constant out of range");
    fr_int = fr;
    /* leave only 23 bits, get rid of the leading 1 */
    fr_int <<= 9;
    sign = 0;
    if (fr < 0) 
	sign = 0x8000;
    high_int = sign | ((exp + 127) << 7) | (fr_int >> 24);
    low_int =  (fr_int>>8) & 0xffff;
    *srtx0p = gen_rtx(CONST_INT, VOIDmode, high_int);
    *srtx1p = gen_rtx(CONST_INT, VOIDmode, low_int);
}
/* 
  This function returns okay if every register in class "class" okay
  for mode "mode".  Otherwise, it returns 0.
*/
int class_ok_for_mode(class, mode)
enum reg_class class;
enum machine_mode mode;
{
  int regno, okay=1;
  if(mode==VOIDmode)
    return okay;
  for(regno=0;regno < FIRST_PSEUDO_REGISTER && okay; regno++)
    if(TEST_HARD_REG_BIT(reg_class_contents[class],regno) &&
       !hard_regno_mode_ok(regno,mode))
      okay=0;
  return okay;
}
    
int
call_operand (op, mode)
    rtx op;
    enum machine_mode mode;
{
    rtx addr;
    if (mode != PMmode) 
	return 0;
    if (GET_MODE(op) != PMmode)
	return 0;
    if (GET_CODE(op) != SYMBOL_REF)
	return 0;
    return 1;
}

/*EK* new... */
int parallel_2_p(i)
    rtx i[];
{
    enum attr_type
	t1 = get_attr_type(i[0]),
	t2 = get_attr_type(i[1]);
    int is_comp_1, is_move_2;

    is_comp_1 = (t1 == TYPE_ALU || t1 == TYPE_MAC || t1 == TYPE_SHIFT);
    if (! is_comp_1) 
	return 0;
    is_move_2 = (t2 == TYPE_DMREAD || t2 == TYPE_PMREAD || 
		 t2 == TYPE_MOVE);
    if (! is_move_2) 
	return 0;
    return 1;

}


/*EK* new... */
int parallel_3_p(i)
    rtx i[];
{
    enum attr_type
	t1 = get_attr_type(i[0]),
	t2 = get_attr_type(i[1]),
	t3 = get_attr_type(i[2]);

    int is_alumac_1, is_dmread_2, is_pmread_3;

    is_alumac_1 = (t1 == TYPE_ALU || t1 == TYPE_MAC);
    if (! is_alumac_1)
	return 0;
    if (t2 != TYPE_DMREAD)
	return 0;
    if (t3 != TYPE_PMREAD)
	return 0;
    return 1;
}

/*EK* new... */
#define CALL_USE_IREG REG_I6
#define CALL_USE_DREG REG_SI
void prereload_calls (insn)
    rtx insn;
{
  rtx call;
  rtx *call_loc;
  rtx body;
  rtx new_insn;

  /* first of all, we should only be called for call insns */
  if(GET_CODE (insn)!=CALL_INSN)
    abort ();

  /* get the thing we're calling, call has two forms:
     (set (reg) (call (mem (x)) (y))) and (call (mem (x)) (y))
     We want the "x" in both cases.
   */

  body=PATTERN (insn);
  if (GET_CODE (body) == SET)
    if(GET_CODE(XEXP(body,1))==CALL &&
       GET_CODE(XEXP(XEXP(body,1),0))==MEM)
      call_loc = &XEXP(XEXP(XEXP(body,1),0),0);
    else
      abort();  /* we don't have (call (mem x)) */

  else if (GET_CODE(body) == CALL)
    if (GET_CODE (XEXP (body,0)) ==MEM)
       call_loc = &XEXP(XEXP(body,0),0);
    else
      abort (); /* we don't have (call (mem x)) */
  else
    abort ();  /* only two forms of calls, and this isn't either one. */

  call = *call_loc;

  if (GET_MODE (call) != PMmode)
    abort();  /* gotta do call a pm address (!) */

  if (GET_CODE (call) == SYMBOL_REF)
    return;  /* there's nothing to do here */


  else if (GET_CODE(call) == REG)
    if (REGNO (call) < FIRST_PSEUDO_REGISTER)
      if (IS_DAG2_IREG (REGNO (call)))
	return;
      else
	{
	  /* if we're calling the wrong hard-reg, simply
	     copy from that hard-reg to i6 and call indirect using i6 */
	  rtx new = gen_rtx (REG,PMmode, CALL_USE_IREG);
	  new_insn=emit_insn_before (gen_rtx (SET, new, call), insn);
	  *call_loc = new;
	  REG_NOTES (new_insn)
	    = gen_rtx (EXPR_LIST, INSN_START_CALL, 0, REG_NOTES (new_insn));
	}
    else  /* we've got a pseudo */
      {
	rtx addr, d,i,ip,c;
	
	if (! (addr=reg_equiv_address[REGNO (call)]))
	  abort();  /* how can a pseudo not have a stack slot ? */
	
	/* we want to turn this:
	   call (mem (mem:PM (plus (m4) (const))))
	   into this:
	     
	   (set i6 const)
	   (set i6 (plus i6 m4))
	   (set si (mem:SM i6))
	   (set i6 si)
	   call (mem (reg:PM i6))

	   */
	if (GET_CODE (addr) != PLUS)
	  abort();  /* it is possible to handle this, I just never expected it */
	
	if (GET_CODE (XEXP(addr,0)) == REG &&
	    REGNO (XEXP (addr,0)) == REG_M4 &&
	    CONSTANT_P (XEXP (addr,1)))
	  c = XEXP (addr,1);
	else
	  abort();  /* this doesn't look like a stack slot ! */
	
	i = gen_rtx (REG, stack_pmode, CALL_USE_IREG);
	ip = gen_rtx (REG, PMmode, CALL_USE_IREG);
	d = gen_rtx (REG, PMmode, CALL_USE_DREG);
	
	new_insn = emit_insn_before ( gen_rtx (SET, stack_pmode, i, c), insn);
	emit_insn_before ( gen_rtx (SET, stack_pmode, i, 
				    gen_rtx (PLUS, stack_pmode, i, XEXP (addr,0))), insn);
	emit_insn_before ( gen_rtx (SET, PMmode, d, gen_rtx (MEM, PMmode, i)), insn);
	emit_insn_before (gen_rtx (SET, PMmode, ip, d), insn);
	*call_loc = ip;
	REG_NOTES (new_insn)
	  = gen_rtx (EXPR_LIST, INSN_START_CALL, 0, REG_NOTES (new_insn));
      }
  else
    abort(); /* I don't know what else we might be calling */
}
    
	
