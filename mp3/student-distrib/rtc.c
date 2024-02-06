#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"

#define NUM_COLS 80

/* Local rtc variables */
volatile uint32_t rtc_tick;
volatile int32_t rtc_status;

/* 
 *  rtc_initialize
 *   DESCRIPTION: Initialize the RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes RTC by sending data to ports
 */
void rtc_initialize(void)
{
    /* Critical section */
    cli();

    /* Disable NMI */
    outb(DISABLE_NMI_A, RTC_PORT);
    outb(DISABLE_NMI_B, RTC_PORT);

    /* Turn on IRQ 8 */
    char prev = inb(CMOS_PORT);
    outb(DISABLE_NMI_B, RTC_PORT);
    outb(prev | PREV_MASK, CMOS_PORT);
    enable_irq(RTC_IRQ);

    /* Set local vars */
    rtc_tick = 0;
    rtc_status = 0;

    sti();
}

/* 
 *  rtc_interrupt_handler
 *   DESCRIPTION: Interrupt handler for rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: RTC functions on computer
 */
void rtc_interrupt_handler(void)
{
    /* Set local vars */
    rtc_tick++;
    rtc_status = 1;

    /* Call test_interrupt to test RTC */
    //test_interrupts();

    // cli();
    // putc('1');
    // update_cursor();
    // if(rtc_tick >= NUM_COLS)
    // {
    //     enter();
    //     vert_scroll();
    //     rtc_tick = 0;
    // }
    // sti();

    /* Write/read ports */
    outb(STAT_REG_C, RTC_PORT);
    inb(CMOS_PORT);

    /* Send EOI */
    send_eoi(RTC_IRQ);
}

/* 
 *  set_frequency
 *   DESCRIPTION: Set frequency for RTC
 *   INPUTS: freq - integer that should be a power of two which will be used to set frequency
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful, otherwise -1
 *   SIDE EFFECTS: RTC frequency will change
 */
int set_frequency(int freq)
{
    int i;     // Loop to check frequency
    int temp;  //
    int count; //
    temp = -1; // flag to check for valid frequency
    count = MAX_FREQ_COUNT; // rate for 1024 hz
    rtc_tick = 0; //reset the counter

    // loop to check if frequency is valid and to find the rate
    for (i = MAX_FREQ; i >= 2; i /= 2)
    {
        if (freq == i)
        {
            temp = count;
            break;
        }
        count++;
    }

    // return -1 if frequency invalid
    if (temp == -1)
    {
        return -1;
    }

    // obtain rate;
    uint8_t rate = (uint8_t)temp;
    rate &= RATE_MASK;

    //begin critical section
    cli();

    // disable nmi and set frequency
    outb(DISABLE_NMI_A, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(DISABLE_NMI_A, RTC_PORT);
    outb((prev & FREQ_MASK) | rate, CMOS_PORT);

    //end critical section
    sti();

    return 0;
}

/* 
 *  rtc_open
 *   DESCRIPTION: Open functionality for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: return value of set_frequency
 *   SIDE EFFECTS: Sets rtc frequency to 2 and the tick to 0
 */
int rtc_open(const uint8_t* filename)
{
    rtc_tick = 0;
    rtc_status = 0;

    // set freqency to 2 and returns
    return set_frequency(MIN_FREQ);
}

/* 
 *  rtc_read
 *   DESCRIPTION: Read functionality for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Checks the rtc status. If there was an interrupt, set the status back to 0, otherwise spin
 */
int rtc_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    //check for rtc interrupt and set status when interupt complete
    while (!rtc_status)
    {
    }
    rtc_status = 0;

    return 0;
}

/* 
 *  rtc_write
 *   DESCRIPTION: Writes to RTC
 *   INPUTS: buffer - What to write to RTC
 *           bytes - How many bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: return value of set_frequency
 *   SIDE EFFECTS: Will write to RTC based on buffer and bytes
 */
int rtc_write(int32_t fd, const void *buffer, int32_t nbytes)
{
    // check if bytes are not 4 and buffer is null; return -1 if true
    if (nbytes != RTC_BUFF_SIZE || buffer == NULL)
    {
        return -1;
    }

    // set frequency based on buffer and return
    return set_frequency(*(int *)buffer);
}

/* 
 *  rtc_close
 *   DESCRIPTION: Close functionality for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Returns 0
 */
int rtc_close(int32_t fd)
{
    return 0;
}
