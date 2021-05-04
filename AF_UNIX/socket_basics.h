#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define DEFAULT_SOCKET_PATHNAME       "socket"
#define FILENAME                      "tmp.txt"
#define BACKLOG                       5
#define COMMAND_LIMIT_SIZE            4096
#define GENERAL_SIZE_LIMIT            255
#define EXIT_FAILURE                  1
#define EXIT_SUCCESS                  0

#define EXIT_PROGRAM                  1
#define MAX_HOSTS                     20

int pid_arr[MAX_HOSTS];

/**
 * (6b) Base functionality
 * (3b) It's working on the linux environment
 * (5b) Signal's used to kill all chain of children
 * (2b) Makefile is working
 * (1b) Comments in english
 * (3b) prompt prikaz on client
 * (3b) switch -c to run as a client
 * (4b) switch -d to run as a deamon
 * (2b) switch -i (in my case --path/-u)
 * (4b) Support for char "*" in grep, list, etc.
 * -> 33
*/

/**
 * Tested commands:
 *  ls
 *  ls;
 *  ls *.c
 *  ls | grep .txt
 *  ls -la
 *  ls ../../
 *  touch test.txt
 *  pwd
 *  echo "Hello world"
 *  echo "$PATH"
 *  help
 *  quit
 *  halt
 *  ...
*/


/**
 * General purpose function to print memory dump
*/
void dump(const unsigned char *data_buffer, const unsigned int length){
    unsigned char byte;
    unsigned int i, j;

    for(i = 0; i < length; i++){
        byte = data_buffer[i];

        /* It shows byte in hexadecimal form */
        printf("%02x ", data_buffer[i]);            

        if(((i % 16 == 15) || (i == length - 1))){
            for(j = 0; j < 15-(i%16); j++)
                printf(" ");
            printf("| ");

            /* It shows characters into the line (16 per line) */
            for(j = (i - (i%16)); j <= i; j++){     
                byte = data_buffer[i];

                if((byte > 31) && (byte < 127))
                    printf("%c", byte);
                else 
                    printf(".");
            }
            printf("\n");
        }
    }
}


/**
 * Escape and interpret some characters
*/
void escape_char(char *command){
    int len = strlen(command);
    char *empty = "";
    char *hashtag = "#";

    /* Loop characters in the command and find special characters */
    for(int i = 0; i < len; i++){
        if (strcmp((command+i), ";") == 0){
            for (int j = i; j < len; j++)
                *(command+j) = *(command + (j + 1));
        } 
        
        if (*(command+i) == *hashtag){
            for (int j = i; j < len; j++)
                *(command+j) = *empty;
        }
    }
}


/**
 * Deamon function - execute and process system call which it will save to the file handler
*/
void send_file(int cfd, char *in_buffer){
    char *data = (char *)malloc(COMMAND_LIMIT_SIZE * sizeof(char));
    char *command = (char *)malloc(COMMAND_LIMIT_SIZE * sizeof(char));
    char *out_buffer = (char *)calloc(COMMAND_LIMIT_SIZE, sizeof(char));
    FILE *fp;

    /* Copy input buffer from the client with Null char */
    strncpy(command, in_buffer, strlen(in_buffer) - 1);
    escape_char(command);
    
    /* Construct basic command */
    strcat(command, " | tee tmp.txt");
    printf("[~] command: %s\n", command);

    /* Execute command and wait a second */
    system(command);
    sleep(1);

    /* open temp file handler */
    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
    }

    /* Load data from the file handler */
    while(fgets(data, COMMAND_LIMIT_SIZE, fp) != NULL){
        strcat(out_buffer, data);
    }

    /* Check if there is any output in file handler */
    if (strlen(out_buffer) == 0){
        write(cfd, "\0", 1);
        return;
    }

    /* Send output from the command back */
    write(cfd, out_buffer, strlen(out_buffer));
}


/**
 * Deamon function - It will process connection and commands from the client
*/
int connection_established_server(int cfd, int *pid_arr, int user_pid){
    char buffer_in[COMMAND_LIMIT_SIZE];
    char buffer_out[COMMAND_LIMIT_SIZE];
    char invite_message[] = "Hello  client!";

    /* Send invite message to the client */
    write(cfd, invite_message, sizeof(invite_message));

    /* Infinite loop that accept commands from the client */
    while(1){
        read(cfd, buffer_in, sizeof(buffer_in));
        printf("[~] Client with PID %d request: %s", user_pid, buffer_in);

        /* If msg contains "Exit" then server exit and chat ended. */
        if (strncmp("quit", buffer_in, 4) == 0) {
            printf("[-] Client Exit...\n");
            break;
        } 
        
        /* Compare if client was not send halt command */
        else if (strncmp("halt", buffer_in, 4) == 0){
            printf("[-] Server Exit...\n");
            return EXIT_PROGRAM;
        } 
        
        /* Process other system commands and help command */
        else {
            if ((strncmp("help", buffer_in, 4) == 0) || (strncmp("-h", buffer_in, 2) == 0)){
                strcpy(buffer_out, "Author: \tMartin Mihalovic\nSubject: \tSystemove programovanie a Assemblery");
                write(cfd, buffer_out, sizeof(buffer_out));
            } else {
                send_file(cfd, buffer_in);
            }

            /* Zeroing buffers */
            bzero(buffer_in, COMMAND_LIMIT_SIZE);
            bzero(buffer_out, COMMAND_LIMIT_SIZE);
        }
    }

    return 0;
}


