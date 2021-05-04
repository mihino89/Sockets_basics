#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SOCKET_PATHNAME       "socket"
#define GENERAL_SIZE_LIMIT            255
#define NORMAL                        0
#define PRINT_USAGE                   1
#define ACT_LIKE_CLIENT               2
#define ACT_LIKE_DEAMON               3

int unix_domain_validate_socket_pathname(int argc, char const *argv[], char *unix_domain_pathname){
    int is_pathname_set = 0;
    int act_like_client = 0;
    int act_like_deamon = 0;
    int print_usage = 0;

    for (int i=1; i < argc; i++){
        // printf("argv: %s %d\n", argv[i], i);
        if(((strcmp(argv[i], "-u") == 0) || (strcmp(argv[i], "--path") == 0 )) && strlen(argv[i+1]) < GENERAL_SIZE_LIMIT){
            is_pathname_set = i;
            strcat(unix_domain_pathname, argv[i+1]);
            i++;
        }
        else if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--client") == 0 ))
            act_like_client = 1;
        else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--deamon") == 0 ))
            act_like_deamon = 1;
        else 
            print_usage = 1;
    }

    if (print_usage == 1 || (act_like_deamon == 1 && act_like_client == 1))
        return PRINT_USAGE;

    if (is_pathname_set == 0)
        strcpy(unix_domain_pathname, DEFAULT_SOCKET_PATHNAME);

    if (act_like_client == 1)
        return ACT_LIKE_CLIENT;
    
    if (act_like_deamon == 1)   
        return ACT_LIKE_DEAMON;
    
    return NORMAL;
}