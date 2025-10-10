/*
 * flash.c
 *
 *  Created on: Oct 4, 2025
 *  Author: Architha
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
// #include "stm32g4xx_hal_spi.h"
#include "stm32g4xx_hal_uart.h"
#include"type.h"
#include "flash.h"
// Define the parameters for the test
#define TEST_DATA_LEN   64 // Length of data to write/read
#define TEST_ADDR       0x00 // Manufacturer Sector (Sector 0) Start Address

// A sample pattern to write to the flash
const uint8_t g_write_pattern[TEST_DATA_LEN] = {
    0xAA, 0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80,
    0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00,
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE, 
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};
extern SPI_HandleTypeDef     hspi1;
int8_t flash_rd_status_reg(void)
{
    uint8_t wr_buff[2] = {0x05, 0x00};   // 0x05 = Read Status Register
    uint8_t rd_buff[2] = {0};            // Receive buffer
    HAL_StatusTypeDef ret;
    uint8_t status_reg = 0;

    // Pull CS low to start communication
    FLASH_CS_LOW();

    // Transmit 0x05 and receive 2 bytes (1 dummy + status)
    ret = HAL_SPI_TransmitReceive(&hspi1, wr_buff, rd_buff, 2, HAL_MAX_DELAY);

    // Pull CS high to end communication
    FLASH_CS_HIGH();

    if (ret == HAL_OK)
    {
        status_reg = rd_buff[1];   // Skip echo of 0x05, actual status in byte[1]

#ifdef SPI_DEBUG_LOG
        UART_Print("FLASH-DRIVER-LOG : Status Register = 0x%02X\r\n", status_reg);
#endif
    }
    else
    {
#ifdef SPI_DEBUG_LOG
        UART_Print("FLASH-DRIVER-LOG : ERROR - SPI Read Failed!\r\n");
#endif
        return -1; // SPI error
    }

    return status_reg;
}

void spi_flash_cs(bool CS)
{
    if (CS) {
        HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);   // CS High
    } else {
        HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET); // CS Low
    }
}

// Check if SPI flash command is over
uint8_t cmd_is_over(void)
{
    uint8_t ret = 1; // TRUE
    uint8_t status = flash_rd_status_reg();

// #ifdef SPI_PART_GD25LB64ESIGR
    // Wait while busy (WIP bit = 0x01)
    while(status & 0x01){
        HAL_Delay(10); // sleep 10 ms
        status = flash_rd_status_reg();
    }
    if(status & 0x01){
        ret = 0; // FALSE
    }
// #endif

    return ret;
}


void flash_wr_enable(void)
{
    uint8_t cmd = 0x06; // Write Enable
    HAL_StatusTypeDef ret;

    FLASH_CS_LOW();
    ret = HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    if (ret != HAL_OK)
    {
        // Replace this with your logger
        UART_Print("FLASH-DRIVER-LOG: HAL_SPI_Transmit failed in flash_wr_enable\r\n");
        return;
    }

    if (cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: write flash_wr_enable success\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: Failed to enable flash write\r\n");
    }
}

// Reset SPI flash
void flash_reset(void)
{
    HAL_StatusTypeDef ret;
    uint8_t cmd;

    // --- Reset Enable (0x66) ---
    cmd = 0x66;
    FLASH_CS_LOW();
    ret = HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    if (ret == HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: Reset Enable (0x66) success\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: Failed Reset Enable (0x66)\r\n");
    }

    // --- Reset Command (0x99) ---
    cmd = 0x99;
    FLASH_CS_LOW();
    ret = HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    if (ret == HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: Reset Command (0x99) success\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: Failed Reset Command (0x99)\r\n");
    }
}

// Read Flash ID
void flash_rd_id(void)
{
    HAL_StatusTypeDef ret;
    uint8_t wr_rd_buff[4] = {0x9F, 0x00, 0x00, 0x00}; // command + dummy bytes

    FLASH_CS_LOW();
    ret = HAL_SPI_TransmitReceive(&hspi1, wr_rd_buff, wr_rd_buff, 4, HAL_MAX_DELAY);
    FLASH_CS_HIGH();

    if (ret == HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: Mfg-ID      : 0x%02X\r\n", wr_rd_buff[1]);
        UART_Print("FLASH-DRIVER-LOG: Mem-Type    : 0x%02X\r\n", wr_rd_buff[2]);
        UART_Print("FLASH-DRIVER-LOG: Mem-Density : 0x%02X\r\n", wr_rd_buff[3]);
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: Failed reading flash ID\r\n");
    }
}

// Erase entire SPI flash chip
void ais140_flash_erase_chip(void)
{
    uint8_t cmd = 0xC7; // Chip Erase command

    // Flush flash storage info before erase
    ais140_flush_spi_flash_storage_info();

    UART_Print("FLASH-DRIVER-LOG: Erasing SPI flash chip\r\n");

    // Enable write
    flash_wr_enable();

    // Send Chip Erase command
    FLASH_CS_LOW();
    if (HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: Failed to send Chip Erase command\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    // Wait a while; full chip erase takes time
    HAL_Delay(10);

    // Check if command is completed
    if(cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: Completed chip erase\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to erase the chip\r\n");
    }
}

// Erase a specific block
void flash_erase_block(int8_t nblock)
{
    if (nblock > MAX_BLOCK_SIZE)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Block > 31 specified\r\n");
        return;
    }

    uint8_t cmd = 0xD8; // Block Erase command
    uint32_t addr = nblock * 0x10000; // 64 KB block size
    uint8_t wr_buff[4] = {
        cmd,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    // Enable write
    flash_wr_enable();

    // Send Block Erase command + address
    FLASH_CS_LOW();
    if (HAL_SPI_Transmit(&hspi1, wr_buff, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to send block erase command\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    // Wait a while; block erase takes time
    HAL_Delay(10);

    // Check if command is completed
    if(cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: Erased a block successfully\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to erase a block\r\n");
    }
}

// Erase a specific sector
void ais140_flash_erase_sector(int16_t nsector)
{
    if ((nsector > MAX_SECTOR_SIZE) || (nsector == 0))
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Sector no is greater than %d or zero\r\n", MAX_SECTOR_SIZE);
        return;
    }

    UART_Print("FLASH-DRIVER-LOG: Going to Erase Sector N(%d)\r\n", nsector);

    uint8_t cmd = 0x20; // Sector Erase command
    uint32_t addr = nsector * 0x1000; // 4 KB sector size
    uint8_t wr_buff[4] = {
        cmd,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    // Enable write
    flash_wr_enable();

    // Send Sector Erase command + address
    FLASH_CS_LOW();
    if (HAL_SPI_Transmit(&hspi1, wr_buff, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to send sector erase command\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    // Wait a while; sector erase takes time
    HAL_Delay(10);

    // Check if command is completed
    if(cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: Sector erase completed successfully\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to erase sector\r\n");
    }

    // Additional delay if needed (datasheet may require longer wait)
    HAL_Delay(1000);
}

// Erase the manufacturer sector (sector 0)
void ais140_flash_erase_mfg_sector(void)
{
    int16_t nsector = 0; // Manufacturer sector

    UART_Print("FLASH-DRIVER-LOG: Going to Erase Sector N(%d)\r\n", nsector);

    uint8_t cmd = 0x20; // Sector Erase command
    uint32_t addr = nsector * 0x1000; // 4 KB sector size
    uint8_t wr_buff[4] = {
        cmd,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    // Enable write
    flash_wr_enable();

    // Send Sector Erase command + address
    FLASH_CS_LOW();
    if (HAL_SPI_Transmit(&hspi1, wr_buff, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to send sector erase command\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    // Wait a while; sector erase takes time
    HAL_Delay(10);

    // Check if command is completed
    if(cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: Manufacturer sector erase completed successfully\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to erase manufacturer sector\r\n");
    }

    // Additional delay if required by datasheet
    HAL_Delay(1000);
}

void ais140_flash_write_data(uint32_t addr, uint8_t *pbuff, uint32_t len)
{
    if(len > 256)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Too many bytes to Page Program write len(%lu)\r\n", len);
        return;
    }

    uint8_t wr_buff[260]; // 4 bytes for command + address, max 256 bytes data
    unsigned int  i;

    // Prepare command + address
    wr_buff[0] = 0x02; // Page Program command
    wr_buff[1] = (addr >> 16) & 0xFF;
    wr_buff[2] = (addr >> 8) & 0xFF;
    wr_buff[3] = addr & 0xFF;

    // Copy data
    for(i = 0; i < len; i++)
    {
        wr_buff[i + 4] = pbuff[i];
    }

    // Enable write
    flash_wr_enable();

    // Send page program command + data
    FLASH_CS_LOW();
    if(HAL_SPI_Transmit(&hspi1, wr_buff, len + 4, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to write data\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    HAL_Delay(10); // Wait for write to complete

    // Check if command is completed
    if(cmd_is_over())
    {
        UART_Print("FLASH-DRIVER-LOG: Page write successfully completed\r\n");
    }
    else
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to write the page\r\n");
    }
}

// Read data from SPI flash (up to 256 bytes per call)
void ais140_flash_read_data(uint32_t addr, uint8_t *pbuff, uint32_t len)
{
    if(len > 256)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Too big buffer to read len(%lu)\r\n", len);
        return;
    }

    uint8_t wr_rd_buff[4 + 256]; // max buffer
    wr_rd_buff[0] = 0x03;             // READ command
    wr_rd_buff[1] = (addr >> 16) & 0xFF;
    wr_rd_buff[2] = (addr >> 8) & 0xFF;
    wr_rd_buff[3] = addr & 0xFF;

    FLASH_CS_LOW();
    if(HAL_SPI_TransmitReceive(&hspi1, wr_rd_buff, wr_rd_buff, len + 4, HAL_MAX_DELAY) != HAL_OK)
    {
        UART_Print("FLASH-DRIVER-LOG: ERROR - Failed to read data\r\n");
        FLASH_CS_HIGH();
        return;
    }
    FLASH_CS_HIGH();

    // Copy read data to pbuff
    memcpy(pbuff, &wr_rd_buff[4], len);
    UART_Print("FLASH-DRIVER-LOG: Read %lu bytes successfully from address 0x%08lX\r\n", len, addr);
}

// Read manufacturer data (sector 0)
void flash_read_mfg_data(uint8_t *pbuff, uint32_t len)
{
    ais140_flash_read_data(0x00, pbuff, len);
}

// Write manufacturer data (sector 0)
void flash_wr_mfg_data(uint8_t *pbuff, uint32_t len)
{
    ais140_flash_write_data(0x00, pbuff, len);
    HAL_Delay(1000); // Wait for write completion
}


/**
 * @brief Performs a full Read-Write-Verify cycle on the Manufacturer Sector (0x00).
 * * NOTE: This is a DESTRUCTIVE test and will erase the data in Sector 0.
 */
void flash_driver_self_test(void)
{
    uint8_t read_buffer[TEST_DATA_LEN] = {0};
    uint8_t i;
    uint8_t status_reg;

    UART_Print("\r\n==============================================\r\n");
    UART_Print("FLASH SELF-TEST: Starting Read/Write Verification\r\n");
    
    // 1. Initial Status and ID Check
    flash_rd_id();
    
    status_reg = flash_rd_status_reg();
    UART_Print("FLASH SELF-TEST: Initial Status Register: 0x%02X\r\n", status_reg);
    
    // ----------------------------------------------------
    // 2. ERASE: Clear the dedicated test sector
    // ----------------------------------------------------
    UART_Print("FLASH SELF-TEST: Starting Sector Erase at 0x%08lX...\r\n", (uint32_t)TEST_ADDR);
    ais140_flash_erase_mfg_sector(); // Calls Sector Erase for sector 0
    // ais140_flash_erase_mfg_sector output will show success/failure

    // ----------------------------------------------------
    // 3. WRITE: Write the test pattern
    // ----------------------------------------------------
    UART_Print("FLASH SELF-TEST: Writing %d bytes of test data...\r\n", TEST_DATA_LEN);
    ais140_flash_write_data(TEST_ADDR, (uint8_t*)g_write_pattern, TEST_DATA_LEN);
    // ais140_flash_write_data output will show success/failure

    // ----------------------------------------------------
    // 4. READ: Read the data back
    // ----------------------------------------------------
    UART_Print("FLASH SELF-TEST: Reading %d bytes back from 0x%08lX...\r\n", TEST_DATA_LEN, (uint32_t)TEST_ADDR);
    ais140_flash_read_data(TEST_ADDR, read_buffer, TEST_DATA_LEN);
    
    // ----------------------------------------------------
    // 5. VERIFY: Compare the written data with the read data
    // ----------------------------------------------------
    if (memcmp(g_write_pattern, read_buffer, TEST_DATA_LEN) == 0)
    {
        UART_Print("FLASH SELF-TEST: ✅ SUCCESS! Data verification passed.\r\n");
        UART_Print("==============================================\r\n");
    }
    else
    {
        UART_Print("FLASH SELF-TEST: ❌ FAILURE! Data mismatch detected.\r\n");
        
        // Detailed failure log
        for (i = 0; i < TEST_DATA_LEN; i++)
        {
            if (g_write_pattern[i] != read_buffer[i])
            {
                UART_Print("MISMATCH at offset 0x%02X: Wrote 0x%02X, Read 0x%02X\r\n", 
                           i, g_write_pattern[i], read_buffer[i]);
            }
        }
        UART_Print("==============================================\r\n");
    }
}