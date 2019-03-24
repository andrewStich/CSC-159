// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use syscalls

#include "k-const.h"    // LOOP
#include "k-data.h"   // removed starting phase 2
#include "sys-call.h"   // all service calls used below

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

void InitProc(void) {
   int i;

   //char str1[] = ".";
   //char str2[] = "";

   vid_mux = MuxCreateCall(1);

   while(1) {
      //WriteCall(STDOUT, (char *)&str1);
      ShowCharCall(0, 0, '.');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service
      
      //WriteCall(STDOUT, (char *)&str2);
      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");   // this is also a kernel service
   }
}

void UserProc(void) {
   int my_pid = GetPidCall();       // get my PID

   char str1[] = "PID xx is running exclusively using the video display...\0";
   char str2[] = "                                                          \0";

   str1[4] = '0' + my_pid / 10;
   str1[5] = '0' + my_pid % 10;

   while(1) {
      MuxOpCall(vid_mux, LOCK);
      
      WriteCall(STDOUT, (char *)&str1);         // Show PID of running process
      SleepCall(50);		       // Delay for half a sec

      WriteCall(STDOUT, (char *)&str2);
      SleepCall(50);

      MuxOpCall(vid_mux, UNLOCK);
   }
}
