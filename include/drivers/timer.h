#ifndef TIMER_H_
#define TIMER_H_

#include "common.h"

extern void init_timer(uint32_t frequency);
extern void sleep(uint32_t seconds);
extern void msleep(uint32_t milliseconds);

#endif