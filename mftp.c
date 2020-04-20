/******************************
 Austin Cari
 CS 360 Final Project
 Prof McCamish @ WSU Vancouver
 April 13th, 2020
******************************/

#include "mftp.h"
#include <netdb.h>

int main(int argc, char *argv[])
{

    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n ERROR %d: inet_pton error occured: <%s>\n", errno, strerror(errno));
        return 1;
    } 

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[12] = 0;
        read(sockfd, recvBuff, 12);
        printf("%s\n", recvBuff);
    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}