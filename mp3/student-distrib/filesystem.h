/*
 * filesystem.h
 * This file declares the structures used in filesystem functions.
 * The main purpose of the filesystem is operate the inode system.
 * To do that we declare several structures that represent the inode
 * system and index them to get the data we are looking for
 * i.e. file data, directory name, file name, file type, file size, etc.
 */
#include "lib.h"

/* Magic numbers */
#define FILENAME_LEN 32
#define BLOCK_SIZE 4096
#define DENTRY_SIZE 64
#define DATA_BLOCK_NUM_SIZE 1023
#define RESERVED_DENTRY 24
#define RESERVED_BOOT_BLOCK 52

/* Structure definitions taken from lecture */

/* Structure for dentries */
typedef struct dentry
{
    int8_t filename[FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[RESERVED_DENTRY];
} dentry_t;

/* Structure for boot block */
typedef struct boot_block
{
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[RESERVED_BOOT_BLOCK];
    dentry_t direntries[DENTRY_SIZE - 1];
} boot_block_t;

/* Structure for inode */
typedef struct inode
{
    int32_t length;
    int32_t data_block_num[DATA_BLOCK_NUM_SIZE];
} inode_t;

/* 
 * Data structure (not referenced in lecture but needed
 * to index data correctly. There is 4096 values in the data block
 * and the data block comes after the inodes block).
 */
typedef struct data
{
    int8_t val[BLOCK_SIZE];
} data_t;

/* 
 * Define variables to store the beginning address for the
 * boot block, inode block, and data block.
 */
boot_block_t* boot_block_addr;
inode_t* inode_addr;
data_t* data_addr;

/* Init file system */
void filesystem_init(boot_block_t* boot_block);

/* Read the dentry by name */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* Read the dentry by index */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* Read the data inside the inode */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Does nothing but still returns a value */
int32_t file_open(const uint8_t* filename);

/* Does nothing but still returns a value */
int32_t file_close(int32_t fd);

/* Does nothing but still returns a value */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/* Read the data inside the inode and store it in a buffer */
int32_t file_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Does nothing but still returns a value */
int32_t directory_open(const uint8_t* filename);

/* Does nothing but still returns a value */
int32_t directory_close(int32_t fd);

/* Does nothing but still returns a value */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* Read the directory data and store it in a buffer */
int32_t directory_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
