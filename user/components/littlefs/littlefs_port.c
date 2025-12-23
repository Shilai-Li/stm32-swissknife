/**
 * @file littlefs_port.c
 * @brief LittleFS Port Implementation for W25Qxx
 */

#include "../littlefs_port.h"
#include <string.h>

// Global LFS instances
lfs_t lfs;
struct lfs_config cfg;

/* 
 * Wrapper functions that match lfs interface 
 */

// Read: Read 'size' bytes from 'block' + 'off'
int lfs_w25q_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    uint32_t addr = (block * c->block_size) + off;
    
    // Check bounds
    if (addr + size > (c->block_count * c->block_size)) {
        return LFS_ERR_IO;
    }

    // Call W25Q driver
    // Note: W25QXX_Read usually takes (Handler, Buffer, Address, Length)
    W25QXX_Read(&w25qxx_handle, (uint8_t*)buffer, addr, size);
    
    return LFS_ERR_OK;
}

// Prog: Program 'size' bytes. 
// LittleFS guarantees that 'size' is a multiple of 'prog_size' (256) 
// and that 'off' is aligned.
int lfs_w25q_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t addr = (block * c->block_size) + off;

    // Call W25Q driver
    W25QXX_Write(&w25qxx_handle, (uint8_t*)buffer, addr, size);

    return LFS_ERR_OK;
}

// Erase: Erase a block. W25Q Sector Erase (4KB)
int lfs_w25q_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t addr = block * c->block_size;
    
    W25QXX_Erase_Sector(&w25qxx_handle, addr);
    
    return LFS_ERR_OK;
}

// Sync: Ensure data is on media. W25QXX_Write is usually blocking until write complete.
// If you use DMA/ISR, you might need to wait here.
int lfs_w25q_sync(const struct lfs_config *c) {
    return LFS_ERR_OK;
}


int LittleFS_Port_Init(void) {
    // 1. Ensure W25Q driver is initialized (User should have done this)
    // We can assume w25qxx_handle.Info is valid if Init was called.
    
    // 2. Setup Config
    memset(&cfg, 0, sizeof(cfg));

    cfg.read  = lfs_w25q_read;
    cfg.prog  = lfs_w25q_prog;
    cfg.erase = lfs_w25q_erase;
    cfg.sync  = lfs_w25q_sync;

    // Attributes for W25Q64 / Q128 etc.
    // Ideally we use info from handle
    cfg.read_size = 1;        // Can read 1 byte
    cfg.prog_size = 256;      // Page size
    cfg.block_size = 4096;    // Sector size (Erase granulariry)
    
    if (w25qxx_handle.Info.SectorCount > 0) {
        cfg.block_count = w25qxx_handle.Info.SectorCount;
    } else {
        // Fallback defaults if not inited properly (dangerous)
        cfg.block_count = 2048; // Assume 8MB (2048 * 4KB)
    }

    cfg.cache_size = 256;         // Read cache (Must be >= prog_size if prog_size > 1? No, cache should be >= read_size)
                                  // Recommendations: cache_size = prog_size usually good.
    cfg.lookahead_size = 256;     // Bitmask for free blocks lookahead (RAM usage)
    cfg.block_cycles = 500;       // Wear leveling metadata logic

    // 3. Mount
    int err = lfs_mount(&lfs, &cfg);
    
    // 4. Reformat if needed
    if (err) {
        // If mount failed, it might be first run. Format.
        lfs_format(&lfs, &cfg);
        err = lfs_mount(&lfs, &cfg);
    }
    
    return err;
}

void LittleFS_Port_DeInit(void) {
    lfs_unmount(&lfs);
}
