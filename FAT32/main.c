#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "fat_filelib.h"

int in_file;
int out_file;

int media_init(char *media_path)
{
    
    if((in_file = open(media_path, O_RDONLY)) < 0)
    {
        perror(media_path);
        exit(1);
    }
    
    if((out_file = open(media_path, O_WRONLY)) < 0)
    {
        perror(media_path);
        exit(1);
    }
    
    return 1;
}

int media_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;
    
    for (i=0;i<sector_count;i++)
    {
        lseek(in_file, 512*sector, SEEK_SET);
        
        if(read(in_file, buffer, 512) < 0)
        {
            perror("read");
            return 0;
        }
        
        sector ++;
        buffer += 512;
    }
    
    return 1;
}

int media_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
{
    unsigned long i;
    
    for (i=0;i<sector_count;i++)
    {
        lseek(out_file, 512*sector, SEEK_SET);
        
        if(write(out_file, buffer, 512) < 0)
        {
            perror("write");
            return 0;
        }
        
        sector ++;
        buffer += 512;
    }
    
    return 1;
}

void show_file_status(FL_FILE *file){
    
    printf("\nshortfile name = %s\n", file->shortfilename);
    printf("file name = %s\n", file->filename);
    printf("path = %s\n", file->path);
    printf("parentcluster = %d\n", file->parentcluster);
    printf("startcluster = %d\n", file->startcluster);
    printf("bytenum = %d\n", file->bytenum);
    printf("filelength = %d\n", file->filelength);
    printf("filelength_changed = %d\n", file->filelength_changed);
    
    printf("file_data_address = %d\n", file->file_data_address);
    printf("file_data_dirty = %d\n", file->file_data_dirty);
    
}

/*
 For test...
 
 Folder structure
 /folder1
 /folder2/folder3
 /folder4/folder5/folder6
 
 Files
 /tmp10.bin - (10bytes)
 /folder1/tmp30.bin - (30bytes)
 /folder2/folder3/tmp50.bin - (50bytes)
 /folder4/folder5/tmp70.bin - (70bytes)
 /folder4/folder5/folder6/tmp100.bin - (100bytes)
 /tmp1G.bin - (1GB)
 /tmp4G.bin - (4GB)
 
 */
void test()
{
    fl_createdirectory("/folder1");
    fl_createdirectory("/folder2");
    fl_createdirectory("/folder2/folder3");
    fl_createdirectory("/folder4");
    fl_createdirectory("/folder4/folder5");
    fl_createdirectory("/folder4/folder5/folder6");
    
    FL_FILE *f1 = fl_fopen("/tmp10.bin", "w");
    FL_FILE *f2 = fl_fopen("/folder1/tmp30.bin", "w");
    FL_FILE *f3 = fl_fopen("/folder2/tmp50.bin", "w");
    FL_FILE *f4 = fl_fopen("/folder4/folder5/tmp70.bin", "w");
    FL_FILE *f5 = fl_fopen("/folder4/folder5/folder6/tmp100.bin", "w");
    FL_FILE *f6 = fl_fopen("/tmp1G.bin", "w");
    FL_FILE *f7 = fl_fopen("/tmp4G.bin", "w");
    
    f1->filelength = 10;
    f1->filelength_changed = 1;
    fl_fclose(f1);
    
    f2->filelength = 30;
    f2->filelength_changed = 1;
    fl_fclose(f2);
    
    f3->filelength = 50;
    f3->filelength_changed = 1;
    fl_fclose(f3);
    
    f4->filelength = 70;
    f4->filelength_changed = 1;
    fl_fclose(f4);
    
    f5->filelength = 100;
    f5->filelength_changed = 1;
    fl_fclose(f5);
    
    f6->filelength = 1000000000;
    f6->filelength_changed = 1;
    fl_fclose(f6);
    
    f7->filelength = 4000000000;
    f7->filelength_changed = 1;
    fl_fclose(f7);
}

/*
 For test...
 
 From PC to FAT32 file system storage
 
 ex)
 file_copy_test("/screen.png", "/Users/Desktop/world/screen.png");
 
 */
void file_copy_test(char *to, char *from){
    
    // File open
    FILE *origin;
    if ((origin = fopen(from, "rb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    
    // Get file size
    fseek(origin, 0, SEEK_END);
    int fsize = (int)ftell(origin);
    fseek(origin, 0, SEEK_SET);
    
    // Read file
#define MAX_BUF 1000000
    
    char buffer[MAX_BUF];
    
    int c;
    for (int i = 0; i < fsize; ++i)
    {
        c = getc(origin);
        
        if (c == EOF)
        {
            buffer[i] = 0x00;
            break;
        }
        
        buffer[i] = c;
    }
    
    // Open or create written file
    FL_FILE *file;
    file = fl_fopen(to, "w");
    
    if (file)
    {
        // Write some data
        if (fl_fwrite(buffer, 1, fsize, file) < fsize)
            printf("ERROR: Write file failed\n");
    }
    else
        printf("ERROR: Create file failed\n");
    
    // Close file
    fl_fclose(origin);
    fl_fclose(file);
}

int main(int argc, char *argv[])
{
    FL_FILE *file;
    
    if (argc<2){
        printf("Usage: \"sudo ./main.out PATH\" \n");
        return 1;
    }

    // Initialise media
    media_init(argv[1]);
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    // List root directory
    fl_listdirectory("/");
    
    // Create File
    file = fl_fopen("/file.bin", "w");
    if (file)
    {
        // Write some data
        unsigned char data[] = { 1, 2, 3, 4 };
        if (fl_fwrite(data, 1, sizeof(data), file) != sizeof(data))
            printf("ERROR: Write file failed\n");
    }
    else
        printf("ERROR: Create file failed\n");
    
    // Close file
    fl_fclose(file);
    
    // Delete File
        if (fl_remove("/file.bin") < 0)
            printf("ERROR: Delete file failed\n");
    
    // List root directory
    fl_listdirectory("/");
    
    fl_shutdown();
    
leave:
    close(out_file);
    close(in_file);
}
