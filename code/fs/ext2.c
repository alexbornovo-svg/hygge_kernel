#include "ext2.h"
#include "io.h"
#include "../code/drivers/ata.h"
#include "../code/libs/kstd.h"

// Static buffer for BGD table (max 32 gruppi, sufficiente per dischi piccoli)
#define MAX_GROUPS 32

ext2_fs_t my_fs;

static bgd_t bgd_table[MAX_GROUPS];

int ext2_init_fs(uint line)
{
    static superblock_t local_sb;
    my_fs.sb = &local_sb;

    // Superblock: byte offset 1024 = LBA 2
    ata_read_sector(2, (uint16_t*)my_fs.sb);
    ata_read_sector(3, (uint16_t*)((uintptr_t)my_fs.sb + 512));

    if (my_fs.sb->ext2_magic != 0xEF53) 
    {
        put_string(line, "[EXT2] Magic mismatch", RED);
        return 1;
    }

    my_fs.block_size      = 1024 << my_fs.sb->log2block_size;
    my_fs.blocks_per_group = my_fs.sb->blocks_per_group;
    my_fs.inodes_per_group = my_fs.sb->inodes_per_group;
    my_fs.total_groups     = my_fs.sb->total_blocks / my_fs.blocks_per_group;
    if (my_fs.sb->total_blocks % my_fs.blocks_per_group != 0)
    {
        my_fs.total_groups++;
    }

    // BDG table
    uint32_t bgd_block = (my_fs.block_size == 1024) ? 2 : 1;
    static uint8_t bgd_buf[4096];
    ext2_read_block(bgd_block, bgd_buf);

    uint32_t groups_to_copy = my_fs.total_groups;
    if (groups_to_copy > MAX_GROUPS) groups_to_copy = MAX_GROUPS;
    kmemcpy(bgd_table, bgd_buf, groups_to_copy * sizeof(bgd_t));
    my_fs.bgds = bgd_table;

    // put_string(line, "[EXT2] Init OK", GREEN);
    return 0;
}

void ext2_debug_magic(uint line)
{
    uint16_t buf[256];
    for (int lba = 0; lba < 8; lba++)
    {
        ata_read_sector(lba, buf);
        uint8_t *b = (uint8_t*)buf;
        uint16_t candidate = b[56] | (b[57] << 8);
        put_fmt(line++, WHITE, "LBA %d: magic candidate = %x", lba, (uint32_t)candidate);
    }
}

void ext2_read_block(uint32_t block_id, uint8_t *buffer)
{
    uint32_t sectors_per_block = my_fs.block_size / 512;
    uint32_t lba = block_id * sectors_per_block;
    for (uint32_t i = 0; i < sectors_per_block; i++)
    {
        ata_read_sector(lba + i, (uint16_t*)(buffer + i * 512));
    }
}

void ext2_read_inode(uint32_t inode_num, inode_t *out)
{
    uint32_t group = (inode_num - 1) / my_fs.inodes_per_group;
    uint32_t index = (inode_num - 1) % my_fs.inodes_per_group;
    uint32_t inode_size = my_fs.sb->inode_size;
    uint32_t table_block = my_fs.bgds[group].inode_table;
    uint32_t byte_offset = index * inode_size;
    uint32_t block_offset = byte_offset / my_fs.block_size;

    static uint8_t buf[4096];
    ext2_read_block(table_block + block_offset, buf);
    kmemcpy(out, buf + (byte_offset % my_fs.block_size), sizeof(inode_t));
}