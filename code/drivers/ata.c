#include "ata.h"
#include "io.h"
#include "types.h"
#include "../code/libs/kstd.h"

ata_drive_t ata_active_drive = {0};

// IDENTIFY
int ata_identify(uint16_t base, uint8_t drive_sel)
{
    // Seleziona il drive
    outb(base + 6, drive_sel);
    io_wait();

    // Floating bus check: 0xFF = no hardware
    if (inb(base + 7) == 0xFF)
        return 0;

    // Reset soft
    outb(base + 2, 0);
    outb(base + 3, 0);
    outb(base + 4, 0);
    outb(base + 5, 0);
    outb(base + 7, 0xEC); // IDENTIFY
    io_wait();

    uint8_t status = inb(base + 7);
    if (status == 0)
        return 0; // drive non presente

    // Aspetta BSY
    int timeout = 100000;
    while ((inb(base + 7) & ATA_STATUS_BSY) && timeout--)
        ;
    if (timeout <= 0)
        return 0;

    // ATAPI 
    uint8_t lba_mid  = inb(base + 4);
    uint8_t lba_high = inb(base + 5);
    if (lba_mid != 0 || lba_high != 0)
        return 0; 

    // Waoit DRQ or ERR
    timeout = 100000;
    while (timeout--) {
        status = inb(base + 7);
        if (status & ATA_STATUS_ERR) return 0;
        if (status & ATA_STATUS_DRQ) break;
    }
    if (timeout <= 0) return 0;

    for (int i = 0; i < 256; i++) inw(base);

    return 1;
}

int ata_detect(uint line)
{
    typedef struct { uint16_t base; uint8_t sel; const char *name; } candidate_t;
    candidate_t candidates[4] = {
        { ATA_PRIMARY_DATA,   ATA_DRIVE_MASTER, "[ATA] Primary Master"   },
        { ATA_PRIMARY_DATA,   ATA_DRIVE_SLAVE,  "[ATA] Primary Slave"    },
        { ATA_SECONDARY_DATA, ATA_DRIVE_MASTER, "[ATA] Secondary Master" },
        { ATA_SECONDARY_DATA, ATA_DRIVE_SLAVE,  "[ATA] Secondary Slave"  },
    };

    for (int i = 0; i < 4; i++) {
        if (ata_identify(candidates[i].base, candidates[i].sel)) {
            ata_active_drive.base      = candidates[i].base;
            ata_active_drive.drive_sel = candidates[i].sel;
            ata_active_drive.present   = 1;
            put_string(line, (char*)candidates[i].name, GREEN);
            return 1;
        }
    }

    put_string(line, "[ATA] No disk found", RED);
    return 0;
}

uint init_ata(uint line)
{
    if (!ata_detect(line)) {
        line = put_string(line, "[ATA] Init failed", RED);
        return line;
    }
    line = put_string(line, "[ATA] Init OK", GREEN);
    return line;
}

// compatibility
int search_ata()
{
    uint16_t val = inw(ATA_PRIMARY_STATUS);
    return (val != 0xFF) ? 1 : 0;
}

void ata_read_sector(uint32_t lba, uint16_t *buffer)
{
    uint16_t base = ata_active_drive.base;
    uint8_t  sel  = ata_active_drive.drive_sel | ((lba >> 24) & 0x0F);

    outb(base + 6, sel);
    outb(base + 2, 1);
    outb(base + 3, (uchar)(lba));
    outb(base + 4, (uchar)(lba >> 8));
    outb(base + 5, (uchar)(lba >> 16));
    outb(base + 7, 0x20); // READ SECTORS

    io_wait();
    while (inb(base + 7) & ATA_STATUS_BSY);
    while (!(inb(base + 7) & ATA_STATUS_DRQ));

    for (int i = 0; i < 256; i++)
    {
        buffer[i] = inw(base);
    }
}

void ata_write_sector(uint32_t lba, uint16_t *buffer)
{
    uint16_t base = ata_active_drive.base;
    uint8_t  sel  = ata_active_drive.drive_sel | ((lba >> 24) & 0x0F);

    outb(base + 6, sel);
    outb(base + 2, 1);
    outb(base + 3, (uchar)(lba));
    outb(base + 4, (uchar)(lba >> 8));
    outb(base + 5, (uchar)(lba >> 16));
    outb(base + 7, 0x30); // WRITE SECTORS

    io_wait();
    while (inb(base + 7) & ATA_STATUS_BSY);
    while (!(inb(base + 7) & ATA_STATUS_DRQ));

    for (int i = 0; i < 256; i++)
    {
        outw(base, buffer[i]);
    }

    outb(base + 7, 0xE7); // CACHE FLUSH
    io_wait();
    while (inb(base + 7) & ATA_STATUS_BSY);
}

uint ata_get_harddisk_vendor(uint line)
{

    uint16_t base = ata_active_drive.base;
    outb(base + 6, ata_active_drive.drive_sel);
    io_wait();
    outb(base + 7, 0xEC);
    io_wait();
    while (inb(base + 7) & ATA_STATUS_BSY);
    while (!(inb(base + 7) & ATA_STATUS_DRQ));

    uint16_t buf[256];
    for (int i = 0; i < 256; i++)
    {
        buf[i] = inw(base);
    }

    char model[41];
    int idx = 0;
    for (int i = 27; i <= 46; i++) 
    {
        model[idx++] = (char)(buf[i] >> 8);
        model[idx++] = (char)(buf[i] & 0xFF);
    }
    model[40] = '\0';

    line = put_string(line, "[ATA] Vendor: ", GREY);
    line = put_string(line, model, GREEN);
    return line;
}