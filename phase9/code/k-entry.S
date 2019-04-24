// k-entry.S, entries to kernel services, build process trapframe

#include "k-const.h"
#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#define K_STACK_SIZE (PROC_STACK_SIZE * 8) // kernel has 32KB stack

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (16K x 1B)
.text

// load proc trapframe to run it
ENTRY(Loader)
   mov 4(%esp, 4), %ebx 	// (copy 4 bytes from 4B above esp to ebx +++++++
   mov %ebx, %esp 		// copy ebx to esp (point esp to process trapframe) ++
   popa 			// pop all (8) registers ++++++++++
   add $4, %esp   		// AS | skip entry id trapframe
   iret				// return from interrupt ++++++++++

// build proc trapframe, and go into kernel
ENTRY(TimerEntry)	                // will have efl, cs, eip pushed by hardware
   pushl $TIMER_INTR
   jmp PreKernel

ENTRY(GetPidEntry)
   pushl $GETPID_CALL 		       // AS | new constant for a sys call
   jmp PreKernel

ENTRY(ShowCharEntry)
   pushl $SHOWCHAR_CALL
   jmp PreKernel

ENTRY(SleepEntry)
   push $SLEEP_CALL
   jmp PreKernel

ENTRY(MuxCreateEntry)
   pushl $MUX_CREATE_CALL
   jmp PreKernel

ENTRY(MuxOpEntry)
   pushl $MUX_OP_CALL
   jmp PreKernel

ENTRY(Term0Entry)
   pushl $TERM0_INTR
   jmp PreKernel

ENTRY(Term1Entry)
   pushl $TERM1_INTR
   jmp PreKernel

ENTRY(ForkEntry)
   pushl $FORK_CALL
   jmp PreKernel

ENTRY(WaitEntry)
   pushl $WAIT_CALL
   jmp PreKernel

ENTRY(ExitEntry)
   pushl $EXIT_CALL
   jmp PreKernel

ENTRY(ExecEntry)
   pushl $EXEC_CALL
   jmp PreKernel

ENTRY(SignalEntry)
   pushl $SIGNAL_CALL
   jmp PreKernel

ENTRY(PauseEntry)
   pushl $PAUSE_CALL
   jmp PreKernel

ENTRY(KillEntry)
   pushl $KILL_CALL
   jmp PreKernel

ENTRY(RandEntry)
   pushl $RAND_CALL
   jmp PreKernel

PreKernel:
   pusha 				// push all (8) registers +++++++++
   mov %esp, %ebx			// copy esp to ebx +++++++++++
   movw $0x10, %ax      	        // set kernel data segment registers
   mov %ax, %ds                 	// set kernel data segment registers
   mov %ax, %es                        	// set kernel data segment registers
   leal k_stack + K_STACK_SIZE, %esp    // set esp to top of kernel stack
   push %ebx				// push ebx (onto the kernel stack) ++++++
   cld					// clear the direction flag of efl +++++++
   call CNAME(Kernel) 			// call C function Kernel ++++++++++
 
