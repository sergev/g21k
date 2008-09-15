/** Dump stuff indexed by reload number **/

#include "reload.c"

#define RLOOP for(r=0;r<n_reloads;r++)
#define f stderr
#define fp fprintf
static char* mark = "";
static char* reg_class_names[] = REG_CLASS_NAMES;

#define PRINT_RTX(X) {if(X)debug_rtx(X);else fp(f,"(NULL); ");}

#define HEAD(S) fp(f,"\n%s %s:\t",S,mark);


/* If secondary reloads are the same for inputs and outputs, define those
   macros here.  */

#ifdef SECONDARY_RELOAD_CLASS
#define SECONDARY_INPUT_RELOAD_CLASS(CLASS, MODE, X) \
  SECONDARY_RELOAD_CLASS (CLASS, MODE, X)
#define SECONDARY_OUTPUT_RELOAD_CLASS(CLASS, MODE, X) \
  SECONDARY_RELOAD_CLASS (CLASS, MODE, X)
#endif

/* If either macro is defined, show that we need secondary reloads.  */
#if defined(SECONDARY_INPUT_RELOAD_CLASS) || defined(SECONDARY_OUTPUT_RELOAD_CLASS)
#define HAVE_SECONDARY_RELOADS
#endif

/* See reload.c and reload1.c for comments on these variables.  */

/* Maximum number of reloads we can need.  */
#define MAX_RELOADS (2 * MAX_RECOG_OPERANDS * (MAX_REGS_PER_ADDRESS + 1))

extern rtx reload_in[MAX_RELOADS];
extern rtx reload_out[MAX_RELOADS];
extern rtx reload_in_reg[MAX_RELOADS];
extern enum reg_class reload_reg_class[MAX_RELOADS];
extern enum machine_mode reload_inmode[MAX_RELOADS];
extern enum machine_mode reload_outmode[MAX_RELOADS];
extern char reload_strict_low[MAX_RELOADS];
extern char reload_optional[MAX_RELOADS];
extern int reload_inc[MAX_RELOADS];
extern int reload_needed_for_multiple[MAX_RELOADS];
extern rtx reload_needed_for[MAX_RELOADS];
extern int reload_secondary_reload[MAX_RELOADS];
extern int reload_secondary_p[MAX_RELOADS];
#ifdef MAX_INSN_CODE
extern enum insn_code reload_secondary_icode[MAX_RELOADS];
#endif
extern int n_reloads;

extern rtx reload_reg_rtx[MAX_RELOADS];

char *reload_when_needed_name[] =
{
    "RELOAD_FOR_INPUT_RELOAD_ADDRESS",
    "RELOAD_FOR_OUTPUT_RELOAD_ADDRESS",
    "RELOAD_FOR_OPERAND_ADDRESS",
#ifdef DSP21XX
    "RELOAD_FOR_SECONDARY_INPUT",
    "RELOAD_FOR_SECONDARY_OUTPUT",
    "RELOAD_FOR_INPUT_MODIFY",
#endif
    "RELOAD_FOR_INPUT",
    "RELOAD_FOR_OUTPUT",
    "RELOAD_OTHER"
};

dri()
{
    int r;
    fp(f,"============================================================\n");
    HEAD("reload_in");			RLOOP PRINT_RTX(reload_in[r]);
    HEAD("reload_out");			RLOOP PRINT_RTX(reload_out[r]);
    HEAD("reload_reg_class");	      RLOOP fp(f,"%s, ",reg_class_names[reload_reg_class[r]]);
    HEAD("reload_inmode");		RLOOP fp(f, "%d, ", reload_inmode[r]);
    HEAD("reload_outmode");		RLOOP fp(f, "%d, ", reload_outmode[r]);
    HEAD("reload_optional");		RLOOP fp(f,"%s, ", reload_optional[r]?"yes":"no");
    HEAD("reload_inc");			RLOOP fp(f, "%d,", reload_inc[r]);
    HEAD("reload_in_reg");		RLOOP PRINT_RTX(reload_in_reg[r]);
    HEAD("reload_reg_rtx");		RLOOP PRINT_RTX(reload_reg_rtx[r]);
    HEAD("reload_nocombine");		RLOOP fp(f, "%d,", reload_nocombine[r]);
    HEAD("reload_needed_for");		RLOOP PRINT_RTX(reload_out[r]);
    HEAD("reload_needed_for_multiple");	RLOOP fp(f, "%d,", reload_needed_for_multiple[r]);
    HEAD("reload_when_needed");		
    RLOOP fp(f, "%s,",
	     reload_when_needed_name[reload_when_needed[r]]);
    HEAD("reload_secondary_reload");	RLOOP fp(f, "%d,", reload_secondary_reload[r]);
    HEAD("reload_secondary_p");		RLOOP fp(f, "%d,", reload_secondary_p[r]);
    HEAD("reload_secondary_icode");	RLOOP fp(f, "%d,", reload_secondary_icode[r]);
    fp(f, "\n");
}

d1ri(r)
    int r;
{
    fp(f,"============================================================\n");
    HEAD("reload_in");			 PRINT_RTX(reload_in[r]);
    HEAD("reload_out");			 PRINT_RTX(reload_out[r]);
    HEAD("reload_reg_class");	       fp(f,"%s, ",reg_class_names[reload_reg_class[r]]);
    HEAD("reload_inmode");		 fp(f, "%d, ", reload_inmode[r]);
    HEAD("reload_outmode");		 fp(f, "%d, ", reload_outmode[r]);
    HEAD("reload_optional");		 fp(f,"%s, ", reload_optional[r]?"yes":"no");
    HEAD("reload_inc");			 fp(f, "%d,", reload_inc[r]);
    HEAD("reload_in_reg");		 PRINT_RTX(reload_in_reg[r]);
    HEAD("reload_reg_rtx");		 PRINT_RTX(reload_reg_rtx[r]);
    HEAD("reload_nocombine");		 fp(f, "%d,", reload_nocombine[r]);
    HEAD("reload_needed_for");		 PRINT_RTX(reload_out[r]);
    HEAD("reload_needed_for_multiple");	 fp(f, "%d,", reload_needed_for_multiple[r]);
    HEAD("reload_when_needed");		
    fp(f, "%s,",
       reload_when_needed_name[reload_when_needed[r]]);
    HEAD("reload_secondary_reload");	 fp(f, "%d,", reload_secondary_reload[r]);
    HEAD("reload_secondary_p");		 fp(f, "%d,", reload_secondary_p[r]);
    HEAD("reload_secondary_icode");	 fp(f, "%d,", reload_secondary_icode[r]);
    fp(f, "\n");
}




/* The code below is needed only for testing register classes.
   (az 2/2/94)   */

#include "gvarargs.h"

#ifdef __USE_STDARGS__
static int test1class(enum reg_class rc, ...)
#else
static int test1class(va_alist)
    va_dcl
#endif
{
    va_list(p);
#ifndef __USE_STDARGS__
  enum reg_class rc;
#endif
    int rn;
    HARD_REG_SET expected;
#ifdef __USE_STDARGS__
  va_start(p, rc);
#else
  va_start(p);
  rc = va_arg(p, enum reg_class);
#endif

    CLEAR_HARD_REG_SET(expected);
    while ((rn = va_arg(p, int)) >= 0)
	SET_HARD_REG_BIT(expected, rn);
    GO_IF_HARD_REG_EQUAL(expected, reg_class_contents[rc], ok);
    printf("\n   Register class %s discrepancy:\n", reg_class_names[rc]);

    printf("Defined:  [");
    for(rn=0; rn<LAST_REG; rn++)
	if (TEST_HARD_REG_BIT(reg_class_contents[rc], rn))
	    printf("%s,", reg_names[rn]);

    printf("]\n");

    printf("Expected: [");
    for(rn=0; rn<LAST_REG; rn++)
	if (TEST_HARD_REG_BIT(expected, rn))
	    printf("%s,", reg_names[rn]);
    printf("]\n");

    return 0;
  ok:
    return 1;
}

