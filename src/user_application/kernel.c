#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include "kernel.h"
#include "file_system.h"

int module_fd;
unsigned char module_buffer[BUFF_LEN_FULL];
struct request inits;
char write_buffer[BUFF_LEN_FULL];

void read_requested(unsigned int offset, unsigned char *buffer, unsigned int offset_count)
{
    read_media(offset/FAT_SECTOR_SIZE, buffer, offset_count/FAT_SECTOR_SIZE);
}

void run_module()
{
    signal(SIGUSR1, file_transfer);
    signal(SIGUSR2, write_request);

    inits.write_buff = write_buffer;

    /* send to kernel module */
    inits.pid = getpid();
    printf("User Pid is %d\n", inits.pid);

    if((module_fd = open("/dev/CloudUSB", O_RDWR)) < 0) {
        puts("Device Open failed!!");
        printf("%d\n", errno);

        return ;
    }

    printf("STRUCT ADDRESS : %p\n", &inits);
    system("sh /home/pi/cloudusb/bin/load_module.sh &");

    if(ioctl(module_fd, INIT, &inits) < 0)
        printf("Error in IOCTL1 errno: %d\n", errno);

    while(1)
        pause();
}

void file_transfer(int signo)
{
    struct return_file files;

    unsigned int offset_count = inits.read_amount; // Block request length
    unsigned int offset = inits.read_file_offset; // Block request start point

    memset(module_buffer, 0x00, BUFF_LEN_FULL);
    read_requested(offset, module_buffer, offset_count);

    files.buf = module_buffer; // substitute buffer address which have file info
    files.nread = inits.read_amount; // substitute buffer length which have file info

    if(ioctl(module_fd, RETURN_FILE, &files) < 0)
        printf("Error in IOCTL2 errno: %d\n", errno);
}

void write_request(int signo)
{
    write_media(inits.write_file_offset / FAT_SECTOR_SIZE, inits.write_buff, inits.write_amount / FAT_SECTOR_SIZE);

    if(ioctl(module_fd, FILE_WRITE_OVER, NULL) < 0)
        printf("Error in IOCTL3 errno: %d\n", errno);
}
