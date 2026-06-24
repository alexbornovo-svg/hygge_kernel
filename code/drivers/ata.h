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

// Primary bus
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SEC_COUNT   0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE_SEL   0x1F6
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_ALT_STATUS  0x3F6

// Secondary bus
#define ATA_SECONDARY_DATA 0x170
#define ATA_SECONDARY_ERROR 0x171
#define ATA_SECONDARY_SEC_COUNT 0x172
#define ATA_SECONDARY_LBA_LOW 0x173
#define ATA_SECONDARY_LBA_MID 0x174
#define ATA_SECONDARY_LBA_HIGH 0x175
#define ATA_SECONDARY_DRIVE_SEL 0x176
#define ATA_SECONDARY_COMMAND 0x177
#define ATA_SECONDARY_STATUS 0x177
#define ATA_SECONDARY_ALT_STATUS 0x376

// compatibility aliases
#define ATA_REG_DATA ATA_PRIMARY_DATA
#define ATA_REG_ERROR ATA_PRIMARY_ERROR
#define ATA_REG_SEC_COUNT ATA_PRIMARY_SEC_COUNT
#define ATA_REG_LBA_LOW ATA_PRIMARY_LBA_LOW
#define ATA_REG_LBA_MID ATA_PRIMARY_LBA_MID
#define ATA_REG_LBA_HIGH ATA_PRIMARY_LBA_HIGH
#define ATA_REG_DRIVE_SEL ATA_PRIMARY_DRIVE_SEL
#define ATA_REG_COMMAND ATA_PRIMARY_COMMAND
#define ATA_REG_STATUS ATA_PRIMARY_STATUS

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

// Drive selector bits
#define ATA_DRIVE_MASTER 0xE0
#define ATA_DRIVE_SLAVE  0xF0

typedef struct 
{
    uint16_t base;       // I/O base (0x1F0 o 0x170)
    uint8_t  drive_sel;  // 0xE0 = master, 0xF0 = slave
    uint8_t  present;
} ata_drive_t;

// global drive (?)
extern ata_drive_t ata_active_drive;

int ata_detect(uint line);
int ata_identify(uint16_t base, uint8_t drive_sel);
void ata_read_sector(uint32_t lba, uint16_t *buffer);
void ata_write_sector(uint32_t lba, uint16_t *buffer);
uint ata_get_harddisk_vendor(uint line);
int search_ata();
uint init_ata(uint line);

#endif