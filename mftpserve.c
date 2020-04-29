 /* 
 * Author: Austin Cari
 * Class: CS 360 Systems programming
 * Prof. McCamish @ WSU Vancouver
 * 
 * mftpserve.c
 *
 * Runs up a small linux server that can do a few tasks
 */

#define PORT_NUMBER 49999 
#define BACKLOG 4
#include "mftp.h"

struct sockaddr_in client;                                
char buff[MAX_CMD_SIZE];                                            

// Create sockaddr_in struct to bind with a socket from createSocket
struct sockaddr_in createServer(int port) {
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family =  AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    return serv;
}

/* Create a socket to bind with a sockaddr_in struct from createServer */
int createSocket() {
    int listenfd, i;
    int flag = 1;
    socklen_t len_one = sizeof(1);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        fprintf(stderr, "Could not create a socket! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    i = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*) &flag, len_one);
    if(i < 0){
        fprintf(stderr, "Connection could not be established properly! Error code: %d - %s \n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return listenfd;
}

/* Establish a data connection with the existing client */
int makeConnection(int fd) {
    int connfd;
    unsigned int len = sizeof(struct sockaddr_in);
    connfd = accept(fd, (struct sockaddr *) &client, &len);
    return connfd; 
}

// Fetch the port
char* getPort(int fd, struct sockaddr_in serv) {
    int  port;
    char* returnPort = malloc(16);  //ports are 16 bytes long;
    struct sockaddr_in portSock;
    unsigned int size = sizeof(portSock);

    // Bind the socket and sockaddr
    if(bind(fd, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
        fprintf(stderr, "Could not bind the file descriptor properly! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    assert(getsockname(fd, (struct sockaddr *) &portSock, &size) != -1);
    port = ntohs(portSock.sin_port);
    sprintf(returnPort, "%d", port);    //strcpy doesnt work here
    return returnPort;
}

// Change server's directory
int rcd(char* input) {
    if(chdir(input) == -1) {
        printf("Invalid pathname! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("The cwd has been changed!\n");
    return 0;
}

// List server's current directory
void rls(int fd) {
    pid_t child;
    if(!(child = fork())) {
        close(1);   //redirect stdout to the client
        dup(fd);
        if(execlp("ls", "ls", "-l", NULL)) {
            write(fd, "EError performing rls\n", 22);
            fprintf(stderr, "Could not perform rls. execlp failed. Error code: %d - %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    printf("The cwd has been sent to a client.\n");
    wait(&child);
    return;
}

// Function for getting a file from the server and save it in client's directory
void get(int fd, int connfd, char* input) {
    int fin, bytes;
    char buf[MAX_CMD_SIZE];

    if((fin = open(input, O_RDONLY, 0600)) < 0) {
        write(connfd, "ECould not perform get\n", 23);
        return;
    }
    write(connfd, "A\n", 2);

    while((bytes = read(fin, buf, MAX_CMD_SIZE)) > 0) {
        write(fd, buf, bytes);
    }
    printf("The file was sent to the client successfully.\n");
    close(fin);
    return;
}

// Function for putting a file from the client to the server
int put(int fd, int connfd, char* input) {
    int filedesc, bytes_read;
    char buf[MAX_CMD_SIZE];

    if((filedesc = open(input, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0) {
        fprintf(stderr, "Could not perform put. Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    write(connfd, "A\n", 2);
    while((bytes_read = read(fd, buf, MAX_CMD_SIZE)) > 0) {
        write(filedesc, buf, bytes_read);
    }
    printf("The file was downloaded to the client successfully.\n");
    close(filedesc);
    return 0;
}


int main(int argc, char** argv) {   
    char* dataPort;
    int socketdatafd = 0, datafd = 0, port = 0;   
    printf("Server booting up...\n");

    /* Optional paramter to set up a specific port*/
    if(argv[1] != NULL) {
        port = atoi(argv[1]);
    }
    else {
        port = PORT_NUMBER; //Port 49999 default
    }

    //Create a socket and server to bind together
    int listenfd = createSocket();
    struct sockaddr_in servAddr = createServer(port);

    //Bind the sockaddr_in and socket together
    if(bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {  // check for bind error
        fprintf(stderr, "Bind failed! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    listen(listenfd, BACKLOG);

    //Wait for new connections here
    printf("Server online! Listening on port %d\n", port);
    while(1) {
        pid_t child;
        int connfd;
        socklen_t length = sizeof(struct sockaddr_in);
        connfd = accept(listenfd, (struct sockaddr *) &client, &length); //blocks until a connection is accepted
        if(connfd < 0) {
            fprintf(stderr, "Connection found but accept failed: Error code: %d - %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }

        //A connection has been accepted. Fork a new process to handle all the requests from the new client
        if(!(child = fork())) {

            //Store the host information in hostEntry
            struct hostent* hostEntry = gethostbyaddr(&(client.sin_addr), sizeof(struct in_addr), AF_INET);
            char* hostName = hostEntry->h_name;

            //Log relevant info to server stdout
            printf("Child process id: %d\n", getpid());
            printf("Connection established at %s\n", hostName);

            //Take care of client commands from here
            while(1) {
                char bufferChar;  
                if(!read(connfd, &bufferChar, 1)) {
                    printf("The connection was interrupted! Child %d exiting...\n", getpid());
                    exit(EXIT_FAILURE);
                }
                int j = 0;                         
                while(bufferChar != '\n'){                   
                    buff[j] = bufferChar;                  
                    j++;                                
                    read(connfd, &bufferChar, 1);         
                }
                buff[j] = '\0';       

                // "Quit"
                if(buff[0] == 'Q') {
                    printf(" %s has disconnected.\n", hostName);
                    write(connfd, "A\n", 2);
                    exit(EXIT_FAILURE); //should be EXIT_SUCCESS
                }

                // Establish a data connection
                if(buff[0] == 'D') {
                    char serverReply[MAX_CMD_SIZE];  
                    socketdatafd = createSocket();
                    struct sockaddr_in dataAddr = createServer(0);
                    dataPort = getPort(socketdatafd, dataAddr);
                    strcpy(serverReply, "A");
                    strcat(serverReply, dataPort);
                    strcat(serverReply, "\n");
                    write(connfd, serverReply, (unsigned int) strlen(serverReply));
                    listen(socketdatafd, 1);
                    datafd = makeConnection(socketdatafd);
                }

                // "Get"
                if(buff[0] == 'G') {
                    if(socketdatafd == -1) {    //needs data connection
                        write(connfd, "EData connection failed!\n", 25);
                    }
                    else {
                        get(datafd, connfd, buff+1);
                        close(datafd);
                        close(socketdatafd);
                    }
                }

                // "Change directory"
                if(buff[0] == 'C') {
                    printf("%s", buff);
                    if((rcd(buff + 1) != -1)) {
                        write(connfd, "A\n", 2);
                    }
                    else {
                        write(connfd, "E'rcd' failed!\n", 13);
                    }
                }

                // "List the contents of a directory"
                if(buff[0] == 'L') {
                    if(!socketdatafd) { //needs data connection
                        write(connfd, "EData connection failed!\n", 25);
                    }
                    else {
                        rls(datafd);
                        write(connfd, "A\n", 2);
                        close(datafd);
                        close(socketdatafd); 
                    }
                }

                // "Put"
                if(buff[0] == 'P') {
                    if(!socketdatafd) { //needs data connection
                        write(connfd, "EData connection failed\n", 25);
                    }
                    else {
                        put(datafd, connfd, buff+1);
                        close(datafd);
                        close(socketdatafd);
                    }
                }
            }
        }
        close(connfd);   //close the newest connection established and listen for more connections
    }

    /* We shouldnt ever get down here, but if it somehow happens, do a little cleanup and exit */
    free(dataPort);
    exit(EXIT_SUCCESS);
}

