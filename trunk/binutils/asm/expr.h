/* @(#)expr.h	1.7 3/21/91 1 */


typedef struct
{
   short expr_type;
   SYMBOL *sym_ptr;
   union
   {
      long int_val;
      double float_val;
      unsigned char hex40_val[DM_WORD_SIZE];
   } value;

} EXPR;

#define EXPR_TYPE_INT        STYPE_ABS + 1
#define EXPR_TYPE_FLOAT      STYPE_ABS + 2
#define EXPR_TYPE_UNDEFINED  STYPE_ABS + 3
#define EXPR_TYPE_HEX40      STYPE_ABS + 4
#define HEX40_STRING_LENGTH  12

#define IS_INT(operand)      ((operand)->expr_type == EXPR_TYPE_INT)
#define IS_FLOAT(operand)    ((operand)->expr_type == EXPR_TYPE_FLOAT)
#define IS_HEX40(operand)    ((operand)->expr_type == EXPR_TYPE_HEX40)
#define IS_SYM_PTR(operand)  ((operand)->sym_ptr != (SYMBOL *) NULL)

#define INT_VAL(x)           ((x).value.int_val)
#define FLOAT_VAL            ((x).value.float_val)

#define EXPR_OP_AND            0x1L
#define EXPR_OP_OR             0x2L
#define EXPR_OP_XOR            0x3L
#define EXPR_OP_SHIFT_RIGHT    0x4L
#define EXPR_OP_SHIFT_LEFT     0x5L
#define EXPR_OP_MOD            0x6L

extern void  expression_add( EXPR *operand1, EXPR *operand2, EXPR *result );
extern void  expression_subtract( EXPR *operand1, EXPR *operand2, EXPR *result );
extern void  expression_multiply( EXPR *operand1, EXPR *operand2, EXPR *result );
extern void  expression_divide( EXPR *operand1, EXPR *operand2, EXPR *result );
extern void  expression_general( EXPR *operand1, EXPR *operand2, EXPR *result, long type );
extern short expression_coerce( EXPR *operand1, EXPR *operand2 );
extern void  expression_process_initializers( EXPR *expr );
extern void  expression_init_to_zero( long length, short word_size );
extern void  expression_hex_string_to_uns( char *string, void *buffer, int width );
