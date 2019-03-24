// k-sr.h, 159

#ifndef __K_SR__
#define __K_SR__

#include "k-type.h" // for func_p_t

// prototype those in k-sr.c here
void NewProcSR(func_p_t);	//AS
void CheckWakeProc(void);
void TimerSR(void);		//AS
void SleepSR(int);
int GetPidSR(void);
void ShowCharSR(int, int, char);


#endif
