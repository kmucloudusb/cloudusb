#ifndef __CONNMODULE_H_
#define __CONNMODULE_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/pid_namespace.h>
#include <asm/siginfo.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

struct request{
    int pid;
    unsigned int read_amount;
    unsigned int write_amount;
    loff_t read_file_offset;
    loff_t write_file_offset;
    char *write_buff;
};

struct read_export{
    unsigned int read_amount;
    char __user *read_buff;
    loff_t read_file_offset;
    ssize_t	nread;
};

struct write_export{
    unsigned int write_amount;
    char __user *write_buff;
    loff_t write_file_offset;
    ssize_t nwritten;
};

struct return_file{
    unsigned char *buff;
    int nread;
};

/* ioctl command */
#define INIT 0
#define RETURN_FILE 1
#define FILE_WRITE_OVER 2

/* Read & Write flag */
#define WAIT_HOST _IOW(0, 0, request)
#define EXECUTE_READ _IOW(0, 1, return_file)
#define EXECUTE_WRITE _IO(0, 2)

void perform_read(struct request *req, struct siginfo *info, struct task_struct *t);
void perform_write(struct request *req, struct siginfo *info, struct task_struct *t);

static int cloud_open(struct inode *inode, struct file *file);
static long cloud_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int cloud_release(struct inode *inode, struct file *filp);

const struct file_operations cloud_operations =
{
open: 		cloud_open,
unlocked_ioctl: 	cloud_ioctl,
release: cloud_release
};

#endif