#ifndef GENERATOR_RELEASE

#define GENERATOR_RELEASE   0
#define GENERATOR_INITIATE  1
#define GENERATOR_READ      2
#define GENERATOR_WRITE     3

typedef unsigned long (* GENERATOR)(void **udata ,int mode, unsigned long adr, unsigned long pla);


#endif
