/*
 * flash.h
 *
 *  Created on: Oct 4, 2025
 *      Author: Architha
 */

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "stm32g4xx_hal_spi.h"
#include "stm32g4xx_hal_uart.h"

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#define MAX_GPS_RECORD_COUNT                                 60000
#define SECTOR_FREE                                          0x00
#define SECTOR_ALLOCED                                       0x01
#define BYTES_PER_SECTOR                                     4096
#define FLASH_DATA_START_ADDRESS                             4096
#define FLASH_DATA_END_ADDRESS                               (MAX_GPS_RECORD_COUNT * 256 + 4096)
#define EVENT_RECORD_SIZE                                    (MAX_GPS_RECORD_COUNT / 8)
#define RECORD_SIZE                                          256
#define MAX_BUFFER_DESCRIPTOR_COUNT               			 30
#define MAX_SECTOR_SIZE                                      4095 // nSector = 4k We use 16MB, So 4KB*4095=16000KB(16MB)
#define MAX_BLOCK_SIZE                                       255  // nblock = 64k cmd for D8 hex We use 16MB, So 64KB*255=16000KB(16MB)


/* Change according to your wiring */
#define FLASH_SPI            hspi1
extern SPI_HandleTypeDef     hspi1;

#define FLASH_CS_PORT        GPIOA
#define FLASH_CS_PIN         GPIO_PIN_4

#define FLASH_CS_LOW()       HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()      HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET)


extern void ais140_flush_spi_flash_storage_info(void);
#endif /* INC_FLASH_H_ */
