#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* 
 * 이방법을 쓰지 않으려면 구글API를 사용할 준비가 완벽히 끝났을때 커널모듈로 ioctl을 이용해 PID를 보내야 한다.
 * 커널모듈은 IOCTL 커맨드 INIT(0)을 받으면 유저프로그램이 블록요청을 받을 준비가 끝났음을 알게된다.
 * 하지만 이방법은 커널 모듈이 PID를 받을때까지 기다리면서 USB연결시간을 지연시키므로 다른해결법을 찾아보는것이 좋다.
 */

void alarmhandler(int signo);

int main( void)
{
    sigset_t sigset;
    sigset_t oldset;
    
    int sec = 0;
    //alarm(2);
 
    
    /* 유저프로그램 실행흐름 최상단에 위치해야 한다. */
    signal(SIGALRM, alarmhandler);
    
    sigemptyset( &sigset);        // 시그널 집합 변수의 내용을 모두 제거합니다.
    sigaddset( &sigset, SIGALRM);  // 시그널 집합 변수에 SIGINT를 추가합니다.
    sigprocmask( SIG_BLOCK, &sigset, &oldset); // sigset에 들어간 SIGALRM을 블록한다.
    /******************************/
    
    /* 구글API를 사용하기위한 이런저런 설정 */
    while(sec < 2){
        sleep(1);
        printf("설정중 %d초경과\n", ++sec);
    }
    alarm(0); // 이런저런 설정중에 signal이 발생해버린다면?
    while(sec < 5){
        sleep(1);
        printf("설정중 %d초경과\n", ++sec);
    }
    /******************************/
    alarm(1);
    sigsuspend( &oldset); // 다시 비어있는 sigoldset를 받는다.
    
    printf("종료\n");
    return 0;
}

void alarmhandler(int signo){
    printf("일어나세요\n");
    int sec = 0;
    while(sec < 2){
        sleep(1);
        printf("%d초 핸들러\n", ++sec);
    }
}





