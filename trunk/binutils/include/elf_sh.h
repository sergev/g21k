/***************************************************************
 *
 * $Workfile: elf_sh.h $
 *
 * Description:	Contains defines and declarations for the SHARC.
 *
 * Copyright (c) 1996 Analog Devices Inc., All Rights Reserved
 *
 * $Log: /Mainline/DebugTools/SHARC (3.3 Release)/INCLUDE/elf_sh.h $
 * 
 * 1     6/02/98 5:57p Mmcguir
 * Checked in patch version that includes ELF support.
 * 
 * 4     9/26/97 9:34a Skafka
 * Add the COFFish header flags which will be needed by the debugger.
 * SHF_TEXT,  EF_RTS, EF_PCS,  EF_ASM,  EF_ANSIC, etc.
 * 
 * 3     8/04/97 11:22a Aluo
 * Add header.
 * 
 ***************************************************************/

#define SHARC_MAGIC_NUM 0x25d6cc1aL



// section flags;
#define SHF_PM          0x00000008L
#define SHF_DM          0x00000010L
#define SHF_RAM	        0x00000020L
#define SHF_ROM         0x00000040L
#define SHF_16          0x00000080L
#define SHF_32          0x00000100L
#define SHF_40          0x00000200L
#define SHF_48          0x00000400L
#define SHF_TEXT		0x00000800L

// elf header flags - needed primarily during the COFF2ELF
// conversion
#define EF_RTS      0x0010 /* C run-time stack location: set for PM, clear for DM       */
#define EF_PCS      0x0020 /* PC-stack location: set for external, clear for internal   */
#define EF_ASM      0x0040 /* source code is assembly                                   */
#define EF_ANSIC    0x0080 /* set if source is ANSI-C, clear if not                     */

#define EF_DSPC     0x0100 /* source code is DSP-C                                      */
#define EF_DAD	    0x0200 /* set if doubles-are-doubles, clear if doubles-are-floats   */
#define EF_REGPARMS 0x0400 /* set if using register parameters, clear if not            */
#define EF_GCC      0x0800 /* set if compiler is g21k, clear if not                     */

 