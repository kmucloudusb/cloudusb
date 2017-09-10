#include "connModule.h"

int cloud_flag = 1;
unsigned int		amount = 0;
loff_t			file_offset = 0;
ssize_t			nread = 0;
char __user *buff;

/* for block request */
struct module_init *inits;
struct return_file *files;

/* for signal */
struct task_struct *t;
struct siginfo info;
int user_pid;

EXPORT_SYMBOL(cloud_flag);
EXPORT_SYMBOL(amount);
EXPORT_SYMBOL(file_offset);
EXPORT_SYMBOL(nread);
EXPORT_SYMBOL(buff);

/*-------------------------------------------------------------------------*/

int cloud_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "CloudUSB chdev open function called\n");
    return 0;
}
int cloud_release(struct inode *inode, struct file *filp) {
    printk (KERN_ALERT "CloudUSB chdev close function called \n");
    return 0;
}

long cloud_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk(KERN_ALERT "CloudUSB_con ioctl called\n");
    switch (cmd)
    {
        case INIT:
            printk(KERN_ALERT "CloudUSB_con ioctl get INIT\n");
            inits = (struct module_init *)(arg);
            user_pid = inits->pid;
            memset(&info, 0 ,sizeof(struct siginfo));
            info.si_signo = SIGUSR1;
            info.si_code = SI_QUEUE;
            rcu_read_lock();
            t = pid_task(find_vpid(user_pid), PIDTYPE_PID);
            rcu_read_unlock();
            if(t == NULL){
                printk(KERN_ALERT "CloudUSB_con no such pid\n");
                return -ENODEV;
            }
            printk(KERN_ALERT "CloudUSB_con init success\n");
            cloud_flag = 0;
            break;
        case RETURN_FILE:
            printk(KERN_ALERT "CloudUSB_con ioctl get RETURN_FILE\n");
            files = (struct return_file *)(arg);
            printk(KERN_ALERT "CloudUSB_con received file_buf: %p\n", files->buf);
            printk(KERN_ALERT "CloudUSB_con received file_nread: %d\n", files->nread);
            printk(KERN_ALERT "CloudUSB_con received file_content: ");
            int i;
            for(i=0;i<files->nread;i++){
                printk(KERN_CONT "%02x ", files->buf[i]);
            }
            printk(KERN_ALERT "\n");
            /* wait for logging */
            msleep(100);
            
            memcpy(buff, files->buf, files->nread);
            
            nread = files->nread;
            cloud_flag = 0;
            break;
        case FILE_WRITE_OVER:
            printk(KERN_ALERT "CloudUSB_con ioctl get FILE_WRITE_OVER\n");
    }
    printk(KERN_ALERT "CloudUSB wait next block request.\n");
    
    /* wait because of context problem */
    while(!cloud_flag){schedule_timeout_uninterruptible(0.001*HZ);}
    
    /* set block request struct and send signal */
    printk(KERN_ALERT "CloudUSB_con receive block request(after wait)\n");
    inits->amount = amount;
    inits->file_offset = file_offset;
    printk(KERN_ALERT "CloudUSB_con request file_offset: %lld\n", inits->file_offset);
    printk(KERN_ALERT "CloudUSB_con request amount: %u\n", inits->amount);
    send_sig_info(SIGUSR1, &info, t);
    return 0;
}

/*-------------------------------------------------------------------------*/

static struct cdev *cloud_cdev;
static dev_t dev;
int init_modules(void)
{
    int ret;
    dev = MKDEV(235, 0);
    ret = register_chrdev_region(dev, 1, "CloudUSB");
    cloud_cdev = cdev_alloc();
    cloud_cdev->ops = &cloud_operations;
    cloud_cdev->owner = THIS_MODULE;
    cdev_init(cloud_cdev, &cloud_operations);
    cdev_add(cloud_cdev, dev, 1);
    return 0;
}

void cleanup_modules(void)
{
    cdev_del(cloud_cdev);
    unregister_chrdev_region(235, 1);
    printk(KERN_ALERT "CloudUSB cleanup\n");
}

module_init(init_modules);
module_exit(cleanup_modules);

