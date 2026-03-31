#include "dht11.h"
#include "gpio.h"

#define DHT11_PIN  GPIO_PIN_0
#define DHT11_PORT GPIOA

static void delay_us(uint32_t us) {
    uint32_t i = us * 8;
    while(i--);
}

uint8_t DHT11_GetData(DHT11_Data *data) {
    uint8_t buf[5] = {0};
    uint8_t retry = 0;

    // 起始信号
    GPIO_InitTypeDef gpio_conf = {0};
    gpio_conf.Pin = DHT11_PIN;
    gpio_conf.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_conf.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &gpio_conf);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);

    // 输入模式
    gpio_conf.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(DHT11_PORT, &gpio_conf);

    // 等待响应（带超时，不卡死）
    while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
        retry++;
        if(retry > 1000) return 1;
    }
    retry = 0;
    while(!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
        retry++;
        if(retry > 1000) return 1;
    }

    // 读数据
    for(uint8_t i=0; i<5; i++) {
        uint8_t byte = 0;
        for(uint8_t j=0; j<8; j++) {
            while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));
            delay_us(40);
            if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) byte |= 1<<(7-j);
            while(!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));
        }
        buf[i] = byte;
    }

    if(buf[0]+buf[1]+buf[2]+buf[3] != buf[4]) return 2;

    data->humi = buf[0];
    data->temp = buf[2];
    return 0;
}


