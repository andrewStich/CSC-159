// k-type.h, 159

#ifndef __K_TYPE__
#define __K_TYPE__

#include "k-const.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {UNUSED, READY, RUN} state_t;

typedef struct {
   	unsigned int reg[8];
   	unsigned int eip;
   	unsigned int cs;
  	unsigned int efl;
} trapframe_t;
/*
 * A TRAPFRAME TAKES UP 44 BYTES OF MEMORY
 */

typedef struct {
   	state_t state;                       // read in 1.html ++++++
	int run_count;
	int total_count;
	trapframe_t *trapframe_p;
} pcb_t;                     

typedef struct {	             // generic queue type
 	 int q[Q_SIZE], tail;        // for a simple queue ++++++  
} q_t;

#endif
