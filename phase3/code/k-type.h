// k-type.h, 159

#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {UNUSED, READY, RUN, SLEEP, SUSPEND} state_t;  // AS | coding hints 2

typedef struct {
   	//unsigned int reg[8];         Not included in phase 2
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax, entry_id, eip, cs, efl;
} trapframe_t;

typedef struct {
   	state_t state;                       // read in 1.html ++++++
	int run_count;
	int total_count;
        int wake_centi_sec;                    // AS | coding hints 2
	trapframe_t *trapframe_p;
} pcb_t;                     

typedef struct {	             // generic queue type
 	 int q[Q_SIZE], tail;        // for a simple queue ++++++  
} q_t;

typedef struct {		// AS | coding_hints.txt 3
	int flag;
	int creater;
	q_t suspend_q;
} mux_t;

#endif
