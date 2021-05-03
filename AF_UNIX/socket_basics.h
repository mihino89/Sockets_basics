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
#define FILENAME                      "send.txt"
#define BACKLOG                       5
#define COMMAND_LIMIT_SIZE            1024
#define GENERAL_SIZE_LIMIT            255
#define EXIT_FAILURE                  1
#define EXIT_SUCCESS                  0

#define EXIT_PROGRAM                  1
#define MAX_HOSTS                     20

void dump(const unsigned char *data_buffer, const unsigned int length){
    unsigned char byte;
    unsigned int i, j;

    for(i = 0; i < length; i++){
        byte = data_buffer[i];
        printf("%02x ", data_buffer[i]);            // zobrazi bajt hexadecimalne

        if(((i % 16 == 15) || (i == length - 1))){
            for(j = 0; j < 15-(i%16); j++)
                printf(" ");
            printf("| ");

            for(j = (i - (i%16)); j <= i; j++){     // Zobrazi znaky z riadky
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
 * (6b) Base functionality
 * (3b) It's working on the linux environment
 * (5b) Signal's used to kill all chain of children
 * (2b) Makefile is working
 * (1b) Comments in english
*/

/**
 * TODO:
 *  (3b) switch -c to run as a client
 *  (2b) switch -i 
 *  (3b) prompt prikaz on client
*/