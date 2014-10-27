#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H
#define INITIAL_TIME_OUT 3
#define NUMBER_OF_RETRANSMITS 12
#include "unp.h"
#include "messageHeader.h"
#include "socketinfo.h"
#include <setjmp.h>
int handleServer(int sockfd, struct sockaddr_in ipAddress,int sliWindowsize, char* filename, sockinfo* clientSocketInfo);
#endif