test_reg_classes()
{

    test1class(A_R_REGS,   REG_AR,  -1);
    test1class(MR0_REG,    REG_MR0, -1);
    test1class(MR1_REG,    REG_MR1, -1);
    test1class(SR1_REG,    REG_SR1, -1);
    test1class(A_Y_REGS,   REG_AY0, REG_AY1, -1);
    test1class(AY_F_REGS,  REG_AY0, REG_AY1, -1);
    test1class(AX_F_REGS,  REG_AX0, REG_AX1, -1);
    test1class(FLOAT_REGS, REG_AX0, REG_AX1, REG_AY0, REG_AY1, -1);
    test1class(M_Y_REGS,   REG_MY0, REG_MY1, -1);
    test1class(AXNR_REGS,  REG_AX0, REG_AX1,         REG_MR0, REG_MR1, 
	       		   REG_SR0, REG_SR1, -1);
    test1class(A_X_REGS,   REG_AX0, REG_AX1, REG_AR, REG_MR0, REG_MR1, 
	       		   REG_SR0, REG_SR1, -1);
    test1class(M_X_REGS,   REG_MX0, REG_MX1, REG_AR, REG_MR0, REG_MR1,
	       		   REG_SR0, REG_SR1, -1);
    test1class(S_E_REGS,   REG_SE, -1);
    test1class(S_I_REGS,   REG_SI, REG_AR, REG_MR0, REG_MR1,
                           REG_SR0, REG_SR1, -1);
    test1class(ALU_REGS,   REG_AX0, REG_AX1, REG_AR, REG_MR0, REG_MR1,
                           REG_SR0, REG_SR1, REG_AY0, REG_AY1, -1);
    test1class(DNAR_REGS,  REG_AX0, REG_AX1, REG_AY0, REG_AY1,
	                   REG_MX0, REG_MX1, REG_MY0, REG_MY1, 
	                   REG_MR0, REG_MR1,
                           REG_SR0, REG_SR1, REG_SI, REG_AY0, REG_AY1, -1);
    test1class(D_REGS,     REG_AX0, REG_AX1, REG_AY0, REG_AY1, REG_AR,
	                   REG_MX0, REG_MX1, REG_MY0, REG_MY1, 
	                   REG_MR0, REG_MR1,
                           REG_SR0, REG_SR1, REG_SI, REG_AY0, REG_AY1, -1);
    test1class(DAG1_IREGS, REG_I0, REG_I1, REG_I2, REG_I3, -1);
    test1class(DAG1_MREGS, REG_M0, REG_M1, REG_M2, REG_M3, -1);
    test1class(DAG2_IREGS, REG_I4, REG_I5, REG_I6, REG_I7, -1);
    test1class(DAG2_MREGS, REG_M4, REG_M5, REG_M6, REG_M7, -1);
    test1class(DM_IREGS,   REG_I0, REG_I1, REG_I2, REG_I3, 
	                   REG_I4, REG_I5, REG_I6, REG_I7, -1);
    test1class(DM_MREGS,   REG_M0, REG_M1, REG_M2, REG_M3, 
	                   REG_M4, REG_M5, REG_M6, REG_M7, -1);
    test1class(DAG1_REGS,  REG_I0, REG_I1, REG_I2, REG_I3,
			   REG_M0, REG_M1, REG_M2, REG_M3, -1);
    test1class(DAG2_REGS,  REG_I4, REG_I5, REG_I6, REG_I7,
			   REG_M4, REG_M5, REG_M6, REG_M7, -1);
    test1class(M_OR_DAG1_IREGS, REG_I0, REG_I1, REG_I2, REG_I3,
				REG_M0, REG_M1, REG_M2, REG_M3,
				REG_M4, REG_M5, REG_M6, REG_M7, -1);
    test1class(DAG_REGS,   REG_I0, REG_I1, REG_I2, REG_I3,
			   REG_M0, REG_M1, REG_M2, REG_M3,
			   REG_I4, REG_I5, REG_I6, REG_I7,
			   REG_M4, REG_M5, REG_M6, REG_M7, -1);
    test1class(CNTR_REGS,   REG_CNTR, -1);
    test1class(CNTR_D_REGS, REG_CNTR,
	                    REG_AX0, REG_AX1, REG_AY0, REG_AY1, REG_AR,
	                    REG_MX0, REG_MX1, REG_MY0, REG_MY1, 
	                    REG_MR0, REG_MR1,
                            REG_SR0, REG_SR1, REG_SI, REG_AY0, REG_AY1, -1);
    test1class(GENERAL_REGS, REG_AX0, REG_AX1, REG_AR, 
	                     REG_MX0, REG_MX1, REG_MY0, REG_MY1,
	                     REG_MR0, REG_MR1,
	                     REG_SE,
	       		     REG_SR0, REG_SR1, REG_SI, REG_AY0, REG_AY1, 
	                     REG_I0, REG_I1, REG_I2, REG_I3,    
	                     REG_M0, REG_M1, REG_M2, REG_M3,        
                             REG_I4, REG_I5, REG_I6, REG_I7,
                             REG_M4, REG_M5, REG_M6, REG_M7, -1);
}

