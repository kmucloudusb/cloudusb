//
// Created by 채한울 on 29/09/2017.
//

#ifndef ALGORITHM_FILE_SYSTEM_H
#define ALGORITHM_FILE_SYSTEM_H

#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define ATTR_EMPTY 0
#define ATTR_FILE 1
#define ATTR_DIR 2
#define ATTR_ELSE 3


#define FAT_RESERVED_AREA_POSITION 0
#define FAT_RESERVED_AREA_BACKUP_POSITION 6

#define FAT_FAT_AREA_POSITION 32
#define FAT_FAT_AREA_BACKUP_POSITION 2076

#define FAT_ROOT_DIR_POSISTION 4120

#define FAT_SECTOR_PER_CLUSTER 8

#define FAT_ROOT_DIRECTORY_FIRST_CLUSTER 2

#define FAT_SECTOR_SIZE 512
#define FAT_CLUSTER_SIZE 4096
#define FAT_DIR_ENTRY_SIZE 32
#define FAT_SHORT_FILE_NAME_LENGTH_FULL 11

#define FAT_SFN_SIZE_FULL 11
#define FAT_SFN_SIZE_PARTIAL 8

#define FILE_NAME_FULL 512
#define FILE_ID_FULL 512
#define PIPE_LEN_FULL 4096
#define CLUSTER_INFO_FULL 1024
#define FAT_FAT_AREA_FULL (FAT_FAT_AREA_BACKUP_POSITION - FAT_FAT_AREA_POSITION)

#define FAT_END_OF_CLUSTER_CHAIN_MARKER 0xF0FFFF0F
#define FAT_CLUSTER_CHAIN_MARKER_LEN 4


#define ENTRY_DIR 0x10
#define ENTRY_FILE 0x20

#define ENTRY_FILENAME_BLANK 0x20
#define ENTRY_NAME_LENGTH 8
#define ENTRY_EXTENDER_LENGTH 3

#define CMD_LEN_FULL 1024
#define ENTRY_PER_CLUSTER 128

struct cluster_info
{
    int attr;
    int dirty;
    int cluster_no;
    
    char filename[FILE_NAME_FULL];
    unsigned char buffer[FAT_CLUSTER_SIZE];
};

struct fat_dir_entry
{
    unsigned char name[11];
    unsigned char attr;
    unsigned char nt_res;
    unsigned char create_time_tenth;
    unsigned char create_time[2];
    unsigned char create_date[2];
    unsigned char last_access_time[2];
    unsigned short first_cluster_high;
    unsigned char write_time[2];
    unsigned char write_date[2];
    unsigned short first_cluster_low;
    unsigned int size;
};


unsigned int get_32bit(unsigned char *buffer);
void record_cluster_no();
void get_filename_from_entry(struct fat_dir_entry *entry, char *filename);
unsigned int get_cluster_from_entry(struct fat_dir_entry *entry);
void record_entry_info(unsigned char *entry);
void upload_file(char *filename);
int write_file(char *filename, unsigned char *buffer, int cluster_no);
void clean_dirty_cluster();
int read_media(unsigned int sector, unsigned char *buffer, unsigned int count);
void download_metadata();
void read_pipe(char *buffer);
void download_file(char *fid);
int fatfs_total_path_levels(char *path);
int fatfs_get_substring(char *path, int levelreq, char *output, int max_len);
int fatfs_split_path(char *full_path, char *path, int max_path, char *filename, int max_filename);
int fatfs_lfn_create_sfn(char *sfn_output, char *filename);
void write_fat_area(int cluster, unsigned int size);


int insert_dir_entry(unsigned char *rootdir_entry, struct fat_dir_entry *entry);
void write_entries();

void create_reserved_area();
void create_fat_area();

#endif //ALGORITHM_FILE_SYSTEM_H

