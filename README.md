Author: Austin Cari
Date Submitted: 4/26/2020
Prof. McCamish @ WSU Vancouver
CS 360 Systems Programming

Final Project

Requirements: 
Organize your code into (at least) three source files:
mftp.c - client code
mftpserve.c - server code
mftp.h - header file with declarations common to both programs
Include a Makeile which will build both executables by default

To build: 
make all

You should see two executables in your directory
mftp and mftpserve

to run the server on the default port, try 
./mftpserve

to run the server on a different port, try
./mftpserver <port>

to run the client, specify the hostname or the IPv4 address
./mftp 127.0.0.1
