#include <common.h>

void printf();

unsigned char getch(){
	unsigned char c = 0;
	while(c == 0)
		c = inw(0x60);
	printf("123\n", 0xaaa);
	return c;
}

char* input(){
	char* str = "";
	asm("cli");
	getch();
	getch();
	getch();
	getch();
	getch();
	asm("sti");

	return str;
}