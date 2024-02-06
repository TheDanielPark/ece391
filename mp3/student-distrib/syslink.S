.data
    arg1 =              16
    arg2 =              20
    arg3 =              24

    halt_num =          1
    execute_num =       2
    read_num =          3
    write_num =         4
    open_num =          5
    close_num =         6
    getargs_num =       7
    vidmap_num =        8
    
    sys_num =           0x80


.text

.globl halt, execute, read, write, open, close, getargs, vidmap


# halt system call
halt:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl $halt_num, %eax
    int $sys_num
    
    popl %ebx
    popl %ecx
    popl %edx

# execute system call
execute:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl $execute_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# read system call
read:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl arg2(%esp), %ecx
    movl arg3(%esp), %edx
    movl $read_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# write system call
write:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl arg2(%esp), %ecx
    movl arg3(%esp), %edx
    movl $write_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# open system call
open:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl $open_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# close system call
close:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl $close_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# getarg system call
getargs:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl arg2(%esp), %ecx
    movl $getargs_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx

# vidmap system call
vidmap:
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl arg1(%esp), %ebx
    movl $vidmap_num, %eax
    int $sys_num

    popl %ebx
    popl %ecx
    popl %edx
