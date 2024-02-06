/*
 * idt_exceptions.c
 * 
 * We will call the initalizing function and handle any interrupts here.
 * For more details check idt_exceptions.h.
*/

#include "lib.h"
#include "types.h"
#include "debug.h"
#include "x86_desc.h"
#include "idt_exceptions.h"
#include "keyboard.h"
#include "rtc.h"
#include "syscall.h"
#include "scheduler.h"

/* 
 * This is the handler table. It will be called upon when
 * the do_irq function is called. This basically functions as
 * a jump table.
*/
void (*handler_table[NUM_VEC])();

/* These are the linkage file functions. For more details check link.S */
extern void rtc();
extern void keyboard();
extern void sysc();
extern void pit();

/* array size is 32 since there are 0-31 intel defined interrupts */
extern void * linkage_array[interruptCount];

/* 
 *  idt_init
 *   DESCRIPTION: Initalizes the IDT. Sets the correct bits,
 *          sets the IDT entries, and fills the handler table
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes IDT and fills the handler table
 */
void idt_init()
{
      /* Loop through all the IDT entries and set their corresponding bits */
      int i;
      for(i = 0; i < NUM_VEC; i++)
      {
            idt[i].seg_selector = KERNEL_CS;
            idt[i].reserved4 = 0;
            idt[i].reserved3 = 0;
            idt[i].reserved2 = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved0 = 0;
            idt[i].size = 1;
            idt[i].dpl = 0;
            idt[i].present = 1;

            /* special case for syscall */
            if(i == syscallHex)
            {
                  idt[i].reserved3 = 1;
                  idt[i].dpl = setUserPerm;
            }

            /* sets the default handler for all the IDT entries */
            SET_IDT_ENTRY(idt[i], default_handler);
      }

      /* 
       * set the exceptions according to the intel manual.
       * 0-20 are intel defined interrupts. 20-31 are intel reserved
       * interrupts. Sets the IDT accordingly using the linkage file.
      */
      for(i = 0; i < interruptCount; i++)
      {
            SET_IDT_ENTRY(idt[i], linkage_array[i]);
      }

      /* user defined interrupts. For rtc, keyboard, etc */
      SET_IDT_ENTRY(idt[keyboardHex], &keyboard);           //keyboard (0x21)
      SET_IDT_ENTRY(idt[rtcHex], &rtc);                     //rtc (0x28)
      SET_IDT_ENTRY(idt[syscallHex], &sysc);                //syscall (0x80)
      SET_IDT_ENTRY(idt[pitHex], &pit);

      /* filling up the handler table */
      handler_table[0] = exception_0;
      handler_table[1] = exception_1;
      handler_table[2] = exception_2;
      handler_table[3] = exception_3;
      handler_table[4] = exception_4;
      handler_table[5] = exception_5;
      handler_table[6] = exception_6;
      handler_table[7] = exception_7;
      handler_table[8] = exception_8;
      handler_table[9] = exception_9;
      handler_table[10] = exception_10;
      handler_table[11] = exception_11;
      handler_table[12] = exception_12;
      handler_table[13] = exception_13;
      handler_table[14] = exception_14;
      handler_table[15] = exception_15;
      handler_table[16] = exception_16;
      handler_table[17] = exception_17;
      handler_table[18] = exception_18;
      handler_table[19] = exception_19;
      handler_table[20] = exception_1;
      handler_table[21] = exception_1;
      handler_table[22] = exception_1;
      handler_table[23] = exception_1;
      handler_table[24] = exception_1;
      handler_table[25] = exception_1;
      handler_table[26] = exception_1;
      handler_table[27] = exception_1;
      handler_table[28] = exception_1;
      handler_table[29] = exception_1;
      handler_table[30] = exception_1;
      handler_table[31] = exception_1;

      /* user defined interrupts */
      handler_table[keyboardHex] = keyboard_inter_handler;
      handler_table[rtcHex] = rtc_interrupt_handler;
      handler_table[pitHex] = pit_interrupt_handler;
}

/* 
 *  do_irq
 *   DESCRIPTION: This is the dispatcher function that works with the jump
 *                table to handle any incoming exceptions.
 *   INPUTS: All 8 registers, vectorr_number
 *   OUTPUTS: Function to be called
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls the function according to the jump table
 */
void do_irq(unsigned long EBX,
            unsigned long ECX,
            unsigned long EDX,
            unsigned long ESI,
            unsigned long EDI,
            unsigned long EBP,
            unsigned long EAX,
            unsigned long vector_num)
{
      /* use the jump table */
      handler_table[vector_num]();
}

/* 
 * defining exception handlers. These are called 
 * whenever an exception occurs.
 */
extern void default_handler()
{
      return;
}

extern void exception_0()
{
      clear();
      printf("\n DIVIDE ERROR");
      while(1);
}

extern void exception_1()
{
      clear();
      printf("\n RESERVED");
      while(1);
}

extern void exception_2()
{
      clear();
      printf("\n NMI Interrupt");
      while(1);
}

extern void exception_3()
{
      clear();
      printf("\n BREAKPOINT");
      while(1);
}

extern void exception_4()
{
      clear();
      printf("\n Overflow");
      while(1);
}

extern void exception_5()
{
      clear();
      printf("\n BOUND Range Exceeded");
      while(1);
}

extern void exception_6()
{
      clear();
      printf("\n Invalid Opcode (Undefined Opcode)");
      while(1);
}

extern void exception_7()
{
      clear();
      printf("\n Device Not Available (No Math Coprocessor)");
      while(1);
}

extern void exception_8()
{
      clear();
      printf("\n Double Fault");
      while(1);
}

extern void exception_9()
{
      clear();
      printf("\n Coprocessor Segment Overrun (reserved)");
      while(1);
}

extern void exception_10()
{
      clear();
      printf("\n Invalid TSS");
      while(1);
}

extern void exception_11()
{
      clear();
      printf("\n Segment Not Present");
      while(1);
}

extern void exception_12()
{
      clear();
      printf("\n Stack-Segment Fault");
      while(1);
}

extern void exception_13()
{
      clear();
      printf("\n General Protection");
      while(1);
}

extern void exception_14()
{
      //clear();
      printf("\n Page Fault");
      while(1);
}

extern void exception_15()
{
      clear();
      printf("\n Intel Reserved");
      while(1);
}

extern void exception_16()
{
      clear();
      printf("\n x87 FPU Floating-Point Error (Math Fault)");
      while(1);
}

extern void exception_17()
{
      clear();
      printf("\n Alignment Check");
      while(1);
}

extern void exception_18()
{
      clear();
      printf("\n Machine Check");
      while(1);
}

extern void exception_19()
{
      clear();
      printf("\n SIMD Floating-Point Exception");
      while(1);
}
