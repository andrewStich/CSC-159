// k-sr.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "k-lib.h"
#include "k-sr.h"

// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_q
void NewProcSR(func_p_t p) {  // arg: where process code starts
   int pid;

   if(QisEmpty(&pid_q)) {     			//pid_q is empty, may occur if too many been created
      cons_printf("Panic: no more process!\n");
      breakpoint();          			// AS |  cannot continue, alternative: breakpoint();
      return;
   }

   pid = DeQ(&pid_q);                                        // alloc PID (1st is 0)
   Bzero((char*)&pcb[pid], sizeof(pcb_t));          	    // clear PCB
   Bzero((char*)&proc_stack[pid][0], PROC_STACK_SIZE);     // clear stack
   pcb[pid].state = READY;                                  // change process state

   if(pid > 0){                           // queue to ready_q if > 0
	EnQ(pid, &ready_q);
   }
   // point trapframe_p to stack & fill it out

   // point to stack top
   pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE - sizeof(trapframe_t)];         
   //pcb[pid].trapframe_p--; 	 	                	 // lower by trapframe size
   pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR; 	 // enables intr
   pcb[pid].trapframe_p->cs = get_cs();                 	 // dupl from CPU
   pcb[pid].trapframe_p->eip = (int)p;                	         // set to code
}

void CheckWakeProc(void){
   int i, num_sleep_proc, temp_proc;
   num_sleep_proc = sleep_q.tail;

   for(i = 0; i < num_sleep_proc; i++){
      temp_proc = DeQ(&sleep_q);
      if(pcb[temp_proc].wake_centi_sec <= sys_centi_sec){
         EnQ(temp_proc, &ready_q);
	 pcb[temp_proc].state = READY;
      }
      else{
         EnQ(temp_proc, &sleep_q);
      }
   }
}

// count run_count and switch if hitting time slice
void TimerSR(void) {

   outportb(PIC_CONTROL, TIMER_DONE);        // AS, prep4 | notify PIC timer done
   
   sys_centi_sec++;
   pcb[run_pid].run_count++;                 // AS | count up run_count
   pcb[run_pid].total_count++;               // AS | count up total_count

   CheckWakeProc();

   if(run_pid == 0) {
      return;
   }
   
   if(pcb[run_pid].run_count == TIME_SLICE) {     // if runs long enough
      pcb[run_pid].state = READY;             	  // move it to ready_q
      EnQ(run_pid, &ready_q);             	  // change its state
      run_pid = NONE;		                  // running proc = NONE
   }
}

void SleepSR(int centi_sec){
   pcb[run_pid].wake_centi_sec = sys_centi_sec + centi_sec;
   pcb[run_pid].state = SLEEP;
   EnQ(run_pid, &sleep_q);
   run_pid = NONE;
}

int GetPidSR(void){
   return run_pid;
}

void ShowCharSR(int row, int col, char ch){
   unsigned short *p = VID_HOME;

   p += row * 80 + col;
   *p = ch + VID_MASK;
}

int MuxCreateSR(int flag) {
   int mutex;

   if(QisEmpty(&mux_q)){
      cons_printf("The OS is out of Mutex's to give");
      breakpoint();
   }
   else {
      mutex = DeQ(&mux_q);
      Bzero((char*)(&mux[mutex].suspend_q), sizeof(q_t));
      mux[mutex].flag = flag;
      mux[mutex].creater = run_pid;
   }
   return mutex;
}

void MuxOpSR(int mux_id, int opcode) {
   int temp_proc;

   if(opcode == LOCK) {	    
      if(mux[mux_id].flag > 0) {	 
	 mux[mux_id].flag--;
      }
      else {
	 EnQ(run_pid, &mux[mux_id].suspend_q); 
	 pcb[run_pid].state = SUSPEND;	  
	 run_pid = NONE;		  
      }
   }
   else if(opcode == UNLOCK) {
      if(QisEmpty(&mux[mux_id].suspend_q)){
	 mux[mux_id].flag++;
      }
      else {
	 temp_proc = DeQ(&mux[mux_id].suspend_q);
	 EnQ(temp_proc, &ready_q);
	 pcb[temp_proc].state = READY;
      }
   }
}

void TermSR(int term_no) {
   int event = inportb(term[term_no].io_base+IIR);

   if(event == TXRDY) {
      TermTxSR(term_no);
   }
   else if(event == RXRDY) {
      TermRxSR(term_no);
   }

   if(term[term_no].tx_missed == TRUE) {
      TermTxSR(term_no);
   }
}

void TermRxSR(int term_no) {
   char ch;

   ch = inportb(term[term_no].io_base + DATA);
   EnQ(ch, &term[term_no].echo_q);

   if(ch == '\r') {
      EnQ('\n', &term[term_no].echo_q);
      EnQ('\r', &term[term_no].echo_q);
      EnQ('\0', &term[term_no].in_q);
   }
   else {
      EnQ(ch, &term[term_no].in_q);
   }

   MuxOpSR(term[term_no].in_mux, UNLOCK);
}

void TermTxSR(int term_no) {
   char temp = '\0';
   
   if(QisEmpty(&term[term_no].out_q) && QisEmpty(&term[term_no].echo_q)) {
      term[term_no].tx_missed = TRUE;
      return;
   }
   
   if(!QisEmpty(&term[term_no].echo_q)) {
      temp = DeQ(&term[term_no].echo_q);
   }else{
      temp = DeQ(&term[term_no].out_q);    
      outportb(term[term_no].io_base+DATA, temp);
      term[term_no].tx_missed = FALSE;
      MuxOpSR(term[term_no].out_mux, UNLOCK);
   }
}

int ForkSR(void) {
   int childPID;
   if(QisEmpty(&pid_q)) {
      cons_printf("Panic: no more process!\n");
      return NONE;
   }

   childPID = DeQ(&pid_q);
   Bzero((char*)&pcb[childPID], sizeof(pcb_t));          	    // clear PCB
   Bzero((char*)&proc_stack[childPID][0], PROC_STACK_SIZE);     // clear stack
   pcb[childPID].state = READY; 
   EnQ(childPID, &ready_q);

   }

int WaitSR(void) {
   for(int i=0; i<
}

void ExitSR(int exit_code) {

}
