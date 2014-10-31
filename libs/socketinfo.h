#ifndef SOCKET_INFO_H
#define SOCKET_INFO_H
#include "unpifiplus.h"
typedef struct {	
	int sockfd;
	struct in_addr ipAddr;
	struct in_addr subnetMask;
	struct in_addr networkAddr;
		
} sockinfo;	


typedef  struct{
	char filename[488];
	struct in_addr ipAddress;
	struct in_addr subnetMask;
	int port;
	int currentSocketDiscriptor;
	int clientInitialWindowSize;
	int serverWindowSize;
	int clientSeqNumber;
}clientInformation;

int getServerBindingSockets(int, sockinfo* );
int getClientBindingSocket(struct sockaddr_in *,int ,sockinfo*);
int isLocal(sockinfo ,struct sockaddr_in );
int isLoopbackAddr(struct sockaddr_in);
struct sockaddr_in getClientSocketDetails(clientInformation currentClientInformation);
int getNewSocket(struct sockaddr_in	sockaddr, clientInformation currentClientInformation);
void connectNewServerSocket(int, clientInformation);
void printSocketDetails(struct sockaddr_in socketDetails);
void connectAgain(int sockFd, int port);
#endif
