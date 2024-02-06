#include "paging.h"
#include "x86_desc.h"

// initialize paging struct
pde_t page_dir[PAGING_SIZE] __attribute__((aligned(four_kb)));
pte_t page_table[PAGING_SIZE] __attribute__((aligned(four_kb)));
pde_t page_virtual_mem[PAGING_SIZE] __attribute__((aligned(four_kb)));

/* 
 *  paging_initialize
 *   DESCRIPTION: initializes paging (page directory, page table, video memory, kernel memory)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void paging_initialize()
{
    // intialize variable for looping
    int i = 0;

    // initialize page directory and page table
    for (i = 0; i < PAGING_SIZE; i++)
    {
        page_dir[i].hex = 0x0;
        page_table[i].hex = 0x0;
    }

    // initialize page table
    page_dir[0].page_table_base_addr_pte = ((uint32_t)page_table >> SHIFT1);
    page_dir[0].read_write_pte = 1;
    page_dir[0].present_pte = 1;
    page_dir[0].user_pte = 1;
    page_dir[0].ps_pte = 0;

    // initialize kernel in page 4mb - 8mb
    page_dir[1].page_table_base_addr_entry = KERNEL_MEM >> SHIFT2;
    page_dir[1].read_write_entry = 1;
    page_dir[1].present_entry = 1;
    page_dir[1].ps_entry = 1;

    // initialize virtual memory in page table
    page_table[VIRUTAL_MEM >> SHIFT1].page_table_base_addr_pte = VIDEO_MEM >> SHIFT1;
    page_table[VIRUTAL_MEM >> SHIFT1].read_write_pte = 1;
    page_table[VIRUTAL_MEM >> SHIFT1].present_pte = 1;
    page_table[VIRUTAL_MEM >> SHIFT1].user_pte = 1;

    load_pde((uint32_t)page_dir);
}

/* 
 *  Loads the page address
 *   DESCRIPTION: Loads the page address
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Loads the page address
 */
inline void load_pde(uint32_t page_dir)
{
    //enable paging (code from osdev)
    asm volatile(
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        "movl %%cr4, %%eax;"
        "orl $0x010, %%eax;"
        "movl %%eax, %%cr4;"
        "movl %%cr0, %%eax;"
        "orl $0x80000000, %%eax;"
        "movl %%eax, %%cr0;"
        :
        : "r"(page_dir)
        : "eax");
}

/* 
 *  Flushes tlb
 *   DESCRIPTION: Flushes tlb
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flushes tlb
 */
inline void flush_tlb() {
       asm volatile (
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        :                      /* no outputs */
        :                      /* no inputs */
        :"%eax"                /* clobbered register */
    );
}
