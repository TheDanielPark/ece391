#ifndef _KEYBOARD_H
#define _KEYBOARD_H

// Magic Numbers
#define keyModes 4
#define keySize 59
#define keyBufferSize 128
#define keyboardIRQ 0x01
#define maxInputLength 127
#define outputBufferStatus 1
#define statusRegister  0x64
#define dataPort    0x60
#define LSHIFT_PRESSED  0x2A
#define RSHIFT_PRESSED  0x36
#define LSHIFT_RELEASED 0xAA
#define RSHIFT_RELEASED 0xB6
#define CAPS_PRESSED    0x3A
#define LCTRL_PRESSED   0x1D
// #define RCTRL_PRESSED   0x1D
#define LCTRL_RELEASED  0x9D
//#define RCTRL_RELEASED  0x9D
#define BACKSPACE   0x0E
#define ENTER   0x1C
#define LALT_PRESSED 0X38
#define LALT_RELEASED 0xB8
#define F1  0x3B
#define F2  0x3C
#define F3  0x3D
#define F4  0x3E
#define F5  0x3F
#define F6  0x40
#define F7  0x41
#define F8  0x42
#define F9  0x43
#define F10 0x44
#define SHIFTVALUE 2
#define MAKEHIGH 1
#define MAKELOW 0
#define PRESSED 1
#define newLine '\n'
#define screenWidth 80
#define spacingSet 2


// Buffer to hold all the keyboard inputs
extern volatile char keyBuffer[maxInputLength];
// Index for the keyBuffer
extern volatile int keyBufferIndex;

// Keyboard methods
void keyboard_init();
void key_press(int key);
void keyboard_inter_handler();

#endif /* _KEYBOARD_H */

