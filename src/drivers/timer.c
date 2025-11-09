#include <drivers/timer.h>
#include <interrupts/isr.h>

static uint32_t tick = 0;

extern void printf();
extern void floppy_tick();

static void timer_callback(registers_t regs)
{
    (void)regs;
    tick++;
    floppy_tick(0);
    floppy_tick(1);
}


void init_timer(uint32_t freq)
{
    register_interrupt_handler(IRQ0,&timer_callback);

    uint32_t divisor = 1193180 / freq;

    outb(0x43,0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor>>8) & 0xFF);

    outb(0x40,l);
    outb(0x40,h);
}

void msleep(uint32_t milliseconds)
{
    uint32_t start = tick;
    while ((tick - start) < milliseconds);
}

void sleep(uint32_t seconds)
{
    msleep(seconds * 1000);
}
