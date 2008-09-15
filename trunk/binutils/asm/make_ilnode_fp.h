/* @(#)make_ilnode_fp.h	1.5 1/4/91 1 */


extern ILNODE *il;
extern ILNODE ilnode;

extern long mult_opcode( long type, long modifier );
extern void make_alu( long xreg, long yreg, long dest_reg, long opcode );
extern void make_shifter( long x_operand, long y_operand, long dest, long opcode, long shift_by );
extern void make_function_unit( long x_operand, long y_operand, long dest, long modifier,
                                long opcode );
extern void make_multiplier( long x_operand, long y_operand, long dest, long opcode, long modifier );
extern void make_transfer( long data_reg, long cu_reg, long direction );
extern int check_multifunction_type( long alu_opcode );
extern long get_multifunction_opcode( long multiplier_opcode, long alu_opcode, long compute_type );
extern int check_multifunction_registers( int mult_x, int mult_y, int alu_x, int alu_y, 
                                          int alu_opcode );

