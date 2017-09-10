#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

typedef struct FILES{
    int pid;
    int *block_require_p; // 이 변수를 init과정에서 넘겨주고 signal로 불려지면 출력해볼것.
} fileS;

int main(){
    fileS files;
    fileS *files_rep1;
    fileS files_rep2;
    int block_require = 0;
    files.block_require_p = &block_require;
    *files.block_require_p = 150;
    
    files.pid = getpid();
    
    unsigned long arg;
    arg = (unsigned long)(&files);
    printf("arg = %lu\n", arg);
    files_rep1 = (fileS *)(arg);
    files_rep2 = *(fileS *)(arg);
    
    printf("pointer0: %p\n", &files);
    printf("pointer1: %p\n", files_rep1);
    printf("pointer2: %p\n", &files_rep2);
//    files_rep1 = &files;
//    files_rep2 = files;
    printf("pid0: %d\n", files.pid);
    printf("pid1: %d\n", files_rep1->pid);
    printf("pid1: %d\n", files_rep2.pid);
    
    
    //files.block_require = 0;
    printf("구조체 주소: %p\n", &files);
    printf("변수 주소: %p\n", files.block_require_p);
    printf("변수 값: %d\n", *files.block_require_p);
}
