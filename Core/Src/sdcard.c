//sd카드에 200MB를 제외한 PRPD,PRPS를 저장할 수 있는 파일 수 각 12019개
//전체 용량 16,012,804,096
//200MB 제외한 용량 15,803,088,896
//PRPD 393,216 bytes
//PRPS 921,600 bytes

#include "main.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdcard.h"

// #define SD_MAKE_RESULT(func, fres)  (((func) << 8) | (fres))

extern FATFS SDFatFS;    /* File system object for SD logical drive */
extern FIL SDFile;       /* File object for SD */
extern char SDPath[4];   /* SD logical drive path */
FRESULT fres;
FILINFO fno;             /* File information object */
DIR dir;                 /* Directory object */

static uint16_t line_idx = 0;
uint32_t open_file_times = 0;
uint8_t SD_flg = 0;                   // SD가드가 존재하지 않거나 error가 발생한 경우 0으로 초기화
uint8_t prpd_write_complete_flag = 1; // prpd데이터를 끝까지 모두 쓴 경우(헤더를 써야함을 알려줌)

// typedef uint16_t SD_RESULT ;          // 삳위 8비트 함수ID 하위 8비트 FRESULT
// SD_RESULT result;

// // 함수 ID 정의
// typedef enum {
//     SD_OK           = 0x00,
//     SD_OPEN         = 0x01,
//     SD_CLOSE        = 0x02,
//     SD_WRITE        = 0x03,
//     SD_SYNC         = 0x04,
//     SD_OPEN_DIR     = 0x05,
//     SD_CLOSE_DIR    = 0x06,
//     SD_READ_DIR     = 0x07,
//     SD_MAKE_DIR     = 0x08,
//     SD_UNLINK       = 0x09,
//     SD_RENAME       = 0x0A,
//     SD_UPDATE_TIME  = 0x0B,
//     SD_GETFREE      = 0x0C,
//     SD_MOUNT        = 0x0D,
//     SD_WRITE_LEN    = 0x0E,
// } SD_FUNC_ID;

// uint16_t save_file_to_sdcard(uint16_t *header_info, uint32_t header_info_len, uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint32_t data_len)
// {
//   if(get_prpd_data_complete_flag == 1 || prpd_write_complete_flag == 1)
//   {
//     if(SD_flg == 1)
//     {
//       fres = save_file_to_sdcard(file_header_temp_buffer, sizeof(file_header_temp_buffer),prpd_data_buffer, sizeof(prpd_data_buffer));
//       if(prpd_write_complete_flag == 1) open_file_times = 0; // is_new_file를 초기화 함으로써 새파일에 저장할 준비
//     }
//     // SD카드 연결이 끊기거나 sd카드 관련 error가 발생한 경우 재초기화
//     else
//     {
//       printf("error\n");
//       check_sd_card_present();

//       // SD카드가 제거된 상태로 MX_SDMMC1_SD_Init()를 하게 되면 ERROR_HANDLER로 빠지므로 SD카드가 삽입된 상태에서 최초 1회 수행
//       if(SD_flg == 1 && sd_init_done_flg == 0)
//       {
//         MX_SDMMC1_SD_Init();
//         sd_init_done_flg = 1;
//       }
//       return result;
//     }
//   }
//   return 0;
// }

/***************************************************************************
 * @brief  process of Save a file to the SD card
 * @param  header_info: Pointer to header information
 * @param  header_info_len: Length of header information in bytes
 * @param  prpd_data: Pointer to PRPD data array
 * @param  data_len: Length of PRPD data in bytes
 * @retval FRESULT: FR_OK on success, otherwise error code
 */
void start_saving(uint16_t *header_info, uint32_t header_info_len, uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint32_t data_len)
{  
  static char filename[128] = {0};
  static char err_filename[128] = {0};
  // uint16_t PRPD_ZIP_buf[256*NUM_CH] = {0};
  // uint16_t PRPD_ZIP_TEMP_buf[256*NUM_CH] = {0};
  uint16_t PRPD_ZIP_buf[256*NUM_CH*2] = {0};
  uint16_t PRPD_ZIP_TEMP_buf[256*NUM_CH*2] = {0};
  uint16_t PRPD_ZIP_buf_len = 0;
  uint16_t PRPD_ZIP_TEMP_buf_len = 0;
  static uint8_t write_err_cnt = 0;

  if(open_file_times == 0)
  {
    fres = mount_sd_card();
    if(fres != FR_OK) goto ERROR_EXIT;
  
    // 남아있는 용량이 200MB 미만인지 확인
    if(SD_GetCapacity() < REMAIN_CAPACITY && fres == FR_OK)
    {
      if(fres != FR_OK) goto ERROR_EXIT;
      // 용량 부족
      printf("Not enough space on SD card\n");
      fres = DeleteOldestFolder();
      // 정해진 파일 수량 삭제 실패
      if(fres != FR_OK) goto ERROR_EXIT;
    }

    /******************************************** OPEN FILE *******************************************/
    fres = SD_OpenFile(filename, header_info);
    if(fres != FR_OK) goto ERROR_EXIT;
    }
    open_file_times ++;

    /*************************************** WRITE HEADER DATA *****************************************/
    if(open_file_times == 1)
    {
      fres = SD_WriteData(header_info, header_info_len, filename);
      if(fres == FR_OK)
      {
        prpd_write_complete_flag = 0;
      }
    }

    // else if(open_file_times > 1)
    // {
    //   /**************************************** DATA COMPRESSION ***************************************/
    //   PRPD_ZIP_TEMP_buf_len = make_prpd_zip_temp_buf(prpd_data, PRPD_ZIP_TEMP_buf, line_idx);
    //   PRPD_ZIP_buf_len = Zip_PRPD(PRPD_ZIP_buf, PRPD_ZIP_TEMP_buf, (PRPD_ZIP_TEMP_buf_len * 2));

    //   /********************************** WRITE PRPD DATA **********************************************/
    //   fres = SD_WriteData(PRPD_ZIP_buf, PRPD_ZIP_buf_len * 2, filename);
    //   if(fres == FR_OK)
    //   {
    //     line_idx++;
    //     if(line_idx == LINE_MAX)
    //     {
    //       line_idx = 0;
    //       get_prpd_data_complete_flag = 0;
    //       prpd_write_complete_flag = 1; // prpd 데이터를 모두 쓴 경우
    //     }
    //   }
    // }

    else if(open_file_times > 1)
    {
      for(uint8_t time = 0; time < WRITE_TIME; time++) // 한번에 2라인씩 저장
      {
        /**************************************** DATA COMPRESSION ***************************************/
        PRPD_ZIP_TEMP_buf_len = make_prpd_zip_temp_buf(prpd_data, PRPD_ZIP_TEMP_buf, line_idx);
        PRPD_ZIP_buf_len = Zip_PRPD(PRPD_ZIP_buf, PRPD_ZIP_TEMP_buf, (PRPD_ZIP_TEMP_buf_len * 2));

        /********************************** WRITE PRPD DATA **********************************************/
        fres = SD_WriteData(PRPD_ZIP_buf, PRPD_ZIP_buf_len * 2, filename);
        if(fres == FR_OK)
        {
          line_idx++;
          if(line_idx == LINE_MAX)
          {
            line_idx = 0;
            get_prpd_data_complete_flag = 0;
            prpd_write_complete_flag = 1; // prpd 데이터를 모두 쓴 경우
            break;
          }
        }
      }
    }

    //WRITE ERROR PROCESSING
    if(fres != FR_OK)
    {
      write_err_cnt++;      

      // error가 1회 발생한 경우 기존파일명_ERR로 변경
      if(write_err_cnt == 1)
      {
        fres = f_close(&SDFile); // 파일 닫기
        if(fres != FR_OK) 
        {
          printf("Failed to close file after write error, error: %d\n", fres);
          // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE, fres);
          goto ERROR_EXIT;
        }

        snprintf(err_filename, sizeof(err_filename), "%.55s_ERR.txt", filename);

        fres = f_rename(filename, err_filename);
        if(fres != FR_OK) 
        {
          printf("Failed to rename file after write error, error: %d\n", fres);
          // SD_RESULT result = SD_MAKE_RESULT(SD_RENAME, fres);
          goto ERROR_EXIT;
        }
      }

      // error가 2회 또는 3회 발생한 경우 기존 ERR파일을 지우고 새 ERR파일로 변경
      else if(write_err_cnt == 2 || write_err_cnt == 3)
      {
        fres = f_close(&SDFile); // 파일 닫기
        if(fres != FR_OK) 
        {
          printf("Failed to close file after write error, error: %d\n", fres);
          // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE, fres);
          goto ERROR_EXIT;
        }

        snprintf(err_filename, sizeof(err_filename), "%.55s_ERR.txt", filename);

        fres = f_unlink(err_filename); // 기존에 동일 이름의 ERR 파일이 존재하면 삭제
        if(fres != FR_OK) 
        {
          printf("Failed to delete existing ERR file after write error, error: %d\n", fres);
          // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, fres);
          goto ERROR_EXIT;
        }

        fres = f_rename(filename, err_filename);
        if(fres != FR_OK) 
        {
          printf("Failed to rename file after write error, error: %d\n", fres);
          // SD_RESULT result = SD_MAKE_RESULT(SD_RENAME, fres);
          goto ERROR_EXIT;
        }
        
        if(write_err_cnt == 3)
        {
          write_err_cnt = 0;
          SD_flg = 0; // 3회 연속 error 발생 시 SD관련 재초기화
        }
      }
      line_idx = 0;
      open_file_times = 0; // 새 파일 생성
      return;
    }

    else
    {
      // 파일 시간 갱신
      fres = f_utime(filename, &fno);
      if (fres != FR_OK)
      {
        printf("Failed to update file time, error: %d\n", fres);
        // SD_RESULT result = SD_MAKE_RESULT(SD_UPDATE_TIME, fres);
        goto ERROR_EXIT;
      }
    }
    // fres = f_sync(&SDFile); // 데이터 플러시
    // if (fres != FR_OK)
    // {
    //   // 동기화 실패
    //   printf("Failed to sync file, error: %d\n", fres);
    //   SD_flg = 0;
    //   return fres;
    // }

    /********** CLOSE FILE **********/
    if(prpd_write_complete_flag == 1)
    {
      fres = f_close(&SDFile);
      if (fres != FR_OK)
      {
        // 파일 닫기 실패
        printf("Failed to close file, error: %d\n", fres);
        // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE, fres);
        goto ERROR_EXIT;
      }
    }
    return;

    ERROR_EXIT:
    SD_flg = 0;
    return;
}


/***************************************************************************
 * @brief  Check if SD card is present
 * @param  None
 * @retval 1: Present, 0: Not Present
 */
uint8_t check_sd_card_present(void)
{
  GPIO_PinState card_detect;
  
  SD_ForceReset();

  // SDMMC1_DET 핀 읽기
  card_detect = HAL_GPIO_ReadPin(SDMMC1_DET_GPIO_Port, SDMMC1_DET_Pin);
  
  /*
   * SD 카드 감지 핀은 보통:
   * - LOW (0) = 카드 삽입됨
   * - HIGH (1) = 카드 없음
   */
  
  if (card_detect == GPIO_PIN_RESET)  // LOW
  {
    //printf("SD card detected\n");
    SD_flg = 1;
    return 1;  // 카드 있음
  }
  else
  {
    //printf("SD card NOT detected\n");
    SD_flg = 0;
    return 0;  // 카드 없음
  }
}

/***************************************************************************
 * @brief  SD card mount function
 * @param  None
 * @retval 
 */
FRESULT mount_sd_card(void)
{
  FRESULT res;

  // 파일 시스템 마운트
  res = f_mount(&SDFatFS, SDPath, 1);  // 마지막 인자를 1로 변경 (즉시 마운트)

  if (res != FR_OK)
  {
    // 마운트 실패
    printf("Failed to mount filesystem, error: %d\n", fres);
    // SD_RESULT result = SD_MAKE_RESULT(SD_MOUNT, fres);
    return res;
  }

  return res;  // 마운트 성공
}


/***************************************************************************
 * @brief  SD 카드에 파일 열기 (새 파일 생성 또는 기존 파일 이어쓰기)
 * @param  filename: 파일 이름 버퍼
 * @param  header_info: 헤더 정보 (새 파일 생성 시 파일명 생성에 사용)
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT SD_OpenFile(char *filename, uint16_t *header_info)
{
  FRESULT res;

  //PRPD 데이터를 끝까지 쓴 경우
  // if (open_file_times == 0)
  // {
    // 새 파일 생성
    res = GetFilename_CreateFolders(filename, header_info);
    if(res == FR_OK || res == FR_EXIST)
      {
        // res = f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
        res = f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE | FA_WRITE);
          if (res != FR_OK)
          {
            printf("Failed to open file, error: %d\n", res);
            // SD_RESULT result = SD_MAKE_RESULT(SD_OPEN, res);
            return res;
          }
      }
    else return res;
  // }
  
  // else
  // {
  //   // 기존 파일 이어쓰기
  //   res = f_open(&SDFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
  //   if (res == FR_OK)
  //   {
  //     res = f_lseek(&SDFile, f_size(&SDFile));
  //     if (res != FR_OK)
  //     {
  //       // 파일 위치 이동 실패
  //       printf("Failed to seek to end of file, error: %d\n", res);
  //       SD_flg = 0;
  //       return res;
  //     }
  //   }
  // }
  return res;
}

/***************************************************************************
 * @brief  SD 카드에 데이터 쓰기 및 파일 시간 갱신
 * @param  data: 쓸 데이터 버퍼
 * @param  data_size: 쓸 데이터 크기
 * @param  filename: 파일 이름 (시간 갱신에 사용)
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT SD_WriteData(const uint16_t *data,uint16_t data_size, const char *filename)
{
  FRESULT res;
  UINT byteswritten;

  // 데이터 쓰기
  res = f_write(&SDFile, data, data_size, &byteswritten);

  // 쓰기 실패 시 오류 처리
  if (res != FR_OK)
  {
    printf("Failed to write to file, error: %d\n", res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_WRITE, res);
    return res;
  }
  // 작성된 바이트수가 요청한 크기와 다를 경우 오류 처리
  else if(byteswritten < data_size)
  {
    printf("Incomplete write: %u of %d bytes written\n", byteswritten, data_size);
    // SD_RESULT result = SD_MAKE_RESULT(SD_WRITE_LEN, FR_DISK_ERR);
    return FR_DISK_ERR;
  }
  return res;
}


/***************************************************************************
 * @brief  SD 카드 용량을 계산하는 함수
 * @param  None
 * @retval SD 카드의 남은 용량 (MB 단위)
 */
uint32_t SD_GetCapacity(void)
{
  FATFS *fs;
  DWORD fre_clust, fre_sect;
  // DWORD tot_sect;

    //SD카드 용량 계산
    fres = f_getfree(SDPath, &fre_clust, &fs);

    if (fres == FR_OK)
    {
      // 전체 sector 수 = (전체 FAT 엔트리 수 - 2) * cluster 당 sector 수
      // tot_sect = (fs->n_fatent - 2) * fs->csize;
      // 빈 sector 수 = 빈 cluster 수 * 한 cluster 당 sector 수
      fre_sect = fre_clust * fs->csize;

      // uint64_t total_bytes = (uint64_t)tot_sect * 512;
      uint64_t free_bytes = (uint64_t)fre_sect * 512;
      // uint64_t used_bytes = total_bytes - free_bytes;

      // uint32_t total_Mb = (uint32_t)(total_bytes / (1024 * 1024));
      uint32_t free_Mb = (uint32_t)(free_bytes / (1024 * 1024));
      // uint32_t used_Mb = (uint32_t)(used_bytes / (1024 * 1024));

      printf("SD card remaining capacity: %lu MB\n", free_Mb);

      return free_Mb;
    }
    else
    {
      printf("f_getfree error: %d\n", fres);
      // SD_RESULT result = SD_MAKE_RESULT(SD_GETFREE, fres);
      SD_flg = 0;

      return 0;
    }
}

/***************************************************************************
 * @brief  파일 이름에서 타임스탬프를 추출하는 함수
 * @param  filename: 파일 이름 문자열
 * @retval 추출된 타임스탬프 (uint32_t 형식)
 */
uint64_t ExtractTimestamp(const char* filename)
{
  uint64_t timestamp = 0;
  char temp[13] = {0};
  uint8_t idx = 0;
    
  // 숫자만 추출
  for (int i = 0; filename[i] != '\0'; i++)
  {
    if (filename[i] >= '0' && filename[i] <= '9')
    {
      temp[idx++] = filename[i];
    }
  }
  temp[idx] = '\0';
    
  // 문자열을 숫자로 변환 atoi는 32비트까지만 지원 하므로 strtoull 사용
  timestamp = strtoull(temp, NULL, 10);
    
  return timestamp;
}



/***************************************************************************
 * @brief  파일 이름 생성 함수
 * @param  filename: 파일 이름을 저장할 버퍼
 * @param  header_info: 헤더 정보
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT GetFilename_CreateFolders(char* filename, uint16_t* header_info)
{
  char year[3] = {0};
  char month[3] = {0};
  char day[3] = {0};
  char hour[3] = {0};
  char date[7] = {0};
  char time[7] = {0};

  strncpy(year,(char*)header_info + 6, 2);
  strncpy(month,(char*)header_info + 8, 2);
  strncpy(day,(char*)header_info + 10, 2);
  strncpy(hour,(char*)header_info + 12, 2);
  strncpy(date,(char*)header_info + 6, 6);
  strncpy(time,(char*)header_info + 12, 6);

  sprintf(filename, "20%s/20%s-%s/20%s-%s-%s/20%s-%s-%s_%s/CMS_%s_%s.txt", year
                                                                         , year, month
                                                                         , year, month, day
                                                                         , year, month, day, hour
                                                                         , date, time);

  FRESULT res = CreateFolders(year, month, day, hour);

  if (res != FR_OK && res != FR_EXIST)
    {
        printf("Failed to create folders: %d\n", res);
        SD_flg = 0;
        return res;
    }

  return res;
}


/***************************************************************************
 * @brief  년/월/일 폴더를 생성하는 함수
 * @param  year: 년도 (2자리, 예: 25)
 * @param  month: 월 (2자리, 예: 01)
 * @param  day: 일 (2자리, 예: 12)
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT CreateFolders(char* year, char* month, char* day, char* hour)
{
  FRESULT res;
  char path[55] = {0};
    
  // 년 폴더 생성 (예: "20xx/")
  sprintf(path, "20%s", year);
  res = f_mkdir(path);
  if (res != FR_OK && res != FR_EXIST)
  {
    printf("Failed to create year folder: %d\n", res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_MAKE_DIR, res);
    return res;
  }
    
  // 월 폴더 생성 (예: "20xx/xx/")
  sprintf(path, "20%s/20%s-%s", year
                              , year, month);
  res = f_mkdir(path);
  if (res != FR_OK && res != FR_EXIST)
  {
    printf("Failed to create month folder: %d\n", res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_MAKE_DIR, res);
    return res;
  }
    
  // 일 폴더 생성 (예: "20xx/xx/xx/")
  sprintf(path, "20%s/20%s-%s/20%s-%s-%s", year
                                         , year, month
                                         , year, month, day);
  res = f_mkdir(path);
  if (res != FR_OK && res != FR_EXIST)
  {
    printf("Failed to create day folder: %d\n", res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_MAKE_DIR, res);
    return res;
  }

  // 시간 폴더 생성 (예: "20xx/xx/xx/xx")
  sprintf(path, "20%s/20%s-%s/20%s-%s-%s/20%s-%s-%s_%s", year
                                                       , year, month
                                                       , year, month, day
                                                       , year, month, day, hour);
  res = f_mkdir(path);
  if (res != FR_OK && res != FR_EXIST)
  {
    printf("Failed to create day folder: %d\n", res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_MAKE_DIR, res);
    return res;
  }

  // // 분 폴더 생성 (예: "20xx/xx/xx/xx")
  // sprintf(path, "20%s/20%s-%s/20%s-%s-%s/20%s-%s-%s_%s/20%s-%s-%s_%s-%s", year
  //                                                                       , year, month
  //                                                                       , year, month, day
  //                                                                       , year, month, day, hour
  //                                                                       , year, month, day, hour, min);
  // res = f_mkdir(path);
  // if (res != FR_OK && res != FR_EXIST)
  // {
  //   printf("Failed to create day folder: %d\n", res);
  //   return res;
  // }
    
  return res;
}


/***************************************************************************
 * @brief  가장 오래된 시간 폴더 삭제 (메인 삭제 함수)
 * @param  None
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 * 
 * 폴더 구조: 년/월/일/시간/파일.txt
 * 예: 20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx/20xx-xx-xx_xx-xx/CMS_260101_010101.txt
 */
