/* This file contains the interrupt table for the ADSP-21060				*/

/* When the C program exits either by returning from main() or by an 			*/
/* explicit or implicit call to exit(), control will transfer to the 			*/
/* label ___lib_prog_term.  Currently, the ___lib_prog_term label is 			*/
/* defined at the end of the reset vector as an IDLE instruction.    			*/
/* If your application needs to perform some operation AFTER the C   			*/
/* program has finished executing, remove the ___lib_prog_term label 			*/
/* from the runtime header, and place it at the beginning of your    			*/
/* code.							     			*/

/* Done by AS on 6/94 to adapt to new fast/super fast dispatchers written		*/


.GLOBAL			___lib_prog_term;		    	/*Termination address	*/
.EXTERN			___lib_setup_c;

.SEGMENT/PM	    	seg_rth;				/*Runtime header segment*/

			NOP;NOP;NOP;NOP;			/*Reserved interrupt	*/

___lib_RSTI:	 	NOP;					/* Not really executed	*/
			CALL ___lib_setup_c;			/* Setup C runtime model*/
			JUMP _main;				/* Begin C program	*/
___lib_prog_term:	JUMP ___lib_prog_term;			/* Stay at this label	*/

			NOP;NOP;NOP;NOP;			/*Reserved interrupt	*/

/* Interrupt vector for status stack/loop stack overflow or PC stack full: 		*/
___lib_SOVFI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+3*6;		/*Base of int table	*/
			
/* Interrupt vector for high priority timer interrupt: */
___lib_TMZHI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+4*6;		/*Base of int table	*/

/* Interrupt vector for external interrupts:*/
___lib_VIRPTI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+5*6;		/*Base of int table	*/
			
___lib_IRQ2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+6*6;		/*Base of int table	*/

___lib_IRQ1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+7*6;		/*Base of int table	*/

___lib_IRQ0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+8*6;		/*Base of int table	*/

			NOP;NOP;NOP;NOP;			/*Reserved interrupt	*/

/* Interrupt vectors for Serial port DMA channels: */
___lib_SPR0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+10*6;		/*Base of int table	*/

___lib_SPR1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+11*6;		/*Base of int table	*/

___lib_SPT0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+12*6;		/*Base of int table	*/

___lib_SPT1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+13*6;		/*Base of int table	*/

/* Interrupt vectors for link port DMA channels: */
___lib_LP2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+14*6;		/*Base of int table	*/

___lib_LP3I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+15*6;		/*Base of int table	*/

/* Interrupt vectors for External port DMA channels: */
___lib_EP0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+16*6;		/*Base of int table	*/

___lib_EP1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+17*6;		/*Base of int table	*/

___lib_EP2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+18*6;		/*Base of int table	*/

___lib_EP3I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+19*6;		/*Base of int table	*/

/* Interrupt vector for Link service request	*/
___lib_LSRQ:		BIT CLR MODE1 0x1000;                   /*Disable interrupts    */
                        JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+20*6;		/*Base of int table	*/

/* Interrupt vector for DAG1 buffer 7 circular buffer overflow				*/
___lib_CB7I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+21*6;		/*Base of int table	*/

/* Interrupt vector for DAG2 buffer 15 circular buffer overflow				*/
___lib_CB15I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+22*6;		/*Base of int table	*/

/* Interrupt vector for lower priority timer interrupt					*/
___lib_TMZLI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+23*6;		/*Base of int table	*/

/* Interrupt vector for fixed point overflow interrupt					*/
___lib_FIXI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+24*6;		/*Base of int table	*/

/* Interrupt vector for floating point overflow interrupt				*/
___lib_FLTOI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+25*6;		/*Base of int table	*/

/* Interrupt vector for floating point underflow interrupt				*/
___lib_FLTUI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+26*6;		/*Base of int table	*/

/* Interrupt vector for floating point invalid operation interrupt			*/
___lib_FLTII:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+27*6;		/*Base of int table	*/

/* Interrupt vectors for user interrupts 0 - 7						*/
___lib_SFT0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+28*6;		/*Base of int table	*/

___lib_SFT1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+29*6;		/*Base of int table	*/

___lib_SFT2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+30*6;		/*Base of int table	*/

___lib_SFT3I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			JUMP ___z3_int_determiner (DB);		/*jmp to finish setting up*/
			DM(I7,M7)=I15;				/*Save I15 (scratch reg)*/
			I15=___lib_int_table+31*6;		/*Base of int table	*/


___z3_int_determiner:	DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=PM(5,I15);				/*get disp to jump to	*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I13=PM(2,I15);				/*rd handler addr (base+2)*/

/* Note:  It's okay to use PM in getting the above values b'cse z3 has a linear memory. 
Therefore dm and pm are the same and we can use either.  */

.ENDSEG;


