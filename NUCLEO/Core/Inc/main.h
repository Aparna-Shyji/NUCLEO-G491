/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MAIN_PWR_ADC_Pin GPIO_PIN_0
#define MAIN_PWR_ADC_GPIO_Port GPIOC
#define BAT_PWR_ADC_Pin GPIO_PIN_1
#define BAT_PWR_ADC_GPIO_Port GPIOC
#define MCU_GSM_EN_Pin GPIO_PIN_2
#define MCU_GSM_EN_GPIO_Port GPIOC
#define MCU_CTS_Pin GPIO_PIN_0
#define MCU_CTS_GPIO_Port GPIOA
#define MCU_RTS_Pin GPIO_PIN_1
#define MCU_RTS_GPIO_Port GPIOA
#define MCU_TX_Pin GPIO_PIN_2
#define MCU_TX_GPIO_Port GPIOA
#define MCU_RX_Pin GPIO_PIN_3
#define MCU_RX_GPIO_Port GPIOA
#define BAT_EN_Pin GPIO_PIN_13
#define BAT_EN_GPIO_Port GPIOB
#define STATUS_Pin GPIO_PIN_14
#define STATUS_GPIO_Port GPIOB
#define WIFI_HTPT_LED_Pin GPIO_PIN_15
#define WIFI_HTPT_LED_GPIO_Port GPIOB
#define OBD_LED_Pin GPIO_PIN_6
#define OBD_LED_GPIO_Port GPIOC
#define GPS_EN_Pin GPIO_PIN_7
#define GPS_EN_GPIO_Port GPIOC
#define GPRS_LED_Pin GPIO_PIN_8
#define GPRS_LED_GPIO_Port GPIOA
#define GPS_LED_Pin GPIO_PIN_10
#define GPS_LED_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
