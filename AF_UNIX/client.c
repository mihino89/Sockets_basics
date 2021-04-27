#include "socket_basics.h"
#include "utilities.c"


void get_shell_text(char *shell_text){
    char shell_text_1[5], shell_text_2[5];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(shell_text_1, "%02d:", tm.tm_hour);
    sprintf(shell_text_2, "%02d  ", tm.tm_min);

    strcat(shell_text, shell_text_1);
    strcat(shell_text, shell_text_2);

    char *user=getenv("USER");
    if(user == NULL)
        exit(EXIT_FAILURE);
    
    strcat(shell_text, user);
    strcat(shell_text, "@student#");
}


void connection_established(int sfd){

    char *shell_text = (char *)malloc(GENERAL_SIZE_LIMIT  * sizeof(char));
    char buffer[COMMAND_LIMIT_SIZE];
    int n = 0;

    bzero(buffer, COMMAND_LIMIT_SIZE);
    read(sfd, buffer, sizeof(buffer));
    printf("---- Connection Established ----\n");

    while (1){
        bzero(buffer, COMMAND_LIMIT_SIZE);

        get_shell_text(shell_text);
        printf("%s", shell_text);
        
        n = 0;
        while ((buffer[n++] = getchar()) != '\n')
            ;
        
        write(sfd, buffer, sizeof(buffer));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buffer, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }

        bzero(shell_text, GENERAL_SIZE_LIMIT);
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


int main(int argc, char const *argv[]){
    char *unix_domain_pathname = (char *)malloc(255 * sizeof(char));
    unix_domain_validate_socket_pathname(argc, argv, unix_domain_pathname);

    unix_domain_client(unix_domain_pathname);
    return 0;
}
