#include <common.h>
#include <drivers/timer.h>



void printf();
void print_char();
void OnKeyDown();

short in_scanf = 0;

u8int char_counter = 0;
char buffer[128];

volatile unsigned char ScanCode;
extern void clear_buffer();
extern void kb_handler();
extern void cls();



short shell_mode = 	1 ;


void read_command(char* command){
	if(command[0] == ' '){
		printf("ERROR: Space before command!", RED);
	}
	else{
		if(strncmp(command, "hello", 6) == 1){	
			printf("Hello, Admin!", 0xeee);
		}
		else if(strncmp(command, "cls", 3) == 1){	
			cls();
		}
		else{
			printf("\nUndefined Command!", RED);
		}
	}
}

char* scanf(char* text){
	clear_buffer();
	char* buff;
    if(in_scanf != 1){
    	printf(text, GREEN);
    	in_scanf = 1;
	    char_counter = 0;
	    while(1){
	        if(in_scanf == 0){
	            buff = buffer;
	            break;
	        }
	    }
	}
	return buff;
    
}