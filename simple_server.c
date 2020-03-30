#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_basics.h"

/**
 * Port, na ktory sa budu uzivatelia prihlasovat
*/
#define PORT 7890                       

int main(void){
    int sockfd, new_sockfd;
 
    struct sockaddr_in host_addr, client_addr;          // informacie o mojej adrese
    socklen_t sin_size;
    int recv_length=1, yes=1;
    char buffer[1024];

    /**
     * PF_INET = socket typu TCP/IP pre rodinu IPv4
     * SOCK_STREAM = typ socketu - prudovy
     * 0 = v rodine PF_INET chcem uba jeden protokol
     * funkcia vrati descriptor soketoveho suboru
    */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        printf("in socket");
    
    /**
     * setsockopt = nastavuje volby socketu
     * SO_REUSEADDR pre znovunadviazanie (na port) s adresou
     * SOL_SOCKET =  nastavuje uroven volby, SOL_SOCKET je volba na urovni socketov 
    */
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        printf("setting socket option SO_REUSEADDR");

    /**
     * Priprava structu host_addr
    */
    host_addr.sin_family = AF_INET;           // Bajtove poradie hosta
    host_addr.sin_port = htons(PORT);         // fn htons prevadza na sietove poradie bajtov
    host_addr.sin_addr.s_addr = 0;            // Automaticky vyplni moju IP adresu
    memset(&(host_addr.sin_zero), '\0', 8);   // Vynuluje zbytok struct

    if(bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
        printf("binding to socket");
    
    /**
     * 5 oznacuje arg pre max velkost stacku (backlog queue)
    */
    if(listen(sockfd, 5) == -1)
        printf("listening on socket");

    /**
     * Cyklus pre akceptaciu prichadzajuceho volania
     * accept vrati novy descriptor socket suboru
    */
    while(1){
        sin_size = sizeof(struct sockaddr_in);
        new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);

        /**
         * inet_ntoa = prevedenie adresovej struktury sin_addr na retazec IP adresy
        */
        if(new_sockfd == -1)
            printf("accepting connection");
        printf("server: got connection from %s port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        send(new_sockfd, "Hello world!\n", 13, 0);
        recv_length = recv(new_sockfd, &buffer, 1024, 0);

        while(recv_length > 0){
            printf("RECV: %d bytes\n", recv_length);
            dump(buffer, recv_length);
            recv_length = recv(new_sockfd, &buffer, 1024, 0);
        }
        close(new_sockfd);
    }
    return 0;
}