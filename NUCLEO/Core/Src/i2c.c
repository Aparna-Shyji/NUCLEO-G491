/*
 * i2c.c
 *
 *  Created on: Oct 9, 2025
 *      Author: Admin
 */

#include <stdio.h>
#include <string.h>
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;


/*---------------------------------------------------------
 * Initialize I2C device
 *---------------------------------------------------------*/
void init_i2c_device(void)
{
    HAL_StatusTypeDef ret;
    char buffer_log[100];

    ret = HAL_I2C_Init(&hi2c1);
    if (ret != HAL_OK)
    {
        snprintf(buffer_log, sizeof(buffer_log), "ERROR: I2C1 init failed\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
        Error_Handler();
        return;
    }

    /* Configure I2C filters for stable operation */
    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);

    snprintf(buffer_log, sizeof(buffer_log), "I2C1 init SUCCESS\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
}

