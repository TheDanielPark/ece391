#include "lib.h"
#include "types.h"
#include "syscall.h"
#include "rtc.h"
#include "terminal.h"
#include "filesystem.h"
#include "paging.h"
#include "x86_desc.h"
#include "scheduler.h"

extern int32_t execute(const uint8_t* command);

/* Keeping count of how many programs are running to make sure we don't exceed */
uint8_t program_count = 0;

/* Temporary getargs flag (checks if arguments are present or not) */
int args_flag = 0;

uint8_t pid_arr[programMax] = {0, 0, 0, 0, 0, 0};

/* fops variable for all the file types */
file_operations_table_pointer_t filesystem_operations_table = {&file_read, &file_write, &file_open, &file_close};
file_operations_table_pointer_t directory_operations_table = {&directory_read, &directory_write, &directory_open, &directory_close};
file_operations_table_pointer_t terminal_operations_table = {&terminal_read, &terminal_write, &terminal_open, &terminal_close};
file_operations_table_pointer_t rtc_operations_table = {&rtc_read, &rtc_write, &rtc_open, &rtc_close};

/* Current pcb pointer */
pcb_t *pcb_current = NULL;

/* halt_handler
 * 	Description: Halts the program that is executing
 * 	Inputs: status
 * 	Outputs: 0 on success -1 on failure
 * 	Side Effects:
 */
int32_t halt_handler(uint8_t status)
{
    /* start of critical section */
    cli();

    uint32_t ebp_parent;
    uint32_t esp_parent;

    /* if only three shells are open, then pass */
    if(pcb_current->pid <= 3)
    {
        printf("Can't exit base shell!");
        return 0;
    }
    /* otherwise decrement program count and current display pid*/
    else
    {
        pid_arr[pcb_current->pid - 1] = 0;
        sched_pid[curr_process] = pcb_current->parent_pid;
        program_count--;
    }

    /* get the parent esp and parent ebp */
    ebp_parent = pcb_current->parent_pcb->ebp;
    esp_parent = pcb_current->parent_pcb->esp;

    /* restore parent paging */
    page_dir[pageDirIndex].hex = 0;
    page_dir[pageDirIndex].page_table_base_addr_entry = (bottomKernal + (pcb_current->parent_pid - 1) * pageSize) >> shiftCount;
    page_dir[pageDirIndex].present_entry = 1;
    page_dir[pageDirIndex].read_write_entry = 1;
    page_dir[pageDirIndex].ps_entry = 1;
    page_dir[pageDirIndex].user_entry = 1;

    /* load page and flush tlb */
    load_pde((uint32_t)page_dir);
    flush_tlb();

    /* restore parent data */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = pcb_current->parent_pcb->esp;

    /* clear out the argbuf */
    int i;
    for (i = 0; i < keyBufferSize; i++)
    {
        pcb_current->argbuf[i] = '\0';
    }

    /* set argbuf flag to low */
    pcb_current->argsflag = 0;

    /* close handlers */
    close_handler(2);
    close_handler(3);
    close_handler(4);
    close_handler(5);
    close_handler(6);
    close_handler(7);

    /* restore parent pcb */
    pcb_current = pcb_current->parent_pcb;

    /* jump back to syscall linkage and also enable interrupts */
    asm volatile(
        "movl %0, %%ebp;"
        "movl %2, %%esp;"
        "movl $0, %%eax;"
        "movb %1, %%al;"
        "sti;"
        "leave;"
        "ret;"
        :
        : "r"(ebp_parent), "r"(status), "r" (esp_parent)
        : "%eax");

    return 0;
}

/* execute_handler
 * 	Description: execute the program given command
 * 	Inputs: command
 * 	Outputs: 0 on success -1 on failure
 * 	Side Effects:
 */
int32_t execute_handler(const uint8_t *command)
{
    /* start of critical section */
    cli();

    int i;                       // Variable to iterate through command
    int j;                       // Variable to iterate through args
    uint8_t exec[keyBufferSize]; // First word in command, file name of program to be executed
    uint8_t args[keyBufferSize]; // should be provided to the new program on request via the getargs system call.
    dentry_t dentry;             // Executable file
    uint8_t entryBuf[byte4];     // EIP byte by byte
    uint32_t EIP;                // EIP
    EIP = 0x0;

    /* Start of sanity check */

    /* clear out argument buffer */
    for (i = 0; i < keyBufferSize; i++)
    {
        args[i] = '\0';
    }

    /* Check to make sure not too many programs are executing */
    if (program_count >= programMax)
    {
        return -1;
    }

    /* NULL check for command */
    if (command == NULL)
    {
        return -1;
    }

    /* Parse command */
    for (i = 0; i < keyBufferSize; i++)
    {
        exec[i] = '\0';
    }

    /* Parsing executable name */
    for (i = 0; command[i] != NULL; i++)
    {
        if (command[i] == ' ' || command[i] == '\n')
        {
            break;
        }
        exec[i] = command[i];
    }

    /* Getting rid of leading spaces */
    while (command[i] == ' ')
    {
        i++;
    }

    /* copy command arguments into temporary argbuf */
    for (j = 0; command[i] != '\0' && i < maxInputLength; j++)
    {
        args[j] = command[i];
        i++;
    }

    /* find out if there is an argument */
    for (i = 0; i < keyBufferSize; i++)
    {
        if (args[i] != '\0')
        {
            args_flag = 1;
            break;
        }
    }

    /* if the last character is overwritten, set flag to 0 */
    if (args[keyBufferSize - 1] != '\0')
    {
        args_flag = 0;
    }

    /* Verify executable is present */
    if (read_dentry_by_name(exec, &dentry) == -1)
    {
        return -1;
    }

    /* Make sure file is executable and load EIP, just using exec as a placeholder */
    read_data(dentry.inode_num, 0, exec, 4);

    /* Checking for executable magic numbers */
    if (exec[0] != exe0 || exec[1] != exe1 || exec[2] != exe2 || exec[3] != exe3)
    {
        return -1;
    }

    /* Load program, get executable EIP */
    read_data(dentry.inode_num, offset_24, entryBuf, byte4);

    for (i = 0; i < byte4; i++)
    {
        EIP |= entryBuf[i] << (i * maskCount);
    }

    int temp, slot;
    for(temp = 0; temp < programMax; temp++)
    {
        if(pid_arr[temp] == 0)
        {
            slot = temp;
            break;
        }
    }

    /* Setup paging */
    page_dir[pageDirIndex].hex = 0;
    page_dir[pageDirIndex].page_table_base_addr_entry = (bottomKernal + slot * pageSize) >> shiftCount;
    page_dir[pageDirIndex].present_entry = 1;
    page_dir[pageDirIndex].read_write_entry = 1;
    page_dir[pageDirIndex].ps_entry = 1;
    page_dir[pageDirIndex].user_entry = 1;

    /* Load page and flush tlb */
    load_pde((uint32_t)page_dir);
    flush_tlb();

    /* Copy executable contents to memory offset */
    int count = 0;
    int bytesRead;
    uint8_t *addr = (uint8_t *)(virtualAddr);
    while (pageSize - count > 0)
    {
        bytesRead = read_data(dentry.inode_num, count, addr + count, pageSize - count);
        if (bytesRead == 0)
        {
            break;
        }
        else if (bytesRead != -1)
        {
            count += bytesRead;
        }
        else
        {
            return -1;
        }
    }

    pcb_t* pcb_child, *pcb_parent;
    if(program_count < 3)
    {
        /* bookkeeping: increment
        * our number of processes, get current esp and ebp,
        * and set the current pcb pointer */

        uint32_t ebp;
        uint32_t esp;
        
        asm volatile(
            "movl %%esp, %0;"
            "movl %%ebp, %1;"
            : "=r"(esp), "=r"(ebp));

        /* set up PCB */
        pcb_child = (pcb_t *)(bottomKernal - (program_count + 1) * kernalStackSize);
        pcb_parent = pcb_child;

        program_count++;
        pcb_child->pid = program_count;
        pcb_child->parent_pid = program_count;
        pcb_child->kesp = bottomKernal - (kernalStackSize * (pcb_child->pid - 1)) - 4;
        pcb_child->parent_pcb = pcb_parent;
        pcb_child->esp = esp;
        pcb_child->ebp = ebp;
        pcb_child->argsflag = 0;

        pid_arr[pcb_child->pid - 1] = 1;
        sched_pid[program_count - 1] = pcb_child->pid;
    }
    else
    {
        /* bookkeeping: increment
        * our number of processes, get current esp and ebp,
        * and set the current pcb pointer */

        for(temp = 0; temp < programMax; temp++)
        {
            if(pid_arr[temp] == 0)
            {
                slot = temp;
                break;
            }
        }

        uint32_t ebp;
        uint32_t esp;
        
        asm volatile(
            "movl %%esp, %0;"
            "movl %%ebp, %1;"
            : "=r"(esp), "=r"(ebp));
        
        /* set up PCB */
        pcb_child = (pcb_t *)(bottomKernal - (slot + 1) * kernalStackSize);
        pcb_parent = pcb_current;

        pcb_child->pid = slot + 1;

        program_count++;

        pcb_child->parent_pid = pcb_current->pid;

        pcb_child->kesp = bottomKernal - (kernalStackSize * (pcb_child->pid - 1)) - 4;
        pcb_child->parent_pcb = pcb_parent;
        pcb_child->esp = esp;
        pcb_child->ebp = ebp;
        pcb_child->argsflag = args_flag;

        pid_arr[slot] = 1;
        sched_pid[curr_terminal] = pcb_child->pid;
    }

    /* Copy over the argument buffer */
    for (i = 0; i < keyBufferSize; i++)
    {
        pcb_child->argbuf[i] = args[i];
    }

    pcb_current = pcb_child;

    /* open stdin and stdout */
    open_handler((uint8_t *)"stdin");
    open_handler((uint8_t *)"stdout");

    /* Context switch to user program */
    pcb_current->ss0 = tss.ss0;
    pcb_current->esp0 = tss.esp0;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = MB_8 - ((slot) * KB_8) - 4;

    /* end of critical section */
    sti();

    /* push IRET context, IRET, and return */
    asm volatile(
        "pushl %0;"
        "pushl %1;"
        "pushfl;"
        "pushl %2;"
        "pushl %3;"
        "iret;"
        "sys_execute_return:;"
        :
        : "r"(USER_DS), "r"(userCount), "r"(USER_CS), "r"(EIP));

    return 0;
}

