#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "socket_basics.h"

#define DEFAULT_SOCKET_PATHNAME       "socket"
#define COMMAND_LIMIT_SIZE            1024


void connection_established(int sfd){

    char buffer[COMMAND_LIMIT_SIZE];
    int n = 0;

    bzero(buffer, COMMAND_LIMIT_SIZE);
    read(sfd, buffer, sizeof(buffer));
    printf("---- Connection Established ----\n");

    while (1){
        bzero(buffer, COMMAND_LIMIT_SIZE);
        printf("client@groundzero: ");
        
        n = 0;
        while ((buffer[n++] = getchar()) != '\n')
            ;
        
        write(sfd, buffer, sizeof(buffer));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buffer, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }

        bzero(buffer, COMMAND_LIMIT_SIZE);
        read(sfd, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
}



void unix_domain_client(char *unix_domain_pathname){

    struct sockaddr_un addr;
    char buffer[1024];
    char s_buffer[1024];
    int cfd, s_length, recv_length = 1, quit = 0;

    if((cfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
		return;
    }

    printf("Client socket fd = %d\n", cfd);

    /**
     * Construct server address, and make the connection. 
    */
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, unix_domain_pathname, sizeof(addr.sun_path) - 1);

    
    if(connect(cfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1){
        perror("connect");
    }

    connection_established(cfd);
}


void unix_domain_validate_socket_pathname(int argc, char const *argv[], char *unix_domain_pathname){

    if (argc <= 1 || (strlen(argv[1]) >= 255)){
        strcpy(unix_domain_pathname, DEFAULT_SOCKET_PATHNAME);
        return;
    } 

    strcpy(unix_domain_pathname, argv[1]);
}


int main(int argc, char const *argv[]){
    char *unix_domain_pathname = (char *)malloc(255 * sizeof(char));
    unix_domain_validate_socket_pathname(argc, argv, unix_domain_pathname);

    unix_domain_client(unix_domain_pathname);
    return 0;
}
