#include "connModule.h"

int cloud_flag = WAIT_HOST;

/* read */
unsigned int read_amount = 0;
char __user *read_buff = NULL;
loff_t read_file_offset = 0;
ssize_t	nread = 0;


/* write */
unsigned int write_amount;
char __user *write_buff = NULL;
loff_t write_file_offset;
ssize_t nwritten = 0;


/* for block request */
struct request *req;
struct return_file *files;

/* for signal */
struct task_struct *t;
struct siginfo info;
int user_pid;

EXPORT_SYMBOL(cloud_flag);

EXPORT_SYMBOL(read_amount);
EXPORT_SYMBOL(read_buff);
EXPORT_SYMBOL(read_file_offset);
EXPORT_SYMBOL(nread);

EXPORT_SYMBOL(write_amount);
EXPORT_SYMBOL(write_buff);
EXPORT_SYMBOL(write_file_offset);
EXPORT_SYMBOL(nwritten);

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
            req = (struct request *)(arg);
            user_pid = req->pid;
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
            break;
        case RETURN_FILE:
            printk(KERN_ALERT "CloudUSB_con ioctl get RETURN_FILE\n");
            files = (struct return_file *)(arg);
            printk(KERN_ALERT "CloudUSB_con received file_buff: %p\n", files->buff);
            printk(KERN_ALERT "CloudUSB_con received file_nread: %d\n", files->nread);
            printk(KERN_ALERT "CloudUSB_con received file_content: ");
            int i;
            for(i=0;i<files->nread;i++){
                printk(KERN_CONT "%02x ", files->buff[i]);
            }
            printk(KERN_ALERT "\n");
            
            /* wait for logging */
            msleep(100);
            
            memcpy(read_buff, files->buff, files->nread);
            nread = files->nread;
            break;
        case FILE_WRITE_OVER:
            printk(KERN_ALERT "CloudUSB_con ioctl get FILE_WRITE_OVER\n");
    }
    
    cloud_flag = WAIT_HOST;
    printk(KERN_ALERT "CloudUSB wait next block request.\n");
    
    /* wait because of context problem, no race condition */
    while(!cloud_flag){schedule_timeout_uninterruptible(0.001*HZ);}
    msleep(100);
    
    /* set block request struct and send signal */
    printk(KERN_ALERT "CloudUSB_con receive block request(after wait)\n");
    if(cloud_flag == EXECUTE_READ)
        perform_read(req, &info, t);
    else
        perform_write(req, &info, t);
    
    return 0;
}

void perform_read(struct request *req, struct siginfo *info, struct task_struct *t){
    printk(KERN_ALERT "CloudUSB_con request read_file_offset: %lld\n", req->read_file_offset);
    printk(KERN_ALERT "CloudUSB_con request read_amount: %u\n", req->read_amount);
    
    req->read_file_offset = read_file_offset;
    req->read_amount = read_amount;
    
    send_sig_info(SIGUSR1, info, t);
}

void perform_write(struct request *req, struct siginfo *info, struct task_struct *t){
    printk(KERN_ALERT "CloudUSB_con request write_file_offset: %lld\n", write_file_offset);
    printk(KERN_ALERT "CloudUSB_con request write_amount: %u\n", write_amount);
    printk(KERN_ALERT "CloudUSB_con received write_buff: %x\n", write_buff);
    printk(KERN_ALERT "CloudUSB_con received req->write_buff: %x", req->write_buff);
    
    nwritten = write_amount; // 일단 그대로 넣어줌.
    req->write_file_offset = write_file_offset;
    req->write_amount = write_amount;
    memcpy(req->write_buff, write_buff, write_amount);
    
    printk(KERN_ALERT "CloudUSB_con send write file_content: ");
    int i;
    for(i=0;i<nwritten;i++){
        printk(KERN_CONT "%02x ", write_buff[i]);
    }
    printk(KERN_ALERT "\n");
    
    send_sig_info(SIGUSR2, info, t);
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

