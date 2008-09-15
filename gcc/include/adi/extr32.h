#ifndef EF

#ifndef HOST_LONG_WORD_SIZE
#define HOST_LONG_WORD_SIZE    32L
#endif

#ifndef BIT_PER_INSTRUCTION
#define BIT_PER_INSTRUCTION    23L
#endif

#define EF(inst, p1, p2)\
     (((inst) & (((1L<<(((long)(p1)-(long)(p2))+1L))-1L)\
        << (long)(p2))) >> (long)(p2))

/* ExtractBit (bit) */
#define EB(inst, p) EF(inst, p, p)

/* ExtractFieldExtend (msb,lsb) */
#define EFE(inst,p1, p2)                   \
     ((EF (inst,p1,p2)                     \
       << (HOST_LONG_WORD_SIZE - ((long)(p1)-(long)(p2)+1L))) \
      >> (HOST_LONG_WORD_SIZE - ((long)(p1)-(long)(p2)+1L)))

#define _GETB24(x) (((((long)(x)[0]<<8l)|(long)(x)[1])<<8l)|(long)(x)[2])
#define GETB24(x) _GETB24(((unsigned char *)(x)))

#define _GETL24(x) (((((long)(x)[2]<<8l)|(long)(x)[1])<< 8l)|(long)(x)[0])
#define GETL24(x) _GETL24(((unsigned char *)(x)))
#endif
