// common.c -- Defines some global functions

#include <common.h>
void printf();

// write a byte out to the specified port
void outb(u16int port, u8int value)
{
	__asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port)
{
	u8int ret;
	__asm__ volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

u16int inw(u16int port)
{
	u16int ret;
	__asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

// Copy len bytes from src to dest.
void memcpy(void *dest, const void *src, u32int len)
{
    const u8int *sp = (const u8int *)src;
    u8int *dp = (u8int *)dest;
    while(len--) 
		*dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(void *dest, u8int val, u32int len)
{
    u8int *temp = (u8int *)dest;
    while (len--) 
		*temp++ = val;
}

// Returns an integral value indicating the relationship between the strings:
// A zero value indicates that both strings are equal.
// A value greater than zero indicates that the first character that does not 
// match has a greater value in str1 than in str2; And a value less than zero indicates the opposite.
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src)
{
	char* tmp = dest;

	while((*dest++ = *src++) != '\0');

	return tmp;
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src)
{
	char* tmp = dest;

	while(*dest)
		dest++;
	while((*dest++ = *src++) != '\0');

	return tmp;
}

short strncmp(const char *str1, const char *str2, uint8_t n){
	for(uint8_t i = 0; i<n; i++){
		if(str1[i] != str2[i]){
			return -1;
		}
	}
	return 0;
}

void itoa(unsigned int value, char* str, int base) {
    char* ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    while (value != 0) {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? tmp_value + '0' : tmp_value + 'a' - 10;
        value /= base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void tolower(char* str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str += 32;
        }
        str++;
    }
}

int strlen(char* str){
    int len = 0;

    for (int i = 0; str[i] != '\0'; i++){
        len++;
    }

    return len;
}