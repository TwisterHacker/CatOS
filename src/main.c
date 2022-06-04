#include <common.h>
#include <interrupts/descriptor_tables.h>
#include <drivers/keyboard.h>
#include <drivers/shell.h>

void cls();
void update_cursor();

int x = 0;
int y = 0;

void printf(char* str, int color){
	static uint16_t* VideoMem = (uint16_t*)0xb8000;
	for(int i = 0; str[i] != '\0'; ++i){
		switch(str[i]){
			case '\n':
			x = 0;
			y++;
			update_cursor();
			break;
		default:
			VideoMem[80*y+x] = (VideoMem[80*y+x] & color) | str[i];
			x++;
			update_cursor();
			break;
		}

		if( x>=80){
			y++;
			x = 0;
			update_cursor();
		}

		if(y >= 25)
        {
        	cls();
        	x=0;
        	y=0;
			update_cursor();
        }
	}
}

void cls(){
	char *vidmem = (char *) 0xb8000;
	unsigned int i=0;
	while(i < (80*25*2)){
		vidmem[i]=' ';
		i++;
		vidmem[i]=0x07;
		i++;
	};
	x = 0;
	y = 0;
};

void update_cursor(){
	uint16_t pos = y * 80 + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}


void kMain(){
	cls();
	printf("Inititializing Descriptor tables : ", 0xddd);
	init_descriptor_tables();
	printf("Done!\n", 0xaaa);

	printf("Allowing Interrupts: ", 0xddd);
	// Allow IRQs
	__asm__ volatile ("sti");
	printf("Done!\n", 0xaaa);

	printf("Inititializing Descriptor tables : ", 0xddd);
	kb_init();
	printf("Done!\n", 0xaaa);

	cls();

	//printf("                             \n _____        _____   _____  \n(_____)      (_____) (_____) \n(_)__(__   _(_)   (_(_)___   \n(_____(_) (_(_)   (_) (___)_ \n(_)   (_)_(_(_)___(_) ____(_)\n(_)    (____)(_____) (_____) \n        __(_)                \n       (___)                 \n", 0x999);
	//printf(" ,_     _\n |\\_,-~/\n / _  _ |    ,--.\n(  @  @ )   / ,-'\n \\  _T_/-._( (\n /         `. \\n|         _  \\ |\n \\ \\ ,  /      |\n  || |-_\\__   /\n ((_/`(____,-'\n", 0xaaa);
	
	printf("  /\\       /\\\n /  \"\"\"\"\"/  \\\n|  \\/\\\"\"\"/\\/  |\n`, \"/ ,`\n====== Y ======\n  \\   -^-   /\n   \\       / \\__,\n  /  `````       \\______,\n |    ```         \" \" \"  \\,\n |     `           \" \"     \\\n |            |     \"    \"  \\\n |    _      /            \"  |\n | \" / \\ \"  /              \" |\n |   | |   |\\__ _______\\    \"|\n/ -  | | -  \\ /   \"   \"   \" /\n\\___/   \\___/ \\____________/   ", 0xaaa);


	input();

	while(1){
	}
}