#include <drivers/timer.h>
#include <interrupts/isr.h>

static uint32_t tick = 0;

extern void printf();

static void timer_callback(registers_t regs)
{
    tick++;
}

void init_timer(uint32_t freq)
{
    // Для начала регистрируем наш callback
    register_interrupt_handler(IRQ0,&timer_callback);

    // Значение, сообщаемое в PIT
    uint32_t divisor = 1193180 / freq;

    // Послать команду
    outb(0x43,0x36);

    // Значение посылается в два этапа
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor>>8) & 0xFF);

    // Посылаем на порт данных
    outb(0x40,l);
    outb(0x40,h);
}

void sleep(uint32_t seconds)
{
  uint32_t timer_ticks;
  timer_ticks = tick + (seconds * 50);
  /*
  kprintf("sleeping for %d seconds ", seconds);*/
  while(tick < timer_ticks);
  printf("sleep done", 0x999);
}