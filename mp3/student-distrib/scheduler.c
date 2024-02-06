#include "scheduler.h"
#include "syscall.h"
#include "i8259.h"
#include "paging.h"
#include "filesystem.h"
#include "x86_desc.h"
#include "keyboard.h"
#include "lib.h"
#include "rtc.h"

int32_t video_addr[4] = {VIDEO_MEM + 1 * KB_4, VIDEO_MEM + 2 * KB_4, VIDEO_MEM + 3 * KB_4, VIDEO_MEM};
volatile uint8_t sched_pid[NUM_TERM] = {0, 0, 0};
volatile uint8_t curr_process = 0;
volatile uint8_t curr_terminal = 0;
terminal_t term_arr[NUM_TERM];

extern int32_t execute(const uint8_t* command);

void pit_init()
{
	/* disable interrupts */
	cli();

    /* define the count variable */
    int count = MAX_CLOCK / DEFAULT_CLOCK;

    /* initializing the PIT */
    outb(PIT_REG, PIT_MODE);
	outb(count & PIT_MASK1, PIT_CHANNEL);
	outb((count & PIT_MASK2) >> 8, PIT_CHANNEL);

    /* enable IRQ 0 */
    enable_irq(PIT_IRQ);

    /* enable interrupts */
    sti();
}

void pit_interrupt_handler()
{
    /* disbale interrupts */
    cli();

    /* send eoi */
    send_eoi(PIT_IRQ);

    /* start scheduler */
    scheduler();

    /* enable interrupts */
    sti();
}

void scheduler()
{
    pcb_t *next_pcb, *cur_pcb;
    uint32_t next_process;
    int32_t cur_kesp, cur_kebp, next_kesp, next_kebp;
    
    if(sched_pid[0] == 0)
    {
        sched_pid[0] = 1;
        video_mem = (char *) video_addr[0];

        next_kesp = bottomKernal - 4;
        next_kebp = bottomKernal - 4;

        asm volatile(
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            :
            : "r" (next_kesp), "r" (next_kebp)

        );

        execute((uint8_t*)"shell");
        return;
    }

    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        : "=r" (cur_kesp), "=r" (cur_kebp)
    );

    cur_pcb = (pcb_t*)(bottomKernal - (sched_pid[curr_process]) * kernalStackSize);
    cur_pcb->esp = cur_kesp;
    cur_pcb->ebp = cur_kebp;

    next_process = (curr_process + 1) % 3;
    video_mem = (char*) video_addr[next_process];

    if(sched_pid[next_process] == 0)
    {
        sched_pid[next_process] = sched_pid[curr_process] + 1;
        curr_process = next_process;

        video_mem = (char*)video_addr[curr_process];

        next_kesp = bottomKernal - (sched_pid[next_process]) * kernalStackSize - 4;
        next_kebp = bottomKernal - (sched_pid[next_process]) * kernalStackSize - 4;
        scheduler_remap_video(next_process);

        asm volatile(
            "movl %0, %%esp;"
            "movl %1, %%ebp;"
            :
            : "r" (next_kesp), "r" (next_kebp)

        );

        execute((uint8_t*)"shell");
        return;
    }

    curr_process = next_process;

    video_mem = (char*) video_addr[curr_process];
    next_pcb = (pcb_t*)(bottomKernal - (sched_pid[next_process]) * kernalStackSize);
    pcb_current = next_pcb;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = bottomKernal - (sched_pid[next_process] - 1) * kernalStackSize - 4;

    if (sched_pid[0] == 4 && cur_pcb->kesp <= 0x7f8f00) {
        cur_pcb->kesp = 0;
    }
    
    page_dir[pageDirIndex].hex = 0;
    page_dir[pageDirIndex].page_table_base_addr_entry = (bottomKernal + (next_pcb->pid - 1) * pageSize) >> shiftCount;
    page_dir[pageDirIndex].present_entry = 1;
    page_dir[pageDirIndex].read_write_entry = 1;
    page_dir[pageDirIndex].ps_entry = 1;
    page_dir[pageDirIndex].user_entry = 1;

    load_pde((uint32_t)page_dir);
    flush_tlb();

    scheduler_remap_video(next_process);
    
    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        : "r" (next_pcb->esp), "r" (next_pcb->ebp)

    );

    sti();

    return;
}

void init_terminal()
{
  int i;
  for(i = 0; i < NUM_TERM; i++)
  {
    term_arr[i].screen_x = 0;
    term_arr[i].screen_y = 0;
    term_arr[i].visible = 0;
    term_arr[i].vid_mem_addr = video_addr[i];
    term_arr[i].newline_tracker = 0;
    term_arr[i].enterFlag = 0;
  }
  curr_terminal = 0;
  term_arr[curr_terminal].visible = 1;
}

int terminal_switch(uint8_t idx)
{
    if (idx == curr_terminal)
    {
        return 0;
    }

    else if (idx >= 3)
    {
        return -1;
    }

    terminal_save_state((int8_t)curr_terminal);
    terminal_restore_state((int8_t)idx);

    update_cursor();

    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].page_table_base_addr_pte = VIDEO_MEM >> 12;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].present_pte = 1;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].read_write_pte = 1;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].user_pte = 1;

    page_virtual_mem[0].page_table_base_addr_pte = VIDEO_MEM >> SHIFT1;
    page_virtual_mem[0].read_write_pte = 1;
    page_virtual_mem[0].user_pte = 1;
    page_virtual_mem[0].present_pte = 1;

    flush_tlb();

    return 0;
}

void terminal_save_state(uint8_t idx)
{
    if(idx >= 3)
    {
        return;
    }
    term_arr[curr_terminal].visible = 0;
    //term_arr[idx].input_location = input_location;
    //term_arr[idx].new_lines_location = new_lines_location;
    memcpy((uint8_t*)term_arr[idx].keyBuf,(uint8_t*)keyBuffer, maxInputLength);
    //memcpy((uint8_t*)term_arr[idx].new_lines,(uint8_t*)new_lines, MAX_INPUT_LENGTH);

    //@TODO: //how to get the corsor
    // terminals[id].screen_x = screen_x;
    // terminals[id].screen_y = screen_y;

    remap_video(idx);
    memcpy((uint8_t*)term_arr[idx].vid_mem_addr, (uint8_t*)VIDEO_MEM, NUM_COLS * NUM_ROWS * 2);
}

void terminal_restore_state(uint8_t idx)
{
    if(idx >= 3)
    {
        return;
    }
    curr_terminal = idx;
    term_arr[curr_terminal].visible = 1;
    //input_location = terminals[id].input_location;
    //new_lines_location = terminals[id].new_lines_location;

    memcpy((uint8_t*)keyBuffer, (uint8_t*)term_arr[idx].keyBuf, maxInputLength);
    //memcpy((uint8_t*)new_lines, (uint8_t*)terminals[id].new_lines, MAX_INPUT_LENGTH);

    remap_video(idx);
    memcpy((uint8_t*)VIDEO_MEM, (uint8_t*)term_arr[idx].vid_mem_addr, NUM_COLS * NUM_ROWS * 2);
}

void remap_video(uint8_t idx)
{
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].page_table_base_addr_pte = term_arr[idx].vid_mem_addr >> 12;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].present_pte = 1;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].read_write_pte = 1;
    page_table[(term_arr[idx].vid_mem_addr & 0x3FF000) >> 12].user_pte = 1;

    page_virtual_mem[0].page_table_base_addr_pte = term_arr[idx].vid_mem_addr >> 12;
    page_virtual_mem[0].present_pte = 1;
    page_virtual_mem[0].read_write_pte = 1;
    page_virtual_mem[0].user_pte = 1;

    flush_tlb();
}


void scheduler_remap_video(uint32_t idx)
{
    /* check the validity of input */
    if (idx < 0 || idx > 2)
    {
        return;
    }

    int32_t phys_vid_addr, vir_vid_addr;

    /* calculate the target physical mem region according to the terminal num */
    if (idx == curr_terminal)
    {
        phys_vid_addr = VIDEO_MEM;
        vir_vid_addr = VIDEO_MEM + (1 + idx) * KB_4;
    }
    else
    {
        phys_vid_addr = VIDEO_MEM + (1 + idx) * KB_4;
        vir_vid_addr = phys_vid_addr;
    }
    //PTE for video memory
    page_table[(vir_vid_addr & 0x3FF000) >> 12].page_table_base_addr_pte = (phys_vid_addr) >> 12;
    page_table[(vir_vid_addr & 0x3FF000) >> 12].present_pte = 1;
    page_table[(vir_vid_addr & 0x3FF000) >> 12].read_write_pte = 1;
    page_table[(vir_vid_addr & 0x3FF000) >> 12].user_pte = 1;

    /* check if the target terminal is using vid map for userprogram */
    //PTE for video memory
    page_virtual_mem[0].page_table_base_addr_entry = (phys_vid_addr) >> 12;
    page_virtual_mem[0].present_entry = 1;
    page_virtual_mem[0].read_write_entry = 1;
    page_virtual_mem[0].user_entry = 1;

    flush_tlb();
}
