#define fp fprintf
#define f stderr

/** Using macro for get_last_num to accomodate arrays of different types */

#define GET_LAST_NUM(ARR,LEN,LVAL,RET) {int l__;RET=LEN-1;for(l__=LEN-1; l__>=0; l__--)\
	if(ARR[l__]==LVAL)RET=l__;else break;}

#define PRINT_RTX(X) {if(X)debug_rtx(X);else fp(f,"(NULL), ");}


#define PH_MAP(ARR) {int r;for(r=0;r<FIRST_PSEUDO_REGISTER;r++)\
			 if(ARR[r]>=0)fp(f,"%d->%d, ",r,ARR[r]);}
#define PH_REG(N)fp(f,N>0?reg_names[N]:"-1")

#define PH_REGS(ARR,L) {int r__,rmax__;GET_LAST_NUM(ARR,\
			   FIRST_PSEUDO_REGISTER,L,rmax__);\
			  for(r__=0;r__<=rmax__;r__++)\
			  {PH_REG(ARR[r__]);fp(f,",");}\
			      if(rmax__<FIRST_PSEUDO_REGISTER-1)fp(f,"...");fp(f,"\n");}



#define PH_RTX(ARR) {int r__,rmax__;GET_LAST_NUM(ARR,FIRST_PSEUDO_REGISTER,0,rmax__);\
			  for(r__=0;r__<=rmax__;r__++)PRINT_RTX(ARR[r__]);\
			      if(rmax__<FIRST_PSEUDO_REGISTER-1)fp(f,"...");}

#define PH_REGS0(ARR,L) {if(ARR)PH_REGS(ARR,L) else fp(f,"NULL\n");}
#define PH_RTX0(ARR)    {if(ARR)PH_RTX(ARR)    else fp(f,"NULL\n");}



#define HEAD(S) fp(f,"%s %s:\t",mark,S)

static char* mark = "=== ";

dri1()
{
    
    HEAD("reg_reloaded_contents");	PH_REGS(reg_reloaded_contents,0);
    HEAD("reg_reloaded_insn");		PH_RTX(reg_reloaded_insn);
    HEAD("spill_reg_rtx");		PH_RTX(spill_reg_rtx);
    HEAD("spill_reg_order");		PH_MAP(spill_reg_order);
    HEAD("spill_reg_store");		PH_RTX(spill_reg_store);
    HEAD("spill_regs");			PH_REGS(spill_regs,0);
    HEAD("potential_reload_regs");	PH_REGS(potential_reload_regs,0);
    HEAD("regs_explicitly_used");	PH_REGS(regs_explicitly_used,0);
    HEAD("spill_stack_slot");		PH_RTX(spill_stack_slot);
    HEAD("spill_stack_slot_width");	PH_REGS(spill_stack_slot_width,0);

    dump_reload_hard_sets();

    dri11();

    fp(f,"\n");
}


#define DHS(NAME,SET) {HEAD(NAME); pclass(SET);}

void pclass();

dump_reload_hard_sets()
{
    DHS("reg_is_output_reload",reg_is_output_reload);
    DHS("forbidden_regs",forbidden_regs);
    DHS("bad_spill_regs",bad_spill_regs);
    DHS("counted_for_groups",counted_for_groups);
    DHS("counted_for_nongroups",counted_for_nongroups);
}

/** the following is dynamically allocated in reload as needed **/

dri11 ()
{
    HEAD("reg_has_output_reload");	PH_REGS0(reg_has_output_reload, 0);
    HEAD("reg_max_ref_width");		PH_REGS0(reg_max_ref_width, 0);

    HEAD("reg_last_reload_reg");	PH_RTX0(reg_last_reload_reg);
    HEAD("reg_equiv_constant");		PH_RTX0(reg_equiv_constant);
    HEAD("reg_equiv_memory_loc");	PH_RTX0(reg_equiv_memory_loc);
    HEAD("reg_equiv_address");		PH_RTX0(reg_equiv_address);
    HEAD("reg_equiv_mem");		PH_RTX0(reg_equiv_mem);
    HEAD("reg_equiv_init");		PH_RTX0(reg_equiv_init);
}

/***
char *reg_has_output_reload;
int *reg_max_ref_width;
rtx *reg_equiv_address;
rtx *reg_equiv_constant;
rtx *reg_equiv_init;
rtx *reg_equiv_mem;
rtx *reg_equiv_memory_loc;
rtx *reg_last_reload_reg;
***/


extern char *reg_names[];

void
pclass(hrs)
    HARD_REG_SET hrs;
{
    int rnum, first = 1;
    fprintf(stderr, "[");
    for(rnum=0; rnum<=LAST_REG; rnum++) {

	if (TEST_HARD_REG_BIT(hrs,rnum))
	{
	    if (!first)
		fprintf(stderr, ",");
	    fprintf(stderr, reg_names[rnum]);
	}
	first = 0;
    }
    fprintf(stderr, "]");
}
