#define DELETE_COUNT 10  // 삭제할 파일 수
#define TOTAL_BYTE 512*1024 // SD카드에 들어갈 전체 데이터 양
#define DIVIDED_WRITEBYTE 2  // TOTAL_BYRE를 몇번에 걸쳐서 쓸지
#define WRITEBYTE (TOTAL_BYTE/DIVIDED_WRITEBYTE) // f_write한번 할 때 쓸 양

// SD card presence check function
uint8_t check_sd_card_present(void);

// SD card save file functions
void save_file_to_sdcard(uint16_t *header_info, uint32_t header_info_len, uint16_t *prpd_data, uint32_t data_len, char *filename);

// SD card open file function
FRESULT SD_OpenFile(char *filename, uint16_t *header_info);

// SD card write data function
FRESULT SD_WriteData(const uint16_t *data,uint32_t data_size, const char *filename);

// SD card mount function
uint8_t mount_sd_card(void);

// SD card get capacity function
uint32_t SD_GetCapacity(void);

// Extract timestamp from filename
uint64_t ExtractTimestamp(const char* filename);

// Find the index of the newest file in an array
// uint8_t FindNewestIndexInArray(char fileArray[][23], uint64_t timestampArray[], uint8_t count);

// Find the oldest 10 files on the SD card
// uint8_t FindOldest10Files(char oldestFiles[][23]);

// Delete the oldest 10 files from the SD card
// void DeleteOldest10Files(void);

// Generate filename based on header info
FRESULT GetFilename_CreateFolders(char* filename, uint16_t* header_info);

// CreateDateFolders
FRESULT CreateFolders(char* year, char* month, char* day, char* hour);

// Delete oldest hour folder main function
void DeleteOldestMinFolder(void);

// Find oldest subfolder
FRESULT FindOldestSubfolder(const char *basePath, char *oldestName);

// Dlete folder and sub files
FRESULT DeleteFolder(const char *path);

// Dlete vacant parent folders
void CleanupEmptyParentFolders(const char *dayPath, const char *monthPath, const char *yearPath);

// Count things that stored in folder
uint8_t CountItemsInFolder(const char *path);

// PRPD data compression function
uint32_t Zip_PRPD(uint16_t *pZipBuf, uint16_t *pRawData, uint32_t data_bytes);

// Force reset the SD card interface
void SD_ForceReset(void);

// extern uint8_t sd_write_buffer[TOTAL_BYTE]; // SD card write buffer

extern uint32_t sd_write_buffer_head; // DMA버퍼의 head 위치

extern uint16_t is_new_file;

extern uint8_t bf_sdflag;

extern uint8_t prpd_write_complete_flag;
