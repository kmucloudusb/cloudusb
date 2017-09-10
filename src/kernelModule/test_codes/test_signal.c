#include <stdio.h>
#include <unistd.h>
#include <signal.h>
//#include <sys/types.h>
//#include <stdilb.h>

void alarmhandler(int signo);

int main(){
	int sec = 0;
	signal(SIGALRM, alarmhandler);
	
    while(1){
        alarm(3);
        printf("알람설정후 pause실행\n");
        pause();
    }
	printf("실행되지 않을곳\n");
	return -1;
}
void alarmhandler(int signo){
	int sec = 0;
	while(sec < 2){
		sleep(1);
		printf("handler에서 %d초 경과\n", ++sec);
	}
}

#define INIT 0
#define RETURN_FILE 1
struct FILE{
    /* ioctl로 커널모듈에 전송할 파일정보가 담긴 구조체 */
};
struct FILE file;

void file_transfer(int signo);

int actual_program(){
    
    /* 모든 구글API설정이 끝난 후 */
    signal(SIGCONT, file_transfer); //
    
    /* 커널모듈로 전송 */
    int pid = getpid();
    ioctl(fd, INIT, pid);
    
    /* pause로 무한대기. signal이 들어오면 해당함수로 */
    while(1)
        pause(0);
}

/*
 * 시그널이 발생하면 실행될 함수.
 * 실행흐름이 이 함수로 넘어왔다가 다시 main함수로 돌아간다.
 * signo: 어떤 시그널에의해 발생했는지, 중요x.
 */
void file_transfer(int signo){
    /* 파일을 전송하기 위해 구글로부터 다운로드 및 구조체 세팅 */
    
    /* ioctl로 파일정보가 담긴 구조체 전달 */
    ioctl(fd, RETURN_FILE, file)
}














