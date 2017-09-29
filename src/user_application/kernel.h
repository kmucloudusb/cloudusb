//
// Created by 채한울 on 29/09/2017.
//

#ifndef ALGORITHM_KERNEL_H
#define ALGORITHM_KERNEL_H

#define INIT 0
#define RETURN_FILE 1
#define BUFF_LEN_FULL 32*512

struct module_init
{
    int pid;
    unsigned int amount;
    long long file_offset;
};

struct return_file
{
    unsigned char *buf;
    int nread;
};

void read_requested(unsigned int offset, unsigned char *buffer, unsigned int offset_count);
void run_module();
void file_transfer(int signo);


#endif //ALGORITHM_KERNEL_H
