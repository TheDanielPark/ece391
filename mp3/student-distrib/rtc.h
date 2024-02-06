#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* Ports and register values */
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define STAT_REG_A 0xA
#define STAT_REG_B 0xB
#define STAT_REG_C 0xC

/* Disable NMI values */
#define DISABLE_NMI_A 0x8A
#define DISABLE_NMI_B 0x8B

/* Masks */
#define NMI_MASK 0x80
#define RTC_IRQ 0x08
#define PREV_MASK 0x40
#define FREQ_MASK 0xF0
#define RATE_MASK 0x0F

/* Max frequency for RTC */
#define MAX_FREQ 1024
#define MIN_FREQ 2
#define MAX_FREQ_COUNT 6

/* Size of bytes to be written to buffer */
#define RTC_BUFF_SIZE 4

/* Initialize rtc */
void rtc_initialize(void);
/* rtc handler */
extern void rtc_interrupt_handler(void);
/* Set frequency of rtc */
int set_frequency(int freq);
/* Open functionaity */
int rtc_open(const uint8_t* filename);
/* Read functionality */
int rtc_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
/* Write functionality */
int rtc_write(int32_t fd, const void* buffer, int32_t bytes);
/* Close functionality */
int rtc_close(int32_t fd);

#endif /* _RTC_H */
