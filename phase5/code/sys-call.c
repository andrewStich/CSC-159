// sys-call.c
// calls to kernel services

#include "k-const.h"
//#include "stddef.h"
#include "k-data.h"
#include "k-lib.h"

int GetPidCall(void) {
   int pid;

   asm("int %1;             // interrupt!
        movl %%eax, %0"     // after, copy eax to variable 'pid'
       : "=g" (pid)         // output
       : "g" (GETPID_CALL)  // input
       : "eax"              // used registers
   );

   return pid;
}

void ShowCharCall(int row, int col, char ch) {
   asm("movl %0, %%eax;     // send in row via eax
        movl %1, %%ebx;     // send in col via ebx
        movl %2, %%ecx;     // send in ch via ecx
        int %3"             // initiate call, %3 gets entry_id
       :                    // no output
       : "g" (row), "g" (col), "g" (ch), "g" (SHOWCHAR_CALL) 
       : "eax", "ebx", "ecx"         // affected/used registers
   );
}

void SleepCall(int centi_sec) {  // # of 1/100 of a second to sleep
   asm("movl %0, %%eax;
   	int %1"
	:
	: "g" (centi_sec), "g" (SLEEP_CALL)
	: "eax"
   );
}

int MuxCreateCall(int flag) {
   int mutex;
   
   asm("movl %1, %%eax;
        int %2;
	movl %%ebx, %0"
	: "=g" (mutex)
	: "g" (flag), "g" (MUX_CREATE_CALL)
	: "eax", "ebx"
   );

   return mutex;
}

void MuxOpCall(int mux_id, int opcode) {
   asm("movl %0, %%eax;
        movl %1, %%ebx;
        int %2"
	:
	: "g" (mux_id), "g" (opcode), "g" (MUX_OP_CALL)
	: "eax", "ebx"
   );
}

void WriteCall(int device, char *str) {
   int my_pid = GetPidCall();
   int row = my_pid;
   int col = 0;
   int term_no;

   if(device == STDOUT){
      while(*str != '\0'){
	 ShowCharCall(row, col, *str);
	 col++;
	 str++;
      }
   }
   else {
      if(device == TERM0_INTR)
	 term_no = 0;
      else if(device == TERM1_INTR)
	 term_no = 1;
	 

      while(*str != '\0') {
	 MuxOpCall(term[term_no].out_mux, LOCK); //look into the mux array at index "term[term_no].out_mux" and lock it.
	 EnQ((int)*str, &term[term_no].out_q);

	 if(device == TERM0_INTR) {  //asm("int $35");
	     asm("int %0"
	       :
	       : "g" (TERM0_INTR)
	     );
	 }
	 else {                      //asm("int $36");
	    asm("int  %0"
	       :
	       : "g" (TERM1_INTR)
	       );
	 }

      str++;
      }
   }
}

void ReadCall(int device, char *str) {
   int term_no;
   int count = 0;
   char ch;

   if(device == TERM0_INTR) {
      term_no = 0;
   } else {
      term_no = 1;
   }

   while(TRUE) {
      MuxOpCall(term[term_no].in_mux, LOCK);
      ch = DeQ(&term[term_no].in_q);
      //how do we add the new char to a string??
      *str = ch;
      if(ch == '\0') return;

      str++;
      count++;
      if(count == STR_SIZE) {
         *str = '\0';
         return;
      }
   }
}

