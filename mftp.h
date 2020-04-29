/******************************
 Austin Cari
 CS 360 Final Project
 Prof McCamish @ WSU Vancouver
 April 13th, 2020

The miniature ftp system consists of two programs (executables), mftpserve and mftp, the server and client respec-
tively.

Both programs will be written in C, to execute in the school's Linux OS environment.

The programs will utilize the Unix TCP/IP sockets interface (library) to communicate with each other across a
TCP/IP network. Only IPv4 addressing will be supported by these programs.

A demonstration of your programs is required. This demonstration must occur in on ENCS laboratory computers.
The instructor will direct the demonstration and observe program behavior.
******************************/

#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>

#define DFL_PORT_NUMBER 49999
#define MAX_CMD_SIZE 1024