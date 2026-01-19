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

extern FATFS SDFatFS;  /* File system object for SD logical drive */
extern FIL SDFile;    /* File object for SD */
extern char SDPath[4]; /* SD logical drive path */
FRESULT fres;
FILINFO fno;         /* File information object */
DIR dir;           /* Directory object */

uint32_t sd_write_buffer_head = 0; //버퍼의 head 위치
uint16_t is_new_file = 0;
uint8_t bf_sdflag = 0; // SD가드가 존재하지 않거나 error가 발생한 경우 0으로 초기화
uint8_t prpd_write_complete_flag = 1; // prpd데이터를 끝까지 모두 쓴 경우(헤더를 써야함을 알려줌)

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
    // printf("SD card detected\n");
    bf_sdflag = 1;
    return 1;  // 카드 있음
  }
  else
  {
    // printf("SD card NOT detected\n");
    bf_sdflag = 0;
    return 0;  // 카드 없음
  }
}

/***************************************************************************
 * @brief  SD card mount function
 * @param  None
 * @retval 1: Success, 0: Fail
 */
uint8_t mount_sd_card(void)
{
  DSTATUS d_status;

  // 파일 시스템 마운트
  fres = f_mount(&SDFatFS, SDPath, 1);  // 마지막 인자를 1로 변경 (즉시 마운트)
  // printf("mount result = %d\n", fres);
  if (fres != FR_OK)
  {
    // 마운트 실패
    printf("Failed to mount filesystem, error: %d\n", fres);

    return 0;
  }

  // 디스크 상태 확인
  d_status = disk_status(0);
  if (d_status != RES_OK)
  {
    // 디스크 초기화 실패
    printf("Disk not ready, status: 0x%02X\n", d_status);

    return 0;
  }

  return 1;  // 마운트 성공
}

/***************************************************************************
 * @brief  Save a file to the SD card
 * @param  filename: Name of the file to be saved
 * @retval None
 */
void save_file_to_sdcard(uint16_t *header_info, uint32_t header_info_len, uint16_t *prpd_data, uint32_t data_len, char *filename)
{  
  if(mount_sd_card() == 1)
  {
    // 남아있는 용량이 200MB 미만인지 확인
    while(SD_GetCapacity() < 200 && fres == FR_OK)
    {
      // 용량 부족
      printf("Not enough space on SD card\n");
      DeleteOldestMinFolder();
      // 정해진 파일 수량 삭제 실패
      if(fres != FR_OK)
      {
        bf_sdflag = 0;
        return;
      }
    }

    /*********** OPEN FILE **********/
    fres = SD_OpenFile(filename, header_info);
    if(fres != FR_OK) return;
    else is_new_file ++;

    /********** WRITE DATA ***********/
    //파일 처음 open 시 헤더 정보 작성
    if(prpd_write_complete_flag == 1)
    {
      fres = SD_WriteData(header_info, header_info_len, filename);
      prpd_write_complete_flag = 0;
    }
    //헤더 정보 모두 작성 시 prpd 데이터 작성
    else 
    {
      fres = SD_WriteData(prpd_data, WRITEBYTE, filename);
    }
    /********** CLOSE FILE **********/
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
      // 파일 닫기 실패
      printf("Failed to close file, error: %d\n", fres);
      bf_sdflag = 0;
      return;
    }
    return;
  }
  bf_sdflag = 0;
  return;
}

/***************************************************************************
 * @brief  SD 카드에 파일 열기 (새 파일 생성 또는 기존 파일 이어쓰기)
 * @param  filename: 파일 이름 버퍼
 * @param  header_info: 헤더 정보 (새 파일 생성 시 파일명 생성에 사용)
 * @param  is_new_file: 파일 열기 플래그 (1: 새 파일, 2: 기존 파일)
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT SD_OpenFile(char *filename, uint16_t *header_info)
{
  FRESULT res;

  if (is_new_file == 0)
  {
    // 새 파일 생성
    res = GetFilename_CreateFolders(filename, header_info);
    if(res == FR_OK || res == FR_EXIST)
      res = f_open(&SDFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
    else return res;
  }
  
  else
  {
    // 기존 파일 이어쓰기
    res = f_open(&SDFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
    if (res == FR_OK)
    {
      res = f_lseek(&SDFile, f_size(&SDFile));
      if (res != FR_OK)
      {
        // 파일 위치 이동 실패
        printf("Failed to seek to end of file, error: %d\n", res);
        bf_sdflag = 0;
        return res;
      }
    }
  }

  if (res != FR_OK)
  {
    printf("Failed to open file, error: %d\n", res);
    bf_sdflag = 0;
  }

  return res;
}

/***************************************************************************
 * @brief  SD 카드에 데이터 쓰기 및 파일 시간 갱신
 * @param  data: 쓸 데이터 버퍼
 * @param  data_size: 쓸 데이터 크기
 * @param  filename: 파일 이름 (시간 갱신에 사용)
 * @retval FRESULT: FR_OK 성공, 그 외 실패
 */
FRESULT SD_WriteData(const uint16_t *data,uint32_t data_size, const char *filename)
{
  FRESULT res;
  UINT byteswritten;

  // 데이터 쓰기 전 캐시 청소 (Cache -> RAM) << ST-LINK끊김으로 인한 조치 효과 없는듯???
  // SCB_CleanDCache_by_Addr((uint32_t*)(data + sd_write_buffer_head), data_size);
  // 데이터 쓰기
  res = f_write(&SDFile, data + sd_write_buffer_head, data_size, &byteswritten);

  // 쓰기 실패 시 오류 처리
  if (res != FR_OK)
  {
    f_close(&SDFile);
    printf("Failed to write to file, error: %d\n", res);
    bf_sdflag = 0;
    return res;
  }
  // 작성된 바이트수가 요청한 크기와 다를 경우 오류 처리
  else if(byteswritten < data_size)
  {
    f_close(&SDFile);
    printf("Incomplete write: %u of %lu bytes written\n", byteswritten, data_size);
    return FR_DISK_ERR;
  }

  // (PRPD데이터를 나눠쓸 횟수 + 1)의 값으로 is_new_file의 값을 나눴을 때 나머지가 1이 아닌 경우 >>> 버퍼 head 업데이트
  if(is_new_file % (DIVIDED_WRITEBYTE + 1) != 1)
  {
    sd_write_buffer_head += (byteswritten / 2); // 타입이 uint16_t 인덱스에 추가할 때는 2로 나눈 값을 더함
    if (sd_write_buffer_head == TOTAL_BYTE / 2) // 위 내용과 마찬가지
    {
      sd_write_buffer_head = 0;
      prpd_write_complete_flag = 1;
    }
  }

  // 파일 시간 갱신
  res = f_utime(filename, &fno);
  if (res != FR_OK)
  {
    printf("Failed to update file time, error: %d\n", res);
    bf_sdflag = 0;
    return res;
  }

  return FR_OK;
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
      bf_sdflag = 0;
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
 * @retval None
 */
FRESULT GetFilename_CreateFolders(char* filename, uint16_t* header_info)
{
  char year[3] = {0};
  char month[3] = {0};
  char day[3] = {0};
  char hour[3] = {0};
  char date[7] = {0};
  char time[7] = {0};

  strncpy(year,(char*)header_info+6,2);
  strncpy(month,(char*)header_info + 8,2);
  strncpy(day,(char*)header_info + 10,2);
  strncpy(hour,(char*)header_info + 12,2);
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
        bf_sdflag = 0;
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
    return res;
  }
    
  // 월 폴더 생성 (예: "20xx/xx/")
  sprintf(path, "20%s/20%s-%s", year
                              , year, month);
  res = f_mkdir(path);
  if (res != FR_OK && res != FR_EXIST)
  {
    printf("Failed to create month folder: %d\n", res);
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
 * @retval None
 * 
 * 폴더 구조: 년/월/일/시간/파일.txt
 * 예: 20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx/20xx-xx-xx_xx-xx/CMS_260101_010101.txt
 */
void DeleteOldestMinFolder(void)
{
  char yearPath[5] = {0};       // "20xx" 4
  char monthPath[13] = {0};     // "20xx/20xx-xx" 12
  char dayPath[24] = {0};       // "20xx/20xx-xx/20xx-xx-xx" 23
  char hourPath[38] = {0};      // "20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx" 37
  // char minPath[55] = {0};       // "20xx/20xx-xx/20xx-xx-xx/20xx-xx-xx_xx/20xx-xx-xx_xx-xx" 
  char tmpPath[128] = {0};      // 임시 path버퍼(snprint경고 해결을 위한)
  char oldestName[22] = {0};    // 가장 오래된 폴더 이름을 저장할 버퍼

  printf("\n=== Finding oldest hour folder ===\n");

  // 1. 가장 오래된 년 폴더 찾기
  fres = FindOldestSubfolder(SDPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No year folders found\n");
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s", oldestName);
  strcpy(yearPath, tmpPath);
  // printf("Oldest year: %s\n", yearPath);

  // 2. 가장 오래된 월 폴더 찾기
  fres = FindOldestSubfolder(yearPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No month folders found in %s\n", yearPath);
    f_unlink(yearPath);
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", yearPath, oldestName);
  strcpy(monthPath, tmpPath);
  // printf("Oldest month: %s\n", monthPath);

  // 3. 가장 오래된 일 폴더 찾기
  fres = FindOldestSubfolder(monthPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No day folders found in %s\n", monthPath);
    f_unlink(monthPath);
    if (CountItemsInFolder(yearPath) == 0)
    {
      f_unlink(yearPath);
    }
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", monthPath, oldestName);
  strcpy(dayPath, tmpPath);
  // printf("Oldest day: %s\n", dayPath);

  // 4. 가장 오래된 시간 폴더 찾기
  fres = FindOldestSubfolder(dayPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No min folders found in %s\n", dayPath);
    CleanupEmptyParentFolders(dayPath, monthPath, yearPath);
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", dayPath, oldestName);
  strcpy(hourPath, tmpPath);
  printf("Oldest min folder to delete: %s\n", hourPath);

  // 5. 시간 폴더와 내부 파일 모두 삭제
  fres = DeleteFolder(hourPath);
  if (fres != FR_OK)
  {
    printf("Failed to delete hour folder: %d\n", fres);
    return;
  }

  // 6. 빈 상위 폴더들 정리
  CleanupEmptyParentFolders(dayPath, monthPath, yearPath);


  // 분단위의 폴더를 생성하는 경우는 4번 내용 지우고 아래 활성화 및 CleanupEmptyParentFolders함수 수정 필요
  #if 0 
  // 4. 가장 오래된 시간 폴더 찾기
  fres = FindOldestSubfolder(dayPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No hour folders found in %s\n", dayPath);
    f_unlink(dayPath);
    if (CountItemsInFolder(monthPath) == 0)
    {
      f_unlink(monthPath);
    }
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", dayPath, oldestName);
  strcpy(hourPath, tmpPath);
  // printf("Oldest day: %s\n", dayPath);

  // 5. 가장 오래된 분 폴더 찾기
  fres = FindOldestSubfolder(hourPath, oldestName);
  if (fres != FR_OK || oldestName[0] == '\0')
  {
    printf("No min folders found in %s\n", hourPath);
    CleanupEmptyParentFolders(hourPath, dayPath, monthPath, yearPath);
    return;
  }
  snprintf(tmpPath, sizeof(tmpPath), "%s/%s", hourPath, oldestName);
  strcpy(minPath, tmpPath);
  printf("Oldest min folder to delete: %s\n", minPath);

  // 6. 분 폴더와 내부 파일 모두 삭제
  fres = DeleteFolder(minPath);
  if (fres != FR_OK)
  {
    printf("Failed to delete hour folder: %d\n", fres);
    return;
  }

  printf("Successfully deleted: %s\n", minPath);

  // 7. 빈 상위 폴더들 정리
  CleanupEmptyParentFolders(hourPath, dayPath, monthPath, yearPath);

  #endif
  printf("=== Cleanup complete ===\n\n");
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
    return res;
  }

  while (1)
  {
    // 디렉터리 엔트리를 1개 읽어 tmpFno에 채움
    res = f_readdir(&tmpDir, &tmpFno);
    //오류가 발생하거나, 더 이상 읽을 엔트리가 없는 경우
    if (res != FR_OK || tmpFno.fname[0] == 0)
    {
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

  f_closedir(&tmpDir);

  if (oldestName[0] == '\0')
  {
    return FR_NO_FILE;
  }

  return FR_OK;
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
    return res;
  }

  while (1)
  {
    //tmpDir을 순회하면서 tmpFno에 정보 저장
    res = f_readdir(&tmpDir, &tmpFno);
    if (res != FR_OK || tmpFno.fname[0] == 0)
    {
      break;
    }

    snprintf(tmpitemPath, sizeof(tmpitemPath), "%s/%s", path, tmpFno.fname);
    strcpy(itemPath ,tmpitemPath);

    if (tmpFno.fattrib & AM_DIR)
    {
      // 하위 폴더 재귀 삭제
      res = DeleteFolder(itemPath);
      if (res != FR_OK)
      {
        f_closedir(&tmpDir);
        return res;
      }
    }
    else
    {
      // 파일 삭제
      res = f_unlink(itemPath);
      if (res != FR_OK)
      {
        printf("Failed to delete file: %s, error: %d\n", itemPath, res);
        f_closedir(&tmpDir);
        return res;
      }
    }
  }

  f_closedir(&tmpDir);

  // 빈 폴더 삭제
  res = f_unlink(path);
  if (res != FR_OK)
  {
    printf("Failed to delete folder: %s, error: %d\n", path, res);
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
void CleanupEmptyParentFolders(const char *dayPath, const char *monthPath, const char *yearPath)
{
  // 일 폴더가 비었는지 확인 후 삭제
  if(CountItemsInFolder(dayPath) == 0)
  {
    if(f_unlink(dayPath) == FR_OK)
    {
      printf("Deleted empty day folder: %s\n", dayPath);
    }

    // 월 폴더가 비었는지 확인 후 삭제
    if(CountItemsInFolder(monthPath) == 0)
    {
      if(f_unlink(monthPath) == FR_OK)
      {
        printf("Deleted empty month folder: %s\n", monthPath);
      }

      // 년 폴더가 비었는지 확인 후 삭제
      if(CountItemsInFolder(yearPath) == 0)
      {
        if(f_unlink(yearPath) == FR_OK)
        {
          printf("Deleted empty year folder: %s\n", yearPath);
        }
      }
    }
  }
}


/***************************************************************************
 * @brief  폴더 내 항목(파일/폴더) 개수 확인
 * @param  path: 확인할 폴더 경로
 * @retval 항목 개수 (오류 시 0)
 */
uint8_t CountItemsInFolder(const char *path)
{
  DIR tmpDir;
  FILINFO tmpFno;
  uint8_t count = 0;

  if (f_opendir(&tmpDir, path) != FR_OK)
  {
    return 0;
  }

  while (f_readdir(&tmpDir, &tmpFno) == FR_OK && tmpFno.fname[0] != 0)
  {
    count++;
  }

  f_closedir(&tmpDir);
  return count;
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
  uint16_t dup_count = 1;                 // 중복 횟수 (1부터 시작)

  // 1번째 인덱스부터 끝까지 순회
  for (uint32_t i = 1; i < num_idx; i++)
  {
    if (pRawData[i] == current_val)
    {
      // 값이 같으면 카운트 증가
      dup_count++;
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

  // 루프 종료 후 남은 마지막 데이터 묶음 처리
  if (dup_count == 1)
  {
    pZipBuf[zip_idx++] = current_val;
  }
  else if (dup_count == 2)
  {
    pZipBuf[zip_idx++] = current_val;
    pZipBuf[zip_idx++] = current_val;
  }
  else
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
  sd_write_buffer_head = 0;
  is_new_file = 0;
  prpd_write_complete_flag = 1;

  //드라이버 언마운트 및 재연결
  f_mount(NULL, SDPath, 0);   // 언마운트
  FATFS_UnLinkDriver(SDPath);    // 드라이버 해제
  FATFS_LinkDriver(&SD_Driver, SDPath);  // 드라이버 재연결 → 플래그 자동 리셋

  HAL_SD_DeInit(&hsd1);       // 하드웨어 해제

  //SDMMC 하드웨어 블록 강제 리셋
  //SDMMC 주변장치의 전원을 껐다 켜는 것과 같은 효과 (레지스터 초기화)
  __HAL_RCC_SDMMC1_FORCE_RESET();
  HAL_Delay(10); //  리셋될 시간 부여
  __HAL_RCC_SDMMC1_RELEASE_RESET();

  HAL_Delay(100);

  return;
}



// void GetFilename_CreateFolders(char* buffer)
// {
//     FILINFO fno;
//     fres = f_stat(SDPath, &fno);
//     if (fres == FR_OK)
//     {
//         snprintf(buffer, bufsize, "%s", fno.fname);
//     }
//     else
//     {
//         snprintf(buffer, bufsize, "Error: %d", fres);
//     }
// }

// /***************************************************************************
//  * @brief  파일명이 날짜-시간 형식인지 확인하는 함수
//  * @param  filename: 파일 이름 문자열
//  * @retval 1: 날짜-시간 형식, 0: 아님
//  */
// // 파일명이 날짜-시간 형식인지 확인
// uint8_t IsDateTimeFile(const char* filename)
// {
//     int len = strlen(filename);
    
//     // 최소 길이 확인
//     if (len < 15) return 0;
    
//     // .txt 확장자 확인
//     if (strstr(filename, ".txt") == NULL && 
//         strstr(filename, ".TXT") == NULL) return 0;
    
//     // 언더스코어 찾기
//     int has_underscore = 0;
//     for (int i = 0; i < len; i++)
//     {
//         if (filename[i] == '_')
//         {
//             has_underscore = 1;
//             break;
//         }
//     }
    
//     return has_underscore;
// }


// /***************************************************************************
//  * @brief  배열에서 가장 최신 파일의 인덱스 찾기 (가장 큰 timestamp)
//  * @param  fileArray: 파일 이름 배열
//  * @param  timestampArray: 타임스탬프 배열
//  * @param  count: 배열의 요소 개수
//  * @retval 가장 최신 파일의 인덱스
//  */
// uint8_t FindNewestIndexInArray(char fileArray[][23], uint64_t timestampArray[], uint8_t count)
// {
//     if (count == 0) return -1;
    
//     int newestIdx = 0;
//     uint64_t newestTime = timestampArray[0];
    
//     for (int i = 1; i < count; i++)
//     {
//         if (timestampArray[i] > newestTime)
//         {
//             newestTime = timestampArray[i];
//             newestIdx = i;
//         }
//     }
    
//     return newestIdx;
// }

// /***************************************************************************
//  * @brief  SD 카드에서 가장 오래된 10개 파일 찾기
//  * @param  oldestFiles: 가장 오래된 10개 파일 이름을 저장할 배열
//  * @retval 찾은 파일 개수
//  */
// uint8_t FindOldest10Files(char oldestFiles[][23])
// { 
//     // 11개 크기의 배열 (10개 + 비교용 1개)
//     char fileBuffer[DELETE_COUNT + 1][23] = {0};
//     uint64_t timestampBuffer[DELETE_COUNT + 1] = {0};
//     int bufferCount = 0;  // 현재 버퍼에 저장된 파일 개수
    
//     printf("\nScanning SD Card\n");
    
//     // 디렉토리 열기
//     fres = f_opendir(&dir, SDPath);
//     if (fres != FR_OK)
//     {
//         printf("Failed to open directory: %d\n", fres);
//         return 0;
//     }
    
//     // SD 카드의 모든 파일 스캔
//     while (1)
//     {
//         fres = f_readdir(&dir, &fno);

//         // 읽기 오류 또는 더 이상 파일이 없으면 종료
//         if(fres != FR_OK || fno.fname[0] == 0) break;
        
//         // 디렉토리는 무시
//         if(fno.fattrib & AM_DIR) continue;
        
//         uint64_t currentTimestamp = ExtractTimestamp(fno.fname);
                        
//         // 버퍼가 10개 미만일 때: 그냥 추가
//         if(bufferCount < DELETE_COUNT)
//         {
//           strcpy(fileBuffer[bufferCount], fno.fname);
//           timestampBuffer[bufferCount] = currentTimestamp;
//           bufferCount++;
//         }
//         // 버퍼가 10개 찼을 때: 11번째 자리에 넣고 비교
//         else
//         {
//           // 11번째 자리(인덱스 10)에 현재 파일 저장
//           strcpy(fileBuffer[DELETE_COUNT], fno.fname);
//           timestampBuffer[DELETE_COUNT] = currentTimestamp;
                
//           // 11개 중에서 가장 최신(큰 timestamp) 파일 찾기
//           int newestIdx = FindNewestIndexInArray(fileBuffer, timestampBuffer, DELETE_COUNT + 1);
                
//           if(newestIdx != DELETE_COUNT)
//           {
//             // printf("  -> Replaced: %s (was newer)\n", fileBuffer[newestIdx]);
                  
//             // 11번째 자리의 파일을 가장 최신 파일 위치로 복사
//             strcpy(fileBuffer[newestIdx], fileBuffer[DELETE_COUNT]);
//             timestampBuffer[newestIdx] = timestampBuffer[DELETE_COUNT];
//           }
//         }
//     }
    
//     f_closedir(&dir);
    
//     printf("\nScan Complete\n");
//     // printf("Total files to delete: %d\n", bufferCount);
    
//     // 최종 결과를 oldestFiles 배열에 복사
//     for (int i = 0; i < bufferCount && i < DELETE_COUNT; i++)
//     {
//         strcpy(oldestFiles[i], fileBuffer[i]);
//         // printf("%d: %s (timestamp: %lu)\n", 
//         //        i+1, oldestFiles[i], timestampBuffer[i]);
//     }
    
//     return bufferCount;
// }

// /***************************************************************************
//  * @brief  SD 카드에서 가장 오래된 10개 파일 삭제
//  * @param  None
//  * @retval None
//  */
// void DeleteOldest10Files(void)
// {
//     char oldestFiles[DELETE_COUNT][23] = {0};
//     uint8_t fileCount;
//     uint8_t count=0;
    
//     // 한 번의 스캔으로 가장 오래된 10개 찾기
//     fileCount = FindOldest10Files(oldestFiles);
    
//     if (fileCount != 10)
//     {
//         printf("No files to delete.\n");
//         return;
//     }
    
//     // 찾은 파일들 삭제
//     for (int i = 0; i < fileCount; i++)
//     {    
//         fres = f_unlink(oldestFiles[i]);
        
//         if(fres != FR_OK)
//         {
//             printf("f_unlink error: %d)\n", fres);
//             break;
//         }
//         else count++;
//     }
    
//     printf("Successfully deleted %d files\n", count);
//     return;
// }
