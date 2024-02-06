#include "i8259.h"
#include "keyboard.h"
#include "lib.h"
#include "tests.h"
#include "terminal.h"
#include "syscall.h"
#include "scheduler.h"


/* terminal_read
 * 	Description: Reads the inputs typed in terminal.
 * 	Inputs: fd, buf, nbytes
 * 	Outputs: Returns number of bytes read.
 * 	Side Effects: Reads the input from the terminal.
 */
int32_t terminal_read(uint32_t fd, uint32_t offset, uint8_t *buf, uint32_t nbytes)
{
    // initialize variables
    int i;
    int8_t count;
    count = 0;

    // get buffer
    uint8_t* tempBuffer = buf;

    //enable interrupts and check if user pressed enter
    sti();
    while(1) {if (term_arr[curr_terminal].enterFlag && (keyBuffer[0] != '\0') && (term_arr[curr_process].visible) == 1) break;}

    //mask interrupts
    cli();
    //iterate through keyboard buffer
    for (i = 0; i < nbytes || i < maxInputLength; i++)
    {
        // if not null
        if (keyBuffer[i] != NULL && keyBuffer[i] != '\n')
        {
            // write to buffer and increment count
            tempBuffer[i] = keyBuffer[i];
            count++;
        }
    }

    //(char*) buf = tempBuffer;
    // iterate through keyboard buffer
    for (i = 0; i < maxInputLength; i++)
    {
        // clear keyboard buffer
        keyBuffer[i] = NULL;
        keyBufferIndex = 0;
    }

    //enable interrupts again
    sti();

    // return chars read
    term_arr[curr_process].screen_x = get_screen_x();
    term_arr[curr_process].screen_y = get_screen_y();
    return count;
}


/* terminal_write
 * 	Description: Writes to the terminal.
 * 	Inputs: fd, buf, nbytes
 * 	Outputs: Remeber of bytes outputted
 * 	Side Effects: Writes the buffer to the terminal.
 */
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes)
{
    cli();

    set_screen_x(term_arr[curr_process].screen_x);
    set_screen_y(term_arr[curr_process].screen_y);

    // initialize variables
    int i;
    int8_t count = 0;
    term_arr[curr_process].newline_tracker = 0;
    term_arr[curr_process].enterFlag = 0;
    

    // get buffer
    char *tempBuffer = (char *)buf;
    
    // loop through buffer
    for (i = 0; i < nbytes; i++)
    {
        // check for \0
        if (tempBuffer[i] != '\0')
        {
            //check if new line
            if (tempBuffer[i] == '\n' && term_arr[curr_process].visible == 1)
            {
                // if new line set tracker to 0
                newline_check_user();
                vert_scroll_user();
                term_arr[curr_terminal].newline_tracker = 0;
            }
            else if(tempBuffer[i] == '\n')
            {
                // if new line set tracker to 0
                newline_check();
                vert_scroll();
                term_arr[curr_process].newline_tracker = 0;
            }
            // print buffer at index i and iterate count and newline tracker
            if(!term_arr[curr_process].visible)
            {
                putc(tempBuffer[i]);
            }
            else
            {
                putc_user(tempBuffer[i]);
            }
            update_cursor();
            count++;
            term_arr[curr_process].newline_tracker++;

            // check if newline tracker greater than num cols
            if (term_arr[curr_process].newline_tracker >= NUM_COLS)
            {
                // call enter, check for vertical scrolling and set newline tracker to 0
                if(!term_arr[curr_process].visible)
                {
                    enter();
                    vert_scroll();
                    term_arr[curr_process].newline_tracker = 0;
                }
                else
                {
                    enter_user();
                    vert_scroll_user();
                    term_arr[curr_terminal].newline_tracker = 0;
                }
            }
        }
    }

    term_arr[curr_process].screen_x = get_screen_x();
    term_arr[curr_process].screen_y = get_screen_y();

    sti();

    // return chars written
    return count;
}

/* terminal_open
 * 	Description: Opens the file but since we have a read-only
 *  system, it does nothing.
 * 	Inputs: filename
 * 	Outputs: Returns 0.
 * 	Side Effects: None.
 */
int32_t terminal_open(const uint8_t *filename)
{
    return 0;
}

/* terminal_close
 * 	Description: Closes the file but since we have a read-only
 *  system, it does nothing.
 * 	Inputs: fd
 * 	Outputs: Returns 0.
 * 	Side Effects: None.
 */
int32_t terminal_close(int32_t fd)
{
    return 0;
}
