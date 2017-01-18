#include <stdio.h>
#include "fat_filelib.h"

#define MEDIA_PATH "/dev/disk1"

int in_file;
int out_file;

int media_init()
{
    
    if((in_file = open(MEDIA_PATH, O_RDONLY)) < 0)
    {
        perror("open in_file");
        exit(1);
    }
    
    if((out_file = open(MEDIA_PATH, O_WRONLY)) < 0)
    {
        perror("open out_file");
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
            goto leave;
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
            goto leave;
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



int main(void)
{
    FL_FILE *file;

    // Initialise media
    media_init();

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
