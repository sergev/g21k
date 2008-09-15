#include "stat-md.h"

extern char *insn_name[];

main ()
{
    int rnum, n_used0 = 0, n_used1 = 0, n_unused = 0;

    read_stat_file();
    printf("Used rules:\n");
    for(rnum=0; rnum < MAX_INSN_CODE; rnum++) {
	int freq0 = rule_use[rnum][0];
	if (freq0 > 0) {
	    int freq1 =  rule_use[rnum][1];
	    if (freq1 != freq0)
		printf("  %-35s %8d (%d)\n",insn_name[rnum], freq0, freq1);
	    else printf("  %-35s %8d\n", insn_name[rnum], freq0);
	    n_used0++;
	    if (freq1) n_used1++;
	}
    }
    printf("Unused rules:\n");
    for(rnum=0; rnum < MAX_INSN_CODE; rnum++) {
	int freq0 = rule_use[rnum][0];
	if (freq0 == 0) {
	    printf("  %s\n", insn_name[rnum]);
	    n_unused++;
	}
    }
    printf("\nMatched: %d , Used: %d, Unused: %d, total: %d\n",
	   n_used0, n_used1, n_unused, MAX_INSN_CODE);

#define PERCENT(N) ((N) * 100.0 / MAX_INSN_CODE)

    printf("Matched: %2.0f%%, Used: %2.0f%%, Unused: %2.0f%%\n",
	   PERCENT(n_used0), PERCENT(n_used1), PERCENT(n_unused));
}


/*** Plugs for calls in insn-output.c ********/

int

asm_out_file,
asm_output_internal_label,
cc0_rtx,
cc_status,
comparison_operator,
compute_binaryop,
compute_unaryop,
const0_rtx,
constm1_rtx,
datareg_operand,
datreg_or_mr_operand,
emit_insn,
gen_CONSTANT_TO_REGISTER,
gen_Constant_To_Register,
gen_Decrement_Data_Reg,
gen_Decrement_Index_Reg,
gen_Increment_Data_Reg,
gen_Increment_Index_Reg,
gen_LOAD,
gen_LOAD_WITH_POST_MODIFY,
gen_LOAD_WITH_PRE_MODIFY,
gen_Load,
gen_Load_Post_Decrement1,
gen_Load_Post_Increment1,
gen_Load_Post_Modify1,
gen_Load_With_Pre_Modify,
gen_Load_With_Pre_Modify_Reverse,
gen_Negative_Decrement_Data_Reg,
gen_Negative_Increment_Data_Reg,
gen_REGISTER_TO_REGISTER,
gen_Register_To_Register,
gen_STORE,
gen_STORE_WITH_POST_MODIFY,
gen_STORE_WITH_PRE_MODIFY,
gen_Store,
gen_Store_Immediate,
gen_Store_Indirect,
gen_Store_Post_Modify1,
gen_Store_To_Constant_Address,
gen_Store_With_Dummy_Post_Modify,
gen_Store_With_Pre_Modify,
gen_Store_With_Pre_Modify_Reverse,
gen_abssf2,
gen_abssi2,
gen_adddf3,
gen_addsf3,
gen_addsi3,
gen_andsi3,
gen_ashlsi3,
gen_ashrsi3,
gen_beq,
gen_bge,
gen_bgeu,
gen_bgt,
gen_bgtu,
gen_ble,
gen_bleu,
gen_blt,
gen_bltu,
gen_bne,
gen_call,
gen_call_value,
gen_cmpdf,
gen_cmpsf,
gen_cmpsi,
gen_cmpstrsi,
gen_data_move_to_memory_postmodify,
gen_data_move_to_register_postmodify,
gen_decrement_register,
gen_Dual_Add_Sub,
gen_Dual_Sub_Add,
gen_extv,
gen_extzv,
gen_fix_truncsfsi2,
gen_fixuns_truncsfsi2,
gen_floatsisf2,
gen_increment_register,
gen_indirect_jump,
gen_insv,
gen_iorsi3,
gen_jump,
gen_label_rtx,
gen_lshrsi3,
gen_movdf,
gen_movdi,
gen_movhi,
gen_movsf,
gen_movsi,
gen_movstrsi,
gen_muldf3,
gen_mulsf3,
gen_mulsi3,
gen_multiply_and_increment,
gen_negdf2,
gen_neghi2,
gen_negsf2,
gen_negsi2,
gen_nop,
gen_one_cmplsi2,
gen_Parallel_Compare_Move,
gen_Parallel_Compute_Move,
gen_Parallel_Decrement_Move,
gen_Parallel_Increment_Move,
gen_Parallel_Move_Compute,
gen_Parallel_Move_Decrement,
gen_Parallel_Move_Increment,
gen_Parallel_Move_Unary,
gen_Parallel_Test_Move,
gen_Parallel_Unary_Move,
gen_reg_rtx,
gen_return,
gen_rtx,
gen_subdf3,
gen_subsf3,
gen_subsi3,
gen_tablejump,
gen_tstsf,
gen_tstsi,
gen_umulsi3,
gen_xorsi3,
general_operand,
hardwired_mregister,
immediate_operand,
immediate_or_datareg_operand,
immediate_or_indexreg_operand,
indexreg_operand,
memory_operand,
mod32_operand,
mod_or_const_operand,
mreg_or_6bit_operand,
nonmemory_operand,
output_asm_insn,
output_asm_label,
output_asm_return_insn,
post_incremented_operand,
post_modified_operand,
register_operand,
scratch_operand,
which_alternative,
gen_Try_Post_Modify,
modifyreg_operand,

gen_subdi3,
gen_adddi3,
gen_double_memory_to_reg,
gen_double_reg_to_memory,
gen_rotrsi3,
gen_rotlsi3,
gen_lshlsi3,
gen_sminsf3,
gen_sminsi3,
gen_smaxsf3,
gen_smaxsi3,
copy_double_memory_to_reg,
copy_double_reg_to_memory,

xxxxxx;
