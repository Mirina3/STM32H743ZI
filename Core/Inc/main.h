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
#include "stm32h7xx_hal.h"

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
#define SPI2_PD_OFFSET_nCS_C_Pin GPIO_PIN_3
#define SPI2_PD_OFFSET_nCS_C_GPIO_Port GPIOE
#define SPI2_PD_OFFSET_nCS_B_Pin GPIO_PIN_4
#define SPI2_PD_OFFSET_nCS_B_GPIO_Port GPIOE
#define SYNC_VIEW_SELECT_Pin GPIO_PIN_5
#define SYNC_VIEW_SELECT_GPIO_Port GPIOE
#define SYNC_GAIN_D0_Pin GPIO_PIN_6
#define SYNC_GAIN_D0_GPIO_Port GPIOE
#define RMII_nINT_WAKE_UP2_Pin GPIO_PIN_13
#define RMII_nINT_WAKE_UP2_GPIO_Port GPIOC
#define RMII_RXER_PHYAD0_Pin GPIO_PIN_0
#define RMII_RXER_PHYAD0_GPIO_Port GPIOC
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define POW_SEN_Pin GPIO_PIN_2
#define POW_SEN_GPIO_Port GPIOC
#define AC_POE_SYNC_Pin GPIO_PIN_3
#define AC_POE_SYNC_GPIO_Port GPIOC
#define AC_POE_SYNC_EXTI_IRQn EXTI3_IRQn
#define RMII_nRESET_Pin GPIO_PIN_0
#define RMII_nRESET_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define SYNC_GAIN_D1_Pin GPIO_PIN_7
#define SYNC_GAIN_D1_GPIO_Port GPIOE
#define SYNC_GAIN_D2_Pin GPIO_PIN_8
#define SYNC_GAIN_D2_GPIO_Port GPIOE
#define SYNC_MUX_SEL0_Pin GPIO_PIN_9
#define SYNC_MUX_SEL0_GPIO_Port GPIOE
#define SYNC_MUX_SEL1_Pin GPIO_PIN_10
#define SYNC_MUX_SEL1_GPIO_Port GPIOE
#define SPI_LE_50V_Pin GPIO_PIN_11
#define SPI_LE_50V_GPIO_Port GPIOE
#define SPI4_BOD_SCK_Pin GPIO_PIN_12
#define SPI4_BOD_SCK_GPIO_Port GPIOE
#define SYNC_NOTCH_SELECT_Pin GPIO_PIN_13
#define SYNC_NOTCH_SELECT_GPIO_Port GPIOE
#define SPI4_BOD_MOSI_Pin GPIO_PIN_14
#define SPI4_BOD_MOSI_GPIO_Port GPIOE
#define SYNC_LED_Pin GPIO_PIN_15
#define SYNC_LED_GPIO_Port GPIOE
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOB
#define RMII_TXD0_Pin GPIO_PIN_12
#define RMII_TXD0_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define SPI2_OFFSET_MOSI_Pin GPIO_PIN_15
#define SPI2_OFFSET_MOSI_GPIO_Port GPIOB
#define SPI_SYNC_Q_nCS_10K_Pin GPIO_PIN_8
#define SPI_SYNC_Q_nCS_10K_GPIO_Port GPIOD
#define SPI_SYNC_FQ_nCS_1K_Pin GPIO_PIN_9
#define SPI_SYNC_FQ_nCS_1K_GPIO_Port GPIOD
#define SYNC_TTL_OUT_Pin GPIO_PIN_10
#define SYNC_TTL_OUT_GPIO_Port GPIOD
#define SYNC_TTL_OUT_EXTI_IRQn EXTI15_10_IRQn
#define RUN_LED_Pin GPIO_PIN_14
#define RUN_LED_GPIO_Port GPIOD
#define HALT_LED_Pin GPIO_PIN_15
#define HALT_LED_GPIO_Port GPIOD
#define USART6_TX_Pin GPIO_PIN_6
#define USART6_TX_GPIO_Port GPIOC
#define USART6_RX_Pin GPIO_PIN_7
#define USART6_RX_GPIO_Port GPIOC
#define SDMMC1_DET_Pin GPIO_PIN_8
#define SDMMC1_DET_GPIO_Port GPIOA
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define USART1_RX_Pin GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define SPI2_PD_OFFSET_nCS_A_Pin GPIO_PIN_11
#define SPI2_PD_OFFSET_nCS_A_GPIO_Port GPIOA
#define USART1_DE_Pin GPIO_PIN_12
#define USART1_DE_GPIO_Port GPIOA
#define POW_CLT_V3P3D_Pin GPIO_PIN_0
#define POW_CLT_V3P3D_GPIO_Port GPIOD
#define POW_CLT_V5P0C_Pin GPIO_PIN_1
#define POW_CLT_V5P0C_GPIO_Port GPIOD
#define SPI2_OFFSET_CLK_Pin GPIO_PIN_3
#define SPI2_OFFSET_CLK_GPIO_Port GPIOD
#define POW_CLT_V3P3C_Pin GPIO_PIN_4
#define POW_CLT_V3P3C_GPIO_Port GPIOD
#define PEAK_RST_Pin GPIO_PIN_5
#define PEAK_RST_GPIO_Port GPIOD
#define SPI_SYNC_OFFSET_nCS_Pin GPIO_PIN_6
#define SPI_SYNC_OFFSET_nCS_GPIO_Port GPIOD
#define SPI_SYNC_FQ_nCS_50K_Pin GPIO_PIN_7
#define SPI_SYNC_FQ_nCS_50K_GPIO_Port GPIOD
#define I2C4_INT_SCL_Pin GPIO_PIN_6
#define I2C4_INT_SCL_GPIO_Port GPIOB
#define I2C4_INT_SDA_Pin GPIO_PIN_7
#define I2C4_INT_SDA_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_8
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB
#define POW_CTL_V5P0A_Pin GPIO_PIN_0
#define POW_CTL_V5P0A_GPIO_Port GPIOE
#define POW_CLT_V3P3E_Pin GPIO_PIN_1
#define POW_CLT_V3P3E_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

#define POW_CLT_V3P3C_OFF() HAL_GPIO_WritePin(POW_CLT_V3P3C_GPIO_Port, POW_CLT_V3P3C_Pin, GPIO_PIN_SET)     // 통신관련 전원 OFF
#define POW_CLT_V3P3C_ON()  HAL_GPIO_WritePin(POW_CLT_V3P3C_GPIO_Port, POW_CLT_V3P3C_Pin, GPIO_PIN_RESET)   // 통신관련 전원 ON
#define POW_CLT_V3P3D_OFF() HAL_GPIO_WritePin(POW_CLT_V3P3D_GPIO_Port, POW_CLT_V3P3D_Pin, GPIO_PIN_SET)     // MCU Board와 Analog board의 Digital 전원 OFF
#define POW_CLT_V3P3D_ON()  HAL_GPIO_WritePin(POW_CLT_V3P3D_GPIO_Port, POW_CLT_V3P3D_Pin, GPIO_PIN_RESET)   // MCU Board와 Analog board의 Digital 전원 ON
#define POW_CLT_V3P3E_OFF() HAL_GPIO_WritePin(POW_CLT_V3P3E_GPIO_Port, POW_CLT_V3P3E_Pin, GPIO_PIN_SET)     // 이더넷 전원 3.3V OFF
#define POW_CLT_V3P3E_ON()  HAL_GPIO_WritePin(POW_CLT_V3P3E_GPIO_Port, POW_CLT_V3P3E_Pin, GPIO_PIN_RESET)   // 이더넷 전원 3.3V ON
#define POW_CTL_V5P0A_OFF() HAL_GPIO_WritePin(POW_CTL_V5P0A_GPIO_Port, POW_CTL_V5P0A_Pin, GPIO_PIN_RESET)   // Analog board +/-5.0V 및 OFF
#define POW_CTL_V5P0A_ON()  HAL_GPIO_WritePin(POW_CTL_V5P0A_GPIO_Port, POW_CTL_V5P0A_Pin, GPIO_PIN_SET)     // Analog board +/-5.0V 및 ON
#define POW_CLT_V5P0C_OFF() HAL_GPIO_WritePin(POW_CLT_V5P0C_GPIO_Port, POW_CLT_V5P0C_Pin, GPIO_PIN_RESET)   // 외부센서 통신전원 5.0V 전원 OFF
#define POW_CLT_V5P0C_ON()  HAL_GPIO_WritePin(POW_CLT_V5P0C_GPIO_Port, POW_CLT_V5P0C_Pin, GPIO_PIN_SET)     // 외부센서 통신전원 5.0V 전원 ON

uint32_t get_fattime(void);

extern SD_HandleTypeDef hsd1;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
