#include "connModule.h"

volatile int cloud_flag = WAIT_HOST;


struct read_export *reads;
struct write_export *writes;


/* for block request */
struct request *req;
struct return_file *files;

/* for signal */
struct task_struct *t;
struct siginfo read_info;
struct siginfo write_info;
int user_pid;

EXPORT_SYMBOL(cloud_flag);
EXPORT_SYMBOL(reads);
EXPORT_SYMBOL(writes);

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
            memset(&read_info, 0 ,sizeof(struct siginfo));
            read_info.si_signo = SIGUSR1;
            read_info.si_code = SI_QUEUE;
            memset(&write_info, 0 ,sizeof(struct siginfo));
            write_info.si_signo = SIGUSR2;
            write_info.si_code = SI_QUEUE;
            rcu_read_lock();
            t = pid_task(find_vpid(user_pid), PIDTYPE_PID);
            rcu_read_unlock();
            if(t == NULL){
                printk(KERN_ALERT "CloudUSB_con no such pid\n");
                return -ENODEV;
            }
            reads = ((struct read_export)*)malloc(sizeof(struct read_export));
            writes = ((struct write_export)*)malloc(sizeof(struct write_export));
            printk(KERN_ALERT "CloudUSB_con init success\n");
            break;
        case RETURN_FILE:
            printk(KERN_ALERT "CloudUSB_con ioctl get RETURN_FILE\n");
            files = (struct return_file *)(arg);
            printk(KERN_ALERT "CloudUSB_con received file_buff: %p\n", files->buff);
            printk(KERN_ALERT "CloudUSB_con received file_nread: %d\n", files->nread);
            printk(KERN_ALERT "CloudUSB_con received file_content: ");
            int i;
            for(i=0;i<files->reads->nread;i++){
                printk(KERN_CONT "%02x ", files->buff[i]);
            }
            printk(KERN_ALERT "\n");
            
            /* wait for logging */
            msleep(100);
            
            memcpy(reads->read_buff, files->buff, files->nread);
            reads->nread = files->nread;
            break;
        case FILE_WRITE_OVER:
            printk(KERN_ALERT "CloudUSB_con ioctl get FILE_WRITE_OVER\n");
    }
    
    cloud_flag = WAIT_HOST;
    printk(KERN_ALERT "CloudUSB wait next block request.\n");
    
    /* wait because of context problem, no race condition */
    while(cloud_flag==WAIT_HOST){schedule_timeout_uninterruptible(0.001*HZ);}
    
    /* set block request struct and send signal */
    printk(KERN_ALERT "CloudUSB_con receive block request(after wait)\n");
    
    if(cloud_flag == EXECUTE_READ)
        perform_read(req, &read_info, t);
    else
        perform_write(req, &write_info, t);
    
    return 0;
}

void perform_read(struct request *req, struct siginfo *info, struct task_struct *t){
    printk(KERN_ALERT "CloudUSB_con request read_file_offset: %lld\n", req->read_file_offset);
    printk(KERN_ALERT "CloudUSB_con request read_amount: %u\n", req->read_amount);
    
    req->read_file_offset = reads->read_file_offset;
    req->read_amount = reads->read_amount;
    
    send_sig_info(SIGUSR1, info, t);
}

void perform_write(struct request *req, struct siginfo *info, struct task_struct *t){
    printk(KERN_ALERT "CloudUSB_con request write_file_offset: %lld\n", writes->write_file_offset);
    printk(KERN_ALERT "CloudUSB_con request write_amount: %u\n", writes->write_amount);
    printk(KERN_ALERT "CloudUSB_con received write_buff: %x\n", writes->write_buff);
    printk(KERN_ALERT "CloudUSB_con received req->write_buff: %x", req->write_buff);
    
    writes->nwritten = writes->write_amount; // 일단 그대로 넣어줌.
    req->write_file_offset = writes->write_file_offset;
    req->write_amount = writes->write_amount;
    memcpy(req->write_buff, writes->write_buff, writes->write_amount);
    
    printk(KERN_ALERT "CloudUSB_con req->write_file_offset: %lld\n", req->write_file_offset);
    printk(KERN_ALERT "CloudUSB_con req->write_amount: %u\n", req->write_amount);
    
    printk(KERN_ALERT "CloudUSB_con send write file_content: ");
    int i;
    for(i=0;i<writes->nwritten;i++){
        printk(KERN_CONT "%02x ", writes->write_buff[i]);
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

