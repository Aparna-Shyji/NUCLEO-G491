/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "stm32g4xx_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FLASH_DISK_SIZE (64 * 1024)  // 64KB fake storage
static uint8_t flash_disk[FLASH_DISK_SIZE];

#define FLASH_FS_START_ADDR   0x08070000U   // Start of Flash region reserved for filesystem
#define FLASH_FS_SIZE         (64 * 1024U)  // 64 KB total
#define FLASH_SECTOR_SIZE     (4096U)       // Sector size (erase granularity)
#define SECTOR_COUNT          (FLASH_FS_SIZE / FLASH_SECTOR_SIZE)


/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

extern UART_HandleTypeDef huart1;
static void UART_Print(const char *msg)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL// _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize(BYTE pdrv)
{
    memset(flash_disk, 0xFF, FLASH_DISK_SIZE);
    Stat = 0;
    UART_Print("FlashDisk: Init OK\r\n");
    return Stat;
}
/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status(BYTE pdrv)
{
    return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
/*DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (sector >= SECTOR_COUNT)
        return RES_PARERR;

    uint32_t addr = FLASH_FS_START_ADDR + (sector * FLASH_SECTOR_SIZE);
    memcpy(buff, (uint8_t*)addr, count * FLASH_SECTOR_SIZE);

    return RES_OK;
}*/
DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	//UART_Print("USER_read called\r\n");
    UINT offset = sector * 512; // sector size is 512 bytes
    UINT bytes = count * 512;

    if ((offset + bytes) > FLASH_DISK_SIZE)
        return RES_PARERR;

    memcpy(buff, &flash_disk[offset], bytes);
    return RES_OK;
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
//#if _USE_WRITE == 1
/*DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (sector >= SECTOR_COUNT)
        return RES_PARERR;

    uint32_t addr = FLASH_FS_START_ADDR + (sector * FLASH_SECTOR_SIZE);
    HAL_StatusTypeDef status;

    // Erase the sector(s) before programming
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError;

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Page = (addr - 0x08000000U) / FLASH_PAGE_SIZE;
    eraseInit.NbPages = count * (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE);

    status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
    if (status != HAL_OK)
        return RES_ERROR;

    // Program flash word-by-word
    for (uint32_t i = 0; i < count * FLASH_SECTOR_SIZE; i += 8)
    {
        uint64_t data64;
        memcpy(&data64, buff + i, sizeof(uint64_t));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, data64);
        if (status != HAL_OK)
            return RES_ERROR;
    }

    return RES_OK;
}*/
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	//UART_Print("USER_write called\r\n");
    UINT offset = sector * 512;
    UINT bytes = count * 512;

    if ((offset + bytes) > FLASH_DISK_SIZE)
        return RES_PARERR;

    memcpy(&flash_disk[offset], buff, bytes);
    return RES_OK;
}
//#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
/*DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res = RES_OK;

    switch (cmd)
    {
        case CTRL_SYNC:
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT:
            *(DWORD*)buff = SECTOR_COUNT;
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE:
            *(WORD*)buff = FLASH_SECTOR_SIZE;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}*/
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            *(DWORD*)buff = FLASH_DISK_SIZE / 512;
            return RES_OK;

        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;

        default:
            return RES_PARERR;

    }
}
#endif /* _USE_IOCTL == 1 */
