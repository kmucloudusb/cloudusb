#include "file_system.h"

static char *path_list = "/home/pi/cloudusb/src/googledrive/list/list.py";
static char *path_downloader = "/home/pi/cloudusb/src/googledrive/download/download.py";
static char *path_uploader = "/home/pi/cloudusb/src/googledrive/upload/upload.py";
static char *path_pipe = "/home/pi/cloudusb/bin/myfifo";

static unsigned char reserved_area[FAT_SECTOR_SIZE];
static unsigned char fat_area[FAT_FAT_AREA_FULL];

static struct cluster_info cluster_info[CLUSTER_INFO_FULL];

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
void fat_init()
{
    // Create boot record area
    create_reserved_area();
    
    // Create fat area
    create_fat_area();
    
    // Set root dir entry type
    set_root_dir_entry();
}

void write_entries()
{
    int i;
    char ch = -1;
    int fd;
    int offset = 0;
    
    char filelist[PIPE_LEN_FULL] = {0};
    char full_path[PIPE_LEN_FULL];
    char path[FILE_NAME_FULL];
    char filename[FILE_NAME_FULL];
    char shortfilename[FAT_SFN_SIZE_FULL];
    char fid[FILE_ID_FULL];
    
    struct fat_dir_entry entry = {0};
    unsigned int fsize;
    
    int dir;
    int cluster = 3;
    
    // Receive information from Google Drive via pipe
    read_pipe(filelist);
    
    while(ch != '\0' && filelist[offset] != 0)
    {
        sscanf(filelist+offset, "%512s %u %s %d", full_path, &fsize, fid, &dir);
        
        // Write on allocation table
        write_fat_area(cluster, fsize);
        
        // Extract short file name for entry
        fatfs_split_path(full_path, path, FILE_NAME_FULL, filename, FILE_NAME_FULL);
        fatfs_lfn_create_sfn(shortfilename, filename);
        memcpy(entry.name, shortfilename, FAT_SFN_SIZE_FULL);
        
        entry.first_cluster_high = (unsigned short) ((cluster & 0xFFFF0000) >> 16);
        entry.first_cluster_low = (unsigned short) cluster;
        
        if (dir) {
            entry.attr = ENTRY_DIR;
            entry.size = 0;
            
            cluster_info[cluster].attr = ATTR_DIR;
        }
        else {
            entry.attr = ENTRY_FILE;
            entry.size = fsize;
            
            download_file(fid);
            
            if ( ((fd = open(fid, O_RDONLY)) >= 0) ) {
                for (i = cluster;
                     i < (cluster + ((fsize / FAT_CLUSTER_SIZE) + ((fsize % FAT_CLUSTER_SIZE) ? 1 : 0)));
                     i++)
                {
                    cluster_info[i].attr = ATTR_FILE;
                    read(fd, cluster_info[cluster].buffer, FAT_CLUSTER_SIZE);
                    strcpy(cluster_info[i].filename, fid);
                }
                
                close(fd);
            }
        }
        
        insert_dir_entry(cluster_info[FAT_ROOT_DIRECTORY_FIRST_CLUSTER].buffer, &entry);
        
        printf("\n[Written Data]\n filename = %s\n attr = %d\n", cluster_info[cluster].filename, cluster_info[cluster].attr);
        
        // Search next empty cluster
        if (fsize == 0 || dir)
            cluster ++;
        else
            cluster += (fsize / FAT_CLUSTER_SIZE) + ((fsize % FAT_CLUSTER_SIZE) ? 1 : 0);
        
        // Search next line first character
        while((ch = *(filelist+(offset++))) != '\n' && ch != '\0');
    }
}

int read_media(unsigned int sector, unsigned char *buffer, unsigned int count)
{
    int i;
    unsigned int offset = 0;
    
    printf("[read] sector = %u\n", sector);
    puts("");
    
    while (count != 0 && count <= 32) {
        // Reserved Area
        if (FAT_RESERVED_AREA_POSITION == sector || sector == FAT_RESERVED_AREA_BACKUP_POSITION) {
            memcpy(buffer + offset,
                   reserved_area + sector % FAT_RESERVED_AREA_BACKUP_POSITION * FAT_SECTOR_SIZE,
                   FAT_SECTOR_SIZE);
            
            offset += FAT_SECTOR_SIZE;
            count --;
            sector += 1;
        }
        
        // Fat Area
        else if (FAT_FAT_AREA_POSITION <= sector && sector < FAT_ROOT_DIR_POSISTION) {
            memcpy(buffer + offset,
                   fat_area + ((sector - FAT_FAT_AREA_POSITION) % (FAT_ROOT_DIR_POSISTION - FAT_FAT_AREA_POSITION) * FAT_SECTOR_SIZE),
                   FAT_SECTOR_SIZE);
            
            offset += FAT_SECTOR_SIZE;
            count --;
            sector += 1;
        }
        
        // Entries
        else if (FAT_ROOT_DIR_POSISTION <= sector && sector < FAT_ROOT_DIR_POSISTION + CLUSTER_INFO_FULL) {
            unsigned int cluster =
            (sector - FAT_ROOT_DIR_POSISTION) / FAT_SECTOR_PER_CLUSTER + FAT_ROOT_DIRECTORY_FIRST_CLUSTER;
            
            memcpy(buffer + offset, cluster_info[cluster].buffer, FAT_CLUSTER_SIZE);
            
            offset += FAT_CLUSTER_SIZE;
            count -= FAT_SECTOR_PER_CLUSTER;
            sector += FAT_SECTOR_PER_CLUSTER;
            
            clean_entries();
        }
        // Meaning less
        else {
            memset(buffer + offset, 0x00, FAT_SECTOR_SIZE);
            
            offset += FAT_SECTOR_SIZE;
            count --;
            sector += 1;
        }
    }
    
    return 1;
}

int write_media(unsigned int sector, unsigned char *buffer, unsigned int count)
{
    int i;
    unsigned int offset = 0;
    
    printf("\n[wrtie] sector = %u cluster = %u size = %u\n",
           sector, (sector - FAT_ROOT_DIR_POSISTION) / FAT_SECTOR_PER_CLUSTER, count);
    
    while (count != 0 && count <= 32) {
        // Fat Area
        if (FAT_FAT_AREA_POSITION <= sector && sector < FAT_ROOT_DIR_POSISTION) {
            int pos = ((sector - FAT_FAT_AREA_POSITION) * FAT_SECTOR_SIZE);
            
            memcpy(fat_area + pos, buffer + offset, FAT_SECTOR_SIZE);
            
            offset += FAT_SECTOR_SIZE;
            count --;
            sector ++;
        }
        
        // Entries
        else {
            int cluster = (sector - FAT_ROOT_DIR_POSISTION) / FAT_SECTOR_PER_CLUSTER + FAT_ROOT_DIRECTORY_FIRST_CLUSTER;
            
            memcpy(cluster_info[cluster].buffer, buffer + offset, FAT_CLUSTER_SIZE);
            cluster_info[cluster].dirty = 1;
            
            printf("((cluster %d is dirty))\n", cluster);
            clean_entries();
            
            offset += FAT_CLUSTER_SIZE;
            count -= FAT_SECTOR_PER_CLUSTER;
            sector += FAT_SECTOR_PER_CLUSTER;
        }
    }
    
    for (i=0; i<512; i++) {
        if (i != 0 && i % 16 == 0)
            puts("");
        printf("%02X ", buffer[i]);
    }
    puts("");
    
    return 1;
}

void record_cluster_no()
{
    int i;
    int count = 0;
    unsigned int cluster_chain_info;
    puts("[record cluster no]");
    
    for (i=FAT_ROOT_DIRECTORY_FIRST_CLUSTER*FAT_CLUSTER_CHAIN_MARKER_LEN;
         i<CLUSTER_INFO_FULL;
         i+=FAT_CLUSTER_CHAIN_MARKER_LEN)
    {
        cluster_chain_info = get_32bit(fat_area + i);
        
        if (cluster_chain_info == FAT_CLUSTER_CHAIN_EMPTY) {
            count = 0;
            continue;
        }
        
        if ((cluster_chain_info & FAT_END_OF_CLUSTER_CHAIN_MARKER) == FAT_END_OF_CLUSTER_CHAIN_MARKER) {
            cluster_info[i/FAT_CLUSTER_CHAIN_MARKER_LEN].cluster_no = count;
            count = 0;
        }
        else {
            cluster_info[i/FAT_CLUSTER_CHAIN_MARKER_LEN].cluster_no = count++;
        }
    }
}

void record_entry_info(unsigned char *entry)
{
    int i;
    char filename[FILE_NAME_FULL];
    struct fat_dir_entry *item;
    
    for (i=0; i<FAT_CLUSTER_SIZE; i+=FAT_DIR_ENTRY_SIZE) {
        item = (struct fat_dir_entry*) (entry + i);
        
        unsigned int cluster = get_cluster_from_entry(item);
        
        if (item->attr == ENTRY_FILE) {
            if (item->name[0] == ENTRY_REMOVED && cluster_info[cluster].dirty) {
                // Need to add remove file function
                cluster_info[cluster].dirty = 0;
                
                continue;
            }
            
            strcpy(filename, cluster_info[cluster].filename);
            
            cluster_info[cluster].attr = ATTR_FILE;
            get_filename_from_entry(item, cluster_info[cluster].filename);
            
            write_file(cluster_info[cluster].filename, cluster_info[cluster].buffer, 0);
            
            if (cluster_info[cluster].dirty) {
                upload_file(cluster_info[cluster].filename);
                cluster_info[cluster].dirty = 0;
                
                printf("<<<Cluster %d is dirty...>>>\n", cluster);
            }
            else {
                printf("<<<Cluster %d is clean...>>>\n", cluster);
            }
        }
        else if (item->attr == ENTRY_DIR) {
            cluster_info[cluster].attr = ATTR_DIR;
        }
    }
}

void clean_dirty_cluster()
{
    int i;
    puts("[clean dirty cluster]");
    
    for (i=FAT_ROOT_DIRECTORY_FIRST_CLUSTER; i<CLUSTER_INFO_FULL; i++) {
        if (cluster_info[i].attr == ATTR_DIR)
            record_entry_info(cluster_info[i].buffer);
    }
}

void write_fat_area(int cluster, unsigned int size)
{
    int loc = 4*cluster;
    int cluster_no = 0;
    
    while (1) {
        cluster_info[cluster].dirty = 0;
        cluster_info[cluster].cluster_no = cluster_no++;
        
        if (size > 4096) {
            fat_area[loc++] = (unsigned char)((++cluster) & 0xFF);
            fat_area[loc++] = (unsigned char)(((cluster) & 0xFF00) >> 8);
            fat_area[loc++] = (unsigned char)(((cluster) & 0xFF0000) >> 16);
            fat_area[loc++] = (unsigned char)(((cluster) & 0xFF000000) >> 24);
            
            size -= 4096;
        }
        else {
            fat_area[loc++] = 0xFF;
            fat_area[loc++] = 0xFF;
            fat_area[loc++] = 0xFF;
            fat_area[loc] = 0x0F;
            
            break;
        }
    }
}

int insert_dir_entry(unsigned char *rootdir_entry, struct fat_dir_entry *entry)
{
    int i;
    
    for (i=0; i<ENTRY_PER_CLUSTER; i+=FAT_DIR_ENTRY_SIZE) {
        if (rootdir_entry[i] == 0x00 || rootdir_entry[i] == 0xE5) {
            memcpy(rootdir_entry + i, entry, sizeof(struct fat_dir_entry));
            
            return 1;
        }
    }
    
    return -1;
}

//-----------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------
unsigned int get_32bit(unsigned char *buffer)
{
    return ( ((unsigned int) buffer[3]<<24)
            + ((unsigned int) buffer[2]<<16)
            + ((unsigned int) buffer[1]<<8)
            + ((unsigned int) buffer[0]) );
}

unsigned int get_cluster_from_entry(struct fat_dir_entry *entry)
{
    return ( (((unsigned int) entry->first_cluster_high) << 16) | entry->first_cluster_low );
}

int read_file(int cluster, unsigned char *buffer, int size)
{
    int fd = open(cluster_info[cluster].filename, O_RDONLY);
    
    read(fd, buffer, size);
    
    close(fd);
    
    return 1;
}

int write_file(char *filename, unsigned char *buffer, int cluster_no)
{
    int i;
    printf("[write pysically] %s\n", filename);
    for (i=0; i<16; i++)
        printf("0x%02X, ", buffer[i]);
    puts("");

    int fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);

    if (errno == ENOENT)
        perror("write - open");

    if (errno == EEXIST)
        fd = open(filename, O_RDWR);

    if (fd >= 0) {
        lseek(fd, cluster_no * FAT_CLUSTER_SIZE, SEEK_SET);
        write(fd, buffer + cluster_no * FAT_CLUSTER_SIZE, FAT_CLUSTER_SIZE);
        close(fd);

        return 1;
    }

    return -1;
}

//-----------------------------------------------------------------------------
// String Processing Functions
//-----------------------------------------------------------------------------
void get_filename_from_entry(struct fat_dir_entry *entry, char *filename)
{
    int i, j;
    char fn_no = 0;
    
    for (i=0; i<ENTRY_NAME_LENGTH; i++) {
        if (entry->name[i] == ENTRY_FILENAME_BLANK)
            continue;
        
        filename[fn_no++] = entry->name[i];
    }
    
    filename[fn_no++] = '.';
    
    for (j=0; j<ENTRY_EXTENDER_LENGTH; j++, i++) {
        if (entry->name[i] == ENTRY_FILENAME_BLANK)
            break;
        
        filename[fn_no++] = entry->name[i];
    }
    
    filename[fn_no] = NULL;
}

int fatfs_total_path_levels(char *path)
{
    int levels = 0;
    char expectedchar;
    
    if (!path)
        return -1;
    
    if (*path == '/') {
        expectedchar = '/';
        path++;
    }
    else if (path[1] == ':' || path[2] == '\\') {
        expectedchar = '\\';
        path += 3;
    }
    else
        return -1;
    
    // Count levels in path string
    while (*path) {
        // Fast forward through actual subdir text to next slash
        for (; *path; ) {
            // If slash detected escape from for loop
            if (*path == expectedchar) { path++; break; }
            path++;
        }
        
        // Increase number of subdirs founds
        levels++;
    }
    
    // Subtract the file itself
    return levels-1;
}

int fatfs_get_substring(char *path, int levelreq, char *output, int max_len)
{
    int i;
    int pathlen=0;
    int levels=0;
    int copypnt=0;
    char expectedchar;
    
    if (!path || max_len <= 0)
        return -1;
    
    if (*path == '/') {
        expectedchar = '/';
        path++;
    }
    else if (path[1] == ':' || path[2] == '\\') {
        expectedchar = '\\';
        path += 3;
    }
    else
        return -1;
    
    // Get string length of path
    pathlen = (int)strlen (path);
    
    // Loop through the number of times as characters in 'path'
    for (i = 0; i<pathlen; i++) {
        // If a '\' is found then increase level
        if (*path == expectedchar) levels++;
        
        // If correct level and the character is not a '\' or '/' then copy text to 'output'
        if ( (levels == levelreq) && (*path != expectedchar) && (copypnt < (max_len-1)))
            output[copypnt++] = *path;
        
        // Increment through path string
        path++;
    }
    
    // Null Terminate
    output[copypnt] = '\0';
    
    // If a string was copied return 0 else return 1
    if (output[0] != '\0')
        return 0;    // OK
    else
        return -1;    // Error
}

int fatfs_split_path(char *full_path, char *path, int max_path, char *filename, int max_filename)
{
    int strindex;
    
    // Count the levels to the filepath
    int levels = fatfs_total_path_levels(full_path);
    if (levels == -1)
        return -1;
    
    // Get filename part of string
    if (fatfs_get_substring(full_path, levels, filename, max_filename) != 0)
        return -1;
    
    // If root file
    if (levels == 0)
        path[0] = '\0';
    else {
        strindex = (int)strlen(full_path) - (int)strlen(filename);
        if (strindex > max_path)
            strindex = max_path;
        
        memcpy(path, full_path, strindex);
        path[strindex-1] = '\0';
    }
    
    return 0;
}

int fatfs_lfn_create_sfn(char *sfn_output, char *filename)
{
    int i;
    int dotPos = -1;
    char ext[3];
    int pos;
    int len = (int)strlen(filename);
    
    // Invalid to start with .
    if (filename[0]=='.')
        return 0;
    
    memset(sfn_output, ' ', FAT_SFN_SIZE_FULL);
    memset(ext, ' ', 3);
    
    // Find dot seperator
    for (i = 0; i< len; i++) {
        if (filename[i]=='.')
            dotPos = i;
    }
    
    // Extract extensions
    if (dotPos!=-1) {
        // Copy first three chars of extension
        for (i = (dotPos+1); i < (dotPos+1+3); i++)
            if (i<len)
                ext[i-(dotPos+1)] = filename[i];
        
        // Shorten the length to the dot position
        len = dotPos;
    }
    
    // Add filename part
    pos = 0;
    for (i=0;i<len;i++) {
        if ( (filename[i]!=' ') && (filename[i]!='.') )
                sfn_output[pos++] = filename[i];
        
        // Fill upto 8 characters
        if (pos==FAT_SFN_SIZE_PARTIAL)
            break;
    }
    
    // Add extension part
    for (i=FAT_SFN_SIZE_PARTIAL;i<FAT_SFN_SIZE_FULL;i++)
        sfn_output[i] = ext[i-FAT_SFN_SIZE_PARTIAL];
    
    return 1;
}

//-----------------------------------------------------------------------------
// Cloud Storage Related Functions
//-----------------------------------------------------------------------------
void download_metadata()
{
    char cmd[CMD_LEN_FULL] = "python ";
    strcat(cmd, path_list);
    strcat(cmd, " --path ");
    strcat(cmd, path_pipe);
    
    system(cmd);
}

void read_pipe(char *buffer)
{
    int fd = open(path_pipe, O_RDONLY);
    read(fd, buffer, PIPE_LEN_FULL);
    
    printf("Recieved : [\n%s\n]\n", buffer);
    
    close(fd);
}

void download_file(char *fid)
{
    printf("[download] %s\n", fid);
    
    char cmd[CMD_LEN_FULL] = "python ";
    strncat(cmd, path_downloader, strlen(path_downloader));
    strcat(cmd, " --fid ");
    strcat(cmd, fid);
    
    system(cmd);
}

void upload_file(char *filename)
{
    printf("[upload] %s\n", filename);
    char cmd[CMD_LEN_FULL] = "python ";
    
    strncat(cmd, path_uploader, strlen(path_uploader));
    strcat(cmd, " --fid ");
    strcat(cmd, filename);
    
    system(cmd);
}

//-----------------------------------------------------------------------------
// Fixed Area Creation Functions
//-----------------------------------------------------------------------------
void create_reserved_area()
{
    // ~ 512 Byte
    unsigned char reserved[] =
            {
                0xEB, 0x58, 0x90, 0x6D, 0x6B, 0x66, 0x73, 0x2E, 0x66, 0x61, 0x74, 0x00, 0x02, 0x08, 0x02, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x20, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x20, 0x00, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x80, 0x00, 0x29, 0x89, 0xA5, 0x0D, 0xEB, 0x4E, 0x4F, 0x20, 0x4E, 0x41, 0x4D, 0x45, 0x20, 0x20,
                0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20, 0x0E, 0x1F, 0xBE, 0x77, 0x7C, 0xAC,
                0x22, 0xC0, 0x74, 0x0B, 0x56, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0x5E, 0xEB, 0xF0, 0x32,
                0xE4, 0xCD, 0x16, 0xCD, 0x19, 0xEB, 0xFE, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6E,
                0x6F, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x64, 0x69,
                0x73, 0x6B, 0x2E, 0x20, 0x20, 0x50, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x20, 0x69, 0x6E, 0x73, 0x65,
                0x72, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x66, 0x6C,
                0x6F, 0x70, 0x70, 0x79, 0x20, 0x61, 0x6E, 0x64, 0x0D, 0x0A, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20,
                0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x74, 0x72, 0x79, 0x20, 0x61,
                0x67, 0x61, 0x69, 0x6E, 0x20, 0x2E, 0x2E, 0x2E, 0x20, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
            };

    memcpy(reserved_area, reserved, FAT_SECTOR_SIZE);
}

void create_fat_area()
{
    memset(fat_area, 0x00, FAT_FAT_AREA_FULL);

    unsigned char fat[] =
            {
                    0xF8, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0x0F, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00
            };

    memcpy(fat_area, fat, sizeof(unsigned char)*16);
}

void set_root_dir_entry()
{
    cluster_info[FAT_ROOT_DIRECTORY_FIRST_CLUSTER].attr = ATTR_DIR;
}

