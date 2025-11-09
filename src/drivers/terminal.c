#include <common.h>
#include <drivers/timer.h>

short in_scanf = 0;

u8int char_counter = 0;
char buffer[128];

extern short in_scanf;
extern void read_command(char* command);
extern void update_cursor();

volatile unsigned char ScanCode;
extern void clear_buffer();
extern void kb_handler();

int x = 0;
int y = 0;

void scroll_screen() {
	static uint16_t* VideoMem = (uint16_t*)0xb8000;
    for (int y = 1; y < 24; y++) {
        for (int x = 0; x < 80; x++) {
            VideoMem[(y - 1) * 80 + x] = VideoMem[y * 80 + x];
        }
    }

    for (int x = 0; x < 80; x++) {
        VideoMem[(24 - 1) * 80 + x] = ' ' | (0x07 << 8);
    }
}


void printf(const char* str, int color){
	static uint16_t* VideoMem = (uint16_t*)0xb8000;
	for(int i = 0; str[i] != '\0'; ++i){
		switch(str[i]){
			case '\n':
			x = 0;
			y++;
			update_cursor();
			break;
		default:
			VideoMem[80 * y + x] = ((uint16_t)color << 8) | (uint8_t)str[i];
			x++;
			update_cursor();
			break;
		}

		if( x>=80){
			y++;
			x = 0;
			update_cursor();
		}

		if(y >= 24)
        {
        	y--;
        	scroll_screen();
			update_cursor();
        }
	}
}

void print_char(char c, int color){
	static uint16_t* VideoMem = (uint16_t*)0xb8000;

	if(c=='\n'){
		x = 0;
		y++;
		update_cursor();
	}else{
		VideoMem[80 * y + x] = ((uint16_t)color << 8) | (uint8_t)c;
		x++;
		update_cursor();
	}
	if( x>=80){
			y++;
			x = 0;
			update_cursor();
		}

	if(y >= 24){
		y--;
        scroll_screen();
		update_cursor();
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

char* scanf(char* text){
	memset(buffer, 0, sizeof(buffer));
	char* buff;
    if(in_scanf != 1){
    	printf(text, WHITE);
    	in_scanf = 1;
	    char_counter = 0;
	    while(1){
	        if(in_scanf == 0){
	        	buffer[char_counter] = '\0';
	            buff = buffer;
	            break;
	        }
	    }
	}
	return buff;
    
}

void clear_buffer(){
    for(short a=0; a<128; a++)
        buffer[a] = 0;
    char_counter = 0; 
}

void BackSpaceDown(){
    char *VideoMem = (char *) 0xb8000;
    if(in_scanf != 0 && char_counter != 0){
        buffer[char_counter-1] = 0;
        char_counter--;
        if(x==0){
            y--;
            x = 79;
        }
        else{
            x--;
        }
        VideoMem[(80*y+x)*2] = ' ';
        VideoMem[(80*y+x)*2+1] = 0x07;
        update_cursor();
    }
}