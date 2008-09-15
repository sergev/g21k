/* @(#)linker_aux.h	1.4  6/11/91 */

typedef struct
{
    AUXENT aux;            /* COFF auxillary entry */
    int    pos;            /* Position of entry in list */
    long   acid;           /* access id */
    long   chain_id;       /* Next aux entry in list */
} LINKER_AUX;

#define LINKER_AUX_SIZE  sizeof(LINKER_AUX)
