typedef enum
{
  BUS_FREE,             /* unallocated */
  BUS_DMA_ADDR,         /* DMA address group */
  BUS_DMA_BUFF,         /* DMA data buffer group */
  BUS_EPA,              /* EP address */
  BUS_EPD,              /* EP data */
  BUS_EXTN_ADDR,        /* external address (pma, epa, or dma) */
  BUS_EXTN_DATA,        /* external data (pmd, epd, or dmd) */
  BUS_SNAFU,            /* something's wrong if we get this */
  Z3_CORE
}                       Bus_Name;

typedef struct
{
  Bus_Name                name;
  Bus_Name                locked_by;
}                       Bus_Obj;

