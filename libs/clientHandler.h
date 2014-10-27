#include "unp.h"
#include"socketinfo.h"
#include"genericUtility.h"
#include "messageHeader.h"
#include "socketinfo.h"
#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#define RECV_BUFFER_LENGTH 1024

typedef struct {
	int pid;
	struct in_addr ipAddress;
	int port;
} clientInfo;


void handleClients(sockinfo*, int,int);

#endif
