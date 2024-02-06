/*
 * Paging.h
 * This is the header file for initializing pages.
 * This file mainly sets the structs.
 */

#include "lib.h"

#define PAGING_SIZE 1024
#define four_kb     4096
#define SHIFT1      12
#define SHIFT2      22
#define KERNEL_MEM 0x400000                // kernel memory address
#define VIDEO_MEM 0xB8000                  // virstual video memory address
#define VIRUTAL_MEM (VIDEO_MEM & 0x3FF000) // video memory address for array

/* Adding structs*/

/* This is the struct for the page directory entry table */
typedef struct pde
{
    union
    {
        uint32_t hex;
        struct
        {
            uint8_t present_pte : 1;
            uint8_t read_write_pte : 1;
            uint8_t user_pte : 1;
            uint8_t pwt_pte : 1;
            uint8_t pcd_pte : 1;
            uint8_t a_pte : 1;
            uint8_t zero_pte : 1;
            uint8_t ps_pte : 1;
            uint8_t g_pte : 1;
            uint8_t avail_pte : 3;
            uint32_t page_table_base_addr_pte : 20;
        } __attribute__((packed));
        struct
        {
            uint8_t present_entry : 1;
            uint8_t read_write_entry : 1;
            uint8_t user_entry : 1;
            uint8_t pwt_entry : 1;
            uint8_t pcd_entry : 1;
            uint8_t a_entry : 1;
            uint8_t d_entry : 1;
            uint8_t ps_entry : 1;
            uint8_t g_entry : 1;
            uint8_t avail_entry : 3;
            uint8_t pat_entry : 1;
            uint16_t reserved_entry : 9;
            uint32_t page_table_base_addr_entry : 10;
        } __attribute__((packed));
    };
} pde_t;

/* This is the struct for page table entries */
typedef struct pte
{
    union
    {
        uint32_t hex;
        struct
        {
            uint8_t present_pte : 1;
            uint8_t read_write_pte : 1;
            uint8_t user_pte : 1;
            uint8_t pwt_pte : 1;
            uint8_t pcd_pte : 1;
            uint8_t a_pte : 1;
            uint8_t d_pte : 1;
            uint8_t pat_pte : 1;
            uint8_t g_pte : 1;
            uint8_t avail_pte : 3;
            uint32_t page_table_base_addr_pte : 20;
        } __attribute__((packed));
    };
} pte_t;

/* defining variables */
extern pde_t page_dir[PAGING_SIZE] __attribute__((aligned(four_kb)));
extern pte_t page_table[PAGING_SIZE] __attribute__((aligned(four_kb)));
extern pde_t page_virtual_mem[PAGING_SIZE] __attribute__((aligned(four_kb)));

/* Initializing paging function */
extern void paging_initialize();

/* Load pde function */
extern inline void load_pde(uint32_t page_dir);

/* Flush tlb function */
extern inline void flush_tlb();
