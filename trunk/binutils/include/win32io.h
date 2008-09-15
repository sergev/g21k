#ifndef _win32io_h
#define _win32io_h

/*
   Because VC++ 4.1 or later does not have old IO functions. That is
   Win95 and NT have protected IO system, Microsoft discourage direct
   IO access. But, Win95 is not a complete protected OS, We can work
   out IO functions use in-line assemble instruction for Win95.
   This method will not work for NT, it need device driver.
   MODIFICATION_BL
*/

/*
  Even VC++ 4.1 does not have old IO functions outp() and inp(), but their protype
  were still in <conio.h>, that is if you want to define your own function, you
  must match the protype exactly. At beginning, I use 'short' instead of 'int', 
  compiler complainted about unmatch  protype 
*/

#ifdef WIN32   
unsigned char		inportb(int portid);
void				outportb(int portid, unsigned char value);
void				outport(int portid, int value);
int					inport(int portid);
int					outp(unsigned int portid, int value);
int					inp(unsigned int portid);
#endif

#endif
