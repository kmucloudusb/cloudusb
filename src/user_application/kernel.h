#ifndef KERNEL_H
#define KERNEL_H

#define INIT _IOW('a', 0, struct request)
#define RETURN_FILE _IOW('a', 1, struct return_file)
#define FILE_WRITE_OVER _IO('a', 2)
#define BUFF_LEN_FULL 32*512

struct request
{
    int pid;
    unsigned int read_amount;
    unsigned int write_amount;
    long long read_file_offset;
    long long write_file_offset;
    char *write_buff;
};

struct return_file
{
    unsigned int *buf;
    int nread;
};

void read_requested(unsigned int offset, unsigned char *buffer, unsigned int offset_count);
void run_module();
void file_transfer(int signo);
void write_request(int signo);

#endif //KERNEL_H

