/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 最终稳定版，零错误零警告
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "dht11.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY1_Pin    GPIO_PIN_12
#define KEY2_Pin    GPIO_PIN_13
#define KEY3_Pin    GPIO_PIN_14
#define KEY4_Pin    GPIO_PIN_15
#define KEY_PORT    GPIOB

#define BEEP_Pin    GPIO_PIN_5
#define BEEP_PORT   GPIOA

#define TEMP_MAX    30
#define HUMI_MAX    70
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t  ui_page = 1;
uint8_t  last_page = 0;
DHT11_Data dht;
char buf[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
uint8_t Key_Scan(void);
void Beep_Control(void);
void UI_Update(void);
void ESP8266_Init(void);
/* USER CODE END PFP */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_NVIC_Init();

  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  DHT11_GetData(&dht);
  ESP8266_Init();
  HAL_GPIO_WritePin(BEEP_PORT, BEEP_Pin, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
      while (1)
{
  // 按键
  uint8_t key = Key_Scan();
  if(key != 0) ui_page = key;

  // 温湿度
  DHT11_GetData(&dht);

  // 显示
  UI_Update();

  // 蜂鸣器
  Beep_Control();

  // 每1秒发送一次（纯英文，无汉字！）
  static uint32_t test_cnt = 0;
  if(HAL_GetTick() - test_cnt >= 1000)
  {
    // 只发英文，永不乱码
    char test_str[32];
    sprintf(test_str, "T:%dC H:%d%%\r\n", dht.temp, dht.humi);
    
    // 发给 ESP8266
    HAL_UART_Transmit(&huart2, (uint8_t*)test_str, strlen(test_str), 100);
    
    // 串口打印（也纯英文）
    printf("SEND: %s", test_str);
    
    test_cnt = HAL_GetTick();
  }

  HAL_Delay(10);
}
}

/* USER CODE BEGIN 4 */
uint8_t Key_Scan(void)
{
  if(HAL_GPIO_ReadPin(KEY_PORT, KEY1_Pin) == GPIO_PIN_SET)
  {
    HAL_Delay(10);
    if(HAL_GPIO_ReadPin(KEY_PORT, KEY1_Pin) == GPIO_PIN_SET)
    {
      while(HAL_GPIO_ReadPin(KEY_PORT, KEY1_Pin) == GPIO_PIN_SET);
      return 1;
    }
  }
  if(HAL_GPIO_ReadPin(KEY_PORT, KEY2_Pin) == GPIO_PIN_SET)
  {
    HAL_Delay(10);
    if(HAL_GPIO_ReadPin(KEY_PORT, KEY2_Pin) == GPIO_PIN_SET)
    {
      while(HAL_GPIO_ReadPin(KEY_PORT, KEY2_Pin) == GPIO_PIN_SET);
      return 2;
    }
  }
  if(HAL_GPIO_ReadPin(KEY_PORT, KEY3_Pin) == GPIO_PIN_SET)
  {
    HAL_Delay(10);
    if(HAL_GPIO_ReadPin(KEY_PORT, KEY3_Pin) == GPIO_PIN_SET)
    {
      while(HAL_GPIO_ReadPin(KEY_PORT, KEY3_Pin) == GPIO_PIN_SET);
      return 3;
    }
  }
  if(HAL_GPIO_ReadPin(KEY_PORT, KEY4_Pin) == GPIO_PIN_SET)
  {
    HAL_Delay(10);
    if(HAL_GPIO_ReadPin(KEY_PORT, KEY4_Pin) == GPIO_PIN_SET)
    {
      while(HAL_GPIO_ReadPin(KEY_PORT, KEY4_Pin) == GPIO_PIN_SET);
      return 4;
    }
  }
  return 0;
}

void Beep_Control(void)
{
  if(dht.temp > TEMP_MAX || dht.humi > HUMI_MAX)
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_Pin, GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(BEEP_PORT, BEEP_Pin, GPIO_PIN_SET);
}

void UI_Update(void)
{
  if(ui_page != last_page)
  {
    OLED_Clear();
    last_page = ui_page;
  }

  uint32_t run_sec = HAL_GetTick() / 1000;
  uint32_t h = run_sec / 3600;
  uint32_t m = (run_sec % 3600) / 60;
  uint32_t s = run_sec % 60;

  switch(ui_page)
  {
    case 1:
      OLED_ShowStr(0, 0, "DHT11 MONITOR");
      sprintf(buf, "T:%dC  H:%d%%", dht.temp, dht.humi);
      OLED_ShowStr(0, 2, buf);
      break;
    case 2:
      OLED_ShowStr(0, 0, "SYS RUN TIME");
      sprintf(buf, "%02d:%02d:%02d", h, m, s);
      OLED_ShowStr(0, 2, buf);
      break;
    case 3:
      OLED_ShowStr(0, 0, "WELCOME!       ");
      OLED_ShowStr(0, 2,"STM32 WIFI SYSTEM");
      break;
    case 4:
      OLED_Clear();
      break;
    default:
      ui_page = 1;
      break;
  }
}

void ESP8266_Init(void)
{
  HAL_Delay(3000);
  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+RST\r\n", 8, 100);
  HAL_Delay(3000);

  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CWMODE=1\r\n", 13, 100);
  HAL_Delay(1000);

  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CWJAP=\"Beres\",\"66666661\"\r\n", 32, 100);
  HAL_Delay(8000);

  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPSTART=\"TCP\",\"192.168.101.237\",8080\r\n", 44, 100);
  HAL_Delay(3000);

  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPMODE=1\r\n", 13, 100);
  HAL_Delay(1000);
  HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CIPSEND\r\n", 11, 100);
  HAL_Delay(1000);
}

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
  return ch;
}
/* USER CODE END 4 */

/**
  * @brief  NVIC Initialization Function
  * @param  None
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif


