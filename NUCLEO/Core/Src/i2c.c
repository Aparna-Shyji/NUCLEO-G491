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
/*---------------------------------------------------------
 * Write data to I2C device
 *---------------------------------------------------------*/
int write_i2c_device(int channel_num, int slave_addr, uint8_t reg,
                            uint8_t *write_buffer, int size)
{
    HAL_StatusTypeDef ret;
    char buffer_log[256];

    if (size > MAX_I2C_WRITE_DATA_BYTES)
    {
        snprintf(buffer_log, sizeof(buffer_log), "ERROR: I2C write size too large\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
        return I2C_FAILURE;
    }

    /* Perform memory write */
    ret = HAL_I2C_Mem_Write(&hi2c1,
                            (uint16_t)(slave_addr << 1),
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            write_buffer,
                            size,
                            HAL_MAX_DELAY);

    if (ret != HAL_OK)
    {
        snprintf(buffer_log, sizeof(buffer_log),
                 "ERROR: I2C write failed (Slave:0x%02X, Reg:0x%02X)\r\n",
                 slave_addr, reg);
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
        return I2C_FAILURE;
    }

    snprintf(buffer_log, sizeof(buffer_log),
             "I2C Write OK: Slave 0x%02X Reg 0x%02X Data[0]=0x%02X\r\n",
             slave_addr, reg, write_buffer[0]);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);

    return I2C_SUCCESS;
}

/*---------------------------------------------------------
 * Read data from I2C device
 *---------------------------------------------------------*/
int read_i2c_device(int channel_num, int slave_addr, uint8_t reg,
                           uint8_t *read_buffer, int size)
{
    HAL_StatusTypeDef ret;
    char buffer_log[256];

    if (size > MAX_I2C_WRITE_DATA_BYTES)
    {
        snprintf(buffer_log, sizeof(buffer_log), "ERROR: I2C read size too large\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
        return I2C_FAILURE;
    }

    /* Perform memory read */
    ret = HAL_I2C_Mem_Read(&hi2c1,
                           (uint16_t)(slave_addr << 1),
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           read_buffer,
                           size,
                           HAL_MAX_DELAY);

    if (ret != HAL_OK)
    {
        snprintf(buffer_log, sizeof(buffer_log),
                 "ERROR: I2C read failed (Slave:0x%02X, Reg:0x%02X)\r\n",
                 slave_addr, reg);
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);
        return I2C_FAILURE;
    }

    snprintf(buffer_log, sizeof(buffer_log),
             "I2C Read OK: Slave 0x%02X Reg 0x%02X Data[0]=0x%02X\r\n",
             slave_addr, reg, read_buffer[0]);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer_log, strlen(buffer_log), HAL_MAX_DELAY);

    return I2C_SUCCESS;
}

