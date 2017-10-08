#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PARAM_BUF_LEN 1024

char user_auth_list[10][PARAM_BUF_LEN];
int num_user = 0;

// user_auth_list.dat에 유저 리스트 관리
// 인증파일(json)을 저장해놓은 유저 목록
int load_user_auth_list(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN]){
    FILE *fp;
    int i;
    
    fp = fopen("./driveAuth/user_auth_list.dat","r");
    if(fp == NULL){
        perror("fopen");
        return -1;
    }

    fscanf(fp, "%d\n", num_user);
    for(i=0; i<*num_user; i++){
        fscanf(fp ,"%s\n", user_auth_list[i]);
        printf("init_user_auth_list(): %s\n", user_auth_list[i]);
    }
    printf("init_user_auth_list(): USER_NUM = %d\n", *num_user);

    fclose(fp);
    return 0;
}


int save_user_auth_list(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN]){
    FILE *fp;
    int i;

    fp = fopen("./driveAuth/user_auth_list.dat","w");

    fprintf(fp ,"%d\n", *num_user);
    for(i=0; i<*num_user; i++){
        fprintf(fp ,"%s\n", user_auth_list[i]);
        printf("save_user_auth_list(): %s\n", user_auth_list[i]);
    }
    printf("save_user_auth_list(): USER_NUM = %d\n", *num_user);
    fclose(fp);
    return 0;
}

int add_user_auth_list(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN], char *account_nickname){
    strcpy(user_auth_list[*num_user], account_nickname);
    (*num_user)++;

    printf("add_user_auth_list(): %s\n", account_nickname);
    printf("add_user_auth_list(): USER_NUM = %d\n", *num_user);

    // 유저 리스트 파일에 저장(동기화)
    save_user_auth_list(num_user, user_auth_list);
    return 0;
}


int users_list_to_str(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN], char *users){
    // 유저 목록을 스트링으로
    int i;
    sprintf(users,"%d",*num_user);
    strcat(users, "@");
    for(i=0; i<*num_user; i++){
        strcat(users, user_auth_list[i]);
        strcat(users, "@");
    }

    printf("user_list_to_str(): %s\n",users);
    return 0;
}


int create_drive_auth_json(char *account_nickname, char *verification_code){
    char auth_python_path[PARAM_BUF_LEN] = "../../googledrive/authority/authority.py";
    char drive_auth_path[PARAM_BUF_LEN] = "./driveAuth/";
    char sys_command[PARAM_BUF_LEN] = {0};

    // 1. 권한 json 생성
    // strcpy(auth_python_path, "../../googledrive/authority/authority.py")
    sprintf(sys_command, "echo %s | python %s --noauth_local_webserver",verification_code, auth_python_path);
    system(sys_command);

    // 2. 권한 json 복사
    sprintf(sys_command, "cp ~/.credentials/drive-python-quickstart.json %s%s_auth.json", drive_auth_path, account_nickname);
    system(sys_command);

    // 3. 유저를 리스트에 추가
    add_user_auth_list(&num_user, user_auth_list, account_nickname);

    return 0;
}

int main(){
    int ret;
    
    char users[PARAM_BUF_LEN] = {0};
    char account_nickname[1024] = "jinheesang3";
    char verification_code[1024] = "4/fuYxI_iN0RIIlol9xmjmMKB3ifLfHCk1I0fpslrPP2M";

    ret = load_user_auth_list(&num_user, user_auth_list);
    if(ret < 0){
        printf("init_user_auth_list(): ERROR\n");
        return -1;
    }

    //create_drive_auth_json(account_nickname, verification_code);
    // add_user_auth_list(&num_user, user_auth_list, "KangKang");
    // add_user_auth_list(&num_user, user_auth_list, "JINJIN");
    // add_user_auth_list(&num_user, user_auth_list, "MAN");

    users_list_to_str(&num_user, user_auth_list, users);
}