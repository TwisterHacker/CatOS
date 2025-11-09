#include <common.h>
#include <interrupts/descriptor_tables.h>
#include <drivers/keyboard.h>
#include <drivers/timer.h>
#include <drivers/terminal.h>
#include <memory/memory_managment.h>
#include <drivers/shell.h>
#include <drivers/fs.h>
#include <fs/fat12.h>

/////// Virtual File System ///////

extern void init_devices();
extern int unmount_device();
extern void list_directory();

///////////////////////////////////

////// FAT_12 //////

extern void detect_floppy_drives();
extern void floppy_reset();
extern void floppy_specify();
extern void floppy_calibrate();
extern void fdc_init();
extern int floppy_read_track();
extern int floppy_write_track();

extern void fat12_parse_boot_sector();
extern void fat12_load_root_dir();
extern void fat12_list_root_dir();
extern void fat12_load_fat();
extern int floppy_open_file();
extern int floppy_close_file();
extern int floppy_write_file();
extern int floppy_list();
extern int floppy_load_dir();
extern int floppy_create_entry();
extern int floppy_remove_entry();

extern uint8_t* file_buf;
extern uint32_t file_size;

extern short flp_a;
extern short flp_b;

///////////////////



uint8_t buffer[512];

char path[32][13] = { NULL };
int path_counter = 0;



void kMain(){
	cls();

	printf("\nKERNEL INIT START:\n", YELLOW);

	printf("\n\nSTAGE 1", YELLOW);
	printf("\n\nInititializing Descriptor tables : ", LIGHTGREY);
	init_descriptor_tables();
	__asm__ volatile ("sti");
	printf("DONE", GREEN);

	printf("\nInititializing keyboard : ", LIGHTGREY);
	kb_init();
	printf("DONE", GREEN);
	printf("\nInititializing timer : ", LIGHTGREY);
	init_timer(1000);
	printf("DONE", GREEN);
	printf("\nInititializing Floppy Disk Controller : ", LIGHTGREY);
	fdc_init();
	printf("DONE", GREEN);
	printf("\nFloppy Disk Controller Reset : ", LIGHTGREY);
	floppy_reset();
	printf("DONE", GREEN);
	printf("\nFloppy Disk Controller Calibrate : ", LIGHTGREY);
	floppy_calibrate();
	printf("DONE", GREEN);
	printf("\nInititializing Terminal : ", LIGHTGREY);
	//clear_buffer();
	printf("DONE", GREEN);

	cls();

	printf("\n\nSTAGE 2", YELLOW);

	detect_floppy_drives();
	
	cls();

	printf("\n\nSTAGE 3", YELLOW);

	printf("\n\nInititializing File System : ", LIGHTGREY);

	init_devices();

	if(flp_a){
		mount_device("flp0", floppy_open_file, floppy_close_file, floppy_write_file, floppy_list, floppy_load_dir, floppy_create_entry, floppy_remove_entry, file_buf, file_size);
		fat12_parse_boot_sector(0);
		fat12_load_root_dir(0);
		fat12_load_fat(0);
	}

	if(flp_b){
		mount_device("flp1", floppy_open_file, floppy_close_file, floppy_write_file, floppy_list, floppy_load_dir, floppy_create_entry, floppy_remove_entry, file_buf, file_size);
		fat12_parse_boot_sector(1);
		fat12_load_root_dir(1);	
		fat12_load_fat(1);
	}

	cls();

	printf("\n  /\\       /\\\n /  \"\"\"\"\"/  \\\n|  \\/\\\"\"\"/\\/  |\n`, \"/ ,`\n====== Y ======\n  \\   -^-   /\n   \\       / \\__,\n  /  `````       \\______,\n |    ```         \" \" \"  \\,\n |     `           \" \"     \\\n |            |     \"    \"  \\\n |    _      /            \"  |\n | \" / \\ \"  /              \" |\n |   | |   |\\__ _______\\    \"|\n/ -  | | -  \\ /   \"   \"   \" /\n\\___/   \\___/ \\____________/   ", GREEN);

	printf("\n\nType \"help\" to see list of commands", WHITE);
	
	printf("\n", 0);

	while(1){
		read_command();
	}
	
}
