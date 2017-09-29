//
// Created by 채한울 on 29/09/2017.
//

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

struct module_init inits;

int module_fd;
unsigned char module_buffer[BUFF_LEN_FULL];

void read_requested(unsigned int offset, unsigned char *buffer, unsigned int offset_count)
{
    read_media(offset/FAT_SECTOR_SIZE, buffer, offset_count/FAT_SECTOR_SIZE);
}

void run_module()
{
    signal(SIGUSR1, file_transfer);

    /* send to kernel module */
    inits.pid = getpid();
    printf("User Pid is %d\n", inits.pid);

    if((module_fd = open("/dev/CloudUSB", O_RDWR)) < 0)
    {
        puts("Device Open failed!!");
        printf("%d\n", errno);

        return ;
    }

    printf("STRUCT ADDRESS : %p\n", &inits);

    if(ioctl(module_fd, INIT, &inits) < 0)
        printf("Error in IOCTL1 errno: %d\n", errno);

    while(1)
        pause();
}

void file_transfer(int signo)
{
    struct return_file files;

    unsigned int offset_count = inits.amount; // Block request length
    unsigned int offset = inits.file_offset; // Block request start point

    memset(module_buffer, 0x00, BUFF_LEN_FULL);
    read_requested(offset, module_buffer, offset_count);

    files.buf = module_buffer; // substitute buffer address which have file info
    files.nread = inits.amount; // substitute buffer length which have file info

    if(ioctl(module_fd, RETURN_FILE, &files) < 0)
        printf("Error in IOCTL2 errno: %d\n", errno);
}