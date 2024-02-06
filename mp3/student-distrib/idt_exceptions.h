/*
 * idt_exceptions.h
 *
 * This is the main header files for the IDT exceptions
 * that will be called on when any exception occurs, i.e.
 * divide by zero error, page fault, etc. The default handler
 * returns nothing but needs to be there to fill the rest of
 * the IDT.
*/

#include "lib.h"
#include "i8259.h"

// Magic numbers
#define interruptCount 32
#define syscallHex  0x80
#define setUserPerm 3
#define keyboardHex 0x21
#define rtcHex 0x28
#define pitHex 0x20


/* IDT initalizing function */
void idt_init();

/* Exception handlers */
extern void default_handler();

extern void exception_0();

extern void exception_1();

extern void exception_2();

extern void exception_3();

extern void exception_4();

extern void exception_5();

extern void exception_6();

extern void exception_7();

extern void exception_8();

extern void exception_9();

extern void exception_10();

extern void exception_11();

extern void exception_12();

extern void exception_13();

extern void exception_14();

extern void exception_15();

extern void exception_16();

extern void exception_17();

extern void exception_18();

extern void exception_19();
