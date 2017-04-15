#include <stdio.h>
#include <stdlib.h>
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

int read_entry_from_pipe(char *pipe_path, char *script_path);
void read_pipe(char *pipe_path, char *buf);

void read_path(char *script_path, char *image_path, char *pipe_path, char *exec_path);

int main(int argc, char *argv[])
{
    char script_path[256];
    char image_path[256];
    char pipe_path[256];
    
    read_path(script_path, image_path, pipe_path, argv[0]);
    
    puts(script_path);
    puts(image_path);
    puts(pipe_path);
    
    if (media_init(image_path) != 1)
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
    
    read_entry_from_pipe(pipe_path, script_path);
    
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

int read_entry_from_pipe(char *pipe_path, char *script_path)
{
    char cmd[300] = "sudo python ";
    strcat(cmd, script_path);
    strcat(cmd, " --path ");
    strcat(cmd, pipe_path);
    system(cmd);
    
#define MAX_BUF 1024
    char buf[MAX_BUF];
    
    // read from pipe
    read_pipe(pipe_path, buf);
    
    int ret;
    int count = 0;
    int loc_buf_point = 0;
    uint32 fsize;
    char ch;
    char filename[FAT_SFN_SIZE_FULL];
    
    while(1)
    {
        count++;
        ret = sscanf(buf+loc_buf_point, "%s %u", filename, &fsize);
        write_file_on_media(filename, fsize);
        
        while((ch = *(buf+(loc_buf_point++))) != '\n' && ch != '\0');
        
        if(ch == '\0')
            break;
    }
    
    return count;
}

void read_pipe(char *pipe_path, char *buf)
{
    int fd;
    
    fd = open(pipe_path, O_RDONLY);
    read(fd, buf, MAX_BUF);
    
    printf("Recieved : [%s]\n", buf);
    
    close(fd);
}

void read_path(char *script_path, char *image_path, char *pipe_path, char *exec_path)
{
    strcpy(script_path, exec_path);
    strcpy(image_path, exec_path);
    strcpy(pipe_path, exec_path);
    
    strcpy(&(script_path[strlen(script_path)-8]), "../../../googledrive/list.py");
    strcpy(&(image_path[strlen(image_path)-8]), "../../../image.dmg");
    strcpy(&(pipe_path[strlen(pipe_path)-8]), "../../../myfifo");
}
