// k-lib.h, 159

#ifndef __K_LIB__
#define __K_LIB__

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"


// prototype those in k-lib.c here +++++++++
void Bzero(char*, int);		//AS
int QisEmpty(q_t*);		//AS
int QisFull(q_t*);		//AS
int DeQ(q_t*);			//AS
void EnQ(int, q_t*);		//AS
void MemCpy((char *), (char *), int);
int StrCmp(char, char);
void Swap(char *, char *);
void Reverse(char *, int, int);
void Itoa(char *, int);

#endif
