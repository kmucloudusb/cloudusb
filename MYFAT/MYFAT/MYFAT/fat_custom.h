//
//  fat_custom.h
//  revolution
//
//  Created by Hanul on 01/05/2017.
//  Copyright © 2017 Hanul. All rights reserved.
//

#ifndef fat_custom_h
#define fat_custom_h

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TRUE 1
#define FALSE 0
#define PATH_LEN_FULL 256
#define CMD_LEN_FULL 1024
#define DIR_ENTRY_TABLE_FULL 1024
#define DATA_ENTRY_TABLE_FULL 1024
#define FILE_ID_LEN_FULL 65
#define PIPE_LEN_FULL 1024*1024
#define FAT_CLUSTER_SIZE 4096
#define FAT_AREA_FULL 4096

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct direntry
{
    uint32 cluster;
    unsigned char entry[FAT_CLUSTER_SIZE];
};

struct dataentry
{
    uint32                  startcluster;
    uint32                  size;
    char                    id[FILE_ID_LEN_FULL];
    uint8                   download;
    int                     fd;
};

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void read_path(char *exec_path);

void create_boot_record();
void create_fat_area();
void create_rootdir_entry();

int read_virtual(uint32 sector, uint8 *buffer, uint32 sector_count);
int write_virtual(uint32 sector, uint8 *buffer, uint32 sector_count);

void download_metadata();
void read_pipe(char *buffer);

int create_direntry(uint32 startcluster);
int create_dataentry(uint32 startcluster, uint32 fsize, char *fid);
void write_entries();

int download_file(char *fid);
int read_file(int fd, unsigned long sector, unsigned char *buffer, unsigned long sector_count);

void read_requested(uint32 offset, unsigned char *buffer, uint32 offset_count);
#endif /* fat_custom_h */
