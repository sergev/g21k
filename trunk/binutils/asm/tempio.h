/* @(#)tempio.h	1.4 1/4/91 1 */



/* A packing scheme is used when writing intermediate files.
 * The most important thing to know is the format of the file, which
 * is a packed word followed by a optional pointer to a symbol name, followed
   by a optional 32 bit value, 48-bit instruction or 40-bit data item.
 * The format of the packed word is as follows:
 *
 * bit nos:
 *   31   30-28   27-22   21-16      15-0
 *  
 * symbol unused  action  num_bits    16-bit value
 * ------ ------  ------  --------  ----------------
 *   x    xxx     xxxxxx   xxxxxx   xxxxxxxxxxxxxxxx
 *
 */

#define ACTION_MASK           0x0fc00000
#define PACK_ACTION(x)        (((unsigned long)(x) << 22) & ACTION_MASK)
#define UNPACK_ACTION(x)      ((ACTION_MASK & (x)) >> 22)

#define NUM_MASK              0x003f0000
#define PACK_NUM_BITS(x)      (((unsigned long) (x) << 16) & NUM_MASK)
#define UNPACK_NUM_BITS(x)    ((NUM_MASK & (x)) >> 16)
    
#define SYMBOL_BIT            0x10000000
#define SYMBOL_PRESENT(x)     ((x) & SYMBOL_BIT)
    
#define VALUE0                0x20000000   /* value is 0 and is not present */
#define VALUE16               0x40000000   /* value is in low order 16 bits */
#define VALUE32               0x80000000   /* value is the next 32-bits     */
#define PACK16(x)             (0x0000ffff & (unsigned long) (x))

#define IS_ZERO(x)                ((x) & VALUE0)
#define WILL_FIT_IN_16_BITS(x)    (((x) & 0xffff0000) == 0)
#define VALUE_IS_16_BITS(x)       ((x) & VALUE16)
#define VALUE_IS_32_BITS(x)       ((x) & VALUE32)

extern short num_open_files;
extern char *temp_file[];

extern FILE *temp_file_create( char *type );
extern void temp_file_write(char *ptr, short num_bytes, unsigned short action, SYMBOL *symbol);
extern void flush_temp_files( void );

#define WRITE_INTERMEDIATE( a, b, c, d ) temp_file_write( (char *) a, b, c, (SYMBOL *) d )
