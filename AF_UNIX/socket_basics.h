#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define DEFAULT_SOCKET_PATHNAME       "socket"
#define BACKLOG                       5
#define COMMAND_LIMIT_SIZE            1024
#define GENERAL_SIZE_LIMIT            255
#define EXIT_FAILURE                  1

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