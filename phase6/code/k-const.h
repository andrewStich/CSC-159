// k-const.h, 159

#ifndef __K_CONST__
#define __K_CONST__

#define NONE -1             // used when none
#define TIMER_INTR 32       // TIMER INTR constant identifier
#define PIC_MASK 0x21       // Programmable Interrupt Controller I/O
//#define MASK ~0x01          // mask for Programmable Interrupt Controller
#define PIC_CONTROL 0x20    // Programmable Interrupt Controller I/O
#define TIMER_DONE 0x60     // sent to PIC when timer service done

#define LOOP 1666666        // handly loop limit exec asm("inb $0x80");
#define TIME_SLICE 310      // max timer count, then rotate process
#define PROC_SIZE 20        // max number of processes
#define PROC_STACK_SIZE 4096     // process stack in bytes
#define Q_SIZE 20           // queuing capacity

#define VID_MASK 0x0f00     // foreground bright white, background black
#define VID_HOME (unsigned short *)0xb8000 // home position, upper-left corner

// Phase 2 constants
#define SYS_CALL 256
#define GETPID_CALL 48	      // AS | coding_hints.txt 2
#define SHOWCHAR_CALL 49      // AS | coding_hints.txt 2
#define SLEEP_CALL 50	      // AS | coding_hints.txt 2

// Phase 3 constants
#define LOCK 1		      // AS | coding_hints.txt 3
#define UNLOCK 2	      // AS | coding_hints.txt 3
#define STDOUT 1	      // AS | coding_hints.txt 3
#define MUX_SIZE 20	      // AS | coding_hints.txt 3
#define STR_SIZE 101	      // AS | coding_hints.txt 3
#define MUX_CREATE_CALL 51    // AS | coding_hints.txt 3
#define MUX_OP_CALL 52	      // AS | coding_hints.txt 3

// Phase 4 constants
#define TERM_SIZE 2
#define TERM0_INTR 35
#define TERM1_INTR 36
#define TERM0_IO_BASE 0x2f8
#define TERM1_IO_BASE 0x3e8
#define TERM0_DONE 0x63
#define TERM1_DONE 0x64
#define TXRDY 2
#define RXRDY 4
#define MASK 0xffffffe6
#define TRUE 1
#define FALSE 0

// Phase 6 constants
#define FORK_CALL 53
#define WRITE_CALL 54
#define EXIT_CALL 55

#endif
