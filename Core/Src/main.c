/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdcard.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd1;

/* USER CODE BEGIN PV */
// size_t _write(int handle, const unsigned char * buffer, size_t size)
// {
//   // ITM 활성화 여부 확인
//   if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) &&
//       (ITM->TCR & ITM_TCR_ITMENA_Msk) &&
//       (ITM->TER & 1UL))
//   {
//     for(int i = 0; i < size; i++) {
//       ITM_SendChar(*buffer++);
//     }
//   }
//   return size;
// }


size_t _write(int handle, const unsigned char * buffer, size_t size)
{
  /* Sending in normal mode */
   for(int i=0; i<size; i++){
      ITM_SendChar(*buffer++);
   }
   return size;
/*
  if(HAL_OK == HAL_UART_Transmit(&huart1, (uint8_t *)buffer, size, 100000))
  {
    return size;
  }
  else
  {
    return -1;
  }
*/
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SDMMC1_SD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
__attribute__((section(".prpd_data_buffer"))) uint16_t prpd_data_buffer[NUM_CH][LINE_MAX][PHASE_MAX];
//test
uint32_t start_time = 0, end_time = 0, elapsed_time = 0;
uint8_t get_prpd_data_complete_flag = 1;
//test
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
#if 0
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
#endif
  MX_GPIO_Init();
  POW_CLT_V3P3C_ON();
  POW_CLT_V3P3D_ON();
  POW_CLT_V3P3E_ON();
  POW_CTL_V5P0A_ON();
  POW_CLT_V5P0C_ON();
  HAL_Delay(500);

  MX_FATFS_Init();
  MX_RTC_Init();
  // MX_SDMMC1_SD_Init();
  HAL_PWREx_EnableBatteryCharging(PWR_BATTERY_CHARGING_RESISTOR_1_5);

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  uint8_t sd_init_done_flg = 0;
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  /********** FOR TEST ***********/
  //버퍼 난수로 채움
  for(uint8_t i=0; i<NUM_CH; i++)
  {
    for(uint32_t j=0; j<LINE_MAX; j++)
    {
      for(uint32_t k=0; k<PHASE_MAX; k++)
      {
        prpd_data_buffer[i][j][k] = (uint16_t)(rand() % 10000);
      }
    }
  }

  // memset(&prpd_data_buffer[0][0][0], '1', sizeof(uint16_t) * LINE_MAX * PHASE_MAX);
  // memset(&prpd_data_buffer[1][0][0], '2', sizeof(uint16_t) * LINE_MAX * PHASE_MAX);
  // memset(&prpd_data_buffer[2][0][0], '3', sizeof(uint16_t) * LINE_MAX * PHASE_MAX);
  // memset(&prpd_data_buffer[3][0][0], '4', sizeof(uint16_t) * LINE_MAX * PHASE_MAX);

  static char temp_header[64] = {0};
  static uint16_t file_header_temp_buffer[32] = {0};
  /*******************************/
  const uint32_t TARGET_PERIOD = 10000; // 10초 (10000ms)
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  HAL_Delay(500);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  HAL_GPIO_WritePin(HALT_LED_GPIO_Port, HALT_LED_Pin, GPIO_PIN_RESET); // Halt LED ON

      //  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      // HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
      // make_header(temp_header, &sDate, &sTime);
      // memcpy(file_header_temp_buffer, temp_header, sizeof(temp_header));

  while (1)
  {
    if(get_prpd_data_complete_flag == 0) 
    {
      HAL_GPIO_WritePin(RUN_LED_GPIO_Port, RUN_LED_Pin, GPIO_PIN_SET);
      uint32_t delay_time = 0;
      if((end_time-start_time) < TARGET_PERIOD)
      {
          delay_time = TARGET_PERIOD - (end_time-start_time);
      }
      HAL_Delay(delay_time);
      start_time = HAL_GetTick();
    }
    get_prpd_data_complete_flag = 1; // 테스트용 prpd측정완료 플래그 강제 설정
    if(get_prpd_data_complete_flag == 1 || prpd_write_complete_flag == 1) // 측정완료 flag로 변경
    {
      if(SD_flg == 1)
      {
        /********** FOR TEST ***********/
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        make_header(temp_header, &sDate, &sTime);
        memcpy(file_header_temp_buffer, temp_header, sizeof(temp_header));
        HAL_GPIO_TogglePin(RUN_LED_GPIO_Port, RUN_LED_Pin);
        /*******************************/ 

        start_saving(file_header_temp_buffer, sizeof(file_header_temp_buffer),prpd_data_buffer, sizeof(prpd_data_buffer));
        end_time = HAL_GetTick();
//        printf("\nData saving time: %lu ms\n", end_time - start_time);
        elapsed_time += (end_time - start_time);
        start_time = end_time;
        // is_new_file를 초기화 함으로써 새파일에 저장할 준비
        if(prpd_write_complete_flag == 1)
        {
//          printf("\nelapsed_time: %lu ms\n", elapsed_time);
          printf("\nFile saved successfully\n\n");
          elapsed_time = 0;
          open_file_times = 0;
        }
      }
      // SD카드 연결이 끊긴 경우 재초기화
      else
      {
        printf("error\n");
        check_sd_card_present();

        // SD카드가 제거된 상태로 MX_SDMMC1_SD_Init()를 하게 되면 ERROR_HANDLER로 빠지므로 SD카드가 삽입된 상태에서 최초 1회 수행
        if(SD_flg == 1 && sd_init_done_flg == 0)
        {
          MX_SDMMC1_SD_Init();
          sd_init_done_flg = 1;
        }
        elapsed_time = 0;
        start_time = HAL_GetTick();
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  
  if (sDate.Year < 25)
  {
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 23;
  sTime.Minutes = 59;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_SET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 17;
  sDate.Year = 25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  }
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 10;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, SYNC_VIEW_SELECT_Pin|SYNC_LED_Pin|POW_CLT_V3P3E_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, SYNC_GAIN_D0_Pin|SYNC_GAIN_D1_Pin|SYNC_GAIN_D2_Pin|SYNC_MUX_SEL0_Pin
                          |SYNC_MUX_SEL1_Pin|SYNC_NOTCH_SELECT_Pin|POW_CTL_V5P0A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RMII_nRESET_GPIO_Port, RMII_nRESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RUN_LED_Pin|POW_CLT_V5P0C_Pin|PEAK_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, HALT_LED_Pin|POW_CLT_V3P3D_Pin|POW_CLT_V3P3C_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : SYNC_VIEW_SELECT_Pin SYNC_GAIN_D0_Pin SYNC_GAIN_D1_Pin SYNC_GAIN_D2_Pin
                           SYNC_MUX_SEL0_Pin SYNC_MUX_SEL1_Pin POW_CTL_V5P0A_Pin POW_CLT_V3P3E_Pin */
  GPIO_InitStruct.Pin = SYNC_VIEW_SELECT_Pin|SYNC_GAIN_D0_Pin|SYNC_GAIN_D1_Pin|SYNC_GAIN_D2_Pin
                          |SYNC_MUX_SEL0_Pin|SYNC_MUX_SEL1_Pin|POW_CTL_V5P0A_Pin|POW_CLT_V3P3E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_nINT_WAKE_UP2_Pin RMII_RXER_PHYAD0_Pin */
  GPIO_InitStruct.Pin = RMII_nINT_WAKE_UP2_Pin|RMII_RXER_PHYAD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : AC_POE_SYNC_Pin */
  GPIO_InitStruct.Pin = AC_POE_SYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(AC_POE_SYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_nRESET_Pin */
  GPIO_InitStruct.Pin = RMII_nRESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RMII_nRESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SYNC_NOTCH_SELECT_Pin */
  GPIO_InitStruct.Pin = SYNC_NOTCH_SELECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SYNC_NOTCH_SELECT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SYNC_LED_Pin */
  GPIO_InitStruct.Pin = SYNC_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SYNC_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SYNC_TTL_OUT_Pin */
  GPIO_InitStruct.Pin = SYNC_TTL_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SYNC_TTL_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RUN_LED_Pin */
  GPIO_InitStruct.Pin = RUN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RUN_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HALT_LED_Pin */
  GPIO_InitStruct.Pin = HALT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(HALT_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDMMC1_DET_Pin */
  GPIO_InitStruct.Pin = SDMMC1_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SDMMC1_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : POW_CLT_V3P3D_Pin POW_CLT_V5P0C_Pin POW_CLT_V3P3C_Pin */
  GPIO_InitStruct.Pin = POW_CLT_V3P3D_Pin|POW_CLT_V5P0C_Pin|POW_CLT_V3P3C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PEAK_RST_Pin */
  GPIO_InitStruct.Pin = PEAK_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(PEAK_RST_GPIO_Port, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(AC_POE_SYNC_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(AC_POE_SYNC_EXTI_IRQn);

  HAL_NVIC_SetPriority(SYNC_TTL_OUT_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(SYNC_TTL_OUT_EXTI_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// sprintf 없이 헤더 생성
void make_header(char *buf, RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  buf[0] = '$'; buf[1] = '$'; buf[2] = '$';
  buf[3] = '$'; buf[4] = '$'; buf[5] = '$';
  
  buf[6] = '0' + (date->Year / 10);
  buf[7] = '0' + (date->Year % 10);
  buf[8] = '0' + (date->Month / 10);
  buf[9] = '0' + (date->Month % 10);
  buf[10] = '0' + (date->Date / 10);
  buf[11] = '0' + (date->Date % 10);
  buf[12] = '0' + (time->Hours / 10);
  buf[13] = '0' + (time->Hours % 10);
  buf[14] = '0' + (time->Minutes / 10);
  buf[15] = '0' + (time->Minutes % 10);
  buf[16] = '0' + (time->Seconds / 10);
  buf[17] = '0' + (time->Seconds % 10);
  
  memcpy(&buf[18], "1234567890", 10);
  buf[28] = '\0';
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x00000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER5;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 전역 interrupt 비활성화
  __disable_irq();
  
  while (1)
  {
     printf("Error Handler\r\n");
     HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
