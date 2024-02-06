#include "i8259.h"
#include "keyboard.h"
#include "lib.h"
#include "tests.h"
#include "terminal.h"
#include "scheduler.h"


// Key modes 0=regular, 1 = caps, 2 = shift, 3 = caps + shift
static int mode = 0;
// 0 = unpressed; 1 = pressed
static int ctrlPressed = 0;
static int altPressed = 0;

volatile char keyBuffer[maxInputLength];
volatile int keyBufferIndex = 0;

/* 
 *  keys_map
 *   DESCRIPTION: Map of each keyboard key with types regular, caps, shift, caps + shift
 *   Ordering goes based on key code set 1
 *   Scan codes referenced from https://wiki.osdev.org/PS2_Keyboard 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
unsigned char keys_map[keyModes][keySize] = {
    // regular
    {0, '\t', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, // esc, ... , backspace | 0 - 14
     0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,    // tab, ... , enter | 15 - 28
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', 0,  // L ctrl, ... , L shift | 29 - 42
     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',',', '.', '/',               // \, ... , / | 42 - 53
     0, '*', 0, ' ', 0},                                                  // R shift, *, L alt, space , caps | 54 - 58
     // caps
    {0, '\t', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, // esc, ... , backspace | 0 - 14
     0,'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,     // tab, ... , enter | 15 - 28
     0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', 0,  // L ctrl, ... , L shift | 29 - 42
     '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',              // \, ... , / | 42 - 53
     0, '*', 0, ' ', 0},                                                  // R shift, *, L alt, space , caps | 54 - 58
     // shift
    {0, '\t', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, // esc, ... , backspace | 0 - 14
     0,'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,     // tab, ... , enter | 15 - 28
     0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', 0,   // L ctrl, ... , L shift | 29 - 42
     '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',               // \, ... , / | 42 - 53
     0, '*', 0, ' ', 0},                                                  // R shift, *, L alt, space , caps | 54 - 58                           
     // caps + shift
    {0, '\t', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, // esc, ... , backspace | 0 - 14
     0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 0,    // tab, ... , enter | 15 - 28
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', 0,   // L ctrl, ... , L shift | 29 - 42
     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?',              // \, ... , / | 42 - 53
     0, '*', 0, ' ', 0}                                                   // R shift, *, L alt, space , caps | 54 - 58
};

/* 
 *  keyboard_init
 *   DESCRIPTION: Initialize the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes keyboard by sending the enable irq signal for the keyboard
 */
void keyboard_init() {
    enable_irq(keyboardIRQ);
}

/* 
 *  key_press
 *   DESCRIPTION: Takes in the scan code for what key was pressed and prints corresponding key
 *   INPUTS: key - Which character in scan code to print
 *   OUTPUTS: keyValue - Which character was printed
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Creates a buffer which stores what keys were pressed and in what order
 */
void key_press(int key) {
    // Skip unknown key presses
    if (key >= keySize) {
        return;
    }
    if (keyBufferIndex < keyBufferSize - spacingSet) {
        char keyValue = keys_map[mode][key];

        // if null skip because handled by interrupt handler
        if (keyValue == NULL) {
            return;
        }

        if (ctrlPressed == PRESSED) {
            if (keyValue == 'l' || keyValue == 'L') {
                reset(); // created in lib.c
                int k;
                for (k = 0; k < keyBufferSize; k++) {
                    keyBuffer[k] = NULL;
                }
                keyBufferIndex = 0;
                return;
            }
            if (keyValue == 'c' || keyValue == 'C') {
                // TODO: Close Program?
                return;
            }
        }

        if (keyBufferIndex != 0 && keyBufferIndex % (screenWidth - 7) == 0) {
            keyBuffer[keyBufferIndex] = newLine;
            keyBufferIndex++;
            enter();
            vert_scroll();
            //putc(newLine);
            update_cursor();
        }

        putc_user(keyValue);

        keyBuffer[keyBufferIndex] = keyValue;
        keyBufferIndex++;
    }
    if (keyBufferIndex == keyBufferSize - 1) {
        keyBuffer[keyBufferIndex] = newLine;
    }

    update_cursor();

    return;
}

/* 
 *  keyboard_inter_handler
 *   DESCRIPTION: Keyboard Interrupt Handler which decides what happens when the keyboard interrupt signal is sent
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls the key press method based on what the key is
 */
void keyboard_inter_handler() {

    outb(outputBufferStatus, statusRegister);
    
	int input;
    input = inb(dataPort);

    // Key modes 0=regular, 1 = caps, 2 = shift, 3 = caps + shift
    switch (input) {
        case LSHIFT_PRESSED:
            mode = mode | SHIFTVALUE;
            break;
        case RSHIFT_PRESSED:
            mode = mode | SHIFTVALUE;
            break;
        case LSHIFT_RELEASED:
            mode = mode & ~SHIFTVALUE;
            break;
        case RSHIFT_RELEASED:
            mode = mode & ~SHIFTVALUE;
            break;
        case CAPS_PRESSED:
            mode = mode ^ MAKEHIGH;
            break;
        case LCTRL_PRESSED:
            ctrlPressed = MAKEHIGH;
            break;
        // case RCTRL_PRESSED:
        case LCTRL_RELEASED:
            ctrlPressed = MAKELOW;
            break;
        // case RCTRL_RELEASED:
        case LALT_PRESSED:
            altPressed = MAKEHIGH;
            break;
        case LALT_RELEASED:
            altPressed = MAKELOW;
            break;
        case F1:
            if (altPressed == 1) {
                terminal_switch(0);
            }
            break;
        case F2:
            if (altPressed == 1) {
                terminal_switch(1);
            }
            break;
        case F3:
            if (altPressed == 1) {
                terminal_switch(2);
            }
            break;
        case BACKSPACE:
            if (keyBufferIndex > 0) {
                if (keyBuffer[keyBufferIndex] == newLine) {
                    keyBuffer[keyBufferIndex] = NULL;
                    keyBufferIndex--;
                }
                backspace();
                keyBuffer[keyBufferIndex] = NULL;
                keyBufferIndex--;
            }
            update_cursor();
            break;
        case ENTER:
            enter_user();
            vert_scroll_user();
            keyBuffer[keyBufferIndex] = newLine;  
            keyBufferIndex++;
            //terminal_read(NULL, 0, (uint8_t *)keyBuffer, keyBufferIndex);
            update_cursor();
            break;

    }

	key_press(input);

    send_eoi(keyboardIRQ);

    return;

}
