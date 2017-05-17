#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fat_filelib.h"
#include "fat_custom.h"

#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#define INIT 0
#define RETURN_FILE 1

struct module_init{
    int pid;
    unsigned int amount;
    long long file_offset;
};

struct return_file{
    char *buf;
    int nread;
};

struct module_init inits;
int fd = -1;

void file_transfer(int signo);

uint8 buffer[FAT_SECTOR_SIZE];

int main(int argc, char *argv[])
{
    // Read paths to script, pipe
    read_path(argv[0]);
    
    // Create boot record area
    create_boot_record();
    
    // Create fat area
    create_fat_area();
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(read_virtual, write_virtual) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    // Create root directory entry
    create_rootdir_entry();
    
    // Download metadata from google drive
    download_metadata();
    
    // Make allocation table
    write_entries();
    
    /* 모든 구글API설정이 끝난 후 */
    signal(SIGCONT, file_transfer);
    
    /* 커널모듈로 전송 */
    inits.pid = getpid();
    printf("User Pid is %d\n", inits.pid);
    
    if((fd = open("/dev/CloudUSB", O_RDWR)) < 0 )
    {
        puts("Device Open failed!!");
        printf("%d\n", errno);
        return -1;
    }
    
    printf("STRUCT ADDRESS : %p\n", &inits);
    
    int ret;
    ret = ioctl(fd, INIT, &inits);
    
    if(ret < 0)
        printf("Error in IOCTL1 errno: %d\r\n", errno);
    
    while(1)
        pause();
    
    fl_shutdown();
}

void file_transfer(int signo){
    int ret = 0;
    struct return_file files;
    printf("유저프로그램 시그널 전달받음\n");
    
    uint32 offset_count = inits.amount; // 블록요청 길이
    uint32 offset = inits.file_offset; // 블록요청 시작지점
    
    memset(buffer, 0x00, FAT_SECTOR_SIZE);
    read_requested(offset, buffer, offset_count);
    
    files.buf = buffer; // 파일정보를 담은 버퍼의 주소를 넣어줌
    files.nread = FAT_SECTOR_SIZE; // 파일정보를 담은 버퍼의 길이를 넣어줌
    
    ret = ioctl(fd, RETURN_FILE, &files); // 파일정보가 담긴 구조체 전달
    
    if(ret < 0)
        printf("Error in IOCTL2 errno: %d\r\n", errno);
}
