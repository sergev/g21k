/** Dump stuff indexed by reload number **/

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
