#include <stdlib.h>
#include <stdio.h>

int create_client_secret_json(char *client_id, char *client_secret){
    FILE *csj_fp;
    char verfication_url[1024] = {0};

    csj_fp = fopen("../../googledrive/authority/client_secret.json","w");
    fprintf(csj_fp,"\
{\"installed\":\n\
    {\n\
        \"client_id\":\"%s\",\n\
        \"project_id\":\"goormUSB\",\n\
        \"auth_uri\":\"https://accounts.google.com/o/oauth2/auth\",\n\
        \"token_uri\":\"https://accounts.google.com/o/oauth2/token\",\n\
        \"auth_provider_x509_cert_url\":\"https://www.googleapis.com/oauth2/v1/certs\",\n\
        \"client_secret\":\"%s\",\n\
        \"redirect_uris\":[\"urn:ietf:wg:oauth:2.0:oob\",\"http://localhost\"]\n\
    }\n\
}"\
    , client_id, client_secret);

    sprintf(verfication_url, "https://accounts.google.com/o/oauth2/auth?scope=https%%3A%%2F%%2Fwww.googleapis.com%%2Fauth%%2Fdrive&redirect_uri=urn%%3Aietf%%3Awg%%3Aoauth%%3A2.0%%3Aoob&response_type=code&client_id=%s&access_type=offline", client_id);
    printf("%s\n", verfication_url);
    fclose(csj_fp);
    return 0;
}

int main(){
    char client_id[1024] = "24205605983-v17h0t4f0788cg3ldaknijstsa1eksm7.apps.googleusercontent.com";
    char client_secret[1024] = "fpijJpuR8koajdRUcuYuA5Ph";

    create_client_secret_json(client_id, client_secret);
}