#include "types.h"
#include "lib.h"
#include "filesystem.h"

/* filesystem_init
 * 	Description: Initializes all the starting pointers for the 
 *  boot block, inode block, and the data block.
 * 	Inputs: Address of the boot block
 * 	Outputs: None
 * 	Side Effects: Stores the starting address of the boot block,
 *  inode block, and the data block. These are used later for indexing.
 */
void filesystem_init(boot_block_t* boot_block)
{
    /* Initialize pointers */
    boot_block_addr = boot_block;
    inode_addr = (inode_t *) (boot_block + 1);
    data_addr = (data_t *) (boot_block + boot_block->inode_count + 1);
}

/* read_dentry_by_name
 * 	Description: Reads the dentry given a name.
 * 	Inputs: fname, dentry
 * 	Outputs: Return -1 on failure, 0 on success
 * 	Side Effects: Stores the dentry filetype and inode_num, given the 
 *  file name.
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    /* Check if name length is invalid or if null and also check if dentry is null */
    if(strlen((int8_t *)fname) > FILENAME_LEN || fname == NULL || dentry == NULL)
    {
        return -1;
    }

    /* Loop through all the dentries and check for the file name */
    int i;
    for(i = 0; i < DENTRY_SIZE; i++)
    {
        /* If file name found */
        if(!strncmp((int8_t *) &(boot_block_addr->direntries[i]), (int8_t *)fname, FILENAME_LEN))
        {
            /* Copy over the file name, the file type, and the inode num */
            memcpy((int8_t *)dentry->filename, (int8_t *)boot_block_addr->direntries[i].filename, FILENAME_LEN);
            dentry->filetype = boot_block_addr->direntries[i].filetype;
            dentry->inode_num = boot_block_addr->direntries[i].inode_num;

            /* Return 0 on success */
            return 0;
        }
    }

    /* Return -1 on failure */
    return -1;
}

/* read_dentry_by_index
 * 	Description: Reads the dentry given an index.
 * 	Inputs: index, dentry
 * 	Outputs: Return -1 on failure, 0 on success
 * 	Side Effects: Stores the dentry filetype and inode_num, given the 
 *  file index.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    /* If an index is out of range or dentry is null, return -1 */
    if(index > boot_block_addr->dir_count || dentry == NULL)
    {
        return -1;
    }
    
    /* Otherwise, copy over the file name, file type, and the inode num */
    memcpy((int8_t *)dentry->filename, (int8_t *)boot_block_addr->direntries[index].filename, FILENAME_LEN);
    dentry->inode_num = boot_block_addr->direntries[index].inode_num;
    dentry->filetype = boot_block_addr->direntries[index].filetype;

    /* Return 0 on success */
    return 0;
}

/* read_data
 * 	Description: Reads the data inside the inode
 *  Inputs: inode, offset, buffer, length.
 * 	Outputs: Return number of bytes read
 * 	Side Effects: Stores the data inside the buffer, given an inode.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    /* If inode is greater the inode count in the boot block or if the buffer is empty, return -1 */
    if(inode > boot_block_addr->inode_count || buf == NULL)
    {
        return -1;
    }

    /* Intialize variables */
    int i, data, idx;
    int read_counter = 0;
    int cur_pos = offset;
    data_t* cur_data_block_ptr;
    inode_t* cur_inode_ptr = (inode_t *) (inode + inode_addr);

    /* Loop through length */
    for(i = 0; i < length; i++)
    {
        /* If the length of the data is greater than the offset + read_counter, then we can copy over the data */
        if(cur_inode_ptr->length >= offset + read_counter)
        {
            idx = cur_pos / BLOCK_SIZE;
            data = cur_pos % BLOCK_SIZE;
            /* Get the data block pointer and copy the value to the buffer */
            cur_data_block_ptr = (data_t *)(data_addr + cur_inode_ptr->data_block_num[idx]);
            buf[i] = cur_data_block_ptr->val[data];

            /* Increment current position and also the number of bytes read */
            cur_pos++;
            read_counter++;
        }
    }

    /* Return number of bytes read */
    return read_counter;

}

/* file_open
 * 	Description: Opens the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return 0
 * 	Side Effects: None
 */
int32_t file_open(const uint8_t* filename)
{
    return 0;
}

/* file_close
 * 	Description: Closes the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return 0
 * 	Side Effects: None
 */
int32_t file_close(int32_t fd)
{
    return 0;
}

/* file_write
 * 	Description: Writes to the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return -1
 * 	Side Effects: None
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/* file_read
 * 	Description: Reads the file given an inode.
 *  Inputs: inode, offset, buffer, length.
 * 	Outputs: Return number of bytes read.
 * 	Side Effects: Stores the data read inside the buffer.
 */
int32_t file_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    return read_data(inode, offset, buf, length);
}

/* directory_open
 * 	Description: Opens the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return 0
 * 	Side Effects: None
 */
int32_t directory_open(const uint8_t* filename)
{
    return 0;
}

/* directory_close
 * 	Description: Closes the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return 0
 * 	Side Effects: None
 */
int32_t directory_close(int32_t fd)
{
    return 0;
}

/* directory_write
 * 	Description: Writes to the file but since we only have a read only system,
 *  it does nothing.
 * 	Inputs: None
 * 	Outputs: Return 0
 * 	Side Effects: None
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/* directory_read
 * 	Description: Reads the directory given an inode.
 *  Inputs: inode, offset, buffer, length.
 * 	Outputs: Return number of bytes read.
 * 	Side Effects: Stores the data read inside the buffer.
 */
int32_t directory_read(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    int i, data, idx;
    int cur_pos = offset;
    idx = cur_pos / FILENAME_LEN;

    /* Loop through the length and also make sure to stay within the directory count */
    for(i = 0; i < length && idx < boot_block_addr->dir_count; i++)
    {
        idx = cur_pos / FILENAME_LEN;
        data = cur_pos % FILENAME_LEN;
        /* Copy over the directory name */
        buf[i] = boot_block_addr->direntries[idx].filename[data];
        cur_pos++;
    }

    /* Return number of bytes read */
    return i;
}
