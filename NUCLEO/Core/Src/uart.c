/**
  * @brief RX Complete Callback — called when 1 byte received
  */
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal_uart.h"
#include<string.h>

extern UART_HandleTypeDef huart1;

// RX buffer
uint8_t rx_data;
uint8_t rx_buffer[100];
uint16_t rx_index = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_UART_Transmit(&huart1, &rx_data, 1, HAL_MAX_DELAY);  // echo back
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);               // restart interrupt
    }
}



void MX_USART1_UART_Init(void)
{

  uint8_t start_msg[] = "\r\n=== STM32 USART1 Loopback Test Started ===\r\n";

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    // Optional: FIFO configuration
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        Error_Handler();
        return -1;
    }

    // Start receiving one byte via interrupt
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);

    // Print startup message
    HAL_UART_Transmit(&huart1, start_msg, strlen((char *)start_msg), HAL_MAX_DELAY);

    return 0;

}
