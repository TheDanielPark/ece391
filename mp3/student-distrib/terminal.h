/*
 *  terminal.h
 *  Handles the reading in terminal and writing to terminal.
*/

#include "keyboard.h"

#ifndef _TERMINAL_H
#define _TERMINAL_H

/* Magic Numbers */
#define maxInputLength 127

/* Reads the input in terminal */
int32_t terminal_read(uint32_t fd, uint32_t offset, uint8_t* buf, uint32_t nbytes);

/* Writes to the terminal, given a buffer */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* Opens the file (does nothing since we have a read-only system) */
int32_t terminal_open(const uint8_t* filename);

/* Closes the file (does nothing since we have a read-only system) */
int32_t terminal_close(int32_t fd);

/* Variable for the terminal buffer */
extern char terminalBuffer[maxInputLength];

#endif /* _KEYBOARD_H */
