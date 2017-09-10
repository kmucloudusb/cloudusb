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

/* ioctl command */
#define INIT 0
#define RETURN_FILE 1
#define FILE_WRITE_OVER 2

MODULE_LICENSE("GPL");

struct module_init{
    int pid;
    unsigned int amount;
    loff_t file_offset;
};

struct return_file{
    unsigned char *buf;
    int nread;
};

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