/* This file contains the interrupt table for the ADSP-21020				*/

/* When the C program exits either by returning from main() or by an 			*/
/* explicit or implicit call to exit(), control will transfer to the 			*/
/* label ___lib_prog_term.  Currently, the ___lib_prog_term label is 			*/
/* defined at the end of the reset vector as an IDLE instruction.    			*/
/* If your application needs to perform some operation AFTER the C   			*/
/* program has finished executing, remove the ___lib_prog_term label 			*/
/* from the runtime header, and place it at the beginning of your    			*/
/* code.							     			*/

.GLOBAL		___lib_prog_term;			    	/*Termination address	*/

.SEGMENT/PM	    seg_rth;					/*Runtime header segment*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

___lib_RSTI:	 	CALL ___lib_setup_hardware;	    	/*Reset Interrupt 	*/
		 	CALL ___lib_setup_processor;
		 	CALL ___lib_setup_environment; 
		 	JUMP _main (DB);	    		/*Begin user progam 	*/
			NOP;NOP;
___lib_prog_term:	IDLE;
			JUMP ___lib_prog_term;			/*Stay at idle */

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vector for status stack/loop stack overflow or PC stack full: 		*/
___lib_SOVFI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+3*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+3*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+3*6+2);		/*read addr of handler	*/

/* Interrupt vector for high priority timer interrupt: */
___lib_TMZOI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+4*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+4*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+4*6+2);		/*read addr of handler	*/
			
/* Interrupt vector for external interrupts:*/
___lib_IRQ3I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+5*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+5*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+5*6+2);		/*read addr of handler	*/

___lib_IRQ2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+6*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+6*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+6*6+2);		/*read addr of handler	*/

___lib_IRQ1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+7*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+7*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+7*6+2);		/*read addr of handler	*/

___lib_IRQ0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+8*6+5);		/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+8*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+8*6+2);		/*read addr of handler	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vector for DAG1 buffer 7 circular buffer overflow				*/
___lib_CB7I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+11*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+11*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+11*6+2);	/*read addr of handler	*/

/* Interrupt vector for DAG2 buffer 15 circular buffer overflow				*/
___lib_CB15I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+12*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+12*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+12*6+2);	/*read addr of handler	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt 	*/

/* Interrupt vector for lower priority timer interrupt					*/
___lib_TMZI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+14*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+14*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+14*6+2);	/*read addr of handler	*/

/* Interrupt vector for fixed point overflow interrupt					*/
___lib_FIXI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+15*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+15*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+15*6+2);	/*read addr of handler	*/

/* Interrupt vector for floating point overflow interrupt				*/
___lib_FLTOI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+16*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+16*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+16*6+2);	/*read addr of handler	*/

/* Interrupt vector for floating point underflow interrupt				*/
___lib_FLTUI:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+17*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+17*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+17*6+2);	/*read addr of handler	*/

/* Interrupt vector for floating point invalid operation interrupt			*/
___lib_FLTII:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+18*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+18*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+18*6+2);	/*read addr of handler	*/

			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/
			NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;	/*Reserved interrupt	*/

/* Interrupt vectors for user interrupts 0 - 7						*/
___lib_USR0I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+24*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+24*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+24*6+2);	/*read addr of handler	*/

___lib_USR1I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+25*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+25*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+25*6+2);	/*read addr of handler	*/

___lib_USR2I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+26*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+26*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+26*6+2);	/*read addr of handler	*/

___lib_USR3I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+27*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+27*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+27*6+2);	/*read addr of handler	*/

___lib_USR4I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+28*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+28*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+28*6+2);	/*read addr of handler	*/

___lib_USI15I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+29*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+29*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+29*6+2);	/*read addr of handler	*/

___lib_USR6I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+30*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+30*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+30*6+2);	/*read addr of handler	*/

___lib_USR7I:		BIT CLR MODE1 0x1000;			/*Disable interrupts 	*/
			DM(I7,M7)=I13;				/*Save I13 (scratch reg)*/
			I13=DM(___lib_int_table+31*6+5);	/*addr of dispatch to jump to*/
			BIT SET MODE2 0x80000;			/*Freeze cache 		*/
			DM(I7,M7)=I15;				/*Save another scratch reg*/
			JUMP (M13, I13) (DB);			/*Jump to dispatcher 	*/
			I15=___lib_int_table+31*6;		/*Int slot next_mask	*/
			I13=dm(___lib_int_table+31*6+2);	/*read addr of handler	*/
.ENDSEG;

