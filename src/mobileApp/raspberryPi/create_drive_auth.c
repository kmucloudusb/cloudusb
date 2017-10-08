#include <stdlib.h>
#include <stdio.h>


#define PARAM_BUF_LEN 1024

int create_drive_auth_json(char *account_nickname, char *verification_code){
    char auth_python_path[PARAM_BUF_LEN] = "../../googledrive/authority/authority.py";
    char drive_auth_path[PARAM_BUF_LEN] = "./driveAuth/";
    char sys_command[PARAM_BUF_LEN] = {0};

    // 1. 권한 json 생성
    // strcpy(auth_python_path, "../../googledrive/authority/authority.py")
    sprintf(sys_command, "echo %s | python %s --noauth_local_webserver",verification_code, auth_python_path);
    system(sys_command);

    // 2. 계정 리스트 추가
    sprintf(sys_command, "cp ~/.credentials/drive-python-quickstart.json %s%s_auth.json", drive_auth_path, account_nickname);
    system(sys_command);

    return 0;
}

int main(){
    char account_nickname[1024] = "jinheesang";
    char verification_code[1024] = "4/0ZvJ2dTVRm2oqXsDacgCNXXS1KD1a1EOtWt4cg7fV10";

    create_drive_auth_json(account_nickname, verification_code);
}