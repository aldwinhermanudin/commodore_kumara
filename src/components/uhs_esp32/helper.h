
#ifndef _helper_
#define _helper_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define NOP() asm volatile ("nop")

unsigned long IRAM_ATTR micros();
unsigned long IRAM_ATTR millis();
void delay(uint32_t ms);
void IRAM_ATTR delayMicroseconds(uint32_t us);
void yield();

#endif
