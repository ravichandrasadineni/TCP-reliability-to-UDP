
# This is a sample Makefile which compiles source files named:
# - tcpechotimeserv.c
# - tcpechotimecli.c
# - time_cli.c
# - echo_cli.c
# and creating executables: "server", "client", "time_cli"
# and "echo_cli", respectively.
#
# It uses various standard libraries, and the copy of Stevens'
# library "libunp.a" in ~cse533/Stevens/unpv13e_solaris2.10 .
#
# It also picks up the thread-safe version of "readline.c"
# from Stevens' directory "threads" and uses it when building
# the executable "server".
#
# It is set up, for illustrative purposes, to enable you to use
# the Stevens code in the ~cse533/Stevens/unpv13e_solaris2.10/lib
# subdirectory (where, for example, the file "unp.h" is located)
# without your needing to maintain your own, local copies of that
# code, and without your needing to include such code in the
# submissions of your assignments.
#
# Modify it as needed, and include it with your submission.

CC = gcc

LIBS = -lresolv -lnsl -pthread -lm unpv13e/libunp.a
	
FLAGS = -g -O0

CFLAGS = ${FLAGS} -I unpv13e/lib
all: client server childServer
OBJECTS=genericUtility.o portutility.o get_ifi_info_plus.o socketinfo.o fileUtility.o clientHandler.o serverHandler.o messageHeader.o childServerUtility.o bufferHandler.o fileSender.o urtt.o clientBufferHandler.o threadUtility.o

childServer : childServer.o $(OBJECTS)
	${CC} ${FLAGS} -o childServer childServer.o $(OBJECTS) ${LIBS}
	
childServer.o: libs/childServer.c
	${CC} ${CFLAGS} -c libs/childServer.c

server: server.o $(OBJECTS)
	${CC} ${FLAGS} -o server server.o $(OBJECTS) ${LIBS}
server.o: server.c 
	${CC} ${CFLAGS} -c server.c 
client: client.o $(OBJECTS)
	${CC} ${FLAGS} -o client client.o $(OBJECTS) ${LIBS}
client.o: client.c 
	${CC} ${CFLAGS} -c client.c 
	
genericUtility.o: libs/genericUtility.c
	${CC} ${CFLAGS} -c libs/genericUtility.c

portutility.o: libs/portutility.c
	${CC} ${CFLAGS} -c libs/portutility.c
	
get_ifi_info_plus.o: libs/get_ifi_info_plus.c
	${CC} ${CFLAGS} -c libs/get_ifi_info_plus.c

socketinfo.o: libs/socketinfo.c
	${CC} ${CFLAGS} -c libs/socketinfo.c
	

fileUtility.o: libs/fileUtility.c
	${CC} ${CFLAGS} -c libs/fileUtility.c
	
clientHandler.o: libs/clientHandler.c
	${CC} ${CFLAGS} -c libs/clientHandler.c
	
serverHandler.o: libs/serverHandler.c
	${CC} ${CFLAGS} -c libs/serverHandler.c
	
messageHeader.o: libs/messageHeader.c
	${CC} ${CFLAGS} -c libs/messageHeader.c
	
childServerUtility.o: libs/childServerUtility.c
	${CC} ${CFLAGS} -c libs/childServerUtility.c
	
bufferHandler.o: libs/bufferHandler.c
	${CC} ${CFLAGS} -c libs/bufferHandler.c

clientBufferHandler.o:libs/clientBufferHandler.c
	${CC} ${CFLAGS} -c libs/clientBufferHandler.c
	
fileSender.o: libs/fileSender.c
	${CC} ${CFLAGS} -c libs/fileSender.c

threadUtility.o: libs/threadUtility.c
	${CC} ${CFLAGS} -c libs/threadUtility.c
	
urtt.o: libs/urtt.c
	${CC} ${CFLAGS} -c libs/urtt.c
	
# pick up the thread-safe version of readline.c from directory "threads"


clean:
	rm server client childServer  $(OBJECTS)

