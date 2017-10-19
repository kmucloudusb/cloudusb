#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PARAM_BUF_LEN 1024
#define MAX_USER_NUM 10


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

int save_user_auth_list_except_index(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN], const int index){
    FILE *fp;
    int i;

    fp = fopen("./driveAuth/user_auth_list.dat","w");

    fprintf(fp ,"%d\n", *num_user);
    for(i=0; i<*num_user; i++){
        if(i == index)
            continue;
        fprintf(fp ,"%s\n", user_auth_list[i]);
        printf("save_user_auth_list(): %s\n", user_auth_list[i]);
    }
    (*num_user)--;
    printf("save_user_auth_list(): USER_NUM = %d\n", *num_user);
    fclose(fp);
    return 0;
}

int add_user_auth_list(int *num_user, char (*user_auth_list)[PARAM_BUF_LEN], char *account_nickname){
    load_user_auth_list(num_user, user_auth_list);
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

int create_drive_auth_json(char *account_nickname, char *verification_code, char *users){
    char auth_python_path[PARAM_BUF_LEN] = "../../googledrive/authority/authority.py";
    char drive_auth_path[PARAM_BUF_LEN] = "./driveAuth/";
    char sys_command[PARAM_BUF_LEN] = {0};
    char user_auth_list[MAX_USER_NUM][PARAM_BUF_LEN];
    int num_user;

    // 1. 권한 json 생성
    // strcpy(auth_python_path, "../../googledrive/authority/authority.py")
    sprintf(sys_command, "echo %s | python %s --noauth_local_webserver",verification_code, auth_python_path);
    system(sys_command);

    // 2. 권한 json 복사
    sprintf(sys_command, "cp ~/.credentials/drive-python-quickstart.json %s%s_auth.json", drive_auth_path, account_nickname);
    system(sys_command);

    // 3. 유저를 리스트에 추가
    add_user_auth_list(&num_user, user_auth_list, account_nickname);

    // 4. 유저 리스트 스트링에 저장
    users_list_to_str(&num_user, user_auth_list, users);

    return 0;
}

int search_user_in_list(char *key_user ,const int num_user,char (*user_auth_list)[PARAM_BUF_LEN]){
    int i;
    for(i=0; i<num_user; i++){
        if(strcmp(key_user, user_auth_list[i]) == 0){
            printf("search_user_in_list(): find %s\n", key_user);
            return i;
        }
    }
    printf("search_user_in_list(): not exist %s\n", key_user);
    return -1;
}

int change_drive_auth_json(char *account_nickname){
    char drive_auth_path[PARAM_BUF_LEN] = "./driveAuth/";
    char sys_command[PARAM_BUF_LEN] = {0};
    char user_auth_list[MAX_USER_NUM][PARAM_BUF_LEN];
    int num_user;
    int ret;

    //유저 로드, 검색
    load_user_auth_list(&num_user, user_auth_list);
    ret = search_user_in_list(account_nickname ,num_user, user_auth_list);
    // 존재하지 않는 유저(해당 이름의 credential 파일이 없음)
    if(ret < 0)
        return -1;
    // 구글 API를 위한 drive_auth.json 을 앱에서 요청한 계정으로 바꿈
    sprintf(sys_command, "cp %s%s_auth.json ~/.credentials/drive-python-quickstart.json", drive_auth_path, account_nickname);
    system(sys_command);
    return 0;
}

int get_auth_users_list(char *users){
    int ret;
    char user_auth_list[MAX_USER_NUM][PARAM_BUF_LEN];
    int num_user;

    ret = load_user_auth_list(&num_user, user_auth_list);
    if(ret < 0){
        return -1;
    }
    ret = users_list_to_str(&num_user, user_auth_list, users);
    if(ret < 0){
        return -1;
    }
    return 0;
}

int delete_drive_auth_json(char *account_nickname){
    char drive_auth[PARAM_BUF_LEN] = {0};
    char user_auth_list[MAX_USER_NUM][PARAM_BUF_LEN];
    int num_user;
    int ret;

    load_user_auth_list(&num_user, user_auth_list);
    ret = search_user_in_list(account_nickname ,num_user, user_auth_list);
    // 존재하지 않는 유저(해당 이름의 credential 파일이 없음)
    if(ret < 0)
        return -1;
    //해당 유저 제외(삭제)하고 리스트파일 저장
    save_user_auth_list_except_index(&num_user, user_auth_list, ret);

    //해당 유저 drive_auth.json 삭제
    sprintf(drive_auth, "./driveAuth/%s_auth.json", account_nickname);
    ret = remove(drive_auth);
    if(ret < 0)
        return -1;

    return 0;
}

int main(){
    int ret;
    
    char users[PARAM_BUF_LEN] = {0};
    char account_nickname[1024] = "jinheesang4";
    char verification_code[1024] = "4/C6hQRk1YuTYjyw5C-wuOwwe6UdjSM9JPSebSekreuxE";

    if(ret < 0){
        printf("init_user_auth_list(): ERROR\n");
        return -1;
    }

    //create_drive_auth_json(account_nickname, verification_code, users);
    //change_drive_auth_json(account_nickname);
    get_auth_users_list(users);
    //delete_drive_auth_json(account_nickname);
}