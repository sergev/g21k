/* @(#) mem_adi.h 1.7@(#) 7/12/95 */
/*************************************************************************

   File:          mem.h

   Description:   Contains all defines and declarations for the memory 
                  function 

*************************************************************************/

/*************************************************************************

                        Memory Blocking Limits

*************************************************************************/

#if defined (ADSPZ3)
#define MEM_MAX_PM_BLOCK (335)				/* I changed MEM_MAX_MAP_BLOCKS        */
#define MEM_MAX_DM_BLOCK (400)              /* from 100 to 1000, I do not know     */
											/* if I need change those number too.  */
											/* they seems large enough.            */
											/* They were only used in              */
											/* gui_coff_load_file(), where do they */
                                            /* come from.          MODIFICATION_BL */
#else
#define MEM_MAX_PM_BLOCK (48)
#define MEM_MAX_DM_BLOCK (32)
#endif
/*************************************************************************

                        Memory types

*************************************************************************/

#define MEM_PM  (0)
#define MEM_DM  (1)

#ifdef ADSPZ3
#define MEM_MAX_PM_ADDRESS  (0xffffffffuL)
#else
#define MEM_MAX_PM_ADDRESS  (0x00ffffffuL)
#endif
#define MEM_MAX_DM_ADDRESS  (0xffffffffuL)


/*************************************************************************

                        Memory Map SPACE

*************************************************************************/

#ifdef ADSPZ3
typedef enum {
	MEM_EMPTY,
	MEM_IOP_REG,
	MEM_INTN_0,
	MEM_INTN_1,
	MEM_MULT_1,
	MEM_MULT_2,
	MEM_MULT_3,
	MEM_MULT_4,
	MEM_MULT_5,
	MEM_MULT_6,
        MEM_MULT_ALL,
	MEM_EXTN_0,
	MEM_EXTN_1,
	MEM_EXTN_2,
	MEM_EXTN_3,
	MEM_EXTN_NB
	} MEM_SPACE;

#endif 
/*************************************************************************

                        Memory Map Stuff 

*************************************************************************/

typedef enum {         /* Memory Access */
       MEM_NONE,
       MEM_RAM,
       MEM_ROM,
       MEM_PORT,
       } MEM_ACCESS;

/*************************************************************************

                        Memory Map Stuff 

*************************************************************************/

#define MEM_MAX_MAP_BLOCKS (1000)	/* 20 was too small for big ach files - mjj    */
									/* 100 was in Release 3.2, that is not enough  */
									/* for some people either, change to 1000 in   */
									/* Release 3.4				MODIFICATION_BL    */
  

typedef struct mem_map_block {  /* Memory Map Block */
       unsigned long start;
       unsigned long end;
       unsigned short access;
       unsigned short valid;
#ifdef ADSPZ3
       unsigned short width;
       MEM_SPACE	space;
#endif 
       } MEM_MAP_BLOCK;

#ifdef ADSPZ3
/*
typedef struct {
  unsigned long start;
  unsigned long end;
  unsigned short access;
  unsigned short type;
  unsigned short width;
  unsigned short space;
} GUI_CMD_COM_MEM_MAP_CHANGE;
*/
#endif 
