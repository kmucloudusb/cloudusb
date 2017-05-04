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
static int _image_fd;
static int _real_fd;

static struct fatfs _fs;
static struct table _table[TABLE_NUMBER_FULL];
static unsigned int _table_size;

static char _script_path[PATH_LEN_FULL];
static char _pipe_path[PATH_LEN_FULL];
static char _image_path[PATH_LEN_FULL];

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
void read_path(char *exec_path)
{
    strcpy(_script_path, exec_path);
    strcpy(_image_path, exec_path);
    strcpy(_pipe_path, exec_path);
    
    strcpy(&(_script_path[strlen(_script_path)-8]), "../../../googledrive/list.py");
    strcpy(&(_image_path[strlen(_image_path)-8]), "../../../image.dmg");
    strcpy(&(_pipe_path[strlen(_pipe_path)-8]), "../../../myfifo");
}

void create_image()
{
    char cmd[CMD_LEN_FULL] = "sudo dd if=/dev/zero of=";
    
    strcpy(&(cmd[strlen(cmd)]), _image_path);
    strcpy(&(cmd[strlen(cmd)]), " bs=1G count=1");
    system(cmd);
    
    memset(cmd, 0x00, sizeof(char)*CMD_LEN_FULL);
    
    strcpy(cmd, "sudo mkdosfs ");
    strcpy(&(cmd[strlen(cmd)]), _image_path);
    system(cmd);
}

int open_image()
{
    if((_image_fd = open(_image_path, O_RDWR)) < 0)
    {
        perror(_image_path);
        return -1;
    }
    
    return _image_fd;
}

int read_image(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;
    
    for (i=0;i<sector_count;i++)
    {
        lseek(_image_fd, 512*sector, SEEK_SET);
        
        if(read(_image_fd, buffer, 512) < 0)
        {
            perror("read");
            return 0;
        }
        
        sector ++;
        buffer += 512;
    }
    
    return 1;
}

int write_image(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;
    
    for (i=0;i<sector_count;i++)
    {
        lseek(_image_fd, 512*sector, SEEK_SET);
        
        if(write(_image_fd, buffer, 512) < 0)
        {
            perror("write");
            return 0;
        }
        
        sector ++;
        buffer += 512;
    }
    
    return 1;
}

void close_image()
{
    close(_image_fd);
}

void download_metadata()
{
    char cmd[CMD_LEN_FULL] = "sudo python ";
    strcat(cmd, _script_path);
    strcat(cmd, " --path ");
    strcat(cmd, _pipe_path);
    system(cmd);
}

// input : <file_name, file_size, file_id>
void read_pipe(char *buffer)
{
    int fd;
    
    fd = open(_pipe_path, O_RDONLY);
    read(fd, buffer, PIPE_LEN_FULL);
    
    printf("Recieved : [%s]\n", buffer);
    
    close(fd);
}

void make_alloc_table(unsigned long table_num, uint32 start_cluster, uint32 fsize, char *fileid)
{
    _table[table_num].start_cluster = start_cluster;
    _table[table_num].size = fsize;
    strcpy(_table[table_num].fileid, fileid);
}

void write_metadata(char *filename, uint32 fsize)
{
    
//    write_file_on_media(filename, fsize);
}

void make_metadata()
{
    char ch = -1;
    int table_num = 0;
    unsigned long offset = 0;
    
    char filelist[PIPE_LEN_FULL];
    
    char filename[FAT_SFN_SIZE_FULL];
    uint32 fsize;
    char fileid[FILE_ID_LEN_FULL];
    
    uint32 empty_cluster_first = 6;
    uint32 cluster_size = 512 * _fs.sectors_per_cluster;
    
    read_pipe(filelist);
    
    while(ch != '\0')
    {
        sscanf(filelist+offset, "%s %u %s", filename, &fsize, fileid);
        
        make_alloc_table(table_num++, empty_cluster_first, fsize, fileid);
        write_metadata(filename, fsize);
        
        while((ch = *(filelist+(offset++))) != '\n' && ch != '\0');
        empty_cluster_first += fsize/cluster_size + ((fsize%cluster_size)? 1: 0);
    }
    
    _table_size = table_num;
    
    /*      Test Code       */
    int i;
    for (i=0; i<table_num; ++i)
        printf("table %2u: %9u %9u %s\n", i, _table[i].start_cluster, _table[i].size, _table[i].fileid);
}

// download from google drive by file ID
// return file descriptor
int download_from_g_drive(char *fileid)
{
    return -1;
}

int read_file(int fd, uint32 sector, uint32 *buffer, uint32 sector_count)
{
    lseek(fd, 512*sector, SEEK_SET);
    
    if(read(fd, buffer, 512 * sector_count) < 0)
    {
        perror("read");
        return 0;
    }
    
    return 1;
}

void ans_request(uint32 offset, uint32 *buffer, uint32 offset_count)
{
    int i;
    int fd = -1;
    
    uint32 cluster = ((offset/512)/_fs.sectors_per_cluster);
    
    for(i=0; i<_table_size; ++i)
        if(_table[i].start_cluster <= cluster && cluster < _table[i].size)
        {
            fd = download_from_g_drive(_table[i].fileid);
            break;
        }
    
    read_file(fd, cluster - _table[i].start_cluster, buffer, offset_count/512);
}