/* read_handler
 * 	Description: Reads command that was given as input
 * 	Inputs: fd, buf, nbytes
 * 	Outputs: return bytes read, -1 on failure
 * 	Side Effects:
 */
int32_t read_handler(int32_t fd, void *buf, int32_t nbytes)
{
    /* if any invalid fd, return -1 */
    if (buf == NULL || fd < fdMin || fd > fdMax)
    {
        return -1;
    }

    /* if fd is stdout, return -1 */
    if (fd == fdMin + 1)
    {
        return -1;
    }

    /* if valid fd, then check if being used */
    if (pcb_current->pcb_arr[fd].flags == 1)
    {
        /* if being used, return the read handler for that fd */
        if (fd == 0)
        {
            return pcb_current->pcb_arr[fd].operations_pointer.read_ptr((uint32_t)fd, 0, buf, (uint32_t)nbytes);
        }
        else
        {
            int ret;
            ret = pcb_current->pcb_arr[fd].operations_pointer.read_ptr(pcb_current->pcb_arr[fd].inode, pcb_current->pcb_arr[fd].file_position, buf, (uint32_t)nbytes);
            pcb_current->pcb_arr[fd].file_position += ret;
            return ret;
        }
    }

    /* else, return -1 */
    return -1;
}

/* write_handler
 * 	Description: write to the system
 * 	Inputs: fd, buf, nbytes
 * 	Outputs: return bytes read, -1 on failure
 * 	Side Effects:
 */
int32_t write_handler(int32_t fd, const void *buf, int32_t nbytes)
{
    /* if any invalid fd, return -1 */
    if (buf == NULL || fd <= fdMin || fd > fdMax)
    {
        return -1;
    }

    /* if valid fd, check if being used */
    if (pcb_current->pcb_arr[fd].flags == 1)
    {
        /* if being used, return the write handler for that fd */
        return pcb_current->pcb_arr[fd].operations_pointer.write_ptr((uint32_t)fd, buf, (uint32_t)nbytes);
    }

    /* else, return -1 */
    return -1;
}

/* open_handler
 * 	Description: open the proper file
 * 	Inputs: filename
 * 	Outputs: return proper value, -1 on failure
 * 	Side Effects:
 */
int32_t open_handler(const uint8_t *filename)
{
    /* start of critical section */
    cli();

    uint32_t fd = -1;
    dentry_t dentry;

    /* check if valid filename */
    if (filename == NULL || *filename == '\0')
    {
        return fd;
    }

    /* if filename is stdin, initialize stdin */
    if (strncmp((int8_t *)filename, "stdin", strlen((int8_t *)filename)) == 0)
    {
        pcb_current->pcb_arr[0].operations_pointer = terminal_operations_table;
        //pcb_current->pcb_arr[0].operations_pointer.write_ptr = NULL;
        pcb_current->pcb_arr[0].inode = 0;
        pcb_current->pcb_arr[0].file_position = 0;
        pcb_current->pcb_arr[0].flags = 1;
        return 0;
    }
    /* if filename is stdout, initialize stdout */
    if (strncmp((int8_t *)filename, "stdout", strlen((int8_t *)filename)) == 0)
    {
        pcb_current->pcb_arr[1].operations_pointer = terminal_operations_table;
        //pcb_current->pcb_arr[1].operations_pointer.read_ptr = NULL;
        pcb_current->pcb_arr[1].inode = 0;
        pcb_current->pcb_arr[1].file_position = 0;
        pcb_current->pcb_arr[1].flags = 1;
        return 1;
    }

    /* get the dentry by name */
    if (read_dentry_by_name(filename, &dentry) == -1)
    {
        return -1;
    }

    /* loop through free pcb blocks */
    int i;
    for (i = 2; i < PCB_SIZE; i++)
    {
        if (pcb_current->pcb_arr[i].flags == 0)
        {
            fd = i;
            break;
        }
    }

    /* if no pcb block was available, return -1 */
    if (fd == -1)
    {
        return -1;
    }
    else
    {
        /* mark the free block as being used and initialize accordingly */
        pcb_current->pcb_arr[fd].file_position = 0;
        pcb_current->pcb_arr[fd].flags = 1;
        pcb_current->pcb_arr[fd].inode = 0;

        //rtc
        if (dentry.filetype == 0)
        {
            pcb_current->pcb_arr[fd].operations_pointer = rtc_operations_table;
            pcb_current->rtc_flag = 1;
        }
        //directory
        else if (dentry.filetype == 1)
        {
            pcb_current->pcb_arr[fd].operations_pointer = directory_operations_table;
        }
        //file
        else if (dentry.filetype == 2)
        {
            pcb_current->pcb_arr[fd].operations_pointer = filesystem_operations_table;
            pcb_current->pcb_arr[fd].inode = dentry.inode_num;
        }
        //invalid, return -1;
        else
        {
            return -1;
        }

        /* in any case return the open function pointer */
        pcb_current->pcb_arr[fd].operations_pointer.open_ptr((uint8_t *)filename);
    }

    /* End of critical section */
    sti();

    /* return the valid fd */
    return fd;
}

/* close_handler
 * 	Description: close file
 * 	Inputs: fd
 * 	Outputs: 0 on success, -1 on failure
 * 	Side Effects:
 */
int32_t close_handler(int32_t fd)
{
    /* start of critical section */
    cli();

    /* check if pcb_current is initialized and returns -1 if not intialized */
    if (pcb_current == NULL)
    {
        return -1;
    }

    /* if any invalid fd, return -1 */
    if (fd < fdMin + 2 || fd > fdMax)
    {
        return -1;
    }

    /* check if fd isn't being used */
    if (pcb_current->pcb_arr[fd].flags == 0)
    {
        return -1;
    }
    else
    {
        /* else set flag to 0 and return 0 */
        pcb_current->pcb_arr[fd].flags = 0;
    }

    /* end critical section */
    sti();

    return 0;
}

/* getargs_handler
 * 	Description: reads the program’s command line arguments into a user-level buffer.
 * 	Inputs: buf (buffer for user-level), nbytes(bytes to be read)
 * 	Outputs: 0 on success -1 on failure
 * 	Side Effects: writes to buffer
 */
int32_t getargs_handler(uint8_t *buf, int32_t nbytes)
{
    /* begin critical section */
    cli();

    /* check if buf is NULL, nbytes is less than 0 and argflag is not set */
    if (buf == NULL || nbytes < 0 || !pcb_current->argsflag)
    {
        return -1;
    }

    /* check if argbuf is not null */
    if (pcb_current->argbuf[0] != NULL)
    {
        /* copy from argbuf to buf */
        strcpy((int8_t *)buf, (int8_t *)pcb_current->argbuf);
    }
    else
    {
        /* else return -1 */
        return -1;
    }

    /* end critical section */
    sti();

    return 0;
}

/* vidmap_handler
 * 	Description:  maps the text-mode video memory into user space at a pre-set virtual address
 * 	Inputs: screen_start(pointer to location in memory to be written to)
 * 	Outputs: 0 on success -1 on failure
 * 	Side Effects: writes to screen_start
 */
int32_t vidmap_handler(uint8_t **screen_start)
{
    /* begin critical section */
    cli();

    /* check if screen start is null and if it points to kernel page */
    if (screen_start == NULL || screen_start == (uint8_t **)KERNEL_MEM)
    {
        return -1;
    }

    /* set up page in page directory at location 1GB */
    page_dir[GB_idx].page_table_base_addr_pte = ((uint32_t)page_virtual_mem >> SHIFT1);
    page_dir[GB_idx].read_write_pte = 1;
    page_dir[GB_idx].present_pte = 1;
    page_dir[GB_idx].user_pte = 1;
    page_dir[GB_idx].ps_pte = 0;

    /* set up page for virtual memory */
    page_virtual_mem[0].page_table_base_addr_pte = VIDEO_MEM >> SHIFT1;
    page_virtual_mem[0].read_write_pte = 1;
    page_virtual_mem[0].user_pte = 1;
    page_virtual_mem[0].present_pte = 1;

    /* flush tlb */
    flush_tlb();

    /* set pointer to screen start to 1GB */
    *screen_start = (uint8_t *)GB_1;

    /* end critical section */
    sti();

    return 0;
}
