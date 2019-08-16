#ifndef _DELAY_H
#define _DELAY_H

#include <stdint.h>
#include "stm32f2xx_hal.h" 
  

void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
#endif

