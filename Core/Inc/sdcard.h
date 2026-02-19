#define TOTAL_BYTE 512*1024     // SD카드에 들어갈 전체 데이터 양
#define WRITE_TIME 3            // 몇줄씩 write할지 설정
#define REMAIN_CAPACITY 200     // SD카드 남은 용량 최소 한계치(MB)
#define NUM_CH 4                //채널 수
#define PHASE_MAX 256           //위상 수
#define LINE_MAX 256            //라인 수

// SD card save file function
uint16_t save_file_to_sdcard(uint16_t *header_info, uint32_t header_info_len, uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint32_t data_len);

// start save file functions
void start_saving(uint16_t *header_info, uint32_t header_info_len, uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint32_t data_len);

// SD card presence check function
uint8_t check_sd_card_present(void);

// SD card mount function
FRESULT mount_sd_card(void);

// SD card open file function
FRESULT SD_OpenFile(char *filename, uint16_t *header_info);

// SD card write data function
FRESULT SD_WriteData(const uint16_t *data,uint16_t data_size, const char *filename);

// SD card get capacity function
uint32_t SD_GetCapacity(void);

// Extract timestamp from filename
uint64_t ExtractTimestamp(const char* filename);

// Generate filename based on header info
FRESULT GetFilename_CreateFolders(char* filename, uint16_t* header_info);

// CreateDateFolders
FRESULT CreateFolders(char* year, char* month, char* day, char* hour);

// Delete oldest hour folder main function
FRESULT DeleteOldestFolder(void);

// Find oldest subfolder
FRESULT FindOldestSubfolder(const char *basePath, char *oldestName);

// Dlete folder and sub files
FRESULT DeleteFolder(const char *path);

// Dlete vacant parent folders
FRESULT CleanupEmptyParentFolders(const char *dayPath, const char *monthPath, const char *yearPath);

// Count things that stored in folder
uint16_t CountItemsInFolder(const char *path);

// Create PRPD ZIP temporary buffer
uint16_t make_prpd_zip_temp_buf(uint16_t (*prpd_data)[LINE_MAX][PHASE_MAX], uint16_t *PRPD_ZIP_TEMP_buf, uint8_t row);

// PRPD data compression function
uint16_t data_compression(uint16_t *pZIP_buf, uint16_t *pPD_Data, uint16_t line, uint8_t ch_max, uint16_t line_max, uint16_t phase_max);
uint32_t Zip_PRPD(uint16_t *pZipBuf, uint16_t *pRawData, uint32_t data_bytes);

// Force reset the SD card interface
void SD_ForceReset(void);

// extern uint8_t sd_write_buffer[TOTAL_BYTE]; // SD card write buffer

extern RTC_HandleTypeDef hrtc;

extern uint32_t open_file_times;

extern uint8_t SD_flg;

extern uint8_t prpd_write_complete_flag;

extern uint32_t elapsed_time;

extern uint8_t get_prpd_data_complete_flag;
