#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include "socket_basics.h"


#define DEFAULT_SOCKET_PATHNAME       "socket"
#define BACKLOG                       5
#define COMMAND_LIMIT_SIZE            1024


void connection_established(int cfd){
    char buffer[COMMAND_LIMIT_SIZE];
    char invite_message[] = "Hello  client!";

    write(cfd, invite_message, sizeof(invite_message));

    // infinite loop for chat
    while(1){
        bzero(buffer, COMMAND_LIMIT_SIZE);

        read(cfd, buffer, sizeof(buffer));
        printf("From client: %s\n", buffer);

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buffer, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }

        bzero(buffer, COMMAND_LIMIT_SIZE);
        strcpy(buffer, "Command not found");

        write(cfd, buffer, sizeof(buffer));
    }
}



void unix_domain_deamon(char *unix_domain_pathname){

    char test[255];
    strcpy(test, unix_domain_pathname);
    
    int unix_domain_pathname_len = strlen(test);

    struct sockaddr_un server_add;
	struct sockaddr_un client_add;

    socklen_t s_len;
    int sfd, cfd, pid, recv_length=1;

    memset(&server_add, 0, sizeof(struct sockaddr_un));
    server_add.sun_family = AF_UNIX;           
    strcpy(server_add.sun_path, unix_domain_pathname);

    s_len=sizeof(struct sockaddr_un);

    if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
		return;
    }
    printf("[*] Server socket us created (fd = %d)\n", sfd);

    // Delete any file that already exists at the address. Make sure the deletion
    // succeeds. If the error is just that the file/directory doesn't exist, it's fine.
    if (remove(unix_domain_pathname) == -1 && errno != ENOENT) {
        perror("remove");
    }

    if(bind(sfd, (const struct sockaddr *) &server_add, s_len) == -1){
        perror("bind");
        return;
    }
    printf("[*] Binded to %s local socket\n", unix_domain_pathname);

    /**
     * 5 oznacuje arg pre max velkost stacku (backlog queue)
    */
    if(listen(sfd, BACKLOG) < 0){
		perror("listen");
		return;
    }
    printf("[*] Listening...\n");
    
	while(1){
		cfd = accept(sfd,(struct sockaddr *)&client_add,&s_len);

		if(cfd<0){
			perror("accept");
			return;
		}
        printf("Connectin accepted from %s\n", client_add.sun_path);
		
        if ((pid = fork()) == 0){
            close(sfd);
            connection_established(cfd);              
        }  else if (pid < 0){
            printf("fork Failed\n");
            exit(1);
        }
        //else if (pid > 0){
        //    printf("Hello from the main proces, with PID: %d.\n", pid);
        //    close(cfd);
        //    printf("Zvysujem counter..\n");
        //} else {
        //    printf("Accepted socket fd = %d, with PID: %d\n", cfd, pid);
        //    connection_established(cfd);  
          //  close(cfd);
        //}        
	}
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

    unix_domain_deamon(unix_domain_pathname);
    return 0;
}
