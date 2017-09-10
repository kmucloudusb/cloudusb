#include <stdio.h>
#include <linux/unistd.h>
#include <signal.h>
#include <linux/wait.h>
#include <sys/types.h>
#include <linux/sched.h>
//#include <pthread.h>
//#include <string.h>

/** for IOCTL **/
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void after_signal();

int main(void){
    signal(SIGUSR1, after_signal); // SIGUSR1시그널이 실행될때 after_signal함수를 실행한다.
    
    DECLARE_WAIT_QUEUE_HEAD(wq);
    int need_to_wait = 1;
    
    //interruptible_sleep_on(&waitqueue);
    DECLARE_WAITQUEUE(entry, current); // 현재 태스크로 entry항목을 만든다.
    
    if (need_to_wait) {
        //add_wait_queue(&wq, &entry); // wq 웨이팅큐에 entry항목을 집어넣고 재운다.
        for (;;) {
            add_wait_queue(&wq, &entry); // wq 웨이팅큐에 entry항목을 집어넣고 재운다.
            set_current_state(TASK_INTERRUPTIBLE);
            if (!need_to_wait)
                break;
            schedule(); // 프로세스를 양보한다.
            if (signal_pending(current)) {
                remove_wait_queue(&wq, &entry);
                return -EINTR; /* or -ERESTARTSYS */
            }
        }
        set_current_state(TASK_RUNNING); // 프로세스 실행상태를 running으로 정의.
        remove_wait_queue(&wq, &entry);
    }
    
    return 0;
}

int after_signal(){
    //ioctl 처리함수
    int fd = -3;
    int ret = 0;
    printf("before open\n");
    //fd = open("/dev/CloudUSB", O_RDWR);
    if( (fd = open("/dev/CloudUSB", O_RDWR)) < 0 )
    {
        printf("Device Open failed!!\r\n");
        printf("%d\n", errno);
        return -1;
    }
    printf("after open\n");
    ret = ioctl(fd,0,NULL);
    if( ret != 0)
    {
        printf("Error in IOCTL ..DeviceStart command\r\n");
        
    }
    printf("after ioctl\n");
}

void garbage(){
    DECLARE_WAIT_QUEUE_HEAD(wq);
    DECLARE_WAITQUEUE(wait, current); // 현재 태스크로 wait항목을 만든다.(wait관련 구조체 정의)
    
    for (;;) {
        add_wait_queue(&wq, &wait); // wq 웨이팅큐에 wait항목을 집어넣고 재운다.
        set_current_state(TASK_INTERRUPTIBLE);
        //        if (condition)
        //            break;
        schedule(); // 프로세스를 양보한다.
        remove_wait_queue(&wq, &wait); // signal_pending
        if (signal_pending(current)) // 인자로 주어진 현재 프로세스가 시그널을 받았는지
            return -ERESTARTSYS;
    }
    set_current_state(TASK_RUNNING);
}
