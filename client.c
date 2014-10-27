#include"libs/portutility.h"
#include"libs/genericUtility.h"
#include"libs/fileUtility.h"
#include "libs/socketinfo.h"
int main(int argc,char *argv[])
{
	int portNumber=0,sliWindowsize=0,randSeed=0,milliSec=0,sockfd;
	float prob=0;
	char filename[512],ipAddrString[INET_ADDRSTRLEN];
	sockinfo clientSocketInfo[100];
	struct sockaddr_in ipAddress;
	readingClientInput(&ipAddress,&portNumber,filename,&sliWindowsize,&randSeed,&prob,&milliSec); //in fileUtility.c
	inet_ntop(AF_INET,&(ipAddress.sin_addr),ipAddrString,INET_ADDRSTRLEN);
	printf("port number is %d\tIP Addr is %s\n",portNumber,ipAddrString);
	sockfd=getClientBindingSocket(&ipAddress,portNumber,clientSocketInfo);
	handleServer(sockfd,ipAddress, sliWindowsize, filename, clientSocketInfo);
	return(0);
}
