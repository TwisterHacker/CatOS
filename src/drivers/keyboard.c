#include <drivers/keyboard.h>
#include <common.h>
#include <interrupts/isr.h>

void printf(); 
void print_char(); 
void update_cursor();

volatile unsigned char ScanCode;

extern u8int char_counter;
extern char buffer[128];
extern short in_scanf;
extern void read_command(char* command);
extern void update_cursor();
extern int x;
extern int y;


void clear_buffer(){
    for(short a=0; a<128; a++)
        buffer[a] = 0;
    char_counter = 0;
}

void OnKeyDown(char key){
    print_char(key, 0xfff);
    if(in_scanf != 0){
        buffer[char_counter] = key;
        char_counter++;
    }
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

//char* scanf(){}

void kb_handler(){

    outb(0x20, 0x20);   // Send EOI
    ScanCode = inw(0x60);    
    if((ScanCode & 128) == 128)
		return;
    else{
    	switch(ScanCode){
    		case 0x02: OnKeyDown('1'); break;
            case 0x03: OnKeyDown('2'); break;
            case 0x04: OnKeyDown('3'); break;
            case 0x05: OnKeyDown('4'); break;
            case 0x06: OnKeyDown('5'); break;
            case 0x07: OnKeyDown('6'); break;
            case 0x08: OnKeyDown('7'); break;
            case 0x09: OnKeyDown('8'); break;
            case 0x0A: OnKeyDown('9'); break;
            case 0x0B: OnKeyDown('0'); break;

            case 0x10: OnKeyDown('q'); break;
            case 0x11: OnKeyDown('w'); break;
            case 0x12: OnKeyDown('e'); break;
            case 0x13: OnKeyDown('r'); break;
            case 0x14: OnKeyDown('t'); break;
            case 0x15: OnKeyDown('z'); break;
            case 0x16: OnKeyDown('u'); break;
            case 0x17: OnKeyDown('i'); break;
            case 0x18: OnKeyDown('o'); break;
            case 0x19: OnKeyDown('p'); break;

            case 0x1E: OnKeyDown('a'); break;
            case 0x1F: OnKeyDown('s'); break;
            case 0x20: OnKeyDown('d'); break;
            case 0x21: OnKeyDown('f'); break;
            case 0x22: OnKeyDown('g'); break;
            case 0x23: OnKeyDown('h'); break;
            case 0x24: OnKeyDown('j'); break;
            case 0x25: OnKeyDown('k'); break;
            case 0x26: OnKeyDown('l'); break;

            case 0x2C: OnKeyDown('y'); break;
            case 0x2D: OnKeyDown('x'); break;
            case 0x2E: OnKeyDown('c'); break;
            case 0x2F: OnKeyDown('v'); break;
            case 0x30: OnKeyDown('b'); break;
            case 0x31: OnKeyDown('n'); break;
            case 0x32: OnKeyDown('m'); break;
            case 0x33: OnKeyDown(','); break;
            case 0x34: OnKeyDown('.'); break;
            case 0x35: OnKeyDown('-'); break;

            case 0x39: OnKeyDown(' '); break;

            case 0x0E: BackSpaceDown(); break;

            case 0x1C: 
                OnKeyDown('\n');
                in_scanf = 0;
                break;



    		//default:
    			//printf("undefined key!", 0xaaa);
    	}
    }
}

void kb_init(){
	register_interrupt_handler(IRQ1, kb_handler);
}

