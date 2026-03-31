#ifndef __DHT11_H
#define __DHT11_H
#include "main.h"

typedef struct {
    uint8_t temp;  // 温度整数
    uint8_t humi;  // 湿度整数
} DHT11_Data;

uint8_t DHT11_GetData(DHT11_Data *data);

#endif


