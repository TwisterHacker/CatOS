#ifndef TERMINAL_H
#define TERMINAL_H

void input();
char* scanf(char* text);
char get_char();
void scroll_screen();
void printf(const char* str, int color);
void print_char(char c, int color);
void cls();
void update_cursor();
void clear_buffer();
void BackSpaceDown();


#endif