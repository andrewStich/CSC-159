// k-sr.c, 159

#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "tools.h"
#include "k-sr.h"
#include "proc.h"

// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_q
void NewProcSR(func_p_t p) {  // arg: where process code starts
   int pid;

   if(QisEmpty(&pid_q)) {     			//pid_q is empty, may occur if too many been created
      cons_printf("Panic: no more process!\n");
      //breakpoint();          			// AS |  cannot continue, alternative: breakpoint();
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
   pcb[pid].trapframe_p->eip = (unsigned int)p;                	         // set to code
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
   int suspend_pid, device;

   ch = inportb(term[term_no].io_base + DATA);

   if(ch == SIGINT){
      if(!QisEmpty(&mux[term[term_no].in_mux].suspend_q)){
	 suspend_pid = mux[term[term_no].in_mux].suspend_q.q[0];
	 if(pcb[suspend_pid].sigint_handler) {
	    //alter runtime stack of suspended process
	    if(term_no == 0){
	       device = TERM0_INTR;
	    }else{
	       device = TERM1_INTR;
	    }
	    WrapperSR(suspend_pid, pcb[suspend_pid].sigint_handler, device);
	 }
      }
   }

   EnQ(ch, &term[term_no].echo_q);

   if(ch == '\r') {
      EnQ('\n', &term[term_no].echo_q);
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
      MuxOpSR(term[term_no].out_mux, UNLOCK);

   }
   outportb(term[term_no].io_base+DATA, temp);
   term[term_no].tx_missed = FALSE;
}

int ForkSR(void) {
   int childPID;
   int mem_diff;
   int *p;
   if(QisEmpty(&pid_q)) {
      cons_printf("Panic: no more process!\n");
      return NONE;
   }

   childPID = DeQ(&pid_q);
   Bzero((char*)&pcb[childPID], sizeof(pcb_t));                 // clear PCB
   //Bzero((char*)&proc_stack[childPID][0], PROC_STACK_SIZE);     // clear stack
   pcb[childPID].state = READY;
   pcb[childPID].ppid = run_pid;
   EnQ(childPID, &ready_q);

   mem_diff = (int)&proc_stack[childPID][0] - (int)&proc_stack[run_pid][0];
   pcb[childPID].trapframe_p = (trapframe_t *)((int)pcb[run_pid].trapframe_p + mem_diff),

   MemCpy((char *)((int)&(proc_stack[run_pid][0]) + mem_diff), &proc_stack[run_pid][0], PROC_STACK_SIZE);

   pcb[childPID].trapframe_p->eax = 0;
   pcb[childPID].trapframe_p->esp = pcb[childPID].trapframe_p->esp + mem_diff;
   pcb[childPID].trapframe_p->ebp = pcb[childPID].trapframe_p->ebp + mem_diff;
   pcb[childPID].trapframe_p->esi = pcb[childPID].trapframe_p->esi + mem_diff;
   pcb[childPID].trapframe_p->edi = pcb[childPID].trapframe_p->edi + mem_diff;
   p = (int *)pcb[childPID].trapframe_p->ebp;

   while(*p != 0) {
      *p += mem_diff;
      p = (int *)*p;
   }

   return childPID;
}

int WaitSR(void) {
   int exit_code;
   int i, x;
   for(i=0; i<PROC_SIZE; i++ ) {
      if((pcb[i].ppid == run_pid) && (pcb[i].state == ZOMBIE)) {
	 break;
      }
   }

   if(i == PROC_SIZE) {
      pcb[run_pid].state = WAIT;
      run_pid = NONE;
      return NONE;
   }

   exit_code = pcb[i].trapframe_p->eax;

   pcb[i].state = UNUSED;
   EnQ(i, &pid_q);

   for(x = 0; x < PAGE_NUM; x++){
      if(page_user[x] == i){
	 page_user[x] = NONE;
      }
   }

   return exit_code;
}

void ExitSR(int exit_code) {
   int x;

   if(pcb[pcb[run_pid].ppid].state != WAIT) {
      pcb[run_pid].state = ZOMBIE;
      run_pid = NONE;
      return;
   }

   pcb[pcb[run_pid].ppid].state = READY;
   EnQ(pcb[run_pid].ppid, &ready_q);
   pcb[pcb[run_pid].ppid].trapframe_p->eax = exit_code;

   pcb[run_pid].state = UNUSED;
   EnQ(run_pid, &pid_q);

   for(x = 0; x < PAGE_NUM; x++){
      if(page_user[x] == run_pid){
	 page_user[x] = NONE;
      }
   }

   run_pid = NONE;
}

void ExecSR(int code_addr, int arg) {
   int i, code_page, stack_page;
   char * code_space_addr;
   char * stack_space_addr;
   code_page = NONE;
   stack_page = NONE;

   // Allocate two DRAM pages, one for code, one for stack space
   for(i = 0; i < PAGE_NUM; i++){
      if(page_user[i] == NONE) {
         if(code_page == NONE) {
            code_page = i;
            continue;
         }
         else if(stack_page == NONE) {
            stack_page = i;
         }
         
         if(code_page != NONE && stack_page != NONE) {
            page_user[code_page] = run_pid;
            page_user[stack_page] = run_pid;
            break;
         }
      }
   }
   code_space_addr = (char *)((code_page * PAGE_SIZE) + RAM);
   stack_space_addr = (char *)((stack_page * PAGE_SIZE) + RAM); 

   MemCpy(code_space_addr, (char *)code_addr, PAGE_SIZE);
   Bzero(stack_space_addr, PAGE_SIZE); 

   stack_space_addr = (int *)stack_space_addr + PAGE_SIZE;
   stack_space_addr--;
   *stack_space_addr = arg;
   stack_space_addr--;

   pcb[run_pid].trapframe_p = (trapframe_t *)p;
   pcb[run_pid].trapframe_p--;
   pcb[run_pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR; 	 // enables intr
   pcb[run_pid].trapframe_p->cs = get_cs();                 	 // dupl from CPU
   pcb[run_pid].trapframe_p->eip = (unsigned int)code_space_addr;
}

void SignalSR(int sig_num, int handler_addr) {
   pcb[run_pid].sigint_handler = handler_addr;
}

void WrapperSR(int pid, int handler_p, int arg) {

   int *p;
   p = (int *)((int)pcb[pid].trapframe_p + sizeof(trapframe_t));

   MemCpy((char *)((int)pcb[pid].trapframe_p - sizeof(int[3])), (char *)((int)pcb[pid].trapframe_p), sizeof(trapframe_t));

   pcb[pid].trapframe_p = (trapframe_t *)((int)pcb[pid].trapframe_p - sizeof(int[3]));

   p--;
   *p = arg;
   p--;
   *p = handler_p;
   p--;
   *p = pcb[pid].trapframe_p->eip

   pcb[pid].trapframe_p->eip = (int)Wrapper;
   
}

void PauseSR() {
   pcb[run_pid].state = PAUSE;
   run_pid = NONE;
}

void KillSR(int pid, int sig_num) {
   int i;
   
   if(pid == 0) {
      for( i=0; i<PROC_SIZE; i++ ) {
         if(pcb[i].ppid == run_pid && pcb[i].state == PAUSE) {
            pcb[i].state = READY;
            EnQ(i, &ready_q);
         }
      }
   }
}

unsigned RandSR() {
   rand = run_pid*rand + A_PRIME;
   rand %= G2;
   return rand;
}
