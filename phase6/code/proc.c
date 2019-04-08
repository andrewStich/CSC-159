// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use sys-calls

#include "k-const.h"   // LOOP
#include "sys-call.h"  // all service calls used below
#include "k-data.h"
#include "k-lib.h"
#include "k-include.h"

void InitTerm(int term_no) {
   int i, j;

   Bzero((char *)&term[term_no].out_q, sizeof(q_t));
   Bzero((char *)&term[term_no].in_q, sizeof(q_t));      // <------------- new
   Bzero((char *)&term[term_no].echo_q, sizeof(q_t));    // <------------- new
   term[term_no].out_mux = MuxCreateCall(Q_SIZE);
   term[term_no].in_mux = MuxCreateCall(0);              // <------------- new

   outportb(term[term_no].io_base+CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
   outportb(term[term_no].io_base+BAUDLO, LOBYTE(115200/9600)); // period of each of 9600 bauds
   outportb(term[term_no].io_base+BAUDHI, HIBYTE(115200/9600));
   outportb(term[term_no].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

   outportb(term[term_no].io_base+IER, 0);
   outportb(term[term_no].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   for(i=0; i<LOOP/2; i++)asm("inb $0x80");
   outportb(term[term_no].io_base+IER, IER_ERXRDY|IER_ETXRDY); // enable TX & RX intr
   for(i=0; i<LOOP/2; i++)asm("inb $0x80");

   for(j=0; j<25; j++) {                           // clear screen, sort of
      outportb(term[term_no].io_base+DATA, '\n');
      for(i=0; i<LOOP/30; i++)asm("inb $0x80");
      outportb(term[term_no].io_base+DATA, '\r');
      for(i=0; i<LOOP/30; i++)asm("inb $0x80");
   }
/*  // uncomment this part for VM (Procomm logo needs a key pressed, here reads it off)
   inportb(term[term_no].io_base); // clear key cleared PROCOMM screen
   for(i=0; i<LOOP/2; i++)asm("inb $0x80");
*/
}

void InitProc(void) {
   int i;

   vid_mux = MuxCreateCall(1);  // create/alloc a mutex, flag init 1

   InitTerm(0);
   InitTerm(1);

   while(1) {
      ShowCharCall(0, 0, '.');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");  // this can also be a kernel service

      ShowCharCall(0, 0, ' ');
      for(i=0; i<LOOP/2; i++) asm("inb $0x80");
   }
}

void Aout(int device) {
   int i;
   int my_pid = GetPidCall();

   char str[] = "xx ( ) Hello, World!\n\r";
   
   str[0] = '0' + my_pid / 10;
   str[1] = '0' + my_pid % 10;

   str[4] = 'A' + my_pid;

   WriteCall(device, str);

   //slide alphabet accross the screen
   for(i = 0; i < 70; i++){
      ShowCharCall(my_pid, i, str[4]);
      SleepCall(10);
      ShowCharCall(my_pid, i, ' ');
   }

   ExitCall(my_pid * 100);

}

void UserProc(void) {
   int device;
   int my_pid = GetPidCall();  // get my PID
   int child_pid, return_code;

   char str1[STR_SIZE] = "PID    > ";         // <-------------------- new
   char str2[STR_SIZE];                       // <-------------------- new
   char child_pid_string[5];
   char return_code_string[5];

   str1[4] = '0' + my_pid / 10;  // show my PID
   str1[5] = '0' + my_pid % 10;

   device = my_pid % 2 == 1? TERM0_INTR : TERM1_INTR;

   while(1) {
      WriteCall(device, str1);  // prompt for terminal input     <-------------- new
      ReadCall(device, str2);   // read terminal input           <-------------- new
      //WriteCall(STDOUT, str2);  // show what input was to PC     <-------------- new
   
      if(StrCmp(str2, "fork\0") == 0) {
	 child_pid = ForkCall();
	 if(child_pid == NONE){
	    WriteCall(device, "Coudn't fork!\0");
	    continue;
	 }else if(child_pid == 0){
	    Aout(device);
	 }else{
	    Itoa(child_pid_string, child_pid);
	    WriteCall(STDOUT, child_pid_string);
	    WriteCall(STDOUT, "\n\r");

	    return_code = WaitCall();
	    Itoa(return_code_string[5], return_code);
	    WriteCall(STDOUT, return_code_string);
	    WriteCall(STDOUT, "\n\r");
	 }
      }
   }
}

