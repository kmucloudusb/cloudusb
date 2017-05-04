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
#define PATH_LEN_FULL 256
#define CMD_LEN_FULL 256
#define TABLE_NUMBER_FULL 1024
#define FILE_ID_LEN_FULL 65
#define PIPE_LEN_FULL 1024

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct table
{
    uint32                  start_cluster;
    uint32                  size;
    char                    fileid[FILE_ID_LEN_FULL];
};

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void read_path(char *exec_path);
void create_image();
int open_image();
int read_image(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int write_image(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
void close_image();
void create_image();
void download_metadata();
void read_pipe(char *buffer);
void make_alloc_table(unsigned long table_num, uint32 start_cluster, uint32 fsize, char *fileid);
void write_metadata(char *filename, uint32 fsize);
void make_metadata();
int download_from_g_drive(char *fileid);
int read_file(int fd, uint32 sector, uint32 *buffer, uint32 sector_count);
void ans_request(uint32 offset, uint32 *buffer, uint32 offset_count);

#endif /* fat_custom_h */
