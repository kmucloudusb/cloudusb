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
#define DIR_ENTRY_TABLE_FULL 1024
#define DATA_ENTRY_TABLE_FULL 1024
#define FILE_ID_LEN_FULL 65
#define PIPE_LEN_FULL 1024*1024

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct direntry
{
    uint32 cluster;
    unsigned char *data;
};

struct dataentry
{
    uint32                  startcluster;
    uint32                  size;
    char                    id[FILE_ID_LEN_FULL];
};

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
//
//  fat_custom.c
//  revolution
//
//  Created by Hanul on 01/05/2017.
//  Copyright © 2017 Hanul. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "fat_filelib.h"
#include "fat_types.h"
#include "fat_custom.h"

//-----------------------------------------------------------------------------
// Locals
//-----------------------------------------------------------------------------
static unsigned char _reserved[1024];
static unsigned char _fat[1024];

static struct direntry *_direntries[DIR_ENTRY_TABLE_FULL];
static struct dataentry *_dataentries[DATA_ENTRY_TABLE_FULL];

extern struct fatfs _fs;

static char _script_path[PATH_LEN_FULL];
static char _pipe_path[PATH_LEN_FULL];

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
void read_path(char *exec_path);

void create_boot_record();
void create_fat();

int read_virtual(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int write_virtual(unsigned long sector, unsigned char *buffer, unsigned long sector_count);

void download_metadata();
void read_pipe(char *buffer);

int create_direntry(uint32 startcluster);
int create_dataentry(uint32 startcluster, uint32 fsize, char *fid);
void write_entries();

int download_file(char *fid);
int read_file(int fd, uint32 sector, unsigned char *buffer, uint32 sector_count);

#endif /* fat_custom_h */
