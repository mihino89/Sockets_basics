#include "socket_basics.h"
#include "utilities.c"


int pid_arr[MAX_HOSTS];


void send_file(int cfd, char *in_buffer){
    char *data = (char *)malloc(COMMAND_LIMIT_SIZE * sizeof(char));
    char *command = (char *)malloc(COMMAND_LIMIT_SIZE * sizeof(char));
    char *out_buffer = (char *)calloc(COMMAND_LIMIT_SIZE, sizeof(char));
    FILE *fp;

    strncpy(command, in_buffer, strlen(in_buffer) - 1);
    strcat(command, " | tee send.txt");

    printf("command: %s\n", command);

    system(command);
    sleep(1);

    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    while(fgets(data, COMMAND_LIMIT_SIZE, fp) != NULL){
        strcat(out_buffer, data);
    }

    if (strlen(out_buffer) == 0){
        write(cfd, "\0", 1);
        return;
    }

    printf("%s %ld %ld\n", out_buffer, sizeof(out_buffer), strlen(out_buffer));
    write(cfd, out_buffer, strlen(out_buffer));
}


int connection_established(int cfd, int *pid_arr, int user_pid){
    char buffer_in[COMMAND_LIMIT_SIZE];
    char buffer_out[COMMAND_LIMIT_SIZE];
    char invite_message[] = "Hello  client!";

    write(cfd, invite_message, sizeof(invite_message));

    // infinite loop for chat
    while(1){
        read(cfd, buffer_in, sizeof(buffer_in));
        printf("Client PID %d request: %s", user_pid, buffer_in);

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("quit", buffer_in, 4) == 0) {
            printf("Client Exit...\n");
            break;
        } else if (strncmp("halt", buffer_in, 4) == 0){
            printf("Server Exit...\n");
            return EXIT_PROGRAM;
        } else {

            if ((strncmp("help", buffer_in, 4) == 0) || (strncmp("-h", buffer_in, 2) == 0)){
                strcpy(buffer_out, "Author: \tMartin Mihalovic\nSubject: \tSystemove programovanie a Assemblery");
                write(cfd, buffer_out, sizeof(buffer_out));
            } else {
                send_file(cfd, buffer_in);
            }

            bzero(buffer_in, COMMAND_LIMIT_SIZE);
            bzero(buffer_out, COMMAND_LIMIT_SIZE);
        }
    }

    return 0;
}


void init_pid_arr(int *pid_arr){
    for(int i=0; i < MAX_HOSTS; i++){
        *(pid_arr+i) = -1;
    }
}


// sigquit() function definition
// TODO - tu by som chcel este prejst pole childov a zavriet ich.
void sigquit(){
    for(int i=0; i < MAX_HOSTS; i++){
        printf("pid: %d\n", *(pid_arr+i));
        if(*(pid_arr+i) != -1)
            kill(*(pid_arr+i), SIGKILL); 
    }

    printf("My child has Killed me!!!\n");
    exit(0);
}


void unix_domain_deamon(char *unix_domain_pathname){

    char test[255];
    init_pid_arr(pid_arr);
    strcpy(test, unix_domain_pathname);
    
    int unix_domain_pathname_len = strlen(test);

    struct sockaddr_un server_add;
	struct sockaddr_un client_add;

    socklen_t s_len;
    int sfd, cfd, pid, status = 0, pid_counter = 0, recv_length=1;

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
    signal(SIGQUIT, sigquit);
    
	while(1){
		cfd = accept(sfd,(struct sockaddr *)&client_add,&s_len);

		if(cfd<0){
			perror("accept");
			return;
		}
        printf("Connectin accepted from %d\n", client_add.sun_family);
		
        if ((pid = fork()) == 0){
            close(sfd);
            status = connection_established(cfd, pid_arr, getpid());   

            if (status == 1){
                pid_t p_process_id;
                pid_t process_id;

                p_process_id = getppid();
                process_id = getpid();
                
                close(cfd);
                close(sfd);

                printf("Child: sending SIGQUIT\n");
                kill(p_process_id, SIGQUIT);

                return;
            }
        } 
        
        else if(pid > 0){
            printf("teraz rodic uloz: %d %d\n", *(pid_arr+pid_counter), pid);
            *(pid_arr+pid_counter) = (pid-1);
 
            pid_counter++;
            close(cfd);
        } else {
            printf("fork Failed\n");
            exit(EXIT_FAILURE);
        }     
	}
}


int main(int argc, char const *argv[]){
    char *unix_domain_pathname = (char *)malloc(255 * sizeof(char));
    unix_domain_validate_socket_pathname(argc, argv, unix_domain_pathname);

    unix_domain_deamon(unix_domain_pathname);
    return 0;
}
