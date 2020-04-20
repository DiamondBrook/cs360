/******************************
 Austin Cari
 CS 360 Final Project
 Prof McCamish @ WSU Vancouver
 April 13th, 2020
******************************/

#include "mftp.h"

#define BACKLOG 4


int main(int argc, char *argv[])
{

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, BACKLOG); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

        pid_t pid = fork();
        if(pid == 0){
        	close(listenfd);
        	while(1){
        		write(connfd, "Hello world!", 12);
        		sleep(1);
        	}

        } else {
        	close(connfd);
        }
     }
}
