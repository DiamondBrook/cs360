 /* 
 * Author: Austin Cari
 * Class: CS 360 Systems programming
 * Prof. McCamish @ WSU Vancouver
 * 
 * mftp.c
 *
 * Runs up a small client to connect to a linux server
 */

#define PORT_NUMBER 49999
#include "mftp.h"

struct hostent* host;
int flag = -1;

/* Creat a connection to the server ID and port*/
int createConnection(char* ID, int port) {
    int fd;
    struct in_addr **addr;   
    struct sockaddr_in servAddr;       
    memset(&servAddr, 0, sizeof(servAddr));
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0) {    //Check to see if socket failed
        fprintf(stderr, "Could not create socket! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    } else {          //This needs to be here. Do not know why
        fd = socket(AF_INET, SOCK_STREAM, 0);
    }

    servAddr.sin_port = htons(port);
    servAddr.sin_family = AF_INET;  
    host = gethostbyname(ID);

    if(!host) {    //check to see if the pathname exists
        fprintf(stderr, "No host specified! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    addr = (struct in_addr **) host->h_addr_list;
    memcpy(&servAddr.sin_addr, *addr, sizeof(struct in_addr));

    if((connect(fd, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0) { //check to see if connection established correctly
        fprintf(stderr, "Connection failed! Error code: %d - %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/* Change the directory on the client machine */
void cd(char* input) {
    if(chdir(input) < 0) {
        fprintf(stderr, "chdir failed! Try a different path? Error code: %d - %s\n", errno, strerror(errno));
        return;
    }
}

/* Read responses from the server */
char* getResponse(int sock) {
    char* response = malloc(MAX_CMD_SIZE);
    int n;
    char curr;
    read(sock, &curr, 1);
    for(n = 0; curr != '\n'; n++) {
        response[n] = curr;
        read(sock, &curr, 1);
    }
    response[n] = '\0';
    return response;
}

// Transfer file from local machine to the server's current working directory
int put(int datafd, int sockfd, char* sCMD, char* pathname){
    int fd, bytes_read;
    char* buff;

    if((fd = open(pathname, O_RDONLY, 0600)) < 0) {
        fprintf(stderr, "ERROR: Put could not be performed.\n"); //, strerror(errno));
        exit(EXIT_FAILURE);
    }
    write(sockfd, sCMD, strlen(sCMD));
    buff = getResponse(sockfd);
    if (buff[0] == 'E'){
        printf("%s\n", strcat(buff+1, "\0"));
        exit(EXIT_FAILURE);
    } 
    else {
        while((bytes_read = read(fd, buff, MAX_CMD_SIZE)) > 0) {
            write(datafd, buff, bytes_read);
        }
    }
    close(fd);
    return 0;
}   

// Function for listing client's directory
void ls() {
    int parent = 0;
    if((parent = fork())) {
        wait(&parent);
        printf("\n");
    }
    else {
        //Set up pipe
        int fd[2], ch = 1;
        pipe(fd);
        int rd = fd[0], wr = fd[1];

        if((ch = fork())) {
            close(wr);  //Redirect stdout to read end of pipe
            close(0);
            dup(rd);
            close(rd);
            wait(&ch);
            execlp("more", "more", "-20", NULL);
        }
        else {
            close(rd);  //Redirect stdin to write end of pipe
            close(1);
            dup(wr);
            close(wr);
            execlp("ls", "ls", "-l", NULL);
        }
    }
}

// Remote ls
void rls(int fd) {
    pid_t parent;
    if((parent = fork())) {
        wait(&parent);
    }
    else {
        close(0);
        dup(fd);
        execlp("more", "more", "-20", NULL);
    }
}

// Gets file from server
void get(int fd, char* input) {
    int filedesc, reader;
    char buff[MAX_CMD_SIZE];

    if((filedesc = open(input, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0) {
        if(flag == -1) {
            fprintf(stderr, "File already exist! Error code: %d - %s\n", errno, strerror(errno));
        }
        return;
    }

    flag = -1;
    while((reader = read(fd, buff, MAX_CMD_SIZE))) {
        write(filedesc, buff, reader);
    }
    close(filedesc);
    return;
}

int main(int argc, char **argv) {
    char* buff;
    char* cCMD = NULL;
    char *pathname;
    char sCMD[MAX_CMD_SIZE], input[MAX_CMD_SIZE];
    int sockfd, fd = 0;

    if(argc < 2) {
        printf("Please enter a valid hostname or IPV4 address!");
        exit(EXIT_FAILURE);
    }

    char* ID = argv[1];
    sockfd = createConnection(ID, PORT_NUMBER);

    if(sockfd < 0) {
        exit(EXIT_FAILURE);
    }

    printf("Welcome user!\nYou will be prompted for input when >>> appears.\n");
    while(1) {
        printf(">>>");
        fgets(input, MAX_CMD_SIZE, stdin);
        printf("\n");

        //Close down the client when the EOF char is recieved from the server
        if(input[0] == EOF) {
            break;
        }
        if(input[0] == '\n') {
            continue;
        }

        //tokenize the input string
        cCMD = strtok(input, " \r\f\v\t\n");

        /* "exit"
        Exit from the server gracefully and let the server know too */
        if(!(strcmp(cCMD, "exit"))) {
            write(sockfd, "Q\n", 2); //tell the server we're leaving
            
            // Handle the server response 
            buff = getResponse(sockfd);
            if(buff[0] == 'A') {
                printf("Disconnected from server.\n");
                exit(EXIT_SUCCESS);
            }
            if(buff[0] == 'E'){  
                printf("Disconnect not performed properly\n");
                exit(EXIT_FAILURE);
            }
        }

        /* "cd"
        Change the directory that the client is currently working in */
        if(strcmp(cCMD, "cd") == 0) { 
            pathname = strtok(NULL, " \r\f\v\t\n"); //no spaces are allowed in the pathname
            cd(pathname);
            fflush(stdout);
        }

        /* "ls"
        View the contents of the client's current working directory*/
        else if(strcmp(cCMD, "ls") == 0) {
            ls();
            fflush(stdout);
        }

        /* "rcd"
        Change the directory that the server is currently working in */
        else if(strcmp(cCMD, "rcd") == 0) {
            fflush(stdout);
            pathname = strtok(NULL, " \r\f\v\t\n");
            if(!pathname) {
                printf("ERROR: pathname invalid!\n");
                continue;
            }
            strcpy(sCMD, "C");         //Send a C cmd to server with pathname appended
            strcat(sCMD, pathname);
            strcat(sCMD, "\n");
            int len = strlen(sCMD);
            write(sockfd, sCMD, len);  //Write the cmd
            buff = getResponse(sockfd);           
            fflush(stdout);                 
            if(buff[0] == 'E') {
                printf("%s\n", strcat(buff + 1, "\n"));
            }
        }

        /* "rls"
        View the contents of the server's current working directory */
        else if(strcmp(cCMD, "rls") == 0) {
            write(sockfd, "D\n", 2);    //establish data connection
            buff = getResponse(sockfd);   
            fflush(stdout);

            //Recieved acknolwdgement that D succeeded
            if(buff[0] == 'A') {
                fd = createConnection(ID, atoi(buff+1));
                if(fd == -1) {
                    fprintf(stderr, "Could not establish connection! Error %d: %s\n", errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                write(sockfd, "L\n", 2);
                buff = getResponse(sockfd);

                //Recieved acknolwdgement that L succeeded
                if(buff[0] == 'A') {
                    rls(fd);

                //Error recieved on L
                } else {
                    printf("ERROR: %s\n", strcat(buff+1, "\n"));
                }

            //Error recieved on D
            } else {
                printf("%s\n", buff+1);
            }   
        }

        /* "show"
        Show the first twently lines of a file in the stdout of the client. Press space to see the next 20 lines*/
        else if(strcmp(cCMD, "show") == 0) {
            fflush(stdout);
            write(sockfd, "D\n", 2);
            buff = getResponse(sockfd);

            //Error if data connection failed
            if(buff[0] == 'E') {
                printf("%s\n", strcat(buff + 1, "\0"));
            }
            else {
                fd = createConnection(ID, atoi(buff+1));
                pathname = strtok(NULL, " \r\f\v\t\n");
                if(!pathname) {
                    printf("ERROR: 'show'\n");
                    continue;
                }
                strcpy(sCMD, "G");
                strcat(sCMD, pathname);
                strcat(sCMD, "\n");
                write(sockfd, sCMD, strlen(sCMD));
                buff = getResponse(sockfd);

                //If the server sent acknolwedgement, read from fd
                if(buff[0] == 'A') {
                    flag = 1;
                    get(fd, pathname);
                    rls(fd);
                }
                else {
                    printf("ERROR: %s\n", strtok(buff+1, "\n"));
                }
            }
        }

        /* "get <pathname>"
        Retrieve the last component of <pathname> from the remote server and store it locally in the cwd*/
        else if(strcmp(cCMD, "get") == 0) {
            write(sockfd, "D\n", 2);
            buff = getResponse(sockfd);
            fflush(stdout);

            //Error recieved
            if(buff[0] == 'E') {
                printf("%s\n", strcat(buff + 1, "\n"));
            }

            //Acknolwedgement recieved 
            else {
                fd = createConnection(ID, atoi(buff + 1));
                pathname = strtok(NULL, " \r\f\v\t\n");
                if(!pathname) {
                    printf("ERROR: 'get' \n");
                    continue;
                }
                strcpy(sCMD, "G");
                strcat(sCMD, pathname);
                strcat(sCMD, "\n");
                write(sockfd, sCMD, strlen(sCMD));
                buff = getResponse(sockfd);
                if(buff[0] == 'A') {
                    flag = -1;
                    if(fd < 0 || pathname == NULL){
                        fprintf(stderr, "Bad pathname or file descriptor!\n [Error code: %d] - %s \n", errno, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    get(fd, pathname);
                    close(fd);
                }
                else {
                    printf("ERROR: %s\n", strtok(buff + 1, "\n"));
                }
            }
        }

        /* "put <pathname>"
        Store the last component of <pathname> from the cwd of the local client to the cwd of the remote server*/
        else if(strcmp(cCMD, "put") == 0) {
            fflush(stdout);
            write(sockfd, "D\n", 2);
            fflush(stdout);
            buff = getResponse(sockfd);

            //Acknolwedgement recieved
            if(buff[0] == 'A') {
                fd = createConnection(ID, atoi(buff+1));
                pathname = strtok(NULL, " \n\t\v\f\r");
                if(!pathname) {
                    printf("ERROR: 'put'\n");
                    continue;
                }
                strcpy(sCMD, "P");
                strcat(sCMD, pathname);
                strcat(sCMD, "\n");
                put(fd, sockfd, sCMD, pathname);
                close(fd);
            } 

            //Error recieved 
            if(buff[0] == 'E')  {
                printf("%s\n", strcat(buff + 1, "\n"));
            }
            
           
            } 

        /* "Help" or "h"
        Display the avaliable commands to the user */
        else if(strcmp(cCMD, "help") == 0 || strcmp(cCMD, "h") == 0) {
            printf("*** Avaliable commands ***\n");
            printf("-> cd <pathname>\n\t-> Change directory on the local machine\n");
            printf("-> rcd <pathname>\n\t-> Change directory on the remote machine\n");
            printf("-> ls\n\t-> Perform ls on the local machine\n");
            printf("-> rls\n\t-> Perform ls on the remote machine\n");
            printf("-> get <pathname>\n\t-> Retrieve <pathname> from the server and store the file in the cwd of the local machine\n");
            printf("-> show <pathname>\n\t-> Write the first 20 lines of the file at <pathname> to client stdout. Press space to continue scrolling\n");
            printf("-> put <pathname>\n\t-> Store the contents at <pathname> from the client in the cwd of the Remotete machine\n");
            printf("-> exit\n\t-> Gracefully exit the client\n");
        }

        /* There was no valid input recognized*/
        else {
            fprintf(stderr, "Command not recognized! Type 'help' to see a list of commands\n");
        }
    }
    free(buff);
    printf("Lost connection to the server.\n");
    exit(EXIT_SUCCESS);
}
