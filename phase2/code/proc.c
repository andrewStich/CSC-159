// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use syscalls

#include "k-const.h"    // LOOP
//#include "k-data.h"   // removed starting phase 2
#include "sys-call.h"   // all service calls used below

/*
void Delay(void) {  // delay CPU for half second by 'inb $0x80' | asm("inb $0x80")
   int i;
   for(i = 0; i < LOOP/2; i++){ 
   	asm("inb $0x80"); 		 // loop to try to delay CPU for about half second
   }
}

void ShowChar(int row, int col, char ch) { // show ch at row, col

   unsigned short *p = VID_HOME; 	 // AS | upper-left corner of display
   
   p += row * 80 + col;
   *p = ch + VID_MASK;
}
*/

void InitProc(void) {
   int i;
   while(1) {
      ShowCharCall( 0, 0, '.');		// AS | show a dot at upper left corner on PC
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service
      
      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service
   }
}

void UserProc(void) {
   int my_pid = GetPidCall();       // get my PID

   while(1) {
      ShowCharCall(my_pid, 0, '0' + my_pid / 10);
      ShowCharCall(my_pid, 1, '0' + my_pid % 10);
      SleepCall(50);		       // Delay for half a sec

      ShowCharCall(my_pid, 0, ' ');
      ShowCharCall(my_pid, 1, ' ');
      SleepCall(50);
   }
}
