#include "io.h"
#include "types.h"
#include "ata.h"
#include "../code/libs/kstd.h"

uint init_ata(uint line)
{
    // line = put_string(line, "[ATA] -- WIP --", GREY);

    int diskfound = search_ata();

    if (diskfound)
    {
        // line = put_string(line, "[ATA] Disk found", GREY);
    }
    else 
    {
        line = put_string(line, "[ATA] Disk not found", RED);
        return line;
    }

    outb(ATA_REG_DRIVE_SEL, 0xE0);
    io_wait();

    outb(ATA_REG_SEC_COUNT, 0);
    outb(ATA_REG_LBA_LOW, 0);
    outb(ATA_REG_LBA_MID, 0);
    outb(ATA_REG_LBA_HIGH, 0);
    // line = put_string(line, "[ATA] REG 2-5 0", GREY);
    io_wait();

    outb(ATA_REG_COMMAND, 0xEC);
    io_wait();
    // line = put_string(line, "[ATA] Command 0xEC (IDENTIFY)", GREY);

    while (inb(ATA_REG_STATUS) & ATA_STATUS_BSY) {
        // Waiting the hardware
    }

    uchar status = inb(ATA_REG_STATUS);
    if (status == 0) 
    {
        // line = put_string(line, "[ATA] Driver doesn't support 0xEC (IDENTIFY)", RED);
        return line;
    }

    while (!(inb(ATA_REG_STATUS) & ATA_STATUS_DRQ)) 
    {
        // Waiting for data to be ready
    }

    // line = put_string(line, "[ATA] Data ready", GREEN);

    return line;
}

int search_ata()
{
    uint16_t val = inw(ATA_REG_STATUS);

    if (val == 0xFF)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ata_read_sector(uint32_t lba, uint16_t *buffer)
{
    outb(ATA_REG_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_REG_SEC_COUNT, 1);                 // Reading sector 1
    outb(ATA_REG_LBA_LOW, (uchar)(lba));        // Bit 0-7
    outb(ATA_REG_LBA_MID, (uchar)(lba >> 8));   // Bit 8-15
    outb(ATA_REG_LBA_HIGH, (uchar)(lba >> 16));

    outb(ATA_REG_COMMAND, 0x20);
    io_wait(); // freetime for the hardware

    while (inb(ATA_REG_STATUS) & 0x80);     // waiting
    while (!(inb(ATA_REG_STATUS) & 0x08));

    for (int i = 0; i < 256; i++) 
    {
        buffer[i] = inw(ATA_REG_DATA);
    }
}

uint ata_get_harddisk_vendor(uint line)
{
    uint16_t identify_buf[256];
    for (int i = 0; i < 256; i++) {
        identify_buf[i] = inw(ATA_REG_DATA);
    }

    char model_string[41];
    int out_idx = 0;

    for (int i = 27; i <= 46; i++) 
    {
        model_string[out_idx]     = (char)(identify_buf[i] >> 8);
        model_string[out_idx + 1] = (char)(identify_buf[i] & 0xFF);
        out_idx += 2;
    }
    model_string[40] = '\0';
    line = put_string(line, "[ATA] Vendor: ", GREY);
    line = put_string(line, model_string, GREEN);

    return line;
}