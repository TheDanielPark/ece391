#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "filesystem.h"
#include "rtc.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* Keyboard Test - Example
 * 	Description: Tests keyboard functionality
 * 	Inputs: None
 * 	Outputs: PASS/FAIL
 * 	Side Effects: None
 */
int keyboard_test() {
	TEST_HEADER;
	
	/*
	printf("\n");
	if(key_press(30) != 'a') {
		return FAIL;
	}
	if(key_press(2) != '1') {
		return FAIL;
	}
	if(key_press(53) != '/') {
		return FAIL;
	}
	*/
	return PASS;
}

/* divide_error
 * 	Description: Tests divide by zero functionality
 * 	Inputs: None
 * 	Outputs: PASS/FAIL
 * 	Side Effects: None
 */
void divide_error_test()
{
	TEST_HEADER;

	int i = 5;
	int j;
	for(j =  0; j < 1; j++)
	{
		i /= j;
	}
}

/* page_fault_test
 * 	Description: Tests page fault
 * 	Inputs: None
 * 	Outputs: PASS/FAIL
 * 	Side Effects: None
 */
void page_fault_test()
{
	TEST_HEADER;
	
	int a;
	int * b;
	b =  (int *) (0x0);
	a = *b;
}

/* page_init_check
 * 	Description: Tests page initialization. Sanity check :)
 * 	Inputs: None
 * 	Outputs: PASS/FAIL
 * 	Side Effects: None
 */
int page_init_check()
{
	TEST_HEADER;

	int video = 0xB8000;
	int kernel = 0x400000;
	
	int a;
	int * b;
	b =  (int *) (video);
	a = *b;

	b = (int *) (kernel);
	a = *b;

	return PASS;
}

/* Checkpoint 2 tests */

/* read_fish_frame0
 * 	Description: Reads the frame0.txt file.
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints the content of the file.
 */
void read_fish_frame0()
{
	int i;
	int size = 200;
	uint8_t buf[size];
	dentry_t dentry;

	for(i = 0; i < size; i++)
	{
		buf[i] = '\0';
	}

	clear();

	read_dentry_by_name((uint8_t *)"frame0.txt", &dentry);
	read_data(dentry.inode_num, 0, buf, size);

	printf("\n\n ");
	for(i = 0; i < size; i++)
	{
		printf("%c", buf[i]);
	}

	printf("\n file name: ");
	terminal_write(NULL, &dentry.filename, FILENAME_LEN);
}

/* read_fish_frame1
 * 	Description: Reads the frame1.txt file.
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints the content of the file.
 */
void read_fish_frame1()
{
	int i;
	int size = 200;
	uint8_t buf[size];
	dentry_t dentry;

	for(i = 0; i < size; i++)
	{
		buf[i] = '\0';
	}

	clear();

	read_dentry_by_name((uint8_t *)"frame1.txt", &dentry);
	read_data(dentry.inode_num, 0, buf, size);

	printf("\n\n ");
	for(i = 0; i < size; i++)
	{
		printf("%c", buf[i]);
	}

	printf("\n file name: ");
	terminal_write(NULL, &dentry.filename, FILENAME_LEN);
}

/* read_large_file
 * 	Description: Reads a large file
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints the content of the file.
 */
void read_large_file()
{
	int i;
	int size = 6000;
	uint8_t buf[size];
	dentry_t dentry;

	for(i = 0; i < size; i++)
	{
		buf[i] = '\0';
	}

	clear();

	read_dentry_by_name((uint8_t *)"verylargetextwithverylongname.tx", &dentry);
	read_data(dentry.inode_num, 0, buf, size);

	printf("\n\n ");
	for(i = 0; i < size; i++)
	{
		printf("%c", buf[i]);
	}

	printf("\n file name: ");
	terminal_write(NULL, &dentry.filename, FILENAME_LEN);
}

/* ls
 * 	Description: Lists the file in the current directory.
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints the file names, file type, and file size
 *	of the files in the directory.
 */
void ls()
{
	int i;

	clear();
	printf("\n\n ");

	for(i = 0; i < (int)boot_block_addr->dir_count; i++)
	{
		dentry_t dentry;
		read_dentry_by_index(i, &dentry);
		inode_t* ptr = dentry.inode_num + inode_addr;
		printf("filename:    ");
		terminal_write(NULL, &dentry.filename, FILENAME_LEN);
		printf(",  file_type: %d, file_size: %d", dentry.filetype, ptr->length);
		printf("\n");
	}
}

/* read_grep
 * 	Description: Reads the grep file
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints the content of the file.
 */
void read_grep()
{
	int i;
	int size = 6000;
	uint8_t buf[size];

	reset();
	
	for(i = 0; i < size; i++)
	{
		buf[i] = '\0';
	}

	printf("\n\n ");

	dentry_t dentry;
	read_dentry_by_name((uint8_t *)"ls", &dentry);
	read_data(dentry.inode_num, 0, buf, size);

	terminal_write(NULL, &buf, size);
}

/* rtc_test_freq
 * 	Description: Shows the changing rtc frequency.
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Prints 1 to the terminal and prints at the
 *	rate at which rtc is set currently.
 */
void rtc_test_freq()
{
	int i;

	set_frequency(2);
	for(i = 0; i < 1000000000; i++) {
		
	}
	clear();
	set_frequency(8);
	for(i = 0; i < 1000000000; i++) {
		
	}
	clear();
	set_frequency(16);
	for(i = 0; i < 1000000000; i++) {
		
	}
	clear();
}

/* key_test
 * 	Description: Clears the terminal.
 * 	Inputs: None
 * 	Outputs: None
 * 	Side Effects: Clears the terminal.
 */
void key_test() 
{
	reset();
}

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */

int syscall_test()
{
	int ret;
	reset();
	ret = execute((const uint8_t*)"shell");
	return ret;
}
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt tests", idt_test());
	// launch your tests here
	//TEST_OUTPUT("keyboard test", keyboard_test());
	//page_fault_test();
	//divide_error_test();
	//TEST_OUTPUT("page fault", page_init_check());
	//read_fish_frame0();
	//read_fish_frame1();
	//read_large_file();
	//ls();
	//read_grep();
	//rtc_test_freq();
	//key_test();
	//read_file_data();
	//read_file_data2();
	//read_dir();
	//rtc_test_freq();
	//reset();
	//syscall_test();
}
