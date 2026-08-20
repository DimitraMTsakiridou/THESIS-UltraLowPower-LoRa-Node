/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : RECEIVER FINAL - BULLETPROOF STATE MACHINE
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "app_subghz_phy.h"
#include "radio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* --- VARIABLES --- */
CRYP_HandleTypeDef hcryp;
RTC_HandleTypeDef hrtc;
SUBGHZ_HandleTypeDef hsubghz;
UART_HandleTypeDef huart2;

/* --- DEBUG VARIABLES --- */
volatile uint32_t rx_success_cnt = 0;
volatile int16_t  rx_rssi = 0;
volatile int8_t   rx_snr = 0;
char last_decrypted_msg[64];

/* --- FLAGS  --- */
volatile bool flag_packet_received = false;
volatile bool flag_rx_error = false;

uint8_t  safe_payload[255];
uint16_t safe_size;

uint32_t my_aes_key[8] = {
    0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00,
    0x00112233, 0x44556677, 0x8899AABB, 0xCCDDEEFF
};

char uart_buf[150];

/* --- PROTOTYPES --- */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_AES_Init(void);
static void MX_USART2_UART_Init(void);
void MX_SUBGHZ_Init(void);
void My_Delay_Ms(uint32_t ms);

/* --- CALLBACKS --- */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    if (size < 255) {
        memcpy(safe_payload, payload, size);
        safe_size = size;
        rx_rssi = rssi;
        rx_snr = snr;
        flag_packet_received = true;
    }
}

void OnRxTimeout(void) { flag_rx_error = true; }
void OnRxError(void)   { flag_rx_error = true; }

RadioEvents_t RadioEvents;

void Radio_Setup_Receiver(void) {
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    Radio.Init(&RadioEvents);
    Radio.SetChannel(865800000);
    Radio.SetPublicNetwork(false); 
    Radio.SetRxConfig(MODEM_LORA, 0, 12, 1, 0, 8, 0, false, 0, true, 0, 0, false, true);

    Radio.Rx(0);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_AES_Init();
  MX_USART2_UART_Init();
  MX_SUBGHZ_Init();

  hcryp.Init.pKey = my_aes_key;

  // Heartbeat
  for(int i=0; i<3; i++) {
      HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);
      My_Delay_Ms(50);
      HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);
      My_Delay_Ms(150);
  }

  // Always RED LED ON 
  HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);

  Radio_Setup_Receiver();

  while (1)
  {
      MX_SubGHz_Phy_Process(); 
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
      // --- SCENARIO 1: Packet Received ---
      if (flag_packet_received) {
          flag_packet_received = false; 

          rx_success_cnt++; 

          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

          uint8_t decrypted[32] = {0};
          if (safe_size >= 36) {
              HAL_CRYP_Init(&hcryp);
              HAL_CRYP_Decrypt(&hcryp, (uint32_t*)&safe_payload[4], 8, (uint32_t*)decrypted, 100);

              memset(last_decrypted_msg, 0, 64);
              strncpy(last_decrypted_msg, (char*)decrypted, 32);

              int len = sprintf(uart_buf, "{\"id\":%d,\"rssi\":%d,\"snr\":%d,\"data\":\"%s\"}\r\n",
                                safe_payload[1], rx_rssi, rx_snr, (char*)decrypted);
              HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, len, 100);
          }

          My_Delay_Ms(1000);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
          Radio.Rx(0);
      }

      // --- SCENARIO 2: Reception Error ---
      if (flag_rx_error) {
          flag_rx_error = false;
          Radio.Rx(0);
      }
  }
}

void My_Delay_Ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i++) {
        for(volatile uint32_t j = 0; j < 1450; j++) { __NOP(); }
    }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}
static void MX_RTC_Init(void) {
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK) { Error_Handler(); }
}
static void MX_AES_Init(void) {
  hcryp.Instance = AES;
  hcryp.Init.DataType = CRYP_DATATYPE_8B;
  hcryp.Init.KeySize = CRYP_KEYSIZE_256B;
  hcryp.Init.Algorithm = CRYP_AES_ECB;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK) { Error_Handler(); }
}
void MX_SUBGHZ_Init(void) {
  hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
  if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK) { Error_Handler(); }
}
static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(LED_STATUS_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void Error_Handler(void) {
  __disable_irq();
  while (1) {
      HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
      for(volatile int i=0;i<50000;i++);
  }
}
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
