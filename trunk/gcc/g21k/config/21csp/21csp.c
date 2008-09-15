/*
 * $Header: /home/gcc/CORETOOLS/gcc/config/21csp/21csp.c,v 1.34 1997/03/13 21:46:32 levb Exp $ 
 * $Source: /home/gcc/CORETOOLS/gcc/config/21csp/21csp.c,v $
 * $Revision: 1.34 $ $Date: 1997/03/13 21:46:32 $
 * $Author: levb $
*/
/*****  THIS FILE REALLY IS config/21xx/21xx.c **********/

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
enum in_section {no_section, in_text, in_data, EXTRA_SECTIONS} in_section;



static char    *fp;
static char    *sp;
static int      Number_of_registers_saved;
static int      Offset_to_save_area_base;
static char    *varname;
static char    *user_segment_name;         /* temp storage of segment name
					       on a declared variable */
static char     reserved_regs[16];
static int      possible_reserved[]={REG_I2, REG_I3, FIRST_PSEUDO_REGISTER};
                    /* the order in the above array is important.  It is the 
		       order the regnames will be appended to the frame save 
		       function name to form the correct frame save function 
		       name */
static int lcount=0;

int  *mod_reg_equiv_const;
int  mod_reg_equiv_size;

#define RET  return ""				 /* Used in machine
						  * description */
#define IS_REG_SAVETYPE(i) 1
#define IS_REG_RESERVED(i) 0

memory_segment  default_memory_segment = DATA_MEM;
memory_segment  code_memory_segment = PROG_MEM;
enum machine_mode stack_pmode;

#ifdef __WATCOMC__
#define strncasecmp(x,y,l) strnicmp(x,y,l)
#define strcasecmp(x,y) stricmp(x,y)
#endif
double frexp (double, int *);
double ldexp (double, int);
double modf (double, double *iptr);

/* Forward declarations */
static char *pick_segment_name();
static void complete_reg_alloc_order();

#define CHECK_NEW_LINE(file) {if ((++lcount % 20) == 0) fprintf(file, "\n\t");}

output_int(FILE *file, long x)
{
  CHECK_NEW_LINE(file);
  fprintf(file, "%d", (x<<16)>>16); /* sign extend this puppy ! */
}

int             max_arg_regs = 2;

static          arg_regs[] = {REG_AX1, REG_AY1, 999};


void put_segment_name(FILE *file) {

  char *segname;
  
  if (user_segment_name) {
    fputs("/seg=",file);
    fputs(user_segment_name,file);
    user_segment_name = 0;
  } else {
    segname = pick_segment_name();
    if(segname)
      fprintf(file,"/seg=%s",segname);
  }
}

pmda_section() 	
{
   in_section = in_pmda_section; 
}

char *pick_section() 
{ 
  switch(in_section) {
  case no_section: case in_text: case in_data:    return "";
  case in_pmda_section:                           return "/pm";
  default:  abort(); }
}

static char *pick_segment_name() 
{
  switch(in_section) {   
  case no_section: case in_text: case in_data: return dmdata_segment; 
  case in_pmda_section:                        return pmdata_segment; 
  default: abort(); }
}

void* function_arg(cum, mode, type, named)
     CUMULATIVE_ARGS cum;
     enum machine_mode mode;
     tree type;
     int named;
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
enum reg_class c1;
enum reg_class c2;
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
	    case REG:				 /* if doing register to
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
	      return;				 /* transfer to memory, no CC
						  * update */
	    default:
	      abort();
	    }
	case REG:
	  switch (GET_CODE(SET_SRC(exp)))
	    {
	    case REG:
	      return;				 /* register to register
						  * transfer, no CC update */
	    case PLUS:
	    case MINUS:
	    case MULT:
	      cc_status.value1 = SET_SRC(exp);
	      cc_status.value2 = SET_DEST(exp);
	      return;
	    case MEM:				 /* same as up above, doing
						  * mem to reg xfer doesn't
						  * change CC, but might
						  * change what CC came from */
	      if (cc_status.value1 && GET_CODE(cc_status.value1) == MEM)
		cc_status.value1 = 0;
	      if (cc_status.value2 && GET_CODE(cc_status.value2) == MEM)
		cc_status.value2 = 0;
	      return;
	    case CONST_INT:			 /* loading a constant
						  * doesn't set it */
	      return;
	    default:
	      abort();
	    }
	default:
	  abort();
	}
    case CALL:
      CC_STATUS_INIT;				 /* Function calls clobber
						  * the cc's.  */
      return;
    default:
      abort();
    }
  
}


/* Because it makes little or no sence to
   have a packed function I will piggy back on that field. */
install_function_as_farcall (tree decl)
{
  tree fd = get_identifier (IDENTIFIER_POINTER (DECL_NAME (decl)));
  DECL_LANG_SPECIFIC (fd) = (void *)1;
}

    int
is_function_farcallable (tree decl)
{
  int inmain;
  /* DECL_LANG_SPECIFIC (get_identifier (IDENTIFIER_POINTER (DECL_NAME (decl))));*/
  inmain = strcmp ("main", current_function_name) == 0;

  if (MAKE_ALL_FUNCTIONS_FARCALLABLE)
    return 1;
  else
    return inmain;
}

    int
is_main_func(char *fnname)
{
  return (strcmp ("main", fnname) == 0);
}

/*
 * RUNTIME ENVIRONMENT:
 *
 *
 *
 */

extern int leaf_function;

typedef struct Stk {
    int pc_size;	/* size of pc saved on the stack */
    int fp_size;	/* size of fp saved on the stack */
} STK_AREAS;

static STK_AREAS Stack_areas;

int
starting_frame_offset()
{
  int sf;
    /* 
     * Returns the offset from FP to the beginning of the local area.
     * This function can be called for non-leaf functions only.  
     * PC is pushed on the stack before space for locals is allocated.
     */

  if (leaf_function_p () 
	&& !is_function_farcallable(current_function_decl))
    sf = 0;
  else if (is_function_farcallable(current_function_decl)) 
    sf = -2;
  else 
    sf = -1;
  return sf;
}

void function_prologue(file, size)
     FILE *file;
     int size; /* size of locals */
{
  extern int current_function_interrupt;
  int needsaframe = 1; /* for now we always save the old frame including */
		       /* the case when it is not changed in the function*/
  int regno;
  int splitline;
  char *mdec;		/* name of reg containing -1 */
  char *mscratch;	/* name of scratch m-reg */
  int farcallable = is_function_farcallable (current_function_decl);

  leaf_function = leaf_function_p ();
  Stack_areas.pc_size = 0;
  if (needsaframe)
    Stack_areas.fp_size = 1;
  else
    Stack_areas.fp_size = 0;
    

  if (current_function_interrupt)
    fprintf (file,"!! Function '%s' is an interrupt service routine\n", 
	     current_function_name);

  mdec = reg_names[PM_MINUS1_MREG];
  mscratch = reg_names[SCRATCH_PM_MREG];

  if (!leaf_function || farcallable) {
    /* load reg STACKA with pc */
    fprintf (file, "\tpop pc;\n");

    if (!INDIRECT_CALLING) {
      /* Save old fp on stack, set new fp, and save pc on stack */
      fprintf (file, "\tdm(%s,%s)=%s, %s=%s;\n\tdm(%s,%s)=STACKA;\n",
	       sp, mdec, fp, fp, sp, 
	       sp, mdec);
      Stack_areas.pc_size++;
      if (farcallable) {
	/* Save 8 msbs of pc on stack */
	fprintf (file, "\tdm(%s,%s)=STACKP;\n",
		 sp, mdec);
	/* Load the indirect jump page with the actual 
	   page of the currently executing function. */
	fprintf (file, "\tIJPG=pageof (_%s);\n", current_function_name);
	Stack_areas.pc_size++;
      }
    }
  }
  else if (needsaframe && !INDIRECT_CALLING)
    /* For leaf, save old fp on stack and set new fp */
    fprintf (file, "\tdm(%s,%s)=%s, %s=%s;\n",
	     sp, mdec, fp, fp, sp);

  if (size)
    /* allocate space for local variables */
    if (size <= 128)
      fprintf (file, "\tmodify(%s, -%d);\n", sp, size);
    else
      fprintf (file, "\t%s=-%d;\n\tmodify(%s,%s);\n", mscratch, size, sp,
	       mscratch);

  Number_of_registers_saved = 0;
  if (strcmp("main", current_function_name) != 0)
    {
      int first_time = 0;
      Number_of_registers_saved = 0;
      splitline = 0;
      for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
	if (regs_ever_live[regno]
	    && !call_used_regs[regno]
	    && IS_REG_SAVETYPE(regno)
	    && !IS_REG_RESERVED(regno)
	    && regno != FRAME_POINTER_REGNUM)
	  {
	    Number_of_registers_saved++;
	    fprintf (file, "\tdm(%s,%s)=%s;\n", sp, mdec, reg_names[regno]);
	  }
    }

  Offset_to_save_area_base = Number_of_registers_saved + size;
  fprintf (file, "!---begin body\n");
  PUT_SDB_PROLOGUE_END (file, current_function_name);
}

void
function_epilogue (file, size)
     FILE *file;
     int size;
{
  int farcallable = is_function_farcallable (current_function_decl);
  int offs_to_save_regs; /* offset to save regs area */
  char *pmmreg = reg_names[SCRATCH_PM_MREG];
  char *dmireg = reg_names[SCRATCH_DM_IREG];
  char *pmireg = reg_names[SCRATCH_PM_IREG];
  char *mdec = reg_names[PM_MINUS1_MREG];
 
  fprintf (file, "!---end body\n");
 
  leaf_function = leaf_function_p ();

  /**********************************************/
  /* Determine offset from fp to save regs area */
  /**********************************************/

  offs_to_save_regs = size + Stack_areas.pc_size + Stack_areas.fp_size;

  /***************************/
  /* Restore saved registers */
  /***************************/
  if (Number_of_registers_saved)
    {
    int regno;
    fprintf (file, "\t%s=%s;\n", pmireg, fp);

    if (offs_to_save_regs) {
      if (offs_to_save_regs <= 128)
	fprintf (file, "\tmodify (%s,-%d);\n", pmireg, offs_to_save_regs);
      else if (offs_to_save_regs <= 256)
	fprintf (file, "\tmodify (%s,-128);\n\tmodify (%s,-%d);\n",
		 pmireg, pmireg, offs_to_save_regs-128);
      else
	fprintf (file, "\t%s=-%d;\n\tmodify (%s,%s);\n", 
		pmmreg, offs_to_save_regs, pmireg, pmmreg);
    }

    for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
      if (regs_ever_live[regno]
	  && !call_used_regs[regno]
	  && IS_REG_SAVETYPE(regno)
	  && !IS_REG_RESERVED(regno)
	  && regno != FRAME_POINTER_REGNUM)
	{
	  fprintf (file, "\t%s=dm(%s,%s);\n", reg_names[regno], pmireg, mdec);
	}
      }
       
  /*********************************/
  /* Restore sp and fp, and return */
  /*********************************/
  if (!leaf_function || farcallable)
    {
      if (farcallable)
      {
	/*
	 * m7=-2;
	 * IJPG=dm(m5,i5);
	 * i1=dm(m7,i5);
	 */
        fprintf (file, "\t%s=-2;\n", pmmreg);
        fprintf (file, "\tIJPG=dm(%s, %s);\n", pmmreg, fp);
	fprintf (file, "\t%s=dm(%s,%s);\n", dmireg, mdec, fp);
      }
      else
      {
	/* i1=dm(m5,i5); */
	fprintf (file, "\t%s=dm(%s,%s);\n", dmireg, mdec, fp);
      }

      /* jump(i1) (db); */
      fprintf (file, "\tjump (%s) (db);\n", dmireg);
    }
  else /* leaf_function && !farcallable */
    {
      /* rts (db); */
      fprintf (file, "\trts (db);\n");
    }

  if (Stack_areas.fp_size)
    /*
     * i4=i5;
     * i5=dm(i5,m5);
     */
    fprintf (file,
             "\t%s=%s;\n"
             "\t%s=dm(%s,%s);\n\n",
             sp,fp,
             fp,fp,
             mdec);
  else
     /* i4=i5; */
    fprintf (file,
             "\t%s=%s;\n",
             sp,fp);

} /* function epilogue */

