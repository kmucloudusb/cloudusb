#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
//#include <stdilb.h>

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

struct module_init inits;
int fd = -1;

void file_transfer(int signo);

int main(){
    int ret = 0;
    
    /* 모든 구글API설정이 끝난 후 */
    signal(SIGCONT, file_transfer); //
    
    /* 커널모듈로 전송 */
    inits.pid = getpid();
    printf("User Pid is %d\n", inits.pid);
    if((fd = open("/dev/CloudUSB", O_RDWR)) < 0 )
    {
        printf("Device Open failed!!\r\n");
        printf("%d\n", errno);
        return -1;
    }
    printf("구조체 주소: %p\n", &inits);
    ret = ioctl(fd, INIT, &inits);
    if(ret < 0)
        printf("Error in IOCTL1 errno: %d\r\n", errno);
    
    /* pause로 무한대기. signal이 들어오면 해당함수로 */
    while(1)
        pause();
    return 0;
}

/*
 * 시그널이 발생하면 실행될 함수.
 * 실행흐름이 이 함수로 넘어왔다가 다시 main함수로 돌아간다.
 * signo: 어떤 시그널에의해 발생했는지, 중요x.
 */
void file_transfer(int signo){
    int ret = 0;
    struct return_file files;
    printf("유저프로그램 시그널 전달받음\n");
    
    inits.amount; // 블록요청 길이
    inits.file_offset; // 블록요청 시작지점
    
    files.buf = NULL; // 파일정보를 담은 버퍼의 주소를 넣어줌
    files.nread = 0; // 파일정보를 담은 버퍼의 길이를 넣어줌
    
    ret = ioctl(fd, RETURN_FILE, &files); // 파일정보가 담긴 구조체 전달
    if(ret < 0)
        printf("Error in IOCTL2 errno: %d\r\n", errno);
}














