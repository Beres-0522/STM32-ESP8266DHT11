#ifndef __OLED_H
#define __OLED_H

#include "main.h"

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowStr(uint8_t x, uint8_t y, char *str);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t c);

#endif

