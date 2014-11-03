#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H
#define INITIAL_TIME_OUT 3
#define NUMBER_OF_RETRANSMITS 12
#include "unp.h"
#include "messageHeader.h"
#include "socketinfo.h"
#include "genericUtility.h"
#include "threadUtility.h"
#include <setjmp.h>
void handleServer(int sockfd, struct sockaddr_in ipAddress,int sliWindowsize, char* filename, sockinfo* clientSocketInfo, sharedBuf *buffer, float *probability);
#endif
