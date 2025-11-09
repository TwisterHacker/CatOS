#ifndef FLOPPY_DISK_CONTROLLER_DRIVER_H
#define FLOPPY_DISK_CONTROLLER_DRIVER_H

#define DOR 0x3F2 // Digital Output Reg.
#define MSR 0x3F4 // Main Status Reg.
#define DR  0x3F5 // Data Reg
#define DIR 0x3F7 // Digital Input Reg.

#define READ_TRACK                  2,	 // * generates IRQ6
#define SPECIFY                     3,   // * set drive parameters
#define SENSE_DRIVE_STATUS          4,
#define WRITE_DATA                  5,   // * write to the disk
#define READ_DATA                   6,   // * read from the disk
#define RECALIBRATE                 7,   // * seek to cylinder 0
#define SENSE_INTERRUPT             8,   // * ack IRQ6, get status of last command
#define WRITE_DELETED_DATA          9,
#define READ_ID                     10,	 // * generates IRQ6
#define READ_DELETED_DATA           12,
#define FORMAT_TRACK                13,  // *
#define DUMPREG                     14,
#define SEEK                        15,  // * seek both heads to cylinder X
#define VERSION                     16,	 // * used during initialization, once
#define SCAN_EQUAL                  17,
#define PERPENDICULAR_MODE          18,	 // * used during initialization, once, maybe
#define CONFIGURE                   19,  // * set controller parameters
#define LOCK                        20,  // * protect controller params from a reset
#define VERIFY                      22,
#define SCAN_LOW_OR_EQUAL           25,
#define SCAN_HIGH_OR_EQUAL          29

struct chs_t{
	short c;
	short h;
	short s;
} chs_t;

extern void detect_floppy_drives();

#endif