//
// Created by lunahc on 29/09/2017.
//

#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

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

#define PIPE_LEN_FULL 4096
#define CMD_LEN_FULL 1024
#define CLUSTER_INFO_FULL 4906
#define FILE_NAME_FULL 512
#define FILE_ID_FULL 512

#define ENTRY_DIR 0x10
#define ENTRY_FILE 0x20
#define ENTRY_REMOVED 0xE5
#define ENTRY_FILENAME_BLANK 0x20
#define ENTRY_PER_CLUSTER 128

#define FAT_RESERVED_AREA_POSITION 0
#define FAT_RESERVED_AREA_BACKUP_POSITION 1
#define FAT_FAT_AREA_POSITION 2
#define FAT_ROOT_DIR_POSITION 2049

#define FAT_ROOT_DIRECTORY_FIRST_CLUSTER 2
#define FAT_SECTOR_SIZE 512
#define FAT_CLUSTER_SIZE 4096
#define FAT_SECTOR_PER_CLUSTER 8
#define FAT_DIR_ENTRY_SIZE 32
#define FAT_SHORT_FILE_NAME_LENGTH_FULL 11
#define FAT_SFN_SIZE_NAME 8
#define FAT_SFN_SIZE_FULL 11
#define FAT_SFN_SIZE_PARTIAL 8
#define ENTRY_NAME_LENGTH 8
#define ENTRY_EXTENDER_LENGTH 3

#define FAT_FAT_AREA_FULL (FAT_ROOT_DIR_POSITION - FAT_FAT_AREA_POSITION)
#define FAT_CLUSTER_CHAIN_MARKER_LEN 4
#define FAT_CLUSTER_CHAIN_EMPTY 0x00
#define FAT_END_OF_CLUSTER_CHAIN_MARKER 0xF0FFFF0F

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

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
int is_reserved_area(unsigned int sector);
int is_fat_area(unsigned int sector);
int is_entry_area(unsigned int sector);
int is_valid_count(unsigned int count);
int is_end_of_filelist(char *filelist, int offset);
int search_next_empty_cluster(int cluster, unsigned int fsize);
int search_next_filelist_offset(char *filelist, int offset);
void record_entry_first_cluster(struct fat_dir_entry *entry, int cluster);
void record_entry_dir(struct fat_dir_entry *entry, int cluster);
void record_entry_file(struct fat_dir_entry *entry, int cluster, char *fid, unsigned int fsize);
void set_dir_entry_info(struct fat_dir_entry *entry, int cluster, char *fid, unsigned int fsize, int dir);

void fat_init();
void sync_with_cloud();
int read_media(unsigned int sector, unsigned char *buffer, unsigned int count);
int write_media(unsigned int sector, unsigned char *buffer, unsigned int count);

void write_fat_area(int cluster, unsigned int size);
void clean_dirty_cluster();
int insert_dir_entry(unsigned char *rootdir_entry, struct fat_dir_entry *entry);
void record_cluster_no();
void record_entry_info(unsigned char *entry);
unsigned int get_32bit(unsigned char *buffer);
unsigned int get_cluster_from_entry(struct fat_dir_entry *entry);
int write_file(char *filename, unsigned char *buffer, int cluster_no);

//-----------------------------------------------------------------------------
// String Processing Functions
//-----------------------------------------------------------------------------
void get_filename_from_entry(struct fat_dir_entry *entry, char *filename);
int fatfs_total_path_levels(char *path);
int fatfs_get_substring(char *path, int levelreq, char *output, int max_len);
int fatfs_split_path(char *full_path, char *path, int max_path, char *filename, int max_filename);
int fatfs_lfn_create_sfn(char *sfn_output, char *filename);

//-----------------------------------------------------------------------------
// Cloud Storage Related Functions
//-----------------------------------------------------------------------------
void upload_file(char *filename);
void download_metadata();
void read_pipe(char *buffer);
void download_file(char *fid);

//-----------------------------------------------------------------------------
// Fixed Area Creation Functions
//-----------------------------------------------------------------------------
void create_fat_area();
void set_root_dir_entry();
void create_reserved_area();

#endif //FILE_SYSTEM_H

