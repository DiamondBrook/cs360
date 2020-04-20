CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -g
OBJS=mftp.o mftpserve.o
EXEC=mftp mftpserve
all: ${OBJS}
	$(CC) $(CFLAGS) -o mftp mftp.o
	${CC} ${CFLAGS} -o mftpserve mftpserve.o
	rm -f ${OBJS}

mftp.o: mftp.c
	${CC} ${CFLAGS} -c mftp.c

mftpserve.o: mftpserve.c
	${CC} ${CFLAGS} -c mftpserve.c

server: 
	./mftpserve

client:
	./mftp 127.0.0.1

clean:
	rm -f ${OBJS} ${EXEC}