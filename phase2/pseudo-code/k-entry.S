// k-entry.S

... same as before here ...

ENTRY(Loader) // loads proc trapframe to run it
   movl 4(%esp), %ebx
   movl %ebx, %esp
   popa
   incr esp by 4 (add $4 to %esp -- skip 'entry_id' in trapframe) <-- !!!
   iret

ENTRY(TimerEntry)
   pushl $TIMER_INTR
   jmp PreKernel

ENTRY(GetPidEntry)
   pushl $GETPIDCALL
   jmp PreKernel

... add two more entries like above...

PreKernel: // build rest of trapframe, switch stack, pass arg, call Kernel
   pusha   // push all: eax ecx edx ebx esp ebp esi edi
   movl %esp, %ebx          // copy esp
   movw $0x10, %ax          // put 16 to ax
   mov %ax, %ds             // set kernel data segment registers ds
   mov %ax, %es             // set kernel data segment registers es
   leal k_stack + K_STACK_SIZE, %esp  // set esp to top of kernel stack
   pushl %ebx               // push (proc TF addr) there
   cld                      // clear direction flag bit
   call CNAME(Kernel)       // call Kernel code

