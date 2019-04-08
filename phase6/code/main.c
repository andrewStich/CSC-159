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
int run_pid, sys_centi_sec, vid_mux;           		// current running PID; if -1, none selected
q_t pid_q, ready_q, sleep_q, mux_q;          		// avail PID and those created/ready to run
pcb_t pcb[PROC_SIZE];               		// Process Control Blocks
char proc_stack[PROC_SIZE][PROC_STACK_SIZE];	// process runtime stacks
mux_t mux[MUX_SIZE];
struct i386_gate *intr_table;    		// intr table's DRAM location
term_t term[TERM_SIZE] = {
      { TRUE, TERM0_IO_BASE},
      { TRUE, TERM1_IO_BASE}
};

void InitKernelData(void) { 			// wheres prototype   // init kernel data
   int i;      
   intr_table = get_idt_base();     		// get intr table location +++++++
   sys_centi_sec = 0;
   


   Bzero((char*)&pid_q, sizeof(q_t));      	// clear 2 queues ++++++
   Bzero((char*)&ready_q, sizeof(q_t));
   Bzero((char*)&sleep_q, sizeof(q_t));
   Bzero((char*)&mux_q, sizeof(q_t));
   for(i = 0; i < Q_SIZE; i++){ 	        //sizeof(q_t)
   	EnQ(i, &pid_q);	                 	// put all PID's to pid queue
   	EnQ(i, &mux_q);
   }

   run_pid = NONE;				//set run_pid to NONE ++++++
}

void InitKernelControl(void) { 			//wheres prototype   // init kernel control
   fill_gate(&intr_table[TIMER_INTR], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[GETPID_CALL], (int)GetPidEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[SHOWCHAR_CALL], (int)ShowCharEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[SLEEP_CALL], (int)SleepEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[MUX_CREATE_CALL], (int)MuxCreateEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[MUX_OP_CALL], (int)MuxOpEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[TERM0_INTR], (int)Term0Entry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[TERM1_INTR], (int)Term1Entry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[FORK_CALL], (int)ForkEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[WAIT_CALL], (int)WaitEntry, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&intr_table[EXIT_CALL], (int)ExitEntry, get_cs(), ACC_INTR_GATE, 0);

   outportb(PIC_MASK, MASK);                   	// AS | prep4 mask out PIC for timer
}

void Scheduler(void) { 				//wheres prototype     // choose run_pid
   if(run_pid > 0) { 
   	return; 				// AS | run_pid is greater than 0, just return; // OK/picked
   }
   if(QisEmpty(&ready_q)) { 			// AS | ready_q is empty:
      run_pid = 0;				// AS | pick 0 as run_pid     // pick InitProc
   }
   else {
      pcb[0].state = READY;			// AS | change state of PID 0 to ready
      run_pid = DeQ(&ready_q); 			// AS | dequeue ready_q to set run_pid
   }
   pcb[run_pid].run_count = 0;                 // AS | reset run_count of selected proc
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
   
   switch(trapframe_p->entry_id){
      case SLEEP_CALL:
	      SleepSR(trapframe_p->eax);
	      break;
      case GETPID_CALL:
	      trapframe_p->eax = GetPidSR();
	      break;
      case TIMER_INTR:
	      TimerSR();
	      break;
      case SHOWCHAR_CALL:
	      ShowCharSR(trapframe_p->eax, trapframe_p->ebx, trapframe_p->ecx);
	      break;
      case MUX_CREATE_CALL:
	      trapframe_p->ebx = MuxCreateSR(trapframe_p->eax);   // Passing flag
	      break;
      case MUX_OP_CALL:
	      MuxOpSR(trapframe_p->eax, trapframe_p->ebx);        // Passing mux_id & op_code
	      break;
      case TERM0_INTR:
	      TermSR(0);
	      outportb(PIC_CONTROL, TERM0_DONE);
	      break;
      case TERM1_INTR:
	      TermSR(1);
	      outportb(PIC_CONTROL, TERM1_DONE);
	      break;	
      case FORK_CALL:
         trapframe_p->eax = ForkSR();
         break;
      case WAIT_CALL:
         trapframe_p->eax = WaitSR();
         break;
      case EXIT_CALL:
         ExitSR(trapframe_p->eax);
         break;
   }
                  
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

