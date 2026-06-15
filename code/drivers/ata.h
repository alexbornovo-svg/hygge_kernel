#ifndef ATA_H
#define ATA_H

#include "types.h"

/*
0x1F0 - Data Register (16-bit, R/W)
0x1F1 - Error/Features
0x1F2 - Sector Count
0x1F3 - LBA low (bits 0-7)
0x1F4 - LBA mid (bits 8-15)
0x1F5 - LBA high (bits 16-23)
0x1F6 - Drive/Head (bits 24-27 + drive select)
0x1F7 - Command/Status
*/

#define ATA_PRIMARY_BASE 0x1F0

#define ATA_REG_DATA       ATA_PRIMARY_BASE + 0
#define ATA_REG_ERROR      ATA_PRIMARY_BASE + 1
#define ATA_REG_FEATURES   ATA_PRIMARY_BASE + 1
#define ATA_REG_SEC_COUNT  ATA_PRIMARY_BASE + 2
#define ATA_REG_LBA_LOW    ATA_PRIMARY_BASE + 3
#define ATA_REG_LBA_MID    ATA_PRIMARY_BASE + 4
#define ATA_REG_LBA_HIGH   ATA_PRIMARY_BASE + 5
#define ATA_REG_DRIVE_SEL  ATA_PRIMARY_BASE + 6
#define ATA_REG_COMMAND    ATA_PRIMARY_BASE + 7
#define ATA_REG_STATUS     ATA_PRIMARY_BASE + 7

#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_BSY  0x80

uint init_ata(uint line);
void ata_read_sector(uint32_t lba, uint16_t *buffer);
uint ata_get_harddisk_vendor(uint line);

#endif