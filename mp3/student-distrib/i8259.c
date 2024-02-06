/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 * Code referenced from https://wiki.osdev.org/PIC
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Helper function for delay */
static inline void io_wait(void);


/* 
 *  i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes PIC by setting masks and sending data to PIC
 */
void i8259_init(void) {
    
    /* Mask out all interrupts */
    master_mask = MASK_ALL;
    slave_mask = MASK_ALL;
    outb(master_mask, MASTER_8259_PORT_DATA);
    outb(slave_mask, SLAVE_8259_PORT_DATA);

    /* Send initialization command to PICs, expects three more data inputs */
    outb(ICW1, MASTER_8259_PORT);
    io_wait();
    outb(ICW1, SLAVE_8259_PORT);
    io_wait();

    /* Send vector offsets */
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    io_wait();
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);
    io_wait();

    /* Send master/slave connection info */
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    io_wait();
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);
    io_wait();

    /* Send additional info about environment */
    outb(ICW4, MASTER_8259_PORT_DATA);
    io_wait();
    outb(ICW4, SLAVE_8259_PORT_DATA);
    io_wait();

    /* Mask interrupt again */
    outb(master_mask, MASTER_8259_PORT_DATA);
    outb(slave_mask, SLAVE_8259_PORT_DATA);

    /* Enable the slave interrupt on master PIC */
    enable_irq(SLAVE_INTR);
}


/* 
 *  enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: irq_num - Which interrupt to unmask
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Unmasks the device interrupt on PIC
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port; // Which PIC to unmask
    uint8_t value; // How we are unmasking
    uint32_t num; // Interrupt number
    num = irq_num;

    /* If out of bounds irq, then return */
    if(irq_num < 0 || irq_num > MAX_IRQ)
    {
        return;
    }

    /* Choose PIC */
    if(irq_num >= 0 && irq_num <= MAX_IRQ_MASTER) {
        port = MASTER_8259_PORT_DATA;
    } else {
        port = SLAVE_8259_PORT_DATA;
        num -= SLAVE_DIFF;
    }

    /* Unmask bit based on irq_num */
    value = inb(port) & ~(1 << num);
    outb(value, port);
}


/* 
 *  disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: irq_num - Which interrupt to mask
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Masks the device interrupt on PIC
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port; // Which PIC to mask
    uint8_t value; // How we are masking
    uint32_t num; // Interrupt number
    num = irq_num;

    /* If out of bounds irq, then return */
    if(irq_num < 0 || irq_num > MAX_IRQ)
    {
        return;
    }

    /* Choose PIC */
    if(irq_num >= 0 && irq_num <= MAX_IRQ_MASTER) {
        port = MASTER_8259_PORT_DATA;
    } else {
        port = SLAVE_8259_PORT_DATA;
        num -= SLAVE_DIFF;
    }

    /* Set the mask bit based on irq_num */
    value = inb(port) | (1 << num);
    outb(value, port);
}


/* 
 *  send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num - Which interrupt to EOI
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends EOI signal to PIC
 */
void send_eoi(uint32_t irq_num) {

    /* If out of bounds irq, then return */
    if(irq_num < 0 || irq_num > MAX_IRQ)
    {
        return;
    }

    /* Check if master/slave PIC and send EOI signal */
    if(irq_num >= 0 && irq_num <= MAX_IRQ_MASTER) {
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    else {
        irq_num -= SLAVE_DIFF;
        outb(EOI | irq_num, SLAVE_8259_PORT);
        outb(EOI | SLAVE_INTR, MASTER_8259_PORT);
    }
}

/* 
 *  io_wait
 *   DESCRIPTION: Delay helper function
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Delays/wait small time for I/O lag
 */
static inline void io_wait(void)
{
    outb(0, 0x80);
}
