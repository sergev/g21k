#pragma inline

/*
   Because VC++ 4.1 or later does not have old IO functions. That is
   Win95 and NT have protected IO system, Microsoft discourage direct
   IO access. But, Win95 is not a complete protected OS, We can work
   out IO functions use in-line assemble instruction for Win95.
   This method will not work for NT, it need device driver.
   MODIFICATION_BL
*/
   
#ifdef WIN32

/* my old win32 io function */
void outport(int portid, int value)
{
	__asm {
		MOV	EDX, portid
		MOV	EAX, value
		OUT	DX, AX
	}
}


void outportb(int portid, unsigned char value)
{
	__asm {
		MOV	EDX, portid
		MOV	AL, value
		OUT	DX, AL
	}
}


int inport(int portid)
{
	__asm {
		MOV	EDX, portid
		IN	AX, DX
		MOVZX	EAX, AX
	}
}


unsigned char inportb(int portid)
{
	__asm {
		MOV	EDX, portid
		IN	AL, DX
		MOVZX	EAX, AL
	}
}

/* add two more which will compatiable with Microsoft VC++ 1.5  */
/* function protypes                                            */
/* Note: inp() and outp() will compile no problem under			*/
/*		 VC++ 4.1 debug mode, but not under release version.	*/
int inp(unsigned int portid)
{
	__asm {
		MOV	EDX, portid
		IN	AX, DX
		MOVZX	EAX, AX
	}
}

int outp(unsigned int portid, int value)
{
	__asm {
		MOV	EDX, portid
		MOV	EAX, value
		OUT	DX, AX
	}
}

#endif
