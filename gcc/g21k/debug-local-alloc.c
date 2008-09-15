#include "local-alloc.c"

#define fp fprintf
#define f  stderr
static char *mark = "";
static char *reg_class_names[] = REG_CLASS_NAMES;

#define PRINT_RTX(X) {if(X)debug_rtx(X);else fp(f,"(NULL); ");}
#define HEAD(S) fp(f,"\n%s %s:\t",S,mark);

#define MODESTR(M) (((M)==SImode)?"SI":((M)==PMmode)?"PM":((M)==DMmode)?"DM":\
((M)==SFmode)?"SF":((M)==VOIDmode)?"VOID":"???")

d1q(q)          /* display a quantity */
     int q;
{
    fp(f,"============================================================\n");
    HEAD("qty_phys_reg");			 PRINT_RTX(qty_phys_reg[q]);
    HEAD("qty_phys_copy_sugg");			 fp(f, "%x", qty_phys_copy_sugg[q]);
    HEAD("qty_phys_sugg");			 fp(f, "%x", qty_phys_sugg[q]);
    HEAD("qty_phys_has_copy_sugg");		 fp(f, "%x", qty_phys_has_copy_sugg[q]);
    HEAD("qty_phys_has_sug");			 fp(f, "%x", qty_phys_has_sugg[q]);
    HEAD("qty_n_refs");                          fp(f, "%d", qty_n_refs[q]);
    HEAD("qty_min_class");                       fp(f, "%s", reg_class_names[qty_min_class[q]]);
    HEAD("qty_birth");                           fp(f, "%i", qty_birth[q]);
    HEAD("qty_death");                           fp(f, "%i", qty_death[q]);
    HEAD("qty_size");                            fp(f,  "%i", qty_size[q]);
    HEAD("qty_mode");                            fp(f, "%s", MODESTR(qty_mode[q]));
    HEAD("qty_n_calls_crossed");                 fp(f, "%i", qty_n_calls_crossed[q]);
    HEAD("qty_alternate_class");                 fp(f, "%s", reg_class_names[qty_alternate_class[q]]);
    HEAD("qty_scratch_rtx");			 PRINT_RTX(qty_scratch_rtx[q]);
    HEAD("qty_first_reg");			 fp(f, "%i (follow reg_next_in_qty)", qty_first_reg[q]);
  }

d1r(r)
     int r;    /* display a register */
{
    fp(f,"============================================================\n");
    HEAD("reg_qty");                              fp(f, "%i",       reg_qty);
    HEAD("reg_n_sets");                           fp(f, "(%i, %i)", reg_n_sets[2*r], reg_n_sets[2*r+1]);
    HEAD("reg_n_deaths");                         fp(f, "(%i, %i)", reg_n_deaths[2*r], reg_n_deaths[2*r+1]);
    HEAD("reg_live_length");                      fp(f, "(%i, %i)", reg_live_length[2*r], reg_live_length[2*r+1]);
    HEAD("reg_n_calls_crossed");                  fp(f, "(%i, %i)", reg_n_calls_crossed[2*r], reg_n_calls_crossed[2*r+1]);
    HEAD("reg_basic_block");                      fp(f, "(%i, %i)", reg_basic_block[2*r], reg_basic_block[2*r+1]);
  }

d1bb(b)
     int b;   /* display a basic block */
{
    fp(f,"============================================================\n");
    HEAD("basic_block_head");                     PRINT_RTX(basic_block_head[b]);
    HEAD("basic_block_end");                      PRINT_RTX(basic_block_end[b]);
}
