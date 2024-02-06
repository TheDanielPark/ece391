#include "types.h"

#define PIT_IRQ         0
#define PIT_CHANNEL     0x40
#define PIT_MODE        0x43
#define PIT_MASK1       0xFF
#define PIT_MASK2       0xFF00
#define PIT_REG         0x36
#define MAX_CLOCK       1193180
#define DEFAULT_CLOCK   100
#define KB_4            0x1000
#define NUM_TERM        3

/* struct for terminal */
typedef struct terminal
{
    uint8_t screen_x;
    uint8_t screen_y;

    uint8_t keyBuf[127];

    uint8_t visible;

    uint32_t vid_mem_addr;

    int8_t newline_tracker;
    int8_t enterFlag;
} terminal_t;

/* array for terminals */
extern terminal_t term_arr[NUM_TERM];

/* variable to keep of which pids need to be assigned the quanta */
extern volatile uint8_t sched_pid[NUM_TERM];

/* variable to keep track of which terminal we are on */
extern volatile uint8_t curr_terminal;

/* variable to keep track of which process is running right now */
extern volatile uint8_t curr_process;

/* initialize PIT */
void pit_init();

/* PIT interrupt handler */
void pit_interrupt_handler();

/* scheduler methods */
void scheduler();

/* terminal methods */
void init_terminal();

void terminal_save_state(uint8_t idx);

void terminal_restore_state(uint8_t idx);

int terminal_switch(uint8_t idx);

void remap_video(uint8_t idx);

void scheduler_remap_video(uint32_t idx);
