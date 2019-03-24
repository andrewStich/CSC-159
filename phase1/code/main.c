// main.c, 159
// OS phase 1
//
// Team Name: BDE (Members: Nicholas Bergamini, Hasan Javed, Andrew Stich)

#include "k-include.h"  // SPEDE includes
#include "k-entry.h"    // entries to kernel (TimerEntry, etc.)
#include "k-type.h"     // kernel data types
#include "k-lib.h"      // small handy functions
#include "k-sr.h"       // kernel service routines
#include "proc.h"       // all user process code here

// kernel data are all here:
int run_pid;                        		// current running PID; if -1, none selected
q_t pid_q, ready_q;                 		// avail PID and those created/ready to run
pcb_t pcb[PROC_SIZE];               		// Process Control Blocks
char proc_stack[PROC_SIZE][PROC_STACK_SIZE];  	// process runtime stacks
struct i386_gate *intr_table;    		// intr table's DRAM location


void InitKernelData(void) { 			// wheres prototype   // init kernel data
   int i;
      
   intr_table = get_idt_base();     		// get intr table location +++++++

   Bzero((char*)&pid_q, sizeof(q_t));      	// clear 2 queues ++++++
   Bzero((char*)&ready_q, sizeof(q_t));
   for(i = 0; i < Q_SIZE; i++){            //sizeof(q_t)
	EnQ(i, &pid_q);	                      	// put all PID's to pid queue
   }
   run_pid = NONE;				//set run_pid to NONE ++++++
}

void InitKernelControl(void) { 			//wheres prototype   // init kernel control
   fill_gate(&intr_table[TIMER_INTR], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);// AS, prep4 | fill out intr table for timer
   outportb(PIC_MASK, MASK);                   	// AS | prep4 mask out PIC for timer
}

void Scheduler(void) { 				//wheres prototype     // choose run_pid
   if(run_pid > 0) { 
   	return; 				// AS | run_pid is greater than 0, just return; // OK/picked
   }
   if(ready_q.tail == 0) { 			// AS | ready_q is empty:
      run_pid = 0;				// AS | pick 0 as run_pid     // pick InitProc
   }
   else {
      pcb[0].state = READY;			// AS | change state of PID 0 to ready
      run_pid = DeQ(&ready_q); 			// AS | dequeue ready_q to set run_pid
   }
   pcb[run_pid].run_count = 0;;                 // AS | reset run_count of selected proc
   pcb[run_pid].state = RUN;                    // AS | upgrade its state to run
}

int main(void) {                          	// OS bootstraps
   InitKernelData(); 				// Call to initialize kernel data
   InitKernelControl();				// Call to initialize kernel control

   NewProcSR(InitProc); 		 	// create InitProc
   Scheduler();
   Loader(pcb[run_pid].trapframe_p); 		// load/run it

   return 0; 					// statement never reached, compiler asks it for syntax
}

void Kernel(trapframe_t *trapframe_p) { 	//where prototype?   // kernel runs
   char ch;

   pcb[run_pid].trapframe_p = trapframe_p; 	// AS | save it
  
   TimerSR();                     		// handle timer intr

   if(cons_kbhit()) { 				// AS | KB of PC is pressed { // check if keyboard pressed
      ch = cons_getchar(); 			// AS | read the key
      if(ch == 'b') { 				// AS | it's 'b':       // 'b' for breakpoint
         breakpoint();                  	// AS | let's go to GDB
      }
      if(ch == 'n') { 				// AS | it's 'n':   // 'n' for new process
	 NewProcSR(UserProc);      		// create a UserProc
     }
   }
   Scheduler();					// AS | call Scheduler()  // may need to pick another proc
   Loader(pcb[run_pid].trapframe_p);
}

