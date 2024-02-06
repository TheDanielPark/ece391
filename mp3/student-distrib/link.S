# link.S
#     This is the main linkage file that will
#     handle exceptions called by the user or whenever
#     an exception occurs.


.extern do_irq

.text

.globl common_interrupt, sys_interrupt
.globl linkage_array

# making rtc, keyboard, and sysc functions global
.globl rtc, keyboard, sysc, pit


common_interrupt:

      # clear interrupt flag
      cli

      # pushing all the registers
      pushl %eax
      pushl %ebp
      pushl %edi
      pushl %esi
      pushl %edx
      pushl %ecx
      pushl %ebx

      # call the function
      call do_irq

      # popping all registers
      popl %ebx
      popl %ecx
      popl %edx
      popl %esi
      popl %edi
      popl %ebp

      # popping eax of the stack
      addl $4, %esp
      
      # popping the vector number off the stack
      addl $4, %esp

      # restoring interrupts and returning back
      sti
      iret

# syscall linkage
sysc:
      # check bounds for syscall
      cmpl $1, %eax
      jl bad_call
      cmpl $10, %eax
      jg bad_call

      # pushing all the registers
      pushl %ebp
      pushl %edi
      pushl %esi

      pushl %edx
      pushl %ecx
      pushl %ebx

      # call the function
      call *sys_jump_table-4(, %eax, 4)

      # popping all registers
      popl %ebx
      popl %ecx
      popl %edx

      popl %esi
      popl %edi
      popl %ebp

      # return back
      iret

# eax is not between 1 and 10, then return with eax = -1
bad_call:
      movl $-1, %eax
      iret

# syscall jump table
sys_jump_table:
      .long halt_handler, execute_handler, read_handler, write_handler, open_handler, close_handler, getargs_handler, vidmap_handler

# all the irq numbers are defined here.
# each label pushes the correct argument defined
# by the intel manual and then calls common_interrupt
# for the interrupt to be handled.
irq_0:
      pushl $0
      jmp common_interrupt

irq_1:
      pushl $1
      jmp common_interrupt

irq_2:
      pushl $2
      jmp common_interrupt

irq_3:
      pushl $3
      jmp common_interrupt

irq_4:
      pushl $4
      jmp common_interrupt

irq_5:
      pushl $5
      jmp common_interrupt

irq_6:
      pushl $6
      jmp common_interrupt

irq_7:
      pushl $7
      jmp common_interrupt

irq_8:
      pushl $8
      jmp common_interrupt

irq_9:
      pushl $9
      jmp common_interrupt

irq_10:
      pushl $10
      jmp common_interrupt

irq_11:
      pushl $11
      jmp common_interrupt

irq_12:
      pushl $12
      jmp common_interrupt

irq_13:
      pushl $13
      jmp common_interrupt

irq_14:
      pushl $14
      jmp common_interrupt

irq_15:
      pushl $15
      jmp common_interrupt

irq_16:
      pushl $16
      jmp common_interrupt

irq_17:
      pushl $17
      jmp common_interrupt

irq_18:
      pushl $18
      jmp common_interrupt

irq_19:
      pushl $19
      jmp common_interrupt

irq_20:
      pushl $20
      jmp common_interrupt

irq_21:
      pushl $21
      jmp common_interrupt

irq_22:
      pushl $22
      jmp common_interrupt

irq_23:
      pushl $23
      jmp common_interrupt

irq_24:
      pushl $24
      jmp common_interrupt

irq_25:
      pushl $25
      jmp common_interrupt

irq_26:
      pushl $26
      jmp common_interrupt

irq_27:
      pushl $27
      jmp common_interrupt

irq_28:
      pushl $28
      jmp common_interrupt

irq_29:
      pushl $29
      jmp common_interrupt

irq_30:
      pushl $30
      jmp common_interrupt

irq_31:
      pushl $31
      jmp common_interrupt

# user defined interrupts start here.
# mainly for rtc and keyboard
keyboard:
      pushl $0x21
      jmp common_interrupt

rtc:
      pushl $0x28
      jmp common_interrupt

pit:
      pushl $0x20
      jmp common_interrupt

# defining linkage array
linkage_array:
      .long irq_0
      .long irq_1
      .long irq_2
      .long irq_3
      .long irq_4
      .long irq_5
      .long irq_6
      .long irq_7
      .long irq_8
      .long irq_9
      .long irq_10
      .long irq_11
      .long irq_12
      .long irq_13
      .long irq_14
      .long irq_15
      .long irq_16
      .long irq_17
      .long irq_18
      .long irq_19
      .long irq_20
      .long irq_21
      .long irq_22
      .long irq_23
      .long irq_24
      .long irq_25
      .long irq_26
      .long irq_27
      .long irq_28
      .long irq_29
      .long irq_30
      .long irq_31
