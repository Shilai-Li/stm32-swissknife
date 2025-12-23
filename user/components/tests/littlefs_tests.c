/**
 * @file littlefs_tests.c
 * @brief LittleFS Test Case
 */

#include "littlefs_port.h"
#include <stdio.h>
#include <string.h>

// Mock Handle if not defined elsewhere
// In real app, this is in main.c
W25QXX_HandleTypeDef w25qxx_handle; 

void Test_LittleFS_Entry(void) {
    // 1. Hardware Init (Mocking SPI/Flash init call)
    // Real code: W25QXX_Init(&w25qxx_handle, &hspi1, ...);
    printf("Initializing W25Qxx...\n");
    // Assume hardware ready for this test wrapper
    
    // 2. LFS Init
    printf("Mounting LFS...\n");
    int err = LittleFS_Port_Init();
    if (err) {
        printf("LFS Mount Failed: %d\n", err);
        return;
    }
    printf("LFS Mounted.\n");

    // 3. Write File
    lfs_file_t file;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    
    uint32_t boot_count = 0;
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
    
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
    
    lfs_file_close(&lfs, &file);
    
    printf("Boot Count: %lu\n", boot_count);
    
    // 4. List Directory
    lfs_dir_t dir;
    struct lfs_info info;
    lfs_dir_open(&lfs, &dir, "/");
    
    while (lfs_dir_read(&lfs, &dir, &info) > 0) {
        printf("Found: %s\tType: %d\tSize: %lu\n", info.name, info.type, info.size);
    }
    
    lfs_dir_close(&lfs, &dir);
    
    LittleFS_Port_DeInit();
}
