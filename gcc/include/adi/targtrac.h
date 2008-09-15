#ifndef _TARGET_TRACE_H
#define _TARGET_TRACE_H

#ifndef _MACHSTATE_H
#include <adi/types.h>
#endif

#ifndef TARGET_BLOCK_SIZE
#define TARGET_BLOCK_SIZE 2048
#endif

typedef unsigned long  address_t;
typedef unsigned long TARGETID;
typedef unsigned long (* TARGETTRACE_PROC)(TARGETID tid,long op, ...);

#ifndef DLLINTERFACE
#define DLLINTERFACE
#endif

TARGETTRACE_PROC DLLINTERFACE TargetAPIinit (void);

#define MEM24BIT 1L

typedef struct tgt_break_point {
  address_t addr;
  int       kind;
  /* The user interface is responsible for allocating this via 
     sizeof (tgt_break_point) + tgt_target_ops->needbpsize */
  char      bpdata[1];  
} tgtbp_t;


/* Error Conditions */
#define ENOTATTACHED -1
#define EUNSUPPORTED  0
#define EOK           1
#define EWAITING      2

/* @section Network Escapes
*/
#define TGT_NETWORK_IOSTREAM 0xFEADFEAD
#define TGT_NETWORK_CONNECT  0xFFFF0001
#define TGT_NETWORK_CLOSE    0xFFFF0002
#define TGT_NETWORK_SHUTDOWN 0xFFFF0003

/* The target Messages */
typedef enum {
#define PROTOCOL(message, enum, ops, proto) message = enum,
#include <adi/protocol.def>
#undef PROTOCOL
} targtrac_enum;



#endif

