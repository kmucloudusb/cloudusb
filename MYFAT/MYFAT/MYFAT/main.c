#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "header.h"

int in_file;
int out_file;

int media_init(char *media_path);
int media_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int media_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
void media_close();

int read_write_entry(void);

void read_pipe(char *pipe_path);

int main(int argc, char *argv[])
{
    if (media_init("/Users/lunahc92/Desktop/test.dmg") != 1)
    {
        puts("media_init error");
        return -1;
    }
    
    fl_init();
    
    if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    read_write_entry();
    
    fl_listdirectory("/");

    media_close();
}

int media_init(char *media_path)
{
    if((in_file = open(media_path, O_RDONLY)) < 0)
    {
        perror(media_path);
        return -1;
    }
    
    if((out_file = open(media_path, O_WRONLY)) < 0)
    {
        perror(media_path);
        return -1;
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

void media_close(){
    close(out_file);
    close(in_file);
}

int read_write_entry(void)
{
    // need to read from google drive
    FILE *fp = fopen("/Users/lunahc92/Desktop/list.txt", "r");
    /*
     list.txt <filename> <filesize>
     ex)
     /tx1.txt 100
     /tx2.txt 300
     /bi1.bin 200
     /bi2.bin 400
     */
    
    int ret;
    int count = 0;
    uint32 fsize;
    char filename[FAT_SFN_SIZE_FULL];
    
    do
    {
        count ++;
        ret = fscanf(fp, "%s %ud", filename, &fsize);
        func(filename, fsize);
    }
    while(ret != -1);
    
    return count;
}

void read_pipe(char *pipe_path)
{
#define MAX_BUF 1024
    
    int fd;
    char buf[MAX_BUF];
    
    fd = open(pipe_path, O_RDONLY);
    read(fd, buf, MAX_BUF);
    printf("Received: %s\n", buf);
    close(fd);
}