FRESULT DeleteOldestFolder(void)
{
  FRESULT res;
  char yearPath[5] = {0};       // "20xx" 4
  char monthPath[13] = {0};     // "20xx/20xx-xx" 12
  char dayPath[24] = {0};       // "20xx/20xx-xx/20xx-xx-xx" 23
  char hourPath[38] = {0};      // "20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx" 37
  // char minPath[55] = {0};       // "20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx/20xx-xx-xx_xx-xx" 
  char tmpPath[128] = {0};      // 임시 path버퍼(snprint경고 해결을 위한)
  char oldestName[22] = {0};    // 가장 오래된 폴더 이름을 저장할 버퍼

  printf("\n=== Finding oldest hour folder ===\n");

  // 1. 가장 오래된 년 폴더 찾기
  res = FindOldestSubfolder(SDPath, oldestName);
  if (res == FR_OK && oldestName[0] == '\0')
  {
    printf("No year folders found\n");
    return res;
  }
  else if (res != FR_OK)
  {
    printf("Error finding oldest year folder: %d\n", res);
    return res;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s", oldestName);
  strcpy(yearPath, tmpPath);
  //printf("Oldest year: %s\n", yearPath);

  // 2. 가장 오래된 월 폴더 찾기
  res = FindOldestSubfolder(yearPath, oldestName);
  if (res == FR_OK && oldestName[0] == '\0')
  {
    printf("No month folders found in %s\n", yearPath);
    res = f_unlink(yearPath);
    if(res != FR_OK)
    {
      printf("Failed to delete empty year folder: %d\n", res);
      // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
      return res;
    }
    return res;
  }
  else if (res != FR_OK)
  {
    printf("Error finding oldest month folder: %d\n", res);
    return res;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", yearPath, oldestName);
  strcpy(monthPath, tmpPath);
  //printf("Oldest month: %s\n", monthPath);

  // 3. 가장 오래된 일 폴더 찾기
  res = FindOldestSubfolder(monthPath, oldestName);
  if (res == FR_OK && oldestName[0] == '\0')
  {
    printf("No day folders found in %s\n", monthPath);
    res = f_unlink(monthPath);
    if(res != FR_OK)
    {
      printf("Failed to delete empty month folder: %d\n", res);
      // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
      return res;
    }
    if (CountItemsInFolder(yearPath) == 0)
    {
      if(fres != FR_OK)
      {
        printf("Failed to count items in folder: %d\n", fres);
        return fres;
      }
      res = f_unlink(yearPath);
      if(res != FR_OK)
      {
        printf("Failed to delete empty year folder: %d\n", res);
        // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
        return res;
      }
    }
    return res;
  }
  else if (res != FR_OK)
  {
    printf("Error finding oldest day folder: %d\n", res);
    return res;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", monthPath, oldestName);
  strcpy(dayPath, tmpPath);
  //printf("Oldest day: %s\n", dayPath);

  // 4. 가장 오래된 시간 폴더 찾기
  res = FindOldestSubfolder(dayPath, oldestName);
  if (res == FR_OK && oldestName[0] == '\0')
  {
    printf("No min folders found in %s\n", dayPath);
    res = CleanupEmptyParentFolders(dayPath, monthPath, yearPath);
    return res;
  }
  else if( res != FR_OK)
  {
    printf("Error finding oldest hour folder: %d\n", res);
    return res;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", dayPath, oldestName);
  strcpy(hourPath, tmpPath);
  printf("Oldest min folder to delete: %s\n", hourPath);

  // 5. 시간 폴더와 내부 파일 모두 삭제
  res = DeleteFolder(hourPath);
  if (res != FR_OK)
  {
    printf("Failed to delete hour folder: %d\n", res);
    return res;
  }

  // 6. 빈 상위 폴더들 정리
  res = CleanupEmptyParentFolders(dayPath, monthPath, yearPath);
  if(res == FR_OK)
  {
    printf("=== Cleanup complete ===\n\n");
  }
  return res;
}


/***************************************************************************
 * @brief  지정된 경로에서 가장 오래된 폴더 이름 찾기
 * @param  basePath: 탐색할 기준 경로
 * @param  oldestName: 가장 오래된 폴더 이름 저장 버퍼
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT FindOldestSubfolder(const char *basePath, char *oldestName)
{
  FRESULT res;
  DIR tmpDir;
  FILINFO tmpFno;
  uint64_t oldestTimestamp = UINT64_MAX;
  uint64_t currentTimestamp;

  oldestName[0] = '\0';

  // basePath경로의 디렉터리 정보를 tmpDir에 저장
  res = f_opendir(&tmpDir, basePath);
  if (res != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_OPEN_DIR, res);
    return res;
  }

  while (1)
  {
    // 디렉터리 엔트리를 1개 읽어 tmpFno에 채움
    res = f_readdir(&tmpDir, &tmpFno);

    // 오류가 발생
    if (res != FR_OK)
    {
      // SD_RESULT result = SD_MAKE_RESULT(SD_READ_DIR, res);
      f_closedir(&tmpDir);
      return res;
    }

    // 더 이상 읽을 항목이 없는 경우
    else if(tmpFno.fname[0] == '\0')
    {
      res = FR_OK;
      break;
    }

    // 폴더가 아니거나, FAT파일시스템의 특수 엔트리일 시((tmpFno.fattrib & (AM_HID | AM_SYS))) 다음 f_readdir로 이동
    if ((!(tmpFno.fattrib & AM_DIR)) || (tmpFno.fattrib & (AM_HID | AM_SYS)))
    {
      continue;
    }

    // 폴더 이름에서 타임스탬프 추출
    currentTimestamp = ExtractTimestamp(tmpFno.fname);

    // 숫자가 작을 수록 더 오래된 것
    if (currentTimestamp < oldestTimestamp)
    {
      oldestTimestamp = currentTimestamp;
      strcpy(oldestName, tmpFno.fname);
    }
  }

  // 읽을 dir이 더 없는 경우는 오류처리하지 않음
  res = f_closedir(&tmpDir);
  if( res != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE_DIR, res);
    return res;
  }

  return res;
}


/***************************************************************************
 * @brief  폴더와 내부 모든 파일 삭제 (재귀)
 * @param  path: 삭제할 폴더 경로
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT DeleteFolder(const char *path)
{
  FRESULT res;
  DIR tmpDir;
  FILINFO tmpFno;
  char itemPath[64];
  char tmpitemPath[320];

  // path에 해당하는 디렉터리를 열고 시작위치를 tmpDir에 설정
  res = f_opendir(&tmpDir, path);
  if (res != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_OPEN_DIR, res);
    return res;
  }

  while (1)
  {
    //tmpDir을 순회하면서 tmpFno에 정보 저장
    res = f_readdir(&tmpDir, &tmpFno);
    if (res == FR_OK && tmpFno.fname[0] == 0)
    {
      break;
    }
    else if (res != FR_OK)
    {
      // SD_RESULT result = SD_MAKE_RESULT(SD_READ_DIR, res);
      return res;
    }

    snprintf(tmpitemPath, sizeof(tmpitemPath), "%s/%s", path, tmpFno.fname);
    strcpy(itemPath ,tmpitemPath);

    if (tmpFno.fattrib & AM_DIR)
    {
      // 하위 폴더 재귀 삭제
      res = DeleteFolder(itemPath);
      if (res != FR_OK)
      {
        return res;
      }
    }
    else
    {
      // 파일 삭제
      res = f_unlink(itemPath);
      if (res != FR_OK)
      {
        // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
        printf("Failed to delete file: %s, error: %d\n", itemPath, res);        
        return res;
      }
    }
  }

  f_closedir(&tmpDir);
  if( res != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE_DIR, res);
    return res;
  }

  // 빈 폴더 삭제
  res = f_unlink(path);
  if (res != FR_OK)
  {
    printf("Failed to delete folder: %s, error: %d\n", path, res);
    // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
  }
  else
  {
    printf("Deleted folder: %s\n", path);
  }

  return res;
}


/***************************************************************************
 * @brief  빈 상위 폴더들 정리 (시간→일→월→년 순서로)
 * @param  dayPath: 일 폴더 경로
 * @param  monthPath: 월 폴더 경로
 * @param  yearPath: 년 폴더 경로
 * @retval None
 */
FRESULT CleanupEmptyParentFolders(const char *dayPath, const char *monthPath, const char *yearPath)
{
  FRESULT res;
  // 일 폴더가 비었는지 확인 후 삭제
  if(CountItemsInFolder(dayPath) == 0)
  {
    res = f_unlink(dayPath);
    if(res == FR_OK)
    {
      printf("Deleted empty day folder: %s\n", dayPath);
    }
    else
    {
      printf("Failed to delete empty day folder: %s, error: %d\n", dayPath, res);
      // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
      return res;
    }
    // 월 폴더가 비었는지 확인 후 삭제
    if(CountItemsInFolder(monthPath) == 0)
    {
      res = f_unlink(monthPath);
      if(res == FR_OK)
      {
        printf("Deleted empty month folder: %s\n", monthPath);
      }
      else
      {
        printf("Failed to delete empty month folder: %s, error: %d\n", monthPath, res);
        // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
        return res;
      }
      // 년 폴더가 비었는지 확인 후 삭제
      if(CountItemsInFolder(yearPath) == 0)
      {
        res = f_unlink(yearPath);
        if(res == FR_OK)
        {
          printf("Deleted empty year folder: %s\n", yearPath);
        }
        else
        {
          printf("Failed to delete empty year folder: %s, error: %d\n", yearPath, res);
          // SD_RESULT result = SD_MAKE_RESULT(SD_UNLINK, res);
          return res;
        }
      }
      else if(CountItemsInFolder(yearPath) == 0xFFFF)
      {
        return fres;
      }
      else return FR_OK; // 빈 폴더가 아닐 시
    }
    else if(CountItemsInFolder(monthPath) == 0xFFFF)
    {
      return fres;
    }
    else return FR_OK; // 빈 폴더가 아닐 시
  }
  else if(CountItemsInFolder(dayPath) == 0xFFFF)
  {
    return fres;
  }
  else return FR_OK; // 빈 폴더가 아닐 시
  return res;
}


/***************************************************************************
 * @brief  폴더 내 항목(파일/폴더) 개수 확인
 * @param  path: 확인할 폴더 경로
 * @retval 항목 개수 (오류 시 0)
 */
uint16_t CountItemsInFolder(const char *path)
{
  DIR tmpDir;
  FILINFO tmpFno;
  uint16_t count = 0;

  fres = f_opendir(&tmpDir, path);
  if (fres != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_OPEN_DIR, fres);
    return 0xFFFF; // Return error value on failure
  }
  
  while (fres = f_readdir(&tmpDir, &tmpFno), fres == FR_OK && tmpFno.fname[0] != 0)
  {
    count++;
  }
  if(fres != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_READ_DIR, fres);
    f_closedir(&tmpDir);
    return 0xFFFF; // Return error value on failure
  }

  fres = f_closedir(&tmpDir);
  if (fres != FR_OK)
  {
    // SD_RESULT result = SD_MAKE_RESULT(SD_CLOSE_DIR, fres);
    return 0xFFFF; // Return error value on failure
  }

  return count;
}

/***************************************************************************
 * @brief PRPD 데이터의 같은 행을 하나의 버퍼에 이어붙이는 함수
 * @param prpd_data: PRPD 데이터 배열
 * @param PRPD_ZIP_TEMP_buf: 이어붙인 데이터를 저장할 버퍼
 * @param row: 이어붙일 행 인덱스
 * @retval None
 */
uint16_t make_prpd_zip_temp_buf(uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint16_t *PRPD_ZIP_TEMP_buf, uint8_t row)
{
  uint16_t PRPD_ZIP_TEMP_buf_idx = 0;

  // 각각의 채널의 같은 행을 하나의 버퍼에 이어붙임
  for(uint8_t i = 0; i < NUM_CH; i++)
  {
    memcpy(&PRPD_ZIP_TEMP_buf[PRPD_ZIP_TEMP_buf_idx], &prpd_data[i][row][0], sizeof(uint16_t) * PHASE_MAX);
    PRPD_ZIP_TEMP_buf_idx += PHASE_MAX;
  }

  return PRPD_ZIP_TEMP_buf_idx;
}


/***************************************************************************
  * @brief  PRPD 데이터 압축 함수
  * @param  pZipBuf   : 압축된 데이터가 저장될 버퍼
  * @param  pRawData  : 원본 PRPD 데이터
  * @param  data_bytes: 원본 데이터의 길이 (Byte 단위)
  * @retval 압축된 데이터의 길이 (Word 단위, uint16_t 개수)
  */
uint32_t Zip_PRPD(uint16_t *pZipBuf, uint16_t *pRawData, uint32_t data_bytes)
{
  if (data_bytes == 0) return 0; // 예외 처리

  uint32_t num_idx = data_bytes / 2;      // 전체 인덱스 개수 (Word)
  uint32_t zip_idx = 0;                   // 압축 버퍼 인덱스
  
  uint16_t current_val = pRawData[0];     // 현재 비교 중인 값
  uint32_t dup_count = 1;                 // 중복 횟수 (1부터 시작)

  // 1번째 인덱스부터 끝까지 순회
  for (uint32_t i = 1; i < num_idx; i++)
  {
    if (pRawData[i] == current_val)
    {
      // 값이 같으면 카운트 증가
      dup_count++;
      // 중복 횟수가 한계치(0xFFF)에 도달하면 강제로 잘라서 저장
      if (dup_count >= 0xFFF)
      {
        pZipBuf[zip_idx++] = dup_count | 0x8000; // 최대치 기록
        pZipBuf[zip_idx++] = current_val;
        
        dup_count = 0; // 카운트 리셋 (다음 루프부터 1이 됨 or 값이 바뀌면 1로 셋팅)
      }
    }
    else
    {
      // 값이 달라지면 지금까지 모은 데이터 기록
      // ----------------------------------------------------
      if (dup_count == 1)
      {
        pZipBuf[zip_idx++] = current_val;
      }
      else if (dup_count == 2)
      {
        pZipBuf[zip_idx++] = current_val;
        pZipBuf[zip_idx++] = current_val;
      }
      else // 3개 이상일 때 압축
      {
        // 비트 15를 1로 세팅하여 압축됨을 표시 (프로토콜에 맞게 dup_count 사용)
        pZipBuf[zip_idx++] = dup_count | 0x8000; 
        pZipBuf[zip_idx++] = current_val;
      }
      // ----------------------------------------------------

      // 새로운 값으로 리셋
      current_val = pRawData[i];
      dup_count = 1;
    }
  }

  // for문 종료 후 남은 마지막 데이터 묶음 처리
  if (dup_count == 1)
  {
    pZipBuf[zip_idx++] = current_val;
  }
  else if (dup_count == 2)
  {
    pZipBuf[zip_idx++] = current_val;
    pZipBuf[zip_idx++] = current_val;
  }
  else // 3개 이상일 때 압축
  {
    pZipBuf[zip_idx++] = dup_count | 0x8000;
    pZipBuf[zip_idx++] = current_val;
  }

  return zip_idx; // 압축된 배열의 길이 반환
}

/***************************************************************************
 * @brief  sd reset function
 * @param  None
 * @retval None
 */
void SD_ForceReset(void)
{
  line_idx = 0;                     // PRPD 라인 인덱스 초기화
  open_file_times = 0;              // 파일 오픈 시도 횟수 초기화
  prpd_write_complete_flag = 1;     // PRPD 데이터 쓰기 완료 플래그 설정
  get_prpd_data_complete_flag = 0;  // PRPD 데이터 측정 다시 시작

  //드라이버 언마운트 및 재연결
  f_mount(NULL, SDPath, 0);               // 언마운트
  FATFS_UnLinkDriver(SDPath);             // 드라이버 해제
  FATFS_LinkDriver(&SD_Driver, SDPath);   // 드라이버 재연결 → 플래그 자동 리셋

  HAL_SD_DeInit(&hsd1);       // 하드웨어 해제

  //SDMMC 하드웨어 블록 강제 리셋
  //SDMMC 주변장치의 전원을 껐다 켜는 것과 같은 효과 (레지스터 초기화)
  __HAL_RCC_SDMMC1_FORCE_RESET();
  HAL_Delay(10); //  리셋될 시간 부여
  __HAL_RCC_SDMMC1_RELEASE_RESET();

  HAL_Delay(100);

  return;
}
