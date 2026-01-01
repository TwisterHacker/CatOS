#include <interrupts/isr.h>
#include <common.h>
#include <drivers/terminal.h>
#include <drivers/timer.h>
#include <drivers/floppy_disk_controller_driver.h>

int floppy_motor_state[2] = {0, 0};
short RecievedIRQ = 0;
short flp_a = 0;
short flp_b = 0;

int floppy_idle_ticks[2] = {0, 0};

uint8_t buffer[512];

int floppy_read_sector(int drive, unsigned cyl, int head, int sector, uint8_t* buffer);

static char * drive_types[8] = {
    "none",
    "360kB 5.25\"",
    "1.2MB 5.25\"",
    "720kB 3.5\"",

    "1.44MB 3.5\"",
    "2.88MB 3.5\"",
    "unknown type",
    "unknown type"
};

// Used by floppy_dma_init and floppy_do_track to specify direction
typedef enum {
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;


// we statically reserve a totally uncomprehensive amount of memory
// must be large enough for whatever DMA transfer we might desire
// and must not cross 64k borders so easiest thing is to align it
// to 2^N boundary at least as big as the block
#define floppy_dmalen 0x4800
static const char floppy_dmabuf[floppy_dmalen]
                  __attribute__((aligned(0x8000)));

void floppy_recieved_irq() {
    int timeout = 1000;
    while (!RecievedIRQ && timeout--) {
        msleep(1);
    }
    if (!RecievedIRQ) {
        printf("Timeout waiting IRQ\n", RED);
    } else {
        RecievedIRQ = 0;
    }
}

void detect_floppy_drives(){
    floppy_idle_ticks[0] = 0;
    floppy_idle_ticks[1] = 0;

    printf("\n\nDetecting Floppy Drives : ", YELLOW);

    printf("\nReading Floppy Drive 1 : ", YELLOW);

     // диск 0, трек 0, головка 0, сектор 1

    if(!floppy_read_sector(0, 0, 0, 1, buffer)){
        printf("DONE", GREEN);
        outb(0x70, 0x10);
        unsigned drives = inb(0x71);
        printf("\nFloppy Drive 1 --- ", WHITE);
        printf(drive_types[drives >> 4], YELLOW);
        flp_a = 1;
    }else{
        flp_a = 0;
    }

    printf("\nReading Floppy Drive 2 : ", YELLOW);

    if(!floppy_read_sector(1, 0, 0, 1, buffer)){
        printf("DONE", GREEN);
        outb(0x70, 0x10);
        unsigned drives = inb(0x71);
        printf("\nFloppy Drive 2 --- ", WHITE);
        printf(drive_types[drives & 0xf], YELLOW);
        printf("\n", WHITE);
        flp_b = 1;
    }else{
        flp_b = 0;
    }
}

void floppy_motor_on(int drive) {
    if(!floppy_motor_state[drive]){
        floppy_motor_state[drive] = 1;
        outb(0x3F2, 0x1C | drive);
        msleep(500);
    }else{
        floppy_idle_ticks[drive] = 0;
    }
}

void floppy_motor_off(int drive) {
    floppy_motor_state[drive] = 0;
    outb(0x3F2, 0x0C | drive); 
}

void floppy_tick(int drive) {
    if (floppy_motor_state[drive]) {
        floppy_idle_ticks[drive]++;
        if (floppy_idle_ticks[drive] >= 10000) {
            floppy_motor_off(drive);
            floppy_motor_state[drive] = 0;
            floppy_idle_ticks[drive] = 0;
        }
    }
}

void floppy_send_command(uint8_t cmd){
	while (!(inb(MSR) & 0x80));
	outb(DR, cmd);
}

uint8_t floppy_read_data(){
    for(int i = 0; i < 100; i++) {
        if(0x80 & (inb(MSR) & 0x80)) {
            return inb(DR);
        }
        msleep(10); // sleep 10 ms
    }
    printf("\nfloppy_read_data: timeout!\n", RED);
    return 0; // not reached
}

void floppy_handler(){
	RecievedIRQ = 1;
	outb(0x20, 0x20);
}

int floppy_calibrate(int drive){
	floppy_motor_on(drive);
    for (int i = 0; i < 10; i++) {
        RecievedIRQ = 0;

        floppy_send_command(0x07);         // RECALIBRATE
        floppy_send_command(drive);

        floppy_recieved_irq();

        floppy_send_command(0x08);         // SENSE INTERRUPT STATUS

        uint8_t st0 = floppy_read_data();
        uint8_t cyl = floppy_read_data();

        (void)st0;

        if (cyl == 0) {
            return 0;
        }
    }

    return 1;
}

void floppy_check_interrupt(int *st0, int *cyl) {
    
    floppy_send_command(8);

    *st0 = floppy_read_data();
    *cyl = floppy_read_data();
}

void floppy_reset() {
    RecievedIRQ = 0;
    outb(DOR, 0x00);
    msleep(500);
    outb(DOR, 0x0C);
    floppy_recieved_irq();

    floppy_send_command(0x08);
    floppy_read_data(); // st0
    floppy_read_data(); // cyl
}

void floppy_check_int(uint32_t* st0, uint32_t* cyl) {
 
    floppy_send_command(8); // FDC_CMD_CHECK_INT
 
    *st0 = floppy_read_data();
    *cyl = floppy_read_data();
}

int floppy_seek(int drive, uint32_t cyl, uint32_t head) {

    uint32_t st0, cyl0;

    floppy_motor_on(drive);

    for (int i = 0; i < 10; i++ ) {
 
        //! send the command
        floppy_send_command(15); // FDC_CMD_SEEK        
        floppy_send_command( (head) << 2 | drive);
        floppy_send_command(cyl);
 
        //! wait for the results phase IRQ
        floppy_recieved_irq();
        floppy_check_int(&st0,&cyl0);
 
        //! found the cylinder?
        if ( cyl0 == cyl)
            return 0;
    }
 

    printf("\nfloppy_seek: 10 retries exhausted\n", RED);
    floppy_motor_off(drive);
    return -1;
}

static void floppy_dma_init(floppy_dir dir) {

    union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

    a.l = (unsigned) &floppy_dmabuf;
    c.l = (unsigned) floppy_dmalen - 1; // -1 because of DMA counting

    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    // (DMA can't deal with such a carry, this is the 64k boundary limit)
    if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)) {
        printf("\nfloppy_dma_init: static buffer problem\n", RED);
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case floppy_dir_read:  mode = 0x46; break;
        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case floppy_dir_write: mode = 0x4a; break;
        default: printf("floppy_dma_init: invalid direction", RED);
                 return; // not reached, please "mode user uninitialized"
    }

    outb(0x0a, 0x06);   // mask chan 2

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte

    outb(0x81, a.b[2]); // external page register

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte

    outb(0x0b, mode);   // set mode (see above)

    outb(0x0a, 0x02);   // unmask chan 2
}

void floppy_specify() {
    floppy_send_command(0x03); // SPECIFY
    floppy_send_command(0xDF); // SRT=13ms, HUT=240us
    floppy_send_command(0x02); // HLT=16ms, ND=0
}

void fdc_init(){
	register_interrupt_handler(IRQ6, floppy_handler);
    floppy_reset();
    floppy_specify();
}

int floppy_do_sector(int drive, int cyl, int head, int sector, floppy_dir dir, uint8_t* buffer) {

    floppy_idle_ticks[drive] = 0;

    RecievedIRQ = 0;
    
    // transfer command, set below
    unsigned char cmd;

    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
    // 
    // Specify multitrack and MFM mode
    static const int flags = 0xC0;
    switch(dir) {
        case floppy_dir_read:
            cmd = 6 | flags;
            break;
        case floppy_dir_write:
            cmd = 5 | flags;
            break;
        default: 

            printf("\nfloppy_do_track: invalid direction\n", RED);
            return 0; // not reached, but pleases "cmd used uninitialized"
    }

    // seek both heads
    if(floppy_seek(drive, cyl, 0)) return -1;
    if(floppy_seek(drive, cyl, 1)) return -1;

    int i;
    for(i = 0; i < 3; i++) {    
        floppy_motor_on(drive);

        // init dma..
        floppy_dma_init(dir);

        msleep(100); // give some time (100ms) to settle after the seeks

        floppy_send_command(cmd);  // set above for current direction
        floppy_send_command((head << 2) | drive);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        floppy_send_command(cyl);  // cylinder
        floppy_send_command(head);    // first head (should match with above)
        floppy_send_command(sector);    // first sector, strangely counts from 1
        floppy_send_command(2);    // bytes/sector, 128*2^x (x=2 -> 512)
        floppy_send_command(18);   // number of tracks to operate on
        floppy_send_command(0x1b); // GAP3 length, 27 is default for 3.5"
        floppy_send_command(0xff); // data length (0xff if B/S != 0)
        
        floppy_recieved_irq(); // don't SENSE_INTERRUPT here!

        // reading 7 status bytes

        // Reading Data
        

        // first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        (void)rse;
        (void)rhe;
        (void)rcy;

        st0 = floppy_read_data();
        st1 = floppy_read_data();
        st2 = floppy_read_data();
        /*
         * These are cylinder/head/sector values, updated with some
         * rather bizarre logic, that I would like to understand.
         *
         */
        rcy = floppy_read_data();
        rhe = floppy_read_data();
        rse = floppy_read_data();
        // bytes per sector, should be what we programmed in
        bps = floppy_read_data();

        int error = 0;

        if(st0 & 0xC0) {
            static char * status[] =
            { 0, "error", "invalid command", "drive not ready" };
            printf("\nfloppy_do_sector: status = ", WHITE);
            printf(status[st0 >> 6], RED);
            error = 1;
        }
        if(st1 & 0x80) {
            printf("floppy-do-sector: end of cylinder\n", RED);
            error = 1;
        }
        if(st0 & 0x08) {
            printf("floppy-do-sector: drive not ready\n", RED);
            error = 1;
        }
        if(st1 & 0x20) {
            printf("floppy-do-sector: CRC error\n", RED);
            error = 1;
        }
        if(st1 & 0x10) {
            printf("floppy-do-sector: controller timeout\n", RED);
            error = 1;
        }
        if(st1 & 0x04) {
            printf("floppy-do-sector: no data found\n", RED);
            error = 1;
        }
        if((st1|st2) & 0x01) {
            printf("floppy-do-sector: no address mark found\n", RED);
            error = 1;
        }
        if(st2 & 0x40) {
            printf("floppy-do-sector: deleted address mark\n", RED);
            error = 1;
        }
        if(st2 & 0x20) {
            printf("floppy-do-sector: CRC error in data\n", RED);
            error = 1;
        }
        if(st2 & 0x10) {
            printf("floppy-do-sector: wrong cylinder\n", RED);
            error = 1;
        }
        if(st2 & 0x04) {
            printf("floppy-do-sector: uPD765 sector not found\n", RED);
            error = 1;
        }
        if(st2 & 0x02) {
            printf("floppy-do-sector: bad cylinder\n", RED);
            error = 1;
        }
        if(bps != 0x2) {
            printf("floppy-do-sector: wanted 512B/sector, got ...\n", RED);
            error = 1;
        }
        if(st1 & 0x02) {
            printf("floppy-do-sector: not writable\n", RED);
            error = 2;
        }

        if(!error) {
            memcpy(buffer, floppy_dmabuf, 512);
            return 0;
        }
        if(error > 1) {
            printf("floppy_do_sector: not retrying..\n", RED);
            floppy_motor_off(drive);
            return -2;
        }
    }

    printf("\nfloppy-do-sector: 3 retries exhausted\n", RED);
    floppy_motor_off(drive);
    return -1;

}

int floppy_read_sector(int drive, unsigned cyl, int head, int sector, uint8_t* buffer) {
    return floppy_do_sector(drive, cyl, head, sector, floppy_dir_read, buffer);
}

int floppy_write_sector(int drive, unsigned cyl, int head, int sector, uint8_t* buffer) {
    memcpy((void*)floppy_dmabuf, buffer, 512);
    return floppy_do_sector(drive, cyl, head, sector, floppy_dir_write, buffer);
}