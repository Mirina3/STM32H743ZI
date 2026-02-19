/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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
#include<stdio.h>
#if 0
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
#endif
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */
// // RAM_D2에 배치 (0x30000000 영역)
// __attribute__((section(".sdio_buffer"))) FATFS SDFatFS;
// __attribute__((section(".sdio_buffer"))) FIL SDFile;

extern RTC_HandleTypeDef hrtc;
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* 1. 현재 RTC 시간과 날짜 읽기 
       (순서 중요: GetTime -> GetDate) */
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* 2. FatFs 포맷으로 변환하여 반환 */
    return (DWORD)(
        ((sDate.Year + 2000 - 1980) << 25) |  // 연도: 1980년 기준 (STM32 RTC는 보통 2000년 기준이므로 보정)
        (sDate.Month << 21) |                 // 월
        (sDate.Date << 16) |                  // 일
        (sTime.Hours << 11) |                 // 시
        (sTime.Minutes << 5) |                // 분
        (sTime.Seconds / 2)                   // 초 (2초 단위)
    );
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
