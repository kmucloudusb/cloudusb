#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fat_filelib.h"
#include "fat_custom.h"

#include <signal.h>
#include <errno.h>
#include <sys/types.h>

int media_init(char *media_path);
int media_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int media_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
void media_close();

#define INIT 0
#define RETURN_FILE 1

typedef struct initS{
    int pid;
    unsigned int *amount;
    loff_t *file_offset_tmp;
} inits;
//
//typedef struct return_fileS{
//    char *buf;
//} return_files;

inits init;
unsigned int amount;
loff_t file_offset_tmp;

char buffer[4096*10];

int main(int argc, char *argv[])
{
    system("sudo insmod mymodule.ko");
    system("./makenode.sh");
    // Read paths to image, script, pipe
    read_path(argv[0]);
    
    // Create FAT32 image file (1GB)
    create_image();
    
    // Open image file
    open_image();
    
    // Initialise File IO Library
    fl_init();
    
    // Attach media access functions to library
    if (fl_attach_media(read_image, write_image) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1;
    }
    
    // Download metadata from google drive
    download_metadata();
    
    // Write metadata on image file & Make allocation table
    make_metadata();
    
//    download_from_g_drive(char *fileid);
//    read_file(int fd, uint32 sector, uint32 *buffer, uint32 sector_count);
//    ans_request(uint32 offset, uint32 *buffer, uint32 offset_count);
    
    
    // List root directory
    fl_listdirectory("/");
    
    system("sudo modprobe g_mass_storage file=/piusb.bin stall=0");
    
    int ret = 0;
    int fd = -1;
    
    /* 모든 구글API설정이 끝난 후 */
    signal(SIGCONT, file_transfer); //
    
    /* 커널모듈로 전송 */
    init.pid = getpid();
    printf("User Pid is %d\n", init.pid);
    init.amount = &amount;
    init.file_offset_tmp = file_offset_tmp;
    if((fd = open("/dev/CloudUSB", O_RDWR)) < 0 )
    {
        printf("Device Open failed!!\r\n");
        printf("%d\n", errno);
        return -1;
    }
    printf("구조체 주소: %p\n", &init);
    ret = ioctl(fd, INIT, &init);
    if(ret < 0)
        printf("Error in IOCTL errno: %d\r\n", errno);
    
    /* pause로 무한대기. signal이 들어오면 해당함수로 */
    while(1)
        pause();

    
    
    close_image();
    
    fl_shutdown();
}


/*
 * 시그널이 발생하면 실행될 함수.
 * 실행흐름이 이 함수로 넘어왔다가 다시 main함수로 돌아간다.
 * signo: 어떤 시그널에의해 발생했는지, 중요x.
 */
void file_transfer(int signo){
    int ret = 0;
    printf("시그널 전달받음\n");
    ans_request(*file_offset_tmp, buffer, *amount);

    /* 파일을 전송하기 위해 구글로부터 다운로드 및 구조체 세팅 */
//    printf("유저프로그램 시그널 전달받음\n");
//    printf("block_require = %d\n", block_require);
    
    
    /* ioctl로 파일정보가 담긴 구조체 전달 */
    ret = ioctl(fd, RETURN_FILE, buffer);
    if(ret < 0)
        printf("Error in IOCTL errno: %d\r\n", errno);
    //ioctl(fd, RETURN_FILE, file)
}

