#ifndef COMMON_H_
#define COMMON_H_

#ifdef __i386__
typedef unsigned int	u32int;
typedef          int	s32int;
typedef unsigned short	u16int;
typedef          short	s16int;
typedef unsigned char	u8int;
typedef          char	s8int;

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int int64_t;
typedef unsigned long long int uint64_t;

typedef uint32_t size_t;
    
#else
#error "Types for non-x86 not implemented."
#endif

#define BLACK 0x00
#define BLUE 0x01
#define GREEN 0x02
#define CYAN 0x03
#define RED 0x04
#define MAGENTA 0x05
#define BROWN 0x06
#define LIGHTGREY 0x07
#define GREY 0x08
#define LIGHTBLUE 0x09
#define LIGHTGREEN 0x0A
#define LIGHTCYAN 0x0B
#define LIGHTRED 0x0C
#define LIGHTMAGENTA 0x0D
#define YELLOW 0x0E
#define WHITE 0x0F

#define NULL 0

#define OS_VER "0.0.2-FAT"



extern void outb(u16int port, u8int value);

extern u8int inb(u16int port);

extern u16int inw(u16int port);

extern void memcpy(void *dest, const void *src, u32int len);

extern void memset(void *dest, u8int val, u32int len);

extern int strcmp(const char* s1, const char* s2);

extern short strncmp(const char *str1, const char *str2, uint8_t n);

extern char *strcpy(char *dest, const char *src);

extern char *strcat(char *dest, const char *src);

extern void itoa(unsigned int value, char* str, int base);

extern void tolower(char* str);

extern int strlen(char* str);

#endif

