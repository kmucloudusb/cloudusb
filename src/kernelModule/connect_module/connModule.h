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

/* ioctl command */
#define INIT _IOW('a', 0, struct request)
#define RETURN_FILE _IOW('a', 1, struct return_file)
#define FILE_WRITE_OVER _IO('a', 2)

/* Read & Write flag */
#define WAIT_HOST 0
#define EXECUTE_READ 1
#define EXECUTE_WRITE 2

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

int perform_read(struct request *req, struct siginfo *info, struct task_struct *t);
int perform_write(struct request *req, struct siginfo *info, struct task_struct *t);

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