#include "socket_basics.h"
#include "utilities.c"


int main(int argc, char const *argv[]){
    char *unix_domain_pathname = (char *)malloc(255 * sizeof(char));
    int status = unix_domain_validate_socket_pathname(argc, argv, unix_domain_pathname);

    printf("status: %d, socket pathname: %s\n", status, unix_domain_pathname);

    if (status == PRINT_USAGE)
        printf("[-] Usage: ./deamon \n\t Optional param: \"-u\" or \"--path\" To set pathname to socket\n\t Optional param: \"-c\" or \"--client\" To set act deamon like client\n");
    else if (status == ACT_LIKE_CLIENT)
        unix_domain_client(unix_domain_pathname);
    else
        unix_domain_deamon(unix_domain_pathname);
    
    return 0;
}
