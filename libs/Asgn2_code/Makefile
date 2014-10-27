
CC = gcc

LIBS =  ../../unpv13e/libunp.a

FLAGS =  -g -O2
CFLAGS = ${FLAGS} -I ../../unpv13e/lib

all: get_ifi_info_plus.o prifinfo_plus.o socketinfo.o
	${CC} -o prifinfo_plus prifinfo_plus.o get_ifi_info_plus.o socketinfo.o ${LIBS}


get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c

prifinfo_plus.o: prifinfo_plus.c
	${CC} ${CFLAGS} -c prifinfo_plus.c

socketinfo.o: socketinfo.c
	${CC} ${CFLAGS} -c socketinfo.c
clean:
	rm prifinfo_plus prifinfo_plus.o get_ifi_info_plus.o socketinfo.o

