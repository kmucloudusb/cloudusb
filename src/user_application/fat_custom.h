//
//  fat_custom.h
//  CloudUSB
//
//  Created by Hanul on 01/05/2017.
//  Copyright © 2017 Hanul. All rights reserved.
//

#ifndef fat_custom_h
#define fat_custom_h

#include "fat_types.h"

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
#define FAT_AREA_FULL 1048576
#define BUFFER_SIZE_FULL 16384
#define INIT 0
#define RETURN_FILE 1
#define FILE_WRITE_OVER 2
#define BUFF_LEN_FULL_SECTOR 32
#define BUFF_LEN_FULL FAT_SECTOR_SIZE*BUFF_LEN_FULL_SECTOR

#define FAT_LOC_BOOT_RECORD 0
#define FAT_LOC_RESERVED_AREA 1
#define FAT_LOC_BOOT_RECORD_BACKUP 6
#define FAT_LOC_RESERVED_AREA_BACKUP 7

#define FAT_ERROR -1
#define FAT_UNCHANGED 0
#define FAT_CREATION 1
#define FAT_EXPANSION 2
#define FAT_DELETION 3
#define FAT_REDUCTION 4

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

struct request
{
    int pid;
    unsigned int read_amount;
    unsigned int write_amount;
    long long read_file_offset;
    long long write_file_offset;
    char *write_buff;
};

struct return_file
{
    uint8 *buf;
    int nread;
};

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void read_path(char *exec_path);

void create_blank();
void create_reserved_area();
void create_fat_area();
void create_rootdir_entry();

int read_virtual(uint32 sector, uint8 *buffer, uint32 sector_count);
int write_virtual(uint32 sector, uint8 *buffer, uint32 sector_count);

void download_metadata();
void read_pipe(char *buffer);

int create_direntry(uint32 startcluster);
int create_dataentry(uint32 startcluster, uint32 fsize, char *fid);
void write_entries();

int search_changed_cluster(uint8 *before, uint8 *after, uint32 *loc);

int download_file(char *fid);
int read_file(int fd, unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int write_file(int fd, unsigned long sector, unsigned char *buffer, unsigned long sector_count);

void read_requested(uint32 offset, unsigned char *buffer, uint32 offset_count);

void run_module();
void file_transfer(int signo);
void write_request(int signo);

#endif /* fat_custom_h */
