// common.h -- Defines typedefs and some global functions

#ifndef COMMON_H_
#define COMMON_H_

// Некоторые определения, чтобы стандартизировать типы
// Эти типы определены для платформы x86
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

#define BLACK 0x000
#define BLUE 0x111
#define GREEN 0x222
#define CYAN 0x333
#define RED 0x444
#define MAGENTA 0x555
#define BROWN 0x666
#define LIGHTGREY 0x777
#define GREY 0x888
#define LIGHTBLUE 0x999
#define LIGHTGREEN 0xAAA
#define LIGHTCYAN 0xBBB
#define LIGHTRED 0xCCC
#define LIGHTMAGENTA 0xDDD
#define YELLOW 0xEEE
#define WHITE 0xFFF



extern void outb(u16int port, u8int value);

extern u8int inb(u16int port);

extern u16int inw(u16int port);

extern void memcpy(void *dest, const void *src, u32int len);

extern void memset(void *dest, u8int val, u32int len);

extern int strcmp(const char *str1, const char *str2);

extern short strncmp(char *str1, char *str2, uint8_t n);

extern char *strcpy(char *dest, const char *src);

extern char *strcat(char *dest, const char *src);


#endif

