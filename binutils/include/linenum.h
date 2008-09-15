
struct lineno
{
    union                       /* if l_lnno == 0... */
    {
	long    l_symndx;       /* then use l_symndx, */
	long    l_paddr;        /* else use l_paddr. */
    } l_addr;
    unsigned short  l_lnno;     /* line number */
} PACKED;

#define     LINENO  struct lineno
#define     LINESZ  sizeof(LINENO)

