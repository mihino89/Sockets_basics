#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SOCKET_PATHNAME       "socket"


void unix_domain_validate_socket_pathname(int argc, char const *argv[], char *unix_domain_pathname){

    if (argc <= 1 || (strcmp(argv[1], "-u") != 0) || (strcmp(argv[1], "--path") != 0 )){
        strcpy(unix_domain_pathname, DEFAULT_SOCKET_PATHNAME);
        return;
    } else if ((strcmp(argv[1], "-u") == 0) || (strcmp(argv[1], "--path") == 0 )){
        if(strlen(argv[2]) >= 255){
            strcpy(unix_domain_pathname, argv[2]);
        }
    }
}