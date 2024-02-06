/*
 * This file is mainly responsible for creating structures and
 * defining function headers that will be used for system call
 * functionality.
*/

#include "lib.h"
#include "types.h"

#define MASK_PCB    0xFFFFE000
#define MB_128      0x08000000
#define KB_8        0x00002000
#define MB_8        0x00800000
#define GB_1        0x40000000
#define GB_idx      256
#define virtualAddr 0x08048000
#define userCount   0x83FFFFC
#define fdMax       7
#define fdMin       0
#define noOffset    0
#define byte4       4
#define PCB_SIZE    8
#define HALT        1
#define EXECUTE     2
#define READ        3
#define WRITE       4
#define OPEN        5
#define CLOSE       6
#define keyBufferSize   128
#define programMax      6
#define bottomKernal    0x800000
#define kernalStackSize 0x2000
#define pageSize        0x400000
#define countCheck  2
#define shiftCount  22
#define pageDirIndex    32
#define offset_24  24
#define maskCount   8
#define exe0    0x7F
#define exe1    0x45
#define exe2    0x4C    
#define exe3    0x46

/* Halth function */
int32_t halt_handler(uint8_t status);

/* Execute function */
int32_t execute_handler(const uint8_t* command);

/* Read function */
int32_t read_handler(int32_t fd, void* buf, int32_t nbytes);

/* Write function */
int32_t write_handler(int32_t fd, const void* buf, int32_t nbytes);

/* Open function */
int32_t open_handler(const uint8_t* filename);

/* Close function */
int32_t close_handler(int32_t fd);

/* Get args function */
int32_t getargs_handler(uint8_t * buf, int32_t nbytes);

/* Vidmap function */
int32_t vidmap_handler(uint8_t** screen_start);

/* Defining structures */

/* File operations table pointer structure */
typedef struct file_operations_table_pointer
{
    int32_t (* read_ptr)    (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
    int32_t (* write_ptr)   (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (* open_ptr)    (const uint8_t* filename);
    int32_t (* close_ptr)   (int32_t fd);
} file_operations_table_pointer_t;

/* File descriptors structure */
typedef struct file_descriptor
{
    file_operations_table_pointer_t operations_pointer;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} file_descriptor_t;

/* PCB structure */
typedef struct pcb
{
    file_descriptor_t pcb_arr[PCB_SIZE];
    uint8_t pid;
    uint8_t parent_pid;
    uint32_t esp;
    uint32_t kesp;
    uint32_t ebp;
    uint32_t ss0;
    uint32_t esp0;
    uint8_t argbuf[keyBufferSize];
    uint8_t argsflag;
    struct pcb* parent_pcb;
    int32_t rtc_val;
    uint32_t rtc_flag;
} pcb_t;

/* Keep tracking of current pcb pointer */
extern pcb_t* pcb_current;

/* Program counter */
extern uint8_t program_count;

extern uint8_t pid_arr[programMax];