/* Initialization array of PIDS to value -1 */
void init_pid_arr(int *pid_arr){
    for(int i=0; i < MAX_HOSTS; i++){
        *(pid_arr+i) = -1;
    }
}


/**
 * SIGQUIT HANDLER - sigquit() function definition
*/
void sigquit(){
    for(int i=0; i < MAX_HOSTS; i++){
        // printf("pid: %d\n", *(pid_arr+i));
        if(*(pid_arr+i) != -1)
            kill(*(pid_arr+i), SIGKILL); 
    }

    printf("[-] My child has Killed me!!!\n");
    exit(0);
}


/**
 * Deamon function - Start running deamon on AF_UNIX domain, and manage client processes
*/
void unix_domain_deamon(char *unix_domain_pathname){

    init_pid_arr(pid_arr);    
    struct sockaddr_un server_add;
	struct sockaddr_un client_add;

    socklen_t s_len;
    int sfd, cfd, pid, status = 0, pid_counter = 0;

    /* Set domain pathname and family */
    memset(&server_add, 0, sizeof(struct sockaddr_un));
    server_add.sun_family = AF_UNIX;           
    strcpy(server_add.sun_path, unix_domain_pathname);

    s_len=sizeof(struct sockaddr_un);

    /* Open deamon server */
    if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
		return;
    }
    printf("[*] Server socket is created (fd = %d)\n", sfd);

    /**
     * Delete any file that already exists at the address. Make sure the deletion succeeds. 
     * If the error is just that the file/directory doesn't exist, it's fine.
    */
    if (remove(unix_domain_pathname) == -1 && errno != ENOENT) {
        perror("remove");
    }

    /* bind socket */
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
    
    /* Infinite loop for accepting connections */
	while(1){
		cfd = accept(sfd,(struct sockaddr *)&client_add,&s_len);

		if(cfd<0){
			perror("accept");
			return;
		}
        printf("[+] Connection accepted from %d\n", client_add.sun_family);
		
        /* Child process */
        if ((pid = fork()) == 0){
            close(sfd);
            status = connection_established_server(cfd, pid_arr, getpid());   

            /* If any child was sent command "halt" */
            if (status == 1){
                pid_t p_process_id;
                p_process_id = getppid();
                
                close(cfd);
                close(sfd);

                /* Kill chain of processes */
                printf("[-] Child: sending SIGQUIT\n");
                kill(p_process_id, SIGQUIT);

                return;
            }
        } 
        
        /* Parent process */
        else if(pid > 0){
            // printf("teraz rodic uloz: %d %d\n", *(pid_arr+pid_counter), pid);
            *(pid_arr+pid_counter) = (pid-1);
 
            pid_counter++;
            close(cfd);
        } else {
            printf("fork Failed\n");
            exit(EXIT_FAILURE);
        }     
	}
}


/**
 * Client function for default prompt shell
*/
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


/**
 * Client function to process communication with deamon
*/
void connection_established_client(int sfd){

    char *shell_text = (char *)malloc(GENERAL_SIZE_LIMIT  * sizeof(char));
    char buffer[COMMAND_LIMIT_SIZE];
    char tmp[GENERAL_SIZE_LIMIT];
    int n = 0;

    bzero(buffer, COMMAND_LIMIT_SIZE);
    read(sfd, buffer, sizeof(buffer));
    printf("---- Connection Established ----\n");

    /* Set default propmt */
    get_shell_text(shell_text);

    while (1){
        bzero(buffer, COMMAND_LIMIT_SIZE);
        /* Print to stdout default prompt text */
        printf("%s", shell_text);
        
        /*Scan user input*/
        n = 0;
        while ((buffer[n++] = getchar()) != '\n')
            ;

        /* if msg contains "Exit" then server exit and chat ended. */
        if (strncmp("quit", buffer, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }

        /* In case User write prompt command */
        else if(strncmp("prompt", buffer, 6) == 0){
            printf("Get new prompt text:");
            scanf("%s", tmp);
            bzero(shell_text, GENERAL_SIZE_LIMIT);
            strncpy(shell_text, tmp, strlen(tmp));
            strcat(shell_text, "#");
        }

        /* Send command on deamon side */
        else if(strncmp(buffer,"\n",1) != 0) {
            write(sfd, buffer, sizeof(buffer));
            bzero(buffer, COMMAND_LIMIT_SIZE);
            read(sfd, buffer, sizeof(buffer));
            printf("%s\n", buffer);
        }
    }
}


/**
 * Client function to established connection with deamon
*/
void unix_domain_client(char *unix_domain_pathname){

    struct sockaddr_un addr;
    int cfd;

    if((cfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
		return;
    }

    printf("Client socket fd = %d\n", cfd);

    /* Construct server address, and make the connection. */
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, unix_domain_pathname, sizeof(addr.sun_path) - 1);

    
    if(connect(cfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1){
        perror("connect");
    }

    connection_established_client(cfd);
}