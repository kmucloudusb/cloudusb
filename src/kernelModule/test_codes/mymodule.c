#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

//#include <sys/types.h>
#include <linux/signal.h>
//#include <linux/unistd.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/pid_namespace.h>
#include <asm/siginfo.h>    //siginfo
#include <linux/sched.h> // timeout
#include <linux/rcupdate.h> //rcu_read_lock

#define INIT 0
#define RETURN_FILE 1

struct module_init{
    int pid;
    unsigned int amount;
    loff_t file_offset;
};

struct return_file{
    char *buf;
    char nread;
};

struct module_init *inits;
struct return_file *files;

MODULE_LICENSE("GPL");

/********************************************************/

static int cloud_major = -1;
static int cloud_open(struct inode *inode, struct file *file);
static long cloud_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static const struct file_operations cloud_operations =
{
    .unlocked_ioctl = 	cloud_ioctl,
    .open = 		cloud_open,
};
static int cloud_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "CloudUSB file open function called\n");
    return 1;
}
extern loff_t file_offset;
extern unsigned int amount;
extern int cloud_flag;
extern char *buf;
extern ssize_t nread;
struct task_struct *t;
struct siginfo info;

static long cloud_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int user_pid;
    
    printk(KERN_ALERT "CloudUSB ioctl called\n");
    switch (cmd)
    {
        case INIT:
            //printk(KERN_ALERT "CloudUSB ioctl get INIT\n");
            //printk(KERN_ALERT "CloudUSB arg %lu\n", arg);
            //printk(KERN_ALERT "CloudUSB arg %p\n", (inits *)(arg));
            inits = (struct module_init *)(arg);
            user_pid = inits->pid;
            memset(&info, 0 ,sizeof(struct siginfo));
            info.si_signo = SIGCONT;
            info.si_code = SI_QUEUE;
            rcu_read_lock();
            //t = find_task_by_vpid(user_pid);
            t = pid_task(find_vpid(user_pid), PIDTYPE_PID);
            rcu_read_unlock();
            if(t == NULL){
                printk(KERN_ALERT "CloudUSB no such pid\n");
                return -ENODEV;
            }
            // send_sig_info(SIGCONT, &info, t);
            break;
        case RETURN_FILE:
            //printk(KERN_ALERT "CloudUSB ioctl get RETURN_FILE\n");
            files = (struct return_file *)(arg);
            buf = files->buf;
            nread = files->nread;
            cloud_flag = 0;
            break;
    }
    while(!cloud_flag){schedule_timeout_uninterruptible(0.001*HZ);} // 블록요청 들어올때까지 기다림.
    inits->amount = amount;
    inits->file_offset = file_offset;
    send_sig_info(SIGCONT, &info, t); // 필요한정보 구조체에 넣은후 블록요청 받았다고 유저프로그램에 알려주었다.
    return 0;
}

/**********************************************************/

int init_module(void)
{
    unregister_chrdev(231, "CloudUSB");
    cloud_major = register_chrdev(235, "CloudUSB", &cloud_operations);
    if (cloud_major < 0) {
        printk(KERN_ERR "CloudUSB - cannot register device\n");
        return cloud_major;
    }else{
        printk(KERN_ERR "Cloud chdev successfully registerd, major num\n");
    }
    return 0;
}

void cleanup_module(void)
{
    unregister_chrdev(235, "CloudUSB");
}


