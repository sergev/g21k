/* @(#)opcodes.h	2.5 4/22/94 */

/* Definitions of different fields used in the 21000 instruction word */

#define G_DATA_MEMORY     0x0L
#define G_PROGRAM_MEMORY  0x1L

/* Memory Access Direction Codes */

#define D_READ           0x0L 
#define D_WRITE          0x1L

/* Cache Register Select Codes */

#define CS_UPPER         0x0L
#define CS_LOWER         0x1L
#define CS_ACAM          0x3L

/* Branch Type Codes */

#define B_JUMP           0x0L
#define B_CALL           0x1L

/* Jump Type Codes */

#define J_NON_DELAYED    0x0L
#define J_DELAYED        0x1L

/* Current Interrupt Clear Type Codes */

#define NO_CI_CLEAR      0x0L
#define CI_CLEAR         0x1L


/* Loop abort codes */

#define A_NO_LOOP_ABORT  0x0L
#define A_LOOP_ABORT     0x1L

/* Index Register Update Codes */

#define U_PRE_MODIFIED     0x0L
#define U_POST_MODIFIED    0x1L

/* Counter Select Codes */

#define C_LOOP_COUNTER   0x0L
#define C_EVENT_COUNTER  0x1L

/* Counter Increment Code */

#define INC_NO           0x0L
#define INC_YES          0x1L

/* Counter Decrement Code */

#define DEC_NO           0x0L
#define DEC_YES          0x1L

/* Computation unit <-> dreg transfer */

#define DREG_TO_CUREG       0x01L
#define CUREG_TO_DREG       0x00L

#define LOOP_STACK_PUSH       0x01L
#define INTERRUPT_STACK_PUSH  0x01L
#define PC_STACK_PUSH         0x01L

#define LOOP_STACK_POP        0x01L
#define INTERRUPT_STACK_POP   0x01L
#define PC_STACK_POP          0x01L

#define S_LOAD_READ_CACHE     0x00L
#define S_TRANSFER            0x01L

/* Computation Unit Selection Codes */

#define ALU              0x0L
#define MULTIPLIER       0x1L
#define SHIFTER          0x2L
#define FUNCTION_UNIT    0x3L
#define DONT_CARE        0xFFL

/* Bit Operation Select Codes */

#define BIT_SET          0x0L
#define BIT_CLEAR        0x1L
#define BIT_TOGGLE       0x2L
#define BIT_TEST         0x4L
#define BIT_XOR          0x5L

/* Used to set bits in the function unit opcodes */

#define FUNCTION_SINGLE_STEP          0x80L
#define FUNCTION_REGISTER_TRANSFER    0x40L
#define FUNCTION_PRECISION20          0x00L
#define FUNCTION_PRECISION24          0x01L
#define FUNCTION_PRECISION32          0x02L
#define FUNCTION_PRECISION40          0x03L

/* Used to set bits in the multiplier opcodes */

#define DESTINATION_REGISTER_FILE     0x00L
#define DESTINATION_MRF_OR_MRB        0x01L

#define SELECT_MRF                    0x00L
#define SELECT_MRB                    0x01L

#define MULTIPLIER_MRF_OR_MRB         0x04L
#define MULTIPLIER_MRB                0x02L

#define A_MASK                        0x04L
#define B_MASK                        0x02L
#define AB_MASK                       0xF9L
#define DESTINATION_IS_REG_FILE(x)    ((((x) & A_MASK) >> 2) == DESTINATION_REGISTER_FILE)
#define DESTINATION_IS_MRF_OR_MRB(x)  ((((x) & A_MASK) >> 2) == DESTINATION_MRF_OR_MRB)
#define SELECT_IS_MRF(x)              ((((x) & B_MASK) >> 1) == SELECT_MRF)
#define IS_FOREGROUND_MR(x)           ((((x) & B_MASK) >> 1) == SELECT_MRF)
#define MAKE_DEST_REG_FILE(opcode)    ((opcode) &= ~A_MASK)
#define MAKE_DEST_MR(opcode)          ((opcode) |= A_MASK)
#define MAKE_SELECT_MRF(opcode)       ((opcode) &= ~B_MASK)
#define MAKE_SELECT_MRB(opcode)       ((opcode) |= B_MASK)




/* Condition codes */

#define COND_EQ              0x00L
#define COND_LT              0x01L
#define COND_LE              0x02L
#define COND_AC              0x03L
#define COND_AV              0x04L
#define COND_MV              0x05L
#define COND_MS              0x06L
#define COND_SV              0x07L
#define COND_SZ              0x08L
#define COND_FLAG0_IN        0x09L
#define COND_FLAG1_IN        0x0AL
#define COND_FLAG2_IN        0x0BL
#define COND_FLAG3_IN        0x0CL
#define COND_TF              0x0DL
#define COND_ECE             0x0EL
#define COND_LCE             0x0FL
#define COND_NOT_LCE         0x0FL   /* Yes, this is correct */
#define COND_NE              0x10L
#define COND_GE              0x11L
#define COND_GT              0x12L
#define COND_NOT_AC          0x13L
#define COND_NOT_AV          0x14L
#define COND_NOT_MV          0x15L
#define COND_NOT_MS          0x16L
#define COND_NOT_SV          0x17L
#define COND_NOT_SZ          0x18L
#define COND_NOT_FLAG1_IN    0x19L 
#define COND_NOT_FLAG2_IN    0x1AL
#define COND_NOT_FLAG3_IN    0x1BL
#define COND_NOT_FLAG4_IN    0x1CL
#define COND_NOT_TF          0x1DL
#define COND_NOT_ECE         0x1EL
#define COND_TRUE            0x1FL
#define COND_FOREVER         0x1FL
#ifndef ADSPZ3
#define COND_BUS_MASTER      0x0EL
#define COND_NOT_BUS_MASTER  0x1EL
#endif /* ADSPZ3 */

/* Computation unit opcodes */


/* ALU */

#define IS_FP_ALU_OP(x)               ((((x) & 0x80L) || ((x) == FLOAT_DUAL_ADDSUB)) \
                                      && ((x) != FIXED_DUAL_ADDSUB))
#define IS_FIXED_ALU_OP(x)            ((!((x) & 0x80L) || ((x) == FIXED_DUAL_ADDSUB)) \
                                      && ((x) != FLOAT_DUAL_ADDSUB))

#define FIXED_ALU_ADD                         0x01L
#define FIXED_ALU_SUB                         0x02L
#define FIXED_ALU_ADD_X_Y_WITH_CARRY          0x05L
#define FIXED_ALU_SUB_WITH_BORROW             0x06L
#define FIXED_ALU_ADD_DIV_BY_2                0x09L
#define FIXED_ALU_COMP                        0x0AL
#define FIXED_ALU_ADD_X_WITH_CARRY            0x25L
#define FIXED_ALU_ADD_X_WITH_BORROW           0x26L
#define FIXED_ALU_INCREMENT                   0x29L
#define FIXED_ALU_DECREMENT                   0x2AL
#define FIXED_ALU_NEGATE                      0x22L
#define FIXED_ALU_ABS                         0x30L
#define FIXED_ALU_PASS                        0x21L
#define FIXED_ALU_AND                         0x40L
#define FIXED_ALU_OR                          0x41L
#define FIXED_ALU_XOR                         0x42L
#define FIXED_ALU_NOT                         0x43L
#define FIXED_ALU_MIN                         0x61L
#define FIXED_ALU_MAX                         0x62L
#define FIXED_ALU_CLIP                        0x63L


#define FP_ALU_ADD                            0x81L
#define FP_ALU_SUB                            0x82L
#define FP_ALU_ABS_X_PLUS_Y                   0x91L
#define FP_ALU_ABS_X_MINUS_Y                  0x92L
#define FP_ALU_ADD_DIV_BY_2                   0x89L
#define FP_ALU_COMP                           0x8AL

#define FP_ALU_NEGATE                         0xA2L
#define FP_ALU_ABS                            0xB0L
#define FP_ALU_PASS                           0xA1L
#define FP_ALU_RND                            0xA5L

#define FP_ALU_LOGB                           0xC1L
#define FP_ALU_SCALB                          0xBDL
#define FP_ALU_MANT                           0xADL
#define FP_ALU_FIX_2OP                        0xD9L
#define FP_ALU_FIX_1OP                        0xC9L
#ifdef ADSPZ3
#define FP_ALU_TRUNC_2OP                      0xDDL	/*New Instruction for Z3*/
#define FP_ALU_TRUNC_1OP                      0xCDL	/*New Instruction for Z3*/
#endif /* ADSPZ3 */
#define FP_ALU_FLOAT_2OP                      0xDAL
#define FP_ALU_FLOAT_1OP                      0xCAL
#define FP_ALU_RECIPS                         0xC4L
#define FP_ALU_RSQRTS                         0xC5L

#define FP_ALU_COPYSIGN                       0xE0L
#define FP_ALU_MIN                            0xE1L
#define FP_ALU_MAX                            0xE2L
#define FP_ALU_CLIP                           0xE3L

#ifndef ADSPZ3
#define FP_ALU_FPACK                          0x290L
#define FP_ALU_FUNPACK                        0x294L
#define FP_ALU_TRUNC                          0x0cdL
#define FP_ALU_TRUNC_BY                       0x0ddL
#endif /* ADSPZ3 */


/* Multiplier */

#define DEFAULT_MULT_MODIFIER               0x01L  /* This indicates the default modifier
                                                    * this is used by the assembler, not
                                                    * an actual modifier
                                                    */
#define MULTIPLIER_FIXED_MULT               0x01L  /* This is a type not a opcode */

#define MULTIPLIER_FIXED_MULT_SSI           0x70L
#define MULTIPLIER_FIXED_MULT_SUI           0x50L
#define MULTIPLIER_FIXED_MULT_USI           0x60L
#define MULTIPLIER_FIXED_MULT_UUI           0x40L
#define MULTIPLIER_FIXED_MULT_SSF           0x78L
#define MULTIPLIER_FIXED_MULT_SUF           0x58L
#define MULTIPLIER_FIXED_MULT_USF           0x68L
#define MULTIPLIER_FIXED_MULT_UUF           0x48L
#define MULTIPLIER_FIXED_MULT_SSIR          0x71L
#define MULTIPLIER_FIXED_MULT_SUIR          0x51L
#define MULTIPLIER_FIXED_MULT_USIR          0x61L
#define MULTIPLIER_FIXED_MULT_UUIR          0x41L
#define MULTIPLIER_FIXED_MULT_SSFR          0x79L
#define MULTIPLIER_FIXED_MULT_SUFR          0x59L
#define MULTIPLIER_FIXED_MULT_USFR          0x69L
#define MULTIPLIER_FIXED_MULT_UUFR          0x49L

#define MULTIPLIER_MULT_ACCUM_ADD           0x02L

#define MULTIPLIER_MULT_ACCUM_ADD_SSI       0xB0L
#define MULTIPLIER_MULT_ACCUM_ADD_SUI       0x90L
#define MULTIPLIER_MULT_ACCUM_ADD_USI       0xA0L
#define MULTIPLIER_MULT_ACCUM_ADD_UUI       0x80L
#define MULTIPLIER_MULT_ACCUM_ADD_SSF       0xB8L
#define MULTIPLIER_MULT_ACCUM_ADD_SUF       0x98L
#define MULTIPLIER_MULT_ACCUM_ADD_USF       0xA8L
#define MULTIPLIER_MULT_ACCUM_ADD_UUF       0x88L
#define MULTIPLIER_MULT_ACCUM_ADD_SSIR      0xB1L
#define MULTIPLIER_MULT_ACCUM_ADD_SUIR      0x91L
#define MULTIPLIER_MULT_ACCUM_ADD_USIR      0xA1L
#define MULTIPLIER_MULT_ACCUM_ADD_UUIR      0x81L
#define MULTIPLIER_MULT_ACCUM_ADD_SSFR      0xB9L
#define MULTIPLIER_MULT_ACCUM_ADD_SUFR      0x99L
#define MULTIPLIER_MULT_ACCUM_ADD_USFR      0xA9L
#define MULTIPLIER_MULT_ACCUM_ADD_UUFR      0x89L

#define MULTIPLIER_MULT_ACCUM_SUB           0x03L

#define MULTIPLIER_MULT_ACCUM_SUB_SSI       0xF0L
#define MULTIPLIER_MULT_ACCUM_SUB_SUI       0xD0L
#define MULTIPLIER_MULT_ACCUM_SUB_USI       0xE0L
#define MULTIPLIER_MULT_ACCUM_SUB_UUI       0xC0L
#define MULTIPLIER_MULT_ACCUM_SUB_SSF       0xF8L
#define MULTIPLIER_MULT_ACCUM_SUB_SUF       0xD8L
#define MULTIPLIER_MULT_ACCUM_SUB_USF       0xE8L
#define MULTIPLIER_MULT_ACCUM_SUB_UUF       0xC8L
#define MULTIPLIER_MULT_ACCUM_SUB_SSIR      0xF1L
#define MULTIPLIER_MULT_ACCUM_SUB_SUIR      0xD1L
#define MULTIPLIER_MULT_ACCUM_SUB_USIR      0xE1L
#define MULTIPLIER_MULT_ACCUM_SUB_UUIR      0xC1L
#define MULTIPLIER_MULT_ACCUM_SUB_SSFR      0xF9L
#define MULTIPLIER_MULT_ACCUM_SUB_SUFR      0xD9L
#define MULTIPLIER_MULT_ACCUM_SUB_USFR      0xE9L
#define MULTIPLIER_MULT_ACCUM_SUB_UUFR      0xC9L

#define MULTIPLIER_SATURATE                 0x04L

#define MULTIPLIER_SATURATE_SI              0x01L
#define MULTIPLIER_SATURATE_UI              0x00L
#define MULTIPLIER_SATURATE_SF              0x09L
#define MULTIPLIER_SATURATE_UF              0x08L

#define MULTIPLIER_RND                      0x05L

#define MULTIPLIER_RND_SF                   0x19L
#define MULTIPLIER_RND_UF                   0x18L

#define MULTIPLIER_CLR_MRF                  0x14L
#define MULTIPLIER_CLR_MRB                  0x16L

#define MULTIPLIER_FP_MULT                  0x30L


/* Shifter */

#define TBD                     0xFFL  /* To be determined */

#define SHIFTER_LSHIFT          0x00L
#define SHIFTER_OR_LSHIFT       0x20L
#define SHIFTER_ASHIFT          0x04L
#define SHIFTER_OR_ASHIFT       0x24L
#define SHIFTER_ROT             0x08L
#define SHIFTER_BCLR            0xC4L
#define SHIFTER_BSET            0xC0L
#define SHIFTER_BTGL            0xC8L
#define SHIFTER_BTST            0xCCL
#define SHIFTER_FDEP            0x44L
#define SHIFTER_FDEP_SIGN_EXT   0x4CL
#define SHIFTER_OR_FDEP         0x64L
#define SHIFTER_OR_FDEP_SIGN_EXT 0x6CL
#define SHIFTER_FEXT            0x40L
#define SHIFTER_FEXT_SIGN_EXT   0x48L
#define SHIFTER_FMERG           TBD   /* 21040 */
#define SHIFTER_PACK            TBD   /* 21040 */
#define SHIFTER_UNPACK          TBD   /* 21040 */
#define SHIFTER_UNPACK_NO_UP    TBD   /* 21040 */
#define SHIFTER_UNPACK_SIGN_EXT TBD   /* 21040 */
#define SHIFTER_UNPACK_NO_SIGN  TBD   /* 21040 */
#define SHIFTER_EXP             0x80L
#define SHIFTER_EXP_EXTENDED    0x84L
#define SHIFTER_LEFTZ           0x88L
#define SHIFTER_LEFTO           0x8CL
#define SHIFTER_NOFZ            TBD   /* 21040 */
#define SHIFTER_NOFO            TBD   /* 21040 */
#define SHIFTER_21040_INST(x)   ((x) == TBD)
#ifdef ADSPZ3
#define SHIFTER_FPACK                          0x90L	/*New Instruction for Z3*/
#define SHIFTER_FUNPACK                        0x94L	/*New Instruction for Z3*/
#endif /* ADSPZ3 */

/* Function unit */

#define FUNCTION_TRANSFER       0x00L
#define FUNCTION_DIVIDE         0x04L
#define FUNCTION_1_DIV_N        0x08L
#define FUNCTION_MOD            0x0CL
#define FUNCTION_SQR            0x10L
#define FUNCTION_SQR_1_DIV_N    0x14L
#define FUNCTION_EXP2           0x18L
#define FUNCTION_LOG2           0x1CL
#define FUNCTION_SIN            0x20L
#define FUNCTION_COS            0x24L
#define FUNCTION_ACT            0x28L
#define FUNCTION_STEP           0x80L

#define DREG_CU_TRANSFER        0x00L
#define FIXED_DUAL_ADDSUB       0x07L
#define FLOAT_DUAL_ADDSUB       0x0FL


/* Multifunction opcodes */

#define MULTI_FIXED_MULT_WITH_ADD                           0x04L
#define MULTI_FIXED_MULT_WITH_SUB                           0x05L
#define MULTI_FIXED_MULT_WITH_AVG                           0x06L
#define MULTI_MRF_MULACC_ADD_WITH_ADD                       0x08L
#define MULTI_MRF_MULACC_ADD_WITH_SUB                       0x09L
#define MULTI_MRF_MULACC_ADD_WITH_AVG                       0x0AL
#define MULTI_MULACC_ADD_WITH_ADD                           0x0CL
#define MULTI_MULACC_ADD_WITH_SUB                           0x0DL
#define MULTI_MULACC_ADD_WITH_AVG                           0x0EL
#define MULTI_MRF_MULACC_SUB_WITH_ADD                       0x10L
#define MULTI_MRF_MULACC_SUB_WITH_SUB                       0x11L
#define MULTI_MRF_MULACC_SUB_WITH_AVG                       0x12L
#define MULTI_MULACC_SUB_WITH_ADD                           0x14L
#define MULTI_MULACC_SUB_WITH_SUB                           0x15L
#define MULTI_MULACC_SUB_WITH_AVG                           0x16L
#define MULTI_FP_MULT_WITH_FP_ADD                           0x18L
#define MULTI_FP_MULT_WITH_FP_SUB                           0x19L
#define MULTI_FP_MULT_WITH_FP_FLOAT                         0x1AL
#define MULTI_FP_MULT_WITH_FP_FIX                           0x1BL
#define MULTI_FP_MULT_WITH_FP_AVG                           0x1CL
#define MULTI_FP_MULT_WITH_FP_ABS                           0x1DL
#define MULTI_FP_MULT_WITH_FP_MAX                           0x1EL
#define MULTI_FP_MULT_WITH_FP_MIN                           0x1FL

#define MULTI_MULT_WITH_DUAL_ADD_SUB                        0x02L
#define MULTI_FP_MULT_WITH_DUAL_ADD_SUB                     0x03L