void
output_asm_return_insn()
{
  fprintf (asm_out_file, "\tjump __%s_end;\n", current_function_name);
}


char           *arch_reserved_registers = 0;
char           *dmdata_segment=0;
char           *pmdata_segment=0;
char           *boot_page_number=0;

void conditional_register_usage()
{
  char           *cp;
  int             done,
  i;

  complete_reg_alloc_order();
  
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
	    if (reg_names[i][0] 
		&& !strncasecmp(reg_names[i], cp, strlen(reg_names[i])))
	      {
		if (fixed_regs[i])
		  error("Can't reserve fixed register \"%s\"", 
			reg_names[i]);
		else if (call_used_regs[i])
		  error("Can't reserve call-used register \"%s\"", 
			reg_names[i]);
		else
		  {
		    /* sometimes register allocation
		       starts from call_used_reg_set,
		       not from fixed_reg_set.  Set 
		       call_used_regs to protect for this */
		    fixed_regs[i] = 1;
		    call_used_regs[i] = 1;  
		  }
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

asm_make_name (file, name)
     FILE           *file;
     char           *name;
{
#ifdef OLD_A21
  char *s= (char *)alloca(strlen(name)+10);
  char *cp;

  strcpy(s,name);
  name=s;

/* Only look at the characters after the slash, if any */
  cp=strrchr(name,'/');
  name=cp?cp+1:name;
  cp=strrchr(name,'\\');
  name=cp?cp+1:name;

/* remap all non alpha-numerics to '_' (2100 likes it that way! ) */
  cp=name;
  while(*++cp)
    if(!isalnum(*cp))
      *cp='_';
#endif

  if (strncmp ("L%", name, 2) == 0)
    fprintf(file, "%s", name);
  else

#ifdef UNDERSCORE_BEFORE
    fprintf (file, "_%s", name);
#else
    fprintf (file, "%s_", name);
#endif
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
  fputs("\t.global\t", file);
  asm_make_name(file, name);
  fputs(";\n", file);
}

asm_output_labelref(file, name)
     FILE           *file;
     char           *name;
{
  fputc('^', file);
#if defined (A21C0) && defined (NEED_UNDERSCORE)
  fputs (name, file);
#else
  asm_make_name(file, name);
#endif
}

asm_output_internal_label(file, prefix, num)
     FILE           *file;
     char           *prefix;
     int             num;
{
  char            s[100];
  
  asm_generate_internal_label(s, prefix, num);
  asm_make_name(file, s);
  fputs(":\n", file);
}

asm_generate_internal_label(label, prefix, num)
     char           *label;
     char           *prefix;
     int             num;
{
#ifdef OLD_A21
  sprintf (label, "%s%s%d", current_function_name?current_function_name:"",prefix, num);
#else
  sprintf (label, "%s%c%d", prefix, '%', num);
#endif
}



asm_output_short(file, value)
     FILE           *file;
     rtx             value;
{
  rtx low, high;

  get_singles(value,&high,&low,0,0);
  CHECK_NEW_LINE(file);
  output_addr_const(file, high);
  fputc(',',file);
  CHECK_NEW_LINE(file);
  output_addr_const(file, low);
}

asm_output_char(file, value)
     FILE           *file;
     rtx             value;
{
  CHECK_NEW_LINE(file);
  output_addr_const(file, value);
}

asm_output_byte(file, value)
     FILE           *file;
     int             value;
{
  CHECK_NEW_LINE(file);
  fprintf(file, "0x0%x", value);
}


asm_output_float(file, value)
     FILE           *file;
     REAL_VALUE_TYPE value;
{
  rtx low, high;
  double_const_to_singles (CONST_DOUBLE_FROM_REAL_VALUE (value, SFmode), &high, &low);

  CHECK_NEW_LINE (file);
  output_addr_const (file, high);
  fputc(',',file);
  CHECK_NEW_LINE (file);
  output_addr_const (file, low);
}

asm_output_double(file, value)
     FILE           *file;
     int             value;
{
  CHECK_NEW_LINE(file);
  asm_output_float (file, value);
}

asm_output_ascii(file, ptr, len)
     FILE           *file;
     char           *ptr;
     int             len;
{
  int             i;
  
  for (i = 0; i < len; i++)
    {
      CHECK_NEW_LINE(file);
      output_int(file,ptr[i]);
      if (i + 1 < len)
	fputc(',',file);
    }
}

generate_global_varibale(file, name, size, rounded)
     FILE           *file;
     char           *name;
     int             size,
       rounded;
{
  varname = name;
  
  fputs(".global\t", file);
  asm_make_name(file, name);
  fprintf(file, ";\n");
  
  fputs(".var",file);
  fputs(pick_section(), file);
  put_segment_name(file);
  fputc('\t', file);
  asm_make_name (file, name);
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
  fputs (".var",file);
  fputs (pick_section(),file);
  put_segment_name(file);
  fputc ('\t', file);
  fputc ('_', file);
  fputs (name, file);
  if (rounded > 1)
    fprintf (file, "[%d]", rounded);
  fprintf (file, ";\n");
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
  
#ifdef OLD_A21
  sprintf(s, "%sL%d", current_function_name?current_function_name:"", value);
#else
  sprintf(s, "L%%%d", value);
#endif
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
  
  lcount=0;
  pattern = PATTERN(table);
  table_len = XVECLEN(pattern, 0);
  data_section();
  asm_generate_internal_label(name, prefix, num);
#ifdef A21C0
  fprintf(file, ".var %s[%d];\n", name, table_len);
  sprintf(jump_table_separator, ".init %s:\t", name);
#else
  fprintf(file, ".var %s_[%d];\n", name, table_len);
  sprintf(jump_table_separator, ".init %s_:\t", name);
#endif
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
      CHECK_NEW_LINE(file);
      fprintf(file, "0,");
    }
  CHECK_NEW_LINE(file);
  fprintf(file,"0");
}

int print_operand_punct_valid_p (code)
     char code;
{
  static char    *legal_codes = "?_.!#LMjJubD-+Ppm";
  char           *p = legal_codes;
  
  while (*p)
    if (code == *p++)
      return 1;
  return 0;
}

dbr_output_seqend (file)
     FILE           *file;
{
  extern rtx final_sequence;
  if (optimize > 0 && flag_delayed_branch)
    {
      int len = dbr_sequence_length();

#if 0
      was this for a single delayed branch slut I can not remember
      well delete it for now.

      if (GET_CODE (XVECEXP (final_sequence, 0, 0)) == CALL_INSN)
	{
	  if (!len) 
	    fputs("\tnop;\n\n", file);
	}
      else
#endif
	switch (len)
	  {
	  case 1:
	    fputs("\tnop;\n\n", file);
	    break;
	  case 0:
	    fputs("\tnop;nop;\n\n", file);
	    break;
	  default:
	    fputc('\n', file);
	    fputc('\n', file);
	    break;
	  }
    }
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
  lcount=0;  
  switch (code)
    {
    case '!':
      if (optimize > 0
	  && flag_delayed_branch
	  && dbr_sequence_length())
	fprintf(file, "(DB)");
      break;
    case '-':
      if (A21C0)
	fprintf (file, "swcond");
      else
	fprintf (file, "neg");
      break;
    case '+':
      if (A21C0)
	fprintf (file, "not swcond");
      else
	fprintf (file, "pos");
      break;
    case 'm':
      if (!REG_P (x) || 
	!TEST_HARD_REG_BIT(reg_class_contents [MR0_REG], REGNO (x)))
	abort ();
      
      fprintf (file, (REGNO (x) == REG_MR0 || REGNO (x) == REG_MR1) ? 
		"mr" : "sr");
      break;

    case 'p':
      if (A21C0) {
	fprintf (file, "(pc, ");
	output_address (x);
	fprintf (file, "-1)");
      } else {
	output_address (x);
	fprintf (file, "-1");
      }
      break;

    case 'P':
      if (A21C0) {
	fprintf (file, "(pc, ");
	output_address (x);
	fprintf (file, ")");
	break;
      } /* otherwise */

    case 'A':
      output_address (x);
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
      
    case 'J':					 /* reverse logic */
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
#ifdef A21C0
	  fprintf(file, "dm(");
#else
	  fprintf(file, "%s(", GET_MODE(XEXP(x,0))==PMmode?"pm":"dm");
#endif
	{
	    /* if this is + or += then ensure that we have the same dag. 
	       if not abort (you need to fix this some where in reload */
	    rtx y = XEXP (x, 0);
	    if ((GET_CODE (y) == PLUS 
		 || GET_CODE (y) == POST_MODIFY)
		&& (REG_P (XEXP (y,0)) 
		    && REG_P (XEXP (y,1))))
		if (!SAME_DAG_REGS (REGNO (XEXP (y,0)), REGNO (XEXP (y,1)))) {
		    extern rtx final_insn;
		    debug_rtx (final_insn);
		    debug_rtx (x);
		    abort ();
		}
	}
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
	fprintf(file, "%s,0", 
		reg_names[REGNO(addr)]);
      else
	fprintf(file, "%s,0", 
		reg_names[REGNO(addr)]);
      break;
#if defined PM_MINUS1_MREG && defined DM_MINUS1_MREG
    case POST_DEC:
      if (IS_DAG2(REGNO(XEXP(addr, 0))))
	fprintf(file, "%s,%s", 
		reg_names[REGNO(XEXP(addr, 0))], 
		reg_names[PM_MINUS1_MREG]);
      else
	fprintf(file, "%s,%s", 
		reg_names[REGNO(XEXP(addr, 0))], 
		reg_names[DM_MINUS1_MREG]);
      break;
#endif
#if defined DM_MINUS1_MREG
    case POST_INC:
      if (IS_DAG2(REGNO(XEXP(addr, 0))))
	fprintf(file, "%s,m5", reg_names[REGNO(XEXP(addr, 0))]);
      else
	fprintf(file, "%s,%s", 
		reg_names[REGNO(XEXP(addr, 0))], 
		reg_names[DM_PLUS1_MREG]);
      break;
#endif
    case POST_MODIFY: {
      rtx base     = XEXP(addr,0);
      rtx modifier = XEXP(addr,1);
      SWITCH_SUBREG (base);
      SWITCH_SUBREG (modifier);
      if (REG_P (modifier))
	fprintf (file, "%s,%s", reg_names[REGNO(base)], reg_names[REGNO(modifier)]);
      else if (A21C0 && CONSTANT_P (modifier) && CONSTANT_8bitP (modifier)) {
	int done = 0;

#ifdef PM_MINUS1_MREG
	if (!done && IS_DAG2_IREG (REGNO (base)) && AINTVAL (modifier) == -1)
	  fprintf (file, "%s,%s", reg_names[REGNO(base)], reg_names [PM_MINUS1_MREG]), done = 1;
#endif
#ifdef PM_PLUS1_MREG
	if (!done && IS_DAG2_IREG (REGNO (base)) && AINTVAL (modifier) == 1)
	  fprintf (file, "%s,%s", reg_names[REGNO(base)], reg_names [PM_PLUS1_MREG]), done = 1;
#endif
#ifdef DM_MINUS1_MREG
	if (!done && IS_DAG1_IREG (REGNO (base)) && AINTVAL (modifier) == -1)
	  fprintf (file, "%s,%s", reg_names[REGNO(base)], reg_names [DM_MINUS1_MREG]), done = 1;
#endif
#ifdef DM_PLUS1_MREG
	if (!done && IS_DAG1_IREG (REGNO (base)) && AINTVAL (modifier) == 1)
	  fprintf (file, "%s,%s", reg_names[REGNO(base)], reg_names [DM_PLUS1_MREG]), done = 1;
#endif
	if (!done)
	  fprintf (file, "%s,%d", reg_names[REGNO(base)], AINTVAL (modifier));
      }
      else 
	abort ();

      break;
    }

    case PLUS:
      if (GET_CODE(XEXP(addr, 0)) == CONST_INT)
	{
	  print_operand_address(file, XEXP(addr, 1));
	  if (AINTVAL (XEXP(addr, 0)) >= 0)
	    fprintf(file, "+");
	  print_operand_address (file, XEXP(addr, 0));
	}
      else if (CONSTANT_ADDRESS_P (XEXP (addr, 0))
	       && CONSTANT_ADDRESS_P (XEXP (addr, 1)))
	{
	  print_operand_address (file, XEXP(addr, 0));
	  fprintf (file, "+");
	  print_operand_address (file, XEXP(addr, 1));	  
	}
      else
	{
#ifdef A21C0
	  if (REG_P (XEXP (addr, 1))
	      && IS_MREG (REGNO (XEXP (addr, 1))))
	    fprintf (file, "%s,", reg_names [REGNO (XEXP (addr, 1))]);

	  else if (CONSTANT_P (XEXP (addr, 1)) && CONSTANT_8bitP (XEXP (addr, 1)))
	    fprintf (file, "%d,", AINTVAL (XEXP (addr, 1)));

	  else if ((REG_P (XEXP (addr, 0))
		    && IS_MREG (REGNO (XEXP (addr, 0)))))
	    {
	      fprintf (file, "%s,%s", 
		       reg_names [REGNO (XEXP (addr, 0))],
		       reg_names [REGNO (XEXP (addr, 1))]);
	      return;
	    }
	  else if ((REG_P (XEXP (addr, 1))
		    && IS_IREG (REGNO (XEXP (addr, 1))))) {
	    
	    fprintf (file, "%s,%s", 
		     reg_names [REGNO (XEXP (addr, 1))],
		     reg_names [REGNO (XEXP (addr, 0))]);
	    return;
	  }
	  else
	    abort ();

	  if (REG_P (XEXP (addr, 0))
	      && IS_IREG (REGNO (XEXP (addr, 0))))
	    fprintf (file, "%s", reg_names [REGNO (XEXP (addr,0))]);
	  else
	    abort ();

	  return;
#if 0
	  print_operand_address (file, XEXP(addr, 0));
	  if (AINTVAL (XEXP (addr, 1)) >= 0)
	    fprintf (file, "+");
	  print_operand_address (file, XEXP(addr, 1));
#endif

#endif
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
	
#ifdef OLD_A21	
	sprintf(s, "%sL%d", 
		current_function_name?current_function_name:"",
		CODE_LABEL_NUMBER(XEXP(addr, 0)));
#else
	sprintf(s, "L%%%d", CODE_LABEL_NUMBER(XEXP(addr, 0)));
#endif
	asm_make_name(file, s);
      }
      break;
    case CODE_LABEL:
      {
	char            s[100];
	
#ifdef OLD_A21
	sprintf(s, "%sL%d", current_function_name?current_function_name:"",
		CODE_LABEL_NUMBER(addr));
#else
	sprintf(s, "L%%%d",  CODE_LABEL_NUMBER(addr));
#endif
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
  asm_output_array_size (file, size)
FILE           *file;
int             size;
{
  fprintf(file, "[%d]", size);
}

/*** Print out the symbol specifying the assignment of ***/
/*** initial values to a static  variable (usualy "=") ***/

void
  asm_output_init_assign (file, internal)
FILE           *file;
int		internal;
{
#ifdef UNDERSCORE_BEFORE
  fprintf(file, ".init\t_%s: ", varname);
#else
  fprintf(file, ".init\t%s_: ", varname);
#endif
  lcount=0;
}


/*** In case when variables have labels different ***/
/*** from those for functions */

void
  asm_output_var_label(file, name)
FILE           *file;
char           *name;
{
  varname = name;

#ifdef UNDERSCORE_BEFORE
  fprintf(file, ".var%s\t_%s;\n", pick_section(),name);
#else
  fprintf(file, ".var%s\t%s_;\n", pick_section(),name);
#endif
}

void
  asm_output_external(file, decl, name)
FILE           *file;
rtx             decl;
char           *name;
{
#ifdef UNDERSCORE_BEFORE
  fprintf(file, ".external\t_%s;\n", name);
#else
  fprintf(file, ".external\t%s_;\n", name);
#endif
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

char            *pmcode_segment = 0;


/******** Functions to print the header and the tail of ********/
/******** the assembler file **********************************/

void
  asm_file_start(file)
FILE           *file;
{
  register char  *module_name = (char *) xmalloc(strlen(main_input_filename) + 6);
  int             len = strlen(main_input_filename);
  char            *bp;
  int             *reserved_ptr, reg;

  reserved_regs[0]=0x0;

  strcpy(module_name, main_input_filename);
  strip_off_ending(module_name, len);
  
  fp = reg_names[FRAME_POINTER_REGNUM];
  sp = reg_names[STACK_POINTER_REGNUM];

  if (A21C0)
    fprintf(file, "!\t\tAnalog Devices ADSP21CSP\n");
  else
    fprintf(file, "!\t\tAnalog Devices ADSP21XX\n");

  if (write_symbols != NO_DEBUG)
    fprintf(file, ".file <%s>;\n",  input_filename);

  if (!A21C0) {
    if (dmdata_segment)
      fprintf (file, ".dmseg %s;\n", dmdata_segment);

    if (pmdata_segment)
      fprintf (file, ".pmseg %s;\n", pmdata_segment);
  }

#ifdef 0
  if (A21C0) {
    /* all global variable go into data memory segment */
      fprintf (file, ".dmseg .data;\n");
  }
#endif

  if (!A21C0)
    {
      fprintf(file, ".MODULE/RAM");
      if( boot_page_number ) {
	for( bp=boot_page_number; *bp && *bp != ' '; bp++){
	  if( *bp>='0' && *bp<='9' )
	    fprintf(file, "/BOOT=%c", *bp);
	}
      }
      if( pmcode_segment ) {
	fprintf(file, "/SEG=%s",pmcode_segment);
      }
      fprintf(file, "\t_");
      asm_make_name(file, module_name);
      fprintf(file, ";\n");
    }
  /* check for possible reserved regs */
  for( reserved_ptr=possible_reserved; 
      (reg = *reserved_ptr) != FIRST_PSEUDO_REGISTER; reserved_ptr++)
    {
      if( fixed_regs[reg] == 1 )
	strcat(reserved_regs, reg_names[reg]);
    }
  
  if (!A21C0)
    {
      if( !strcmp(reserved_regs, "") )
	{
	  fprintf(file, ".external ___lib_save_frame;\n");
	  fprintf(file, ".external ___lib_restore_frame;\n");
      } else {
	fprintf(file, ".external ___lib_save_frame_no_%s;\n",
		reserved_regs);
	fprintf(file, ".external ___lib_restore_frame_no_%s;\n",
		reserved_regs);
      }
    }
}

void
  asm_file_end(file)
FILE           *file;
{
  if (!A21C0)
    fprintf(file, ".ENDMOD;\n");
  
#if defined(KEEP_STAT) && defined(ADSP_PROFILE_STATUS)
  if (KEEP_STAT)
    write_stat_file();
#endif
}


/*
 * operand Predicates
 */

int register_or_one_operand(op,mode)
register rtx    op;
enum machine_mode mode;
{
  return register_operand(op,mode) ||
    (GET_CODE(op)==CONST_INT && (INTVAL(op)==1 || INTVAL(op)==-1));
}

int register_or_bert_operand (op,mode)
     register rtx    op;
     enum machine_mode mode;
{
  if (register_operand(op,mode))
    return 1;
  else if (GET_CODE (op) == CONST_INT
	   && const_ok_for_letter_p (INTVAL (op), 'M'))
    return 1;
  else
    return 0;
}

int register_or_zero_operand(op,mode)
register rtx    op;
enum machine_mode mode;
{
  return register_operand(op,mode) ||
    (GET_CODE(op)==CONST_INT && INTVAL(op)==0);
}

int dreg_operand (op, mode)
    register rtx    op;
    enum machine_mode mode;
{
    return register_operand(op,mode) 
	&& reg_class_subset_p (REGNO_REG_CLASS (REGNO (op)), D_REGS);
}


int register_or_const8bit_operand (rtx op, enum machine_mode mode)
{
  return register_operand (op, mode) || CONSTANT_8bitP (op);
}

int register_or_constint_operand (rtx op, enum machine_mode mode)
{
  return register_operand(op,mode) || CONST_INT_P (op);
}

/*
 * Return 1 if OP is a register and is not a subreg reference or 
 * an immediate value of mode MODE. 
*/
    int
nonsubreg_nonmemory_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
  return (GET_CODE (op) != SUBREG && nonmemory_operand (op, mode));
}

/*
 * Return 1 if OP is a register and is not a subreg of mode MODE. 
*/
    int
nonsubreg_register_operand (op, mode)
     register rtx op;
     enum machine_mode mode;
{
    return (GET_CODE (op) != SUBREG && register_operand (op, mode));
}

/*
 * Return 1 if OP is a a register of AR register class 
*/
    int
ar_register_operand(op, mode)
     register rtx op;
     enum machine_mode mode;
{
    return register_operand(op,mode)
      && GET_CODE (op) == REG
      && reg_class_subset_p (REGNO_REG_CLASS (REGNO (op)), A_R_REGS);
}

/* 
 * Return next insn which is neither NOTE nor USE
 */
    rtx
next_nonnote_nonuse_insn(insn)

    rtx insn;
{
  while (insn)
    {
      insn = NEXT_INSN (insn);
      if (insn == 0 
        || !(GET_CODE (insn) == NOTE 
            || (GET_CODE (insn) == INSN
	    && GET_CODE (PATTERN (insn)) == USE)))
        break;
    }

  return insn;
}


/* 
 * Return next insn which is not NOTE or not USE or not CLOBBER 
 * or not same reg assignment, Rx = Rx.
 */
    static rtx
next_machine_insn(insn)
    rtx insn;
{
  rtx body, op0, op1;
  enum rtx_code insn_code, body_code;

  while (insn)
    {
      insn = NEXT_INSN (insn);
      if (0 == insn )
        break;

      insn_code = GET_CODE (insn);
      if (INSN == insn_code) {
        body = PATTERN (insn);
        body_code = GET_CODE (body);
      } else {
        body = 0;
        body_code = 0;
      }

      if (SET == body_code) {
	op0 = XEXP (body, 0);
	op1 = XEXP (body, 1);
      } else {
        op0 = 0;
        op1 = 0;
      }

      if (!(NOTE == insn_code
            || (INSN == insn_code && USE == body_code)
            || (INSN == insn_code && CLOBBER == body_code)
            || (INSN == insn_code && SET == body_code
		&& REG == GET_CODE (op0) && REG == GET_CODE (op1)
		&& REGNO (op0) == REGNO (op1)))) 
        break;
    }

  return insn;
}
/*
 * For indirect calls return reg rtx, otherwise return NULL.
 * This code depends on call being part of parallel pattern.
 */
    rtx
icall_reg (op)
     register rtx op;
{
    rtx  x;

    /* get call insn */
    if (0 != op 
        && INSN == GET_CODE (op) 
	&& SEQUENCE == GET_CODE (PATTERN (op))
	&& CALL_INSN == GET_CODE (XVECEXP (PATTERN (op), 0, 0))
	&& TYPE_ICALL == get_attr_type (XVECEXP (PATTERN (op), 0, 0))) {
	x = XVECEXP (PATTERN (XVECEXP (PATTERN (op), 0, 0)), 0, 2);
    } else if ( 0 != op 
	&& CALL_INSN == GET_CODE (op)
	&& TYPE_ICALL == get_attr_type (op)) {
	x = XVECEXP (PATTERN (op), 0, 2);
    } else {
        return NULL;
    }

    /* We have two cases: reg=call(i) and call(i) */
    if (GET_CODE (x) == SET && GET_CODE (XEXP (x,1)) == CALL) {
	/* get ireg of call(ireg) */
	x = XEXP (XEXP (XEXP (x, 1), 0),0);
    } else if (GET_CODE (x) == CALL) {
	x = XEXP (XEXP (x, 0), 0);
    }

    if (GET_CODE (x) == REG) {
	return x;
    } else {
	return NULL;
    }
}

    int
subreg_reg_operand(op, mode)
    register rtx    op;
    enum machine_mode mode;
{
    enum machine_mode mode2;

    if (SImode == mode && GET_CODE(op)==SUBREG) {
      SWITCH_SUBREG(op);
      mode2 = GET_MODE(op);
      if (DImode == mode2) {
	  return register_operand(op,mode2);
      }
    } 
    return 0;
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

/* 
 * If delay is needed return 1, otherwise return 0.
 * On 21csp01, for memory with wait states, if the result 
 * of a DAG register load is immediately used 
 * in a memory reference instruction then 1 cycle delay is needed.
 * This function should work for the following two cases.
 * Case1: Return From a Function:
 *_foo:   ............
 *      ............
 *      ............
 *      JUMP    (I1) (db);
 *      i4=i5;
 *      i5=dm(i5,m5);
 *      ............
 *      ............
 *      ............
 *      call    _foo;
 *@I1:  ax1=dm(-2,i5);
 *
 *Case2. Sequential instructions.
 *
 *      i6=dm(m0,i1);
 *      ar=dm(m7,i6);
 * or
 *
 *      i6=dm(m0,i1);
 *label:ar=dm(m7,i6);
 *
 * See PR 1467. --lev 10/10/96
 */

int
insn_need_delay(insn)
    rtx insn;
{
    enum machine_mode mode;
    rtx x, y, z; /* temporaries */
    enum rtx_code insn_code = GET_CODE(insn);
    int retval = 0;

    if (!(ADSP21CSP01_ONLY && 'i' == GET_RTX_CLASS (insn_code))) {
      return retval;
    }

    /* 
     * check that our insn is addr_reg = dm(addr) 
     * or a call insn 
     */
    switch (insn_code) {
      case INSN:
        if (SET == GET_CODE (PATTERN (insn))) {
        /* Get operands and the mode of the set insn */
        rtx op0 = XEXP (PATTERN (insn), 0);
        rtx op1 = XEXP (PATTERN (insn), 1);
        mode = GET_MODE (op1);
        if (REG == GET_CODE (op0) || SUBREG == GET_CODE (op0)) {
	  SWITCH_SUBREG (op0);
	  /* Check whether the insn is a DAG reg load from memory */
          if (IS_DAG_REG (REGNO(op0)) && memory_operand (op1, mode)) {
	    /* Find next machine insn */
            for (x = next_machine_insn (insn); 
	      NULL != x && 'i' != GET_RTX_CLASS (GET_CODE (x));
		  x = next_machine_insn (x));
  
	    if (NULL == x) {
	      /* We reached the last insn in the function body.
	       * Note that epilogue starts with "i1=dm(m5,i5)".
	       * Thus we do not need to emit "nop", return 0.
	       */
	      return retval;
	    }
  
	    /* If the next insn is a set insn then check whether
	     * any of its operands is a memory reference like dm(Ix,My), where
	     * Ix or My was loadded from the memory in the previous insn.
	     * For now, if it is PARALLEL or SEQUENCE, do not do further
	     * analysis, return 1.
	     */
	    if (SEQUENCE == GET_CODE(PATTERN(x))
		|| (CALL_INSN != GET_CODE (x) 
                && PARALLEL == GET_CODE(PATTERN(x)))) {
	      retval = 1;
	    } else if (SET == GET_CODE(PATTERN(x))) {
	      y = XEXP (PATTERN (x), 0);
	      z = XEXP (PATTERN (x), 1);
	      if ((memory_operand (y, GET_MODE(y)) 
			  && reg_mentioned_p (op0, y)) 
		  || (memory_operand (z, GET_MODE(z))
			   && reg_mentioned_p (op0, z))) {
	        retval = 1;
	      }
            }
          }
        }
      } 
      break;
    case CALL_INSN:
      /* Find next insn */
      for (x = next_machine_insn (insn); 
	NULL != x && 'i' != GET_RTX_CLASS (GET_CODE (x));
	    x = next_machine_insn (x));

	  if (NULL == x
              && !is_function_farcallable(current_function_decl)) {
	    /* We reached the last insn in the function body.
	    * Note that unless the function is not farcallable, e.g., main,
            * the epilogue may start with "i1=dm(m5,i5)",
	    * therefore, we have the following instruction sequence:
	    * i5=dm(i5,m5);
	    * i1=dm(m5,i5);
	    * and we must emit "nop", return 1.
	    */
	    return 1;
	  } else if (NULL == x) {
            return 0;
          }

      /* If the next insn is a set insn then check whether
       * any of its operands is a memory reference like dm(Ix,My).
       * For now, if it is PARALLEL or SEQUENCE, return 1.
       */
      if (SEQUENCE == GET_CODE(PATTERN(x))
	  || (CALL_INSN != GET_CODE (x)
          && PARALLEL == GET_CODE(PATTERN(x)))) {
        retval = 1;
      } else if (SET == GET_CODE(PATTERN(x))) {
	y = XEXP (PATTERN (x), 0);
	z = XEXP (PATTERN (x), 1);
	if (MEM == GET_CODE (y) ) {
	  x = y;
	} else if (MEM == GET_CODE (z)) {
	  x = z;
	} else {
	  return retval;
	}
	switch (GET_CODE (XEXP (x, 0))) {
	  case PLUS:
	  case POST_MODIFY:
	  case REG:
	  case SUBREG:
	  case PRE_MODIFY:
	    retval = 1;
	    break;
	  default:
	    break;
	}
      }
      break;
    default:
      break;
    }
    return retval;
}

/*** post_modified_operand(op, mode) returns 1 if OP is a memory operand **/
/*** with POST_INC or POST_DEC ***/


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



asm_declare_object_name(file, name, decl, internal)
     FILE           *file;
     char           *name;
     tree            decl;
     int             internal;
{
  int             size = int_size_in_bytes(TREE_TYPE(decl));
  char           *segname;

  varname = name;
  if (size < 0)
    abort();
  fprintf(file, ".var");
  if( TREE_MEMSEG(decl)==PROG_MEM ){
    fprintf(file, "/pm");
    segname = pmdata_segment;
  } else {
    segname = dmdata_segment;
  }
  if( segname ) fprintf(file, "/seg=%s\t", segname);
           else fprintf(file, "\t");
#if 0
  if (internal)
    fputs (name, file);
  else
#endif
    asm_make_name(file, name);
  if (size != 1)
    fprintf(file, "[%d]", size);
  fprintf(file, ";\n");
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

   'M'  alu BERT
   'P'  pass BERT
*/

/* If this table is ever lost it can be found in the ac0 assembler. */
int valid_alu_constants[] = {
#define DEFBERT9(constant, YY, CC, BO, bit) constant,
#include <adi/bertcons.def>
#undef DEFBERT9
};
#define LEN_valid_alu_constants (sizeof (valid_alu_constants)/sizeof(int))

int valid_pass_constants[] = {
#define DEFBERT9(constant, YY, CC, BO, bit) constant,
#include <adi/bertcons.def>
#undef DEFBERT9
};
#define LEN_valid_pass_constants (sizeof (valid_pass_constants)/sizeof(int))
int 
const_ok_for_letter_p(value, letter)
     int value;
     char letter;
{
  unsigned int i;
  int low, high = -1;
  int result = 0;
  
  switch (letter)
    {
    case 'K':
      high = 0x003fff;
      low = -(high + 1);
      return (low <= value) && (value <= high);

    case 'J':
      return (value == 1);

    case 'N':
      return (value == -1);

    case 'L':
      high = 0x07f;
      low = -(high + 1);
      return (low <= value) && (value <= high);

    case 'M':
      for (i=0;i<LEN_valid_alu_constants;i++)
	if (value == valid_alu_constants[i] || value == -valid_alu_constants[i])
	  return 1;
      return 0;

    case 'P':
      for (i=0;i<LEN_valid_pass_constants;i++)
	if (value == valid_pass_constants[i])
	  return 1;
    default:
      return 0;
    }
}



static int reloaded_reg_in_class(rtx reg, enum reg_class class, int rindex[])
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


extern int      somewhere_in_reload;		 /* this is defined in
						  * reload1.c to be true
						  * during almost the entire
						  * reload pass */
/*
   ;=;  M       dirty birdy constants. (alu)
   ;=;  P       dirty birdy constants. (pass)
   ;=;
   ;=;  These things are just too too complicated for my little mind.
   ;=;
   ;=;  so lets do it correctly this time....
   ;=;   'm' which is any legit address that passes memory_operand
   ;=;   will represent all memory moves and then we will work from there
   ;=;  our moves need to know how to move to all registers.
   ;=;   and this is done with a pre or a post modify address (registers only)
   ;=;   'Q' which is this type of operand dm|pm who cares on this part.
   ;=;
   ;=;  look ahead at our future.  we will need to know nothing else accept
   ;=;  for maybe:
   ;=;   'R' which ensures the mode is DM
   ;='   'S' which ensures the mode is PM
   ;=;  although I'm skeptical that this will even need to be here.
   ;=;   
   ;=; we use to have this.
   ;=;	Q	dm(imm)
   ;=;	R	dm(dag1i,m)     dmdread
   ;=;	S	dm(dag2i,m)     dmpread
   ;=;  T	pm(imm)
   ;=;	U	pm(dag2i,m)
   ;=;
   ;=;  W       %Dm(mreg, ireg)   i.e. (plus ireg mreg)
   ;=;  Y       %Dm(const8, ireg) i.e. (plus ireg const)
   ;=;  ^       %Dm(ireg, const8) i.e. (post_modify ireg const8)
*/
/*
  R	dm(dag1i,m)
  S	dm(dag2i,m)
  U	pm(dag2i,m)

  Q	dm(imm)
  T     pm(imm)  ( <-- this is a valid addressing mode only for calls/jumps )
  */

    int
extra_constraint (op, c)
    rtx op;
    char c;
{
/*
 * 21csp extra constraints:
 *	Q - any pre-modify or post-modify-update (reg,reg) mode.
 *	R - post-modify-update (i-reg,m-reg) mode. (Instr Type 2).
 */
    rtx addr, op0, op1;
    enum machine_mode addr_mode;
    int ret = 0;

    if (MEM != GET_CODE (op)) {
	return 0;
    }

    addr = XEXP (op, 0);
    if (!(POST_MODIFY == GET_CODE (addr)
	|| PLUS == GET_CODE (addr))) {
	return 0;
    }

    addr_mode = GET_MODE (addr);
    op0 = XEXP (addr, 0);
    op1 = XEXP (addr, 1);

    if (REG_P (op0) && REG_P (op1)) {
    
	switch (c) {
	    case 'Q' :
		/* pre-modify or post-modify-update of form (reg,reg) */
		ret = 1;
		break;
	    case 'R' :
		/* Return TRUE if it is not a pre-modify. */
		if (POST_MODIFY == GET_CODE (addr)
		    && REG_OK_FOR_BASE_STRICT_P (op0, addr_mode)
		    && REG_OK_FOR_INDEX_STRICT_P (op1, addr_mode)
		    && SAME_DAG_REGS (REGNO (op0), REGNO (op1))) {
		    ret = 1;
		} 
		break;
	    default:
		break;
	} /* end switch */
    } /* end if */
    return ret;
} /* extra_constraint */


/** Note that constant address is allowed only in DMmode) **/

int
go_if_legitimate_address_strict (mode, x)
     enum machine_mode mode;
     rtx x;
{
  int ok = 0;
  enum machine_mode addr_mode = GET_MODE(x);
  
  /* (query-pr "1041 --full") */
  if (0 && (mode == SFmode || mode == DImode) &&
      (addr_mode == DMmode))
  {
      ok = CONSTANT_ADDRESS_P(x);
  }
  else
  {
    /*
     * The first part of if condition is commented because addressing 
     * mode like this "(mem/s:DM (reg:DM 32 i0))" is illegal.
     * See pr 201. 12/14/95 --lev
     */
    if (/*(REG_P(x) && REG_OK_FOR_BASE_STRICT_P(x, addr_mode)) ||*/
	((addr_mode == DMmode) && CONSTANT_ADDRESS_P(x)))
      ok = 1;
    else if ((GET_CODE (x) == POST_DEC || GET_CODE (x) == POST_INC)
	     && REG_P(XEXP(x, 0))
	     && REG_OK_FOR_BASE_STRICT_P(XEXP(x, 0),addr_mode))
      ok = 1;

    else if ((GET_CODE (x) == POST_MODIFY)
	     && REG_P (XEXP(x, 0))
	     && REG_OK_FOR_BASE_STRICT_P(XEXP(x, 0), addr_mode)
	     && REG_P (XEXP (x, 1))
	     && REG_OK_FOR_INDEX_STRICT_P (XEXP (x, 1), addr_mode)) 
    {
	if (REG_P (XEXP (x, 1))
	    && addr_mode == DMmode
	    && REG_OK_FOR_INDEX_STRICT_P (XEXP (x, 1), addr_mode)
	    && ((IS_DAG1_IREG (REGNO (XEXP (x, 0)))
		 && IS_DAG1_MREG (REGNO (XEXP (x, 1))))
		||	(IS_DAG2_IREG (REGNO (XEXP (x, 0)))
			 && IS_DAG2_MREG (REGNO (XEXP (x, 1))))))
	    ok = 1;
	else
	    ok = 0;
    }

    else if (GET_CODE (x) == POST_MODIFY
	     && REG_P (XEXP (x, 0))
	     && REG_OK_FOR_BASE_STRICT_P (XEXP(x, 0), addr_mode)
	     && (CONSTANT_P (XEXP (x, 1)) && CONSTANT_8bitP (XEXP (x, 1))))
	   {
	       ok = 1;
	   }
    /* The 21csp has mem i+m i+k */
    else if (GET_CODE (x) == PLUS
	     && addr_mode == DMmode
	     && REG_P (XEXP (x, 0))
	     && REG_OK_FOR_BASE_STRICT_P (XEXP (x, 0), addr_mode))
      {
	if (CONSTANT_8bitP (XEXP (x, 1)))
	  ok = 1;
	
	else if (REG_P (XEXP (x, 1))
		 && addr_mode == DMmode
		 && REG_OK_FOR_INDEX_STRICT_P (XEXP (x, 1), addr_mode)
		 && ((IS_DAG1_IREG (REGNO (XEXP (x, 0)))
		      && IS_DAG1_MREG (REGNO (XEXP (x, 1))))
		     ||	(IS_DAG2_IREG (REGNO (XEXP (x, 0)))
			 && IS_DAG2_MREG (REGNO (XEXP (x, 1))))))
	  ok = 1;
      }
  }
  return ok;
} /* go_if_legitimate_address_strict */

int
go_if_legitimate_address_non_strict (mode, x)
     enum machine_mode mode;
     rtx x;
{
  enum machine_mode addr_mode = GET_MODE(x);

  if (GET_CODE (x) == SYMBOL_REF)
    return 1;

  if ((REG_P(x) && REG_OK_FOR_BASE_NON_STRICT_P(x, addr_mode) &&
       (x != virtual_stack_vars_rtx)))
    return 1;

  if ((addr_mode != PMmode) && (CONSTANT_ADDRESS_P(x)))
    return 1;

  else if (POST_MODIFY_P(x))
    {
      rtx base, modifier;
      int base_ok, modifier_ok;
      
      base = XEXP(x,0);
      SWITCH_SUBREG(base);
      base_ok = 
	REG_P(base) && REG_OK_FOR_BASE_NON_STRICT_P(base, addr_mode);
      
      if (! base_ok) 
	return 0;

      if (GET_CODE (x) != POST_MODIFY)
	return 1;

      modifier = XEXP(x,1);
      SWITCH_SUBREG(modifier);

      modifier_ok =
	(REG_P(modifier) && REG_OK_FOR_MODIFY_NON_STRICT_P(modifier))
	  || (A21C0 && CONSTANT_P (modifier) && CONSTANT_8bitP (modifier));
      
      if (! modifier_ok)
	return 0;
      return 1;
    }

  else if (GET_CODE (x) == PLUS
	   && (addr_mode == DMmode || addr_mode == PMmode))
    {
      rtx base, index;
      base = XEXP (x, 0);
      index = XEXP (x, 1);
      SWITCH_SUBREG (base);
      SWITCH_SUBREG (index);
      /* Well its too late reload has run and our address is fudged so
	 lets allow index+base. */
      if (reload_completed
	  && REG_P (index) 
	  && REG_P (base)
	  && REG_OK_FOR_BASE_STRICT_P (index, addr_mode)
	  && REG_OK_FOR_INDEX_STRICT_P (base, addr_mode))
	{
	  rtx swapr = index;
	  index = base;
	  base = swapr;
	}

      /* The C0 has mem i+m i+k */
      if (REG_P (base)
	  && REG_OK_FOR_BASE_NON_STRICT_P (base, addr_mode))
	{
	  if (CONSTANT_8bitP (index))
	    return 1;
	  
	  if (REG_P (index)
	      && REG_OK_FOR_INDEX_NON_STRICT_P (index, addr_mode))
	    {
	      int b1 = REGNO (base);
	      int i1 = REGNO (index);
	      
	      if (b1 >= FIRST_PSEUDO_REGISTER
		  || i1 >= FIRST_PSEUDO_REGISTER)
		return 1;
	      
	      else if ((IS_DAG1_IREG (b1)
			&& IS_DAG1_MREG (i1))
		       || (IS_DAG2_IREG (b1)
			   && IS_DAG2_MREG (i1)))
		return 1;
	    }
	}
    }
  return 0;
}


int  hard_regno_mode_ok(int regno, enum machine_mode mode)
{
  int             ok = 0;
  
  switch (mode)
    {
    case PMmode:
    case DMmode:
      ok = TEST_HARD_REG_BIT (reg_class_contents [GENERAL_REGS], regno);
      break;
    case SImode:
      ok = TEST_HARD_REG_BIT (reg_class_contents [GENERAL_REGS /*CNTR_D_REGS*/], regno);
      break;
    case DImode:
    case SFmode:
    case DFmode:
      ok = hard_regno_mode_ok (regno, SImode)
	&& hard_regno_mode_ok (regno+1, SImode);
      break;
    default:
      break;
    }
  return ok;
}

int
legitimize_address (x, output_mode)
     rtx *x;
     enum machine_mode output_mode;
{
#ifdef A21C0
  int dec = 0, inc = 0;
  rtx new1,  new2;
  enum machine_mode addr_mode = GET_MODE (*x);

  if (0 && REG_P (*x))
    {

      /*rtx zero = gen_reg_rtx (GET_MODE (*x));*/
      *x = gen_rtx (PLUS, GET_MODE (*x), *x, const0_rtx);
      return 1;
    }
  /* Lets try and eliminate thoose dreadful hardwired mregs. */
  else if (((dec = GET_CODE (*x) == POST_DEC)
      || (inc = GET_CODE (*x) == POST_INC))
      && (REG_P (XEXP (*x, 0))))
    {
      rtx ireg = XEXP (*x, 0);
      enum machine_mode mode = GET_MODE (ireg);
      rtx const_reg = gen_reg_rtx (mode);

      emit_insn (gen_rtx (SET, mode, const_reg, 
			  (inc 
			   ? const1_rtx 
			   : (dec 
			      ? constm1_rtx
			      : (abort (), NULL_RTX)))));

      *x = gen_rtx (POST_MODIFY, mode, ireg, const_reg);
      return 1;
    }

  else if (GET_CODE (*x) == PLUS
	   && REG_P (XEXP (*x, 0))
	   && CONSTANT_P (XEXP (*x, 1))
	    )
    {
      rtx ireg = XEXP (*x, 0);
      enum machine_mode mode = GET_MODE (ireg);
      rtx const_reg = gen_reg_rtx (mode);

      emit_insn (gen_rtx (SET, mode, const_reg, XEXP (*x, 1)));
      *x = gen_rtx (PLUS, mode, ireg, const_reg);
      return 1;
    }

#ifdef A21C0_has_index_registers
  else if (GET_CODE (*x) == PLUS
	   && REG_P (XEXP (*x, 0)) 
	   && REG_P (XEXP (*x, 1))
	   && REG_OK_FOR_INDEX_STRICT_P (XEXP (*x, 0), addr_mode)
	   && REG_OK_FOR_BASE_STRICT_P (XEXP (*x, 1), addr_mode)

#if 0
	   && ((IS_DAG1_MREG (REGNO (XEXP (*x, 0))) 
		&& IS_DAG1_IREG (REGNO (XEXP (*x, 1))))
	       || (IS_DAG2_MREG (REGNO (XEXP (*x, 0))) 
		   && IS_DAG2_IREG (REGNO (XEXP (*x, 1)))))
#endif
	)

    {
      rtx temp = XEXP (*x, 0);
      XEXP (*x, 0) = XEXP (*x, 1);
      XEXP (*x, 1) = temp;
      return 1;
    }

#endif
  
#endif
  return 0;
}



    int
class_max_nregs(class, mode)
    enum reg_class class;
    enum machine_mode mode;
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
  rtx a,b;

  if (REG_P (x) && REGNO (x) > FIRST_PSEUDO_REGISTER
      && reg_equiv_mem[REGNO (x)])
    x = reg_equiv_mem[REGNO (x)];

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
	      if (!A21C0 && CONSTANT_ADDRESS_P(XEXP(x,0)))
		abort(); /* we've got trouble -- shouldn't be transferring to constant adress */
	      goto is_dm;
	    case DMmode:

	    is_dm:
	      if (CONSTANT_P (XEXP (x, 0)))
		if (!reg_class_subset_p (class, D_REGS))
		  return D_REGS;
		else
		  return NO_REGS;

	      if (A21C0)
		if (REG_P (XEXP (x,0)))
		  return NO_REGS;

		else if (REG_P (XEXP (XEXP (x, 0), 0))
			 && REG_P (XEXP (XEXP (x, 0), 1)))
		  return NO_REGS;

	      if(!reg_class_subset_p (class, D_REGS))
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
	      return  D_REGS;
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
#ifdef A21C0
	  /* This fails.... */
	  if (REG_P (XEXP (x, 0))
	      && REG_OK_FOR_BASE_STRICT_P (XEXP (x, 0), GET_MODE (x))
	      && CONSTANT_ADDRESS_P (XEXP (x, 1))
	      && ! CONSTANT_8bitP (XEXP (x, 1)))
	    return class == DAG1_IREGS /*IS_DAG1_IREG (REGNO (XEXP (x, 0)))*/ ? DAG1_MREGS : DAG2_MREGS;
	      
	  if (REG_P (XEXP (x, 0))
              && REG_OK_FOR_BASE_STRICT_P (XEXP (x, 0), GET_MODE (x))
              && CONSTANT_ADDRESS_P (XEXP (x, 1))
              && CONSTANT_8bitP (XEXP (x, 1)))
	    return DM_IREGS;


	  if (reg_class_subset_p (class, DM_IREGS))
	    if ((REG_P (XEXP (x,0)) && IS_DAG1_MREG (REGNO (XEXP (x,0)))) 
		|| (REG_P (XEXP (x,1)) && IS_DAG1_MREG (REGNO (XEXP (x,1)))))
	      return NO_REGS;
	    else 
	      return DAG2_IREGS;
	  else if(reg_class_subset_p (class, PM_IREGS))
	    if ((REG_P(XEXP(x,0)) && IS_DAG2_MREG(REGNO(XEXP(x,0)))) 
		|| (REG_P(XEXP(x,1)) && IS_DAG2_MREG(REGNO(XEXP(x,1)))))
	      return NO_REGS;
	    else
	      return DAG1_IREGS;
#else
	  if (reg_class_subset_p (class, DAG1_IREGS))
	    if((REG_P (XEXP (x,0)) && IS_DAG1_MREG (REGNO (XEXP (x,0)))) 
	       || (REG_P (XEXP (x,1)) && IS_DAG1_MREG (REGNO (XEXP (x,1)))))
	      return NO_REGS;
	    else
	      return DAG1_MREGS;
	  else if(reg_class_subset_p (class, DAG2_IREGS))
	    if((REG_P(XEXP(x,0)) && IS_DAG2_MREG(REGNO(XEXP(x,0)))) ||
	       (REG_P(XEXP(x,1)) && IS_DAG2_MREG(REGNO(XEXP(x,1)))))
	      return NO_REGS;
	    else
	      return DAG2_MREGS;
#endif
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
		  if (!REG_OK_FOR_BASE_STRICT_P(base, GET_MODE(addr)))     
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
		if (REG_OK_FOR_BASE_STRICT_P(addr, GET_MODE(addr)))
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
      break;
    default:
      return NO_REGS;
    }
  return NO_REGS;
}

int use_secondary_reload_patterns=1;


dm_incoming_reload(op, mode)
     rtx             op;
     enum machine_mode mode;
{
    int regno;
    rtx temp_op;

    if (! use_secondary_reload_patterns)
	return 0;

    /* Secondary reload register for memory */
    if (GET_CODE (op) == MEM
        && GET_CODE (XEXP (op, 0)) == PLUS
        && REG_P (XEXP (XEXP (op, 0), 0))
        && CONSTANT_P (XEXP (XEXP (op, 0), 1)))
        return 1;

#if 0

    if (GET_CODE (op) == MEM
        && GET_CODE (XEXP (op, 0)) == PLUS
        && REG_P (XEXP (XEXP (op, 0), 0))
        && REG_P (XEXP (XEXP (op, 0), 1))) 
    {
/*
	int reg0 = REGNO (XEXP (XEXP (op, 0), 0));
	int reg1 = REGNO (XEXP (XEXP (op, 0), 1));
	if (!SAME_DAG_REGS(reg0,reg1))
*/
	    return 1;
    }
#endif

    /* Secondary reload register for value of reg+const8 */
    if ((GET_CODE (op) == PLUS || GET_CODE (op) == POST_MODIFY)
	&& REG_P (XEXP (op, 0))
        && CONSTANT_8bitP (XEXP (op, 1))
	)
        return 1;

#if 0
    /* Secondary reload register for value of const */
    if ((GET_CODE (op) == PLUS || GET_CODE (op) == POST_MODIFY)
	&& REG_P (XEXP (op, 0))
        && CONSTANT_P (XEXP (op, 1))
	)
        return 1;
#endif

    /* Secondary reload register for a memory pseudo reg.
     * Move from memory to non-DREG requires a secondary register.
     * Perennial test 21c2004.c. 9/1/95 -- lev
     */
    temp_op = op;
    SWITCH_SUBREG (temp_op);
    if (GET_CODE(temp_op) == REG) {
	regno = REGNO(temp_op);
	if (IS_PSEUDO(regno) && reg_equiv_mem[regno] != 0)
	    return 1;
    }

    /* Secondary reload register for memory contents of dm(reg) */
    /* Verify whether this is needed at all, 9/1/95 --lev */
    if (GET_CODE (op) == MEM
        && (REG_P (XEXP (op, 0))))
	return 1;

    return 0;
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
	emit_insn (gen_rtx (SET, GET_MODE (scratch_reg), scratch_reg, from));
	emit_insn (gen_rtx (SET, GET_MODE (reg), reg, scratch_reg));
#ifndef A21C0
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
#endif
      }

	break;

      case PLUS:
	{
	  rtx a1 = XEXP (from,0);
	  rtx a2 = XEXP (from,1);
	  if (CONSTANT_P (a2)
	      && REG_P (a1) && REGNO (a1) == FRAME_POINTER_REGNUM) {
	    if (IS_MREG (REGNO (scratch_reg))) {
	      emit_insn (gen_rtx (SET, mode, scratch_reg, a2));
	      emit_insn (gen_rtx (SET, mode, reg, a1));
	      emit_insn (gen_rtx (SET, mode, reg, gen_rtx (PLUS, mode, reg, scratch_reg)));
	      break;
	    }
	  }
	}
	    
	
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
	  rtx addr = XEXP(eqx, 0),
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
#ifdef 0
	  emit_insn (gen_rtx (SET, mode, (MEM, mode, scratch_reg), 
			      interm_reg));
#endif
	  emit_insn (gen_rtx (SET, mode, scratch_reg, interm_reg));
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


int  reg_in_class(class, regno)
enum reg_class  class;
int             regno;
{
  return TEST_HARD_REG_BIT(reg_class_contents[class], regno);
}


enum call_modifier_tag
{
  MODIFY_D, MODIFY_DD
};


/* starting time is 15:32:04 */
/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: /usr/gcc/bin/gperf -D -p -g -o -t -N is_library_function 21libs.gperf  */
/* Command-line:
 /usr/gcc/bin/gperf -D -p -g -o -t -N is_library_function -k1,3,$ 21libs.gperf
 */
struct lib_func { char *name; int num_pptrs; int dountils; int df; int rp; enum call_modifier_tag modifier;  };

#define TOTAL_KEYWORDS 2
#define MIN_WORD_LENGTH 6
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 7
/* maximum key range = 2, duplicates = 0 */

#ifdef __GNUC__
inline
#endif
static unsigned int
hash (str, len)
     register char *str;
     register int unsigned len;
{
  static unsigned char asso_values[] =
    {
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     8, 8, 8, 8, 8, 8, 8, 8, 8, 1,
     8, 8, 8, 8, 8, 0, 8, 8, 8, 8,
     8, 0, 8, 8, 8, 8, 8, 8,
    };
  return len + asso_values[str[len - 1]] + asso_values[str[0]];
}

#ifdef __GNUC__
inline
#endif
struct lib_func *
is_library_function (str, len)
     register char *str;
     register unsigned int len;
{
  static struct lib_func wordlist[] =
    {
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"strcpy",   2, -1, 0, 0, MODIFY_DD},
      {"memcpy",   2, -1, 0, 0, MODIFY_DD},
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register char *s = wordlist[key].name;

          if (*s == *str && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}

/* ending time is 15:32:04 */

int is_function_a_library_routine(char *name)
{
    if (is_library_function(name,strlen(name)))
	return 1;
    return 0;
}

int does_function_have_polymorphic_property(char *name)
{
  struct lib_func *lf = is_library_function(name,strlen(name));
  if (lf)
    return lf->num_pptrs;
  return 0;
}


enum machine_mode
  extract_segment_pmode(x)
memory_segment  x;
{
  switch (x)
    {
    case PROG_MEM:
    /* pm is a synonym for dm on the 21csp */
    /*  return PMmode; */
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
  int rev=0;
  static expand_move_double_mem_to_reg(rtx, rtx, rtx);
  
  get_singles(to,   &to0,   &to1,   1, &rev);
  get_singles(from, &from0, &from1, 0, NULL);
  
  if (GET_CODE(to1) == SUBREG)
    to1   = gen_rtx (STRICT_LOW_PART, SImode, to1);
  
  if(rev)
    {
      rtx temp=from0;
      from0=from1;
      from1=temp;
    }
  emit_move_insn(to0, from0);
  emit_move_insn(to1, from1);
}

/*
  Take "big" (DI,SF) mode double_rtx, get to SImode rtx's
  for its components and put then in srtx0p and srtx1p.
  */

get_singles(double_rtx, srtx0p, srtx1p,  to, revp)
     rtx double_rtx, *srtx0p, *srtx1p;
     int to, *revp;
{
  switch (GET_CODE(double_rtx))
    {
    case REG:
      *srtx1p = gen_rtx (SUBREG, SImode, double_rtx, 1);
      *srtx0p = gen_rtx (SUBREG, SImode, double_rtx, 0);
      break;
    case CONST_DOUBLE:
      if(to)
	abort();
      else
	double_const_to_singles(double_rtx, srtx0p, srtx1p);
      break;
    case MEM:
      get_single_mems(double_rtx, srtx0p, srtx1p, revp);
      break;
    case CONST_INT:
      {
	int x=INTVAL(double_rtx);
	*srtx1p = GEN_INT((x<<16)>>16);
	*srtx0p = GEN_INT(x>>16);
      }
      break;
    case SUBREG:
      /* Granted, this looks pretty weird,
	 but if the source contains something like:
	 union { float f; int i; } u1;
	 then we'll end up generating (subreg:SF (reg:DI) 0)
	 and we need to handle it somehow, so...*/
      get_singles(SUBREG_REG(double_rtx), srtx0p, srtx1p, to, revp);
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
     int *revp;
     rtx double_rtx, *srtx0p, *srtx1p;
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
	  one=gen_reg_rtx (mode),
	  addr1=gen_reg_rtx (mode),
	  addr0;
/*
	emit_insn (gen_rtx (SET, mode, one, CONST1_RTX(mode)));
	emit_insn (gen_rtx (SET, mode, addr1, addr));
	emit_insn (gen_rtx (SET, mode, addr1, gen_rtx (PLUS, mode, addr1, one)));
	*srtx0p = gen_rtx (MEM, SImode, addr);
	*srtx1p = gen_rtx (MEM, SImode, addr1);
*/
	*srtx0p = gen_rtx (MEM, SImode, addr);
	*srtx1p = gen_rtx (MEM, SImode, gen_rtx (PLUS, mode, addr, CONST1_RTX (mode)));
      }
      break;
    case POST_MODIFY:
      {
	rtx
	  base     = XEXP(addr, 0), 
	  modifier = XEXP(addr, 1);
	if(REG_P(modifier) && 
	   IS_PSEUDO (REGNO (modifier)) &&
	   mod_reg_equiv_const &&
	   mod_reg_equiv_size >= REGNO (modifier) &&
	   mod_reg_equiv_const[REGNO (modifier)]==-1 &&
	   revp)
	  *revp=1;
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
#ifndef A21C0
	if (!CONSTANT_P(base) || ! CONSTANT_P(offset))
	  abort();
#endif
	*srtx0p = gen_rtx(MEM, SImode, addr);
	*srtx1p = gen_rtx(MEM, SImode, plus_constant(addr, 1));
      }
      break;
    default:
      abort();
    }
}

#include <math.h>

#if defined(__GNUDOS__) || defined(__WATCOMC__) || defined(NEED_ISINF)
/* 
  I copied this union definition and isnan() and isinf() from glibc-1.05.1
  on 20-May-1993.  I needed them in order to link on the pc since dgjpp's
  libm.a doesn't have them.
*/
union ieee754_double
  {
    double d;
    
    /* This is the IEEE 754 double-precision format.  */
    struct
      {
	/* Together these comprise the mantissa.  */
	unsigned int mantissa1:32;
	unsigned int mantissa0:20;
	unsigned int exponent:11;
	unsigned int negative:1;
      } ieee;
  };


/* paraphrased from glibc */
int isinf(double value)
{
  union ieee754_double u;
  u.d=value;
  /* An IEEE 754 infinity has an exponent with the
     maximum possible value and a zero mantissa.  */
  if ((u.ieee.exponent & 0x7ff) == 0x7ff &&
      u.ieee.mantissa0 == 0 && u.ieee.mantissa1 == 0)
    return u.ieee.negative ? -1 : 1;

  return 0;
}
int isnan(double value)
{
  union ieee754_double u;

  u.d = value;

  /* IEEE 754 NaN's have the maximum possible
     exponent and a nonzero mantissa.  */
  return ((u.ieee.exponent & 0x7ff) == 0x7ff &&
	  (u.ieee.mantissa0 != 0 || u.ieee.mantissa1 != 0));
}

#endif
void double_const_to_singles(double_rtx, srtx0p, srtx1p)
    rtx double_rtx, *srtx0p, *srtx1p;    
{
    REAL_VALUE_TYPE value;
    double fr;
    int exp;
    unsigned int high_int, low_int;
    unsigned int fr_int, sign;

    REAL_VALUE_FROM_CONST_DOUBLE(value, double_rtx);

    if(value==0)
      {
	*srtx1p= *srtx0p = const0_rtx;
	return;
      }
    if(isinf(value))
      {
	*srtx0p=gen_rtx(CONST_INT, VOIDmode, 0x7f80);
	*srtx1p=const0_rtx;
	return;
      }
    if(isnan(value))
      {
	*srtx0p=gen_rtx(CONST_INT, VOIDmode, 0x7fff);
	*srtx1p=constm1_rtx;
	return;
      }

    /* checkout X3.159-1989 (X3J11) section 4.5.4.2 */
    fr = frexp (value, &exp);
    exp--;
    /* quantize the mantissa */
    fr = ldexp (fr,24);

#if defined(__GNUDOS__) || defined(__WATCOMC__)
/*
    modf(fr+.5*((fr<0)?-1:1),&fr);
*/
    /* If compiled with g386 */
    modf(fr+.5,&fr);
#else
    modf(fr+copysign(0.5,fr),&fr);
#endif
  
    /* Check the exponent range) */
    if (exp > 127 || exp < -126)
	error("float constant out of range");
    sign = 0;
    if (fr < 0) 
	sign = 0x8000;
    fr_int = abs(fr);
    if (fr_int > 16777215) {
        fr_int = 16777215;
    }
    /* leave only 23 bits, get rid of the leading 1 */
    fr_int <<= 9;
    high_int = sign | ((exp + 127) << 7) | (fr_int >> 25);
    low_int =  (fr_int>>9) & 0xffff;
    *srtx0p = gen_rtx(CONST_INT, VOIDmode, high_int);
    *srtx1p = gen_rtx(CONST_INT, VOIDmode, low_int);
}

/* 
  This function returns okay if every register in class "class" okay
  for mode "mode".  Otherwise, it returns 0.
*/
int class_ok_for_mode (class, mode)
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
    is_move_2 = (t2 == TYPE_DMDREAD 
		 || t2 == TYPE_PMREAD 
		 || t2 == TYPE_DMPREAD
		 || t2 == TYPE_MOVE);
    if (! is_move_2) 
	return 0;
    return 1;

}


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
    if (t2 != TYPE_DMDREAD)
	return 0;
    if (t3 != TYPE_PMREAD)
	return 0;
    return 1;
}

void            select_section(tr, reloc)
    tree            tr;
    int             reloc;

{
    if (TREE_MEMSEG(tr) == PROG_MEM)
	pmda_section();
    else
	data_section();
}

my_abort()
{
  abort();
}


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
     *cp2=NULL;
  if((cp2=strchr(cp,'\n'))!=NULL)
     *cp2=NULL;

  if(!strcasecmp(cp,"inline_data"))
    error("#pragma inline_data is obsolete, use asm()");
  else if(!strcasecmp(cp,"inline"))
    error("#pragma inline is obsolete, use asm()");
  else if(!strcasecmp(cp,"reserved"))
    error("#pragma reserved is obsolete, try -mreserved");
  else if(!strcasecmp(cp,"ADSP2100"))
    error("#pragma adsp2100 is obsolete, try asm()");
  else
    {
      cp2=alloca(strlen(cp)+30);
      sprintf(cp2,"#pragma %s is not supported",cp);
      warning(cp2);
    }
  ungetc('\n',stream);
  return;
}
    
static rtx current_arg_pointer=NULL_RTX;
static int arg_pointer_base=-1;
static int current_offset;
int current_function_interrupt = 0;

void clear_better_rtx (rtx argp)
{
  if(REG_P(argp))
    {
      current_arg_pointer = gen_reg_rtx (GET_MODE (argp));
      emit_move_insn (current_arg_pointer, argp);
      arg_pointer_base= REGNO(argp);
      current_offset = 0;
    }
  else
      current_arg_pointer = NULL_RTX;
}
rtx find_better_rtx (rtx parm)
{
  int this_offset, this_base;

  if(GET_CODE (parm) == MEM &&
     REG_P (XEXP (parm,0)) &&
     current_arg_pointer &&
     REGNO (XEXP (parm,0))==arg_pointer_base)
    return gen_rtx (MEM, GET_MODE (parm), current_arg_pointer);
  
  if(GET_CODE (parm) == MEM &&
     GET_CODE (XEXP (parm,0)) == PLUS &&
     REG_P (XEXP (XEXP (parm,0),0)) &&
     CONST_INT_P (XEXP (XEXP (parm,0),1)))
    {
      this_offset = INTVAL (XEXP (XEXP (parm,0),1));
      this_base = REGNO (XEXP (XEXP (parm,0),0));

      if (!current_arg_pointer)
	{
	  arg_pointer_base=this_base;
	  current_offset = this_offset;
	  current_arg_pointer = gen_reg_rtx ( GET_MODE (XEXP(parm, 0)));
	  emit_move_insn (current_arg_pointer, GEN_INT( this_offset));
	  emit_move_insn (current_arg_pointer, 
			  gen_rtx (PLUS, 
				   GET_MODE(current_arg_pointer),
				   current_arg_pointer,
				   XEXP (XEXP (parm,0),0)));
	  current_offset = this_offset;
	}
      else
	{
	  rtx temp;
	  if (arg_pointer_base != this_base)
	    return validize_mem (parm);
	  temp=gen_reg_rtx(GET_MODE(current_arg_pointer));
	  emit_move_insn(temp, GEN_INT(this_offset - current_offset));
	  emit_move_insn(current_arg_pointer, 
			 gen_rtx(PLUS, GET_MODE(current_arg_pointer),
				 current_arg_pointer,
				 temp));
	  current_offset=this_offset;
	}
      return gen_rtx (MEM, GET_MODE (parm), current_arg_pointer);
    }
  return validize_mem (parm);
}
			 
user_defined_segment(tree decl)
{
    tree segment_node = GET_SEG_ATTR(decl);
    if (segment_node)
    {
	user_segment_name = IDENTIFIER_POINTER(segment_node);
    } else {
        user_segment_name = 0;
    }
}
  

/* Obvious,  count the number of machine instructions that
   are going to be generated by start, end */

int count_instructions_inclusive (rtx start, rtx end)
{
  rtx x;
  int count = 0;
  for (x = start; x == end; x = NEXT_INSN (x))
    {
      count += get_attr_length (x);
    }
  return count;
}

#if 0
print_regset (rs)
     HARD_REG_SET *rs;
{
  register HOST_WIDE_INT *scan_tp = *rs;
  register int i;
  int rn = 0;
  printf ("{");
  for (i = 0; i < HARD_REG_SET_LONGS; i++)
    {
      int b;
      for (b = 0; b < 32; b++, rn++)
	{
	  if (rn >= FIRST_PSEUDO_REGISTER)
	    break;
	  if (*scan_tp & 1<<b)
	    printf ("%s ", reg_names[rn]);
	}
      *scan_tp++;
    }
  printf ("}\n");
  return 0;
}
#endif

char *reg_cost_string = 0;
int   register_cost = 3;

void get_register_cost()
{
    if (reg_cost_string)
	if (sscanf(reg_cost_string, "%d", &register_cost) != 1)
	    error("Register Cost must be an integer");
}


/* Returns 1 if x1 and x2 are almost equal with possible exception of modes 
   This is a very limited function. It should work in the context for which 
   it has been written, i.e. peep-hole optimizing sequence x=y;y=x; where
   the two assignments can have different modes
   */
   
int
rtx_similar_p(rtx x1, rtx x2)
{
    char *fmt;
    int i;

    RTX_CODE code1 = GET_CODE(x1), code2 = GET_CODE(x2);
    if (code1 != code2)
	return 0;
    switch(code1) {
      case POST_MODIFY:
      case PRE_MODIFY:
	return 0;
      case REG:
	return REGNO(x1) == REGNO(x2);
      case CONST_INT:
	return INTVAL(x1) == INTVAL(x2);
      default: {
	  fmt = GET_RTX_FORMAT(code1);
	  for(i=GET_RTX_LENGTH(code1)-1; i>=0; i--) {
	      switch (fmt[i]) {
		case 'i':
		case 'n':
		  if (XINT(x1,i) != XINT(x2,i))
		      return 0;
		  break;
		case 'e':
		  if (! rtx_similar_p(XEXP(x1,i), XEXP(x2,i)))
		      return 0;
		  break;
		default:
		  return 0;
	      }
	  }
	  return 1;
      }
    }
}

/*
  Make sure that reg_alloc_order is complete.
*/

void complete_reg_alloc_order()
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

/*
  This predicate is a stricter version of "register_operand" -- it will
  not allow subreg's of double registers.
  This is useful for preventing the combiner from merging SUBREG's back
  into double-precision insns for adddi3 and mulsidi3.
  (az 2/11/94)
*/

int single_reg(op, mode)
    register rtx op;
    enum machine_mode mode;
{
    if (GET_MODE (op) != mode && mode != VOIDmode)
	return 0;
    if (GET_CODE(op) == REG)
	return 1;
    return 0;
}

int simple_reg_move_p(insn)
    rtx insn;
{
    rtx pattern, src, dest;
    if (GET_CODE(insn) != INSN)
	return 0;
    pattern = PATTERN(insn);
    if (GET_CODE(pattern) != SET)
	return 0;
    dest = SET_DEST(pattern);
    if (GET_CODE(dest) != REG)
	return 0;
    src  = SET_SRC(pattern);
    if (GET_CODE(src) != REG)
	return 0;
    return 1;
}


/**************************************************************************
  Walk the stream and change each (subreg:SI (reg:DI)) to a new pseudo reg.
  Abort if find a reg:DI outside subreg.
***************************************************************************/

static rtx *regmap;

static rtx get_reg_map (regno, word, mode)
    int regno, word;
    enum machine_mode mode;
{
    rtx sreg, (*rm)[][2];

    rm = (void*)regmap;
    sreg = (*rm)[regno][word];
    if (! sreg) {
	if (regno<FIRST_PSEUDO_REGISTER)
	    sreg = gen_rtx(REG, SImode, regno+word);
	else
	    sreg = gen_reg_rtx(SImode);
	(*rm)[regno][word] = sreg;
    }
    if (mode == SImode)
	return sreg;
    else
	return gen_rtx(SUBREG, mode, sreg, 0);
}

static int current_insn_code;

static int found_strict_low_part=0;

static rtx switch_the_subreg(x)
    rtx x;
{
    rtx newreg;
    enum machine_mode mode = GET_MODE(x);

    if (GET_CODE(x) == REG &&
	GET_MODE(x) == DImode)
	abort();
	    
    if (GET_CODE(x) == STRICT_LOW_PART) {
	/* We are changing the shape of the insn, 
	   so it will have to be re-recognized */
	found_strict_low_part = 1;
	current_insn_code = -1;
	return (switch_the_subreg(XEXP(x,0)));
    }

    if (GET_CODE(x) == SUBREG &&
	(GET_MODE(x) == SImode ||
	 GET_MODE(x) == DMmode ||
	 GET_MODE(x) == PMmode)) {
	rtx dreg = SUBREG_REG(x);
	int rn, w = SUBREG_WORD(x);

	/* Check case of two subregs of the same size modes PM/DM/SI */

	if (GET_MODE(dreg) != DImode)
	    return 0;

	if (GET_CODE(dreg) != REG)
	    abort();
	rn = REGNO(dreg);

	if (GET_MODE(dreg) != DImode)
	    return 0;
	newreg = get_reg_map(rn, w, mode);
	return newreg;
    }
    return 0;
}

static void switch_subregs_in_rtx(x)
    rtx x;
{
    unsigned int i,j;
    rtx el, new_rtx;
    char *fmt = GET_RTX_FORMAT(GET_CODE(x));
    for (i=0; i<GET_RTX_LENGTH(GET_CODE(x)); i++) {
	switch(fmt[i]) {
	  case 'e':
	    el = XEXP(x,i);
	    if (el) {
		if ((new_rtx = switch_the_subreg(el)))
		    XEXP(x,i) = new_rtx;
		else
		    switch_subregs_in_rtx(el);
	    }
	    break;
	  case 'E':
	  case 'V':
	    for(j=0; j<XVECLEN(x,i); j++) {
		rtx el = XVECEXP(x,i,j);
		if (el) {
		    if ((new_rtx=switch_the_subreg(el)))
			XVECEXP(x,i,j) = new_rtx;
		    else
			switch_subregs_in_rtx(el);
		}
	    }
	    break;
	  default:;
	}
    }
}

/* 
  Change clobber or use of a double reg to two clobbers of singles.
  We actually create two new insns and insert the AFTER the original one.
  There is a chance that this might create a problem in complicated 
  insns where it is important that clobber is in parallel with the rest
  of the stuff. At the moment (az 18/3/94) I can't think of anything like
  this. 

  Originally I tried to change CLOBBERs and USEs to a PARALLEL of two 
  new CLOBBER/USE patterns, but those are deleted by flow. Generally gcc 
  has a belief that such PARALLELs are no-op's.
*/

static void expand_double_side_effects (insn, pattern)
    rtx insn, pattern;
{
    rtx reg_rtx = XEXP(pattern, 0);
    if (GET_CODE(reg_rtx) == REG && 
	GET_MODE(reg_rtx) == DImode) {
	rtx new_pat, c1_rtx, c0_rtx, newreg0, newreg1;
	int rn = REGNO(reg_rtx);
	newreg0 = get_reg_map(rn,0,SImode);
	newreg1 = get_reg_map(rn,1,SImode);
	c0_rtx = gen_rtx(GET_CODE(pattern), SImode, newreg0);
	c1_rtx = gen_rtx(GET_CODE(pattern), SImode, newreg1);
	emit_insn_after(c1_rtx, insn);
	emit_insn_after(c0_rtx, insn);
    }
}

static void delete_reg_equality(rtx insn)
{
    rtx note;
    if ((note=find_reg_note (insn, REG_EQUAL, 0)))
	remove_note(insn, note);
    if ((note=find_reg_note (insn, REG_EQUIV, 0)))
	remove_note(insn, note);
}

void get_rid_of_double_regs (f)
    rtx f;
{
    rtx insn;
    int nregs = max_reg_num();
    int i,j;

    regmap = (rtx*) alloca(nregs * 2 * sizeof(rtx));
    for (i=0; i<nregs; i++)
	for(j=0; j<2; j++) regmap[2*i+j] = 0;
    
    for(insn=f; insn; insn=NEXT_INSN(insn)) {
	if (GET_CODE(insn) == INSN) {
	    rtx pattern = PATTERN(insn);
	    found_strict_low_part = 0;
	    current_insn_code = INSN_CODE(insn);
	    if (pattern && (GET_CODE (pattern) == CLOBBER ||
			    GET_CODE (pattern) == USE))
		expand_double_side_effects(insn, pattern);
	    else
		switch_subregs_in_rtx(pattern);
	    if (found_strict_low_part)
		delete_reg_equality(insn);
	    if (current_insn_code < 0)
		INSN_CODE(insn)= recog(PATTERN(insn), insn, NULL_PTR);
	}
    }
    regmap = 0;
}



static spill_area_offset = 0;
static rtx spill_base = 0;
#define SPILL_AREA_NAME "Register_Spill_Area_"

void init_spill_area()
{
    spill_base = gen_rtx(SYMBOL_REF, DMmode, SPILL_AREA_NAME);
    spill_area_offset = 0;
}

rtx assign_spill_area_slot(enum machine_mode mode, int size)
{
    rtx x, addr, offset;
    offset = gen_rtx(CONST_INT, VOIDmode, spill_area_offset);
    spill_area_offset += size;
    addr = gen_rtx(PLUS, DMmode, spill_base, offset);
    x = gen_rtx(MEM, mode, addr);
    return x;
}

void emit_spill_area()
{
    if (spill_area_offset > 0)
	fprintf(asm_out_file, ".var %s_[%d];\n", 
		SPILL_AREA_NAME, spill_area_offset);
}



/* 
  Return comparison code (i.e. LT,LTU.GEU,EQ, etc) of INSN.
  If INSN doesn't look like a conditional branch -- return UNKNOWN.
  Note: (CONST_INT 0) is not always equal (eq) to const0_rtx. (az 3/30/94).
*/

enum rtx_code get_comp_code(insn)
    rtx insn;
{
    rtx  body, dest, src, comp_rtx, comp_rhs, comp_lhs;
    enum rtx_code comp_code = UNKNOWN;

    body = PATTERN(insn);
    if (GET_CODE(body) != SET)
	return UNKNOWN;
    dest = SET_DEST(body);
    if (dest != pc_rtx)
	return UNKNOWN;
    src = SET_SRC(body);
    if (GET_CODE(src) != IF_THEN_ELSE)
	return UNKNOWN;
    comp_rtx = XEXP(src, 0);
    comp_code = GET_CODE(comp_rtx);
    comp_lhs = XEXP(comp_rtx,0);
    comp_rhs = XEXP(comp_rtx,1);
    
    if (comp_lhs != cc0_rtx)
	return UNKNOWN;
    if (GET_CODE (comp_rhs) != CONST_INT)
	return UNKNOWN;
    if (INTVAL (comp_rhs) != 0)
	return UNKNOWN;
    return comp_code;
}

int unsigned_comparison_p (rtx insn)
{
    rtx comp_insn = NEXT_INSN(insn);
    enum rtx_code comp_code;

    if (GET_CODE (comp_insn) != JUMP_INSN)
	abort();
    comp_code = get_comp_code(comp_insn);
    return (comp_code == LEU || comp_code == LTU ||
	    comp_code == GEU || comp_code == GTU);
}

#if 0
enum reg_class regno_reg_class[] = {
AX_F_REGS,	/* REG_AX0 0 */
AX_F_REGS,	/* REG_AX1 1 */
A_Y_REGS,	/* REG_AY0 2 */
A_Y_REGS,	/* REG_AY1 3 */
M_X_REGS,	/* REG_MX0 4 */
M_X_REGS,	/* REG_MX1 5 */
M_Y_REGS,	/* REG_MY0 6 */
M_Y_REGS,	/* REG_MY1 7 */
ALL_REGS,	/* REG_MR2 8 */
A_X_REGS,	/* REG_MR1 9 */
MR0_REG,	/* REG_MR0 10 */
ALL_REGS,	/* REG_MF  11 */
S_I_REGS,	/* REG_SI	12 */
S_E_REGS,	/* REG_SE	13 */
SR1_REG,	/* REG_SR1	14 */
A_X_REGS,	/* REG_SR0 15 */
A_R_REGS,	/* REG_AR  16 */
ALL_REGS,	/* REG_SB  17 */
ALL_REGS,	/* REG_PX  18 */
CNTR_REGS,	/* REG_CNTR 19 */
ALL_REGS,	/* REG_ASTAT 20 */
ALL_REGS,	/* REG_MSTAT 21 */
ALL_REGS,	/* REG_SSTAT 22 */
ALL_REGS,	/* REG_IMASK 23 */
ALL_REGS,	/* REG_ICNTL 24 */
ALL_REGS,	/* REG_TX0   25 */
ALL_REGS,	/* REG_TX1   26 */
ALL_REGS,	/* REG_RX0   27 */
ALL_REGS,	/* REG_RX1   28 */
ALL_REGS,	/* REG_IFC   29 */
ALL_REGS,	/* REG_MR	  30 */
ALL_REGS,	/* REG_SR	  31 */
DAG1_IREGS,	/* REG_I0  32 */
DAG1_IREGS,	/* REG_I1  33 */
DAG1_IREGS,	/* REG_I2  34 */
DAG1_IREGS,	/* REG_I3  35 */
DAG2_IREGS,	/* REG_I4  36 */
DAG2_IREGS,	/* REG_I5  37 */
DAG2_IREGS,	/* REG_I6  38 */
DAG2_IREGS,	/* REG_I7  39 */
DAG1_MREGS,	/* REG_M0  40 */
DAG1_MREGS,	/* REG_M1  41 */
DAG1_MREGS,	/* REG_M2  42  */
DAG1_MREGS,	/* REG_M3  43  */
DAG2_MREGS,	/* REG_M4  44  */
DAG2_MREGS,	/* REG_M5  45 */
DAG2_MREGS,	/* REG_M6  46  */
DAG2_MREGS,	/* REG_M7  47  */
GENERAL_REGS,	/* REG_L0  48 */
GENERAL_REGS,	/* REG_L1  49  */
GENERAL_REGS,	/* REG_L2  50  */
GENERAL_REGS,	/* REG_L3  51  */
GENERAL_REGS,	/* REG_L4  52  */
GENERAL_REGS,	/* REG_L5  53 */
GENERAL_REGS,	/* REG_L6  54  */
GENERAL_REGS,	/* REG_L7  55  */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#endif

emit_macdisi3 (operands, code)
     rtx *operands;
     RTX_CODE code;
{
  rtx oreg;
  if (GET_MODE (operands[0]) == DImode ) {

    if (GET_MODE (operands[1]) == SImode) {
      emit_insn (gen_rtx (SET, SImode,
			  gen_rtx (SUBREG, SImode, operands[0], 1),
			  operands[1]));
      operands[1] = operands[0];
    }

    emit_insn (gen_rtx (SET, VOIDmode, 
			operands[0],
			gen_rtx (code, DImode, 
				 operands[1],
				 gen_rtx (MULT, DImode,
					  operands[2],
					  operands[3]))));
  } else { /* SImode */
    oreg = gen_reg_rtx (DImode);
    emit_insn (gen_rtx (SET, SImode,
			gen_rtx (SUBREG, SImode, oreg, 1),
			operands[1]));

    emit_insn (gen_rtx (SET, DImode, 
			oreg,
			gen_rtx (code, DImode, 
				 oreg,
				 gen_rtx (MULT, DImode,
					  operands[2],
					  operands[3]))));
#if 0
    emit_insn (gen_rtx (SET, VOIDmode, 
			operands[0],
			gen_rtx (SUBREG, SImode, oreg, 1)));
#endif
  }
}


