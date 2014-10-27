#include "unp.h"
#include "socketinfo.h"
#include "messageHeader.h"
#include "childServerUtility.h"
#include "genericUtility.h"
#include <setjmp.h>
#define INITIAL_TIME_OUT 3
#define NUMBER_OF_RETRANSMITS 12
int currentSequenceNumber=0;
int currentAckNumber = 0;
int currentClientSeqNumber =0;

static sigjmp_buf jmpbuf;
struct itimerval timer;

static void sig_alarm(int signo) {
	siglongjmp(jmpbuf,1);

}

int establishSecondHandshake(clientInformation currentClientInformation) {
	 int returnValue=0;
	 char port[512];
	 socklen_t len=INET_ADDRSTRLEN;
	 struct sockaddr_in	sockaddr;
	 int currentRetransmissions =0,time=0;
	 getsockname(currentClientInformation.currentSocketDiscriptor,(SA*)&sockaddr,&len);
	 int newSockfd =getNewSocket(sockaddr, currentClientInformation);
	 getsockname(newSockfd,(SA*)&sockaddr,&len);
	 currentSequenceNumber = getRandomSequenceNumber(10000);
	 currentAckNumber = currentClientInformation.clientSeqNumber +1;
	 hdr initialHeader = build_header(currentSequenceNumber, currentAckNumber,1,0,currentClientInformation.serverWindowSize,0);
	 snprintf(port, 10,"%d",ntohs(sockaddr.sin_port));
	 struct sockaddr_in clientAddress = getClientSocketDetails(currentClientInformation);
	 recvMsg = buildMessage(NULL,&revcHeader,NULL );
	 Signal(SIGALRM, sig_alarm); 
	 sendagain :
	 	returnValue = sendMessage(newSockfd,&clientAddress,  &initialHeader,  port);
		if(returnValue < 0 ) {	
			perror("failure while sending first SYN for new socket :");
			exit(2);
		}
		returnValue = sendMessage(currentClientInformation.currentSocketDiscriptor,&clientAddress, &initialHeader,  port);
		if(returnValue < 0 ) {	
			perror("failure while sending first SYN for old socket:");
			exit(2);
		}
		
	 	if(salarm(INITIAL_TIME_OUT)>0) {
			printf("timer already set somewhere \n");
			exit(2);
		}
		
		if (sigsetjmp(jmpbuf,1)!=0) {
			if(currentRetransmissions >=NUMBER_OF_RETRANSMITS) {
					 char ipAddrString[INET_ADDRSTRLEN];
					 inet_ntop(AF_INET,&(clientAddress.sin_addr),ipAddrString,INET_ADDRSTRLEN);
					 int printPort = ntohs(clientAddress.sin_port);
				printf("Tried connecting to client with ip Address : %s and port Number %d 13 times but failed\n",ipAddrString,printPort );
				exit(1);
			}
			else {
				currentRetransmissions++;
				goto sendagain; 
			}
		}
		
		do {
			returnValue = recvmsg(newSockfd,&recvMsg, 0);
		}while(returnValue < 0);
		
		if(returnValue <0) {
			perror("failure while recieving first ACK Timed Out :");
			exit(2);
		}
		
		if(returnValue ==0) {
			printf("failure while recieving first ACK : Timed Out /n");
			exit(2);
		}
		currentClientSeqNumber = revcHeader.seq;
		currentSequenceNumber++;
		salarm(0);
	
		close(currentClientInformation.currentSocketDiscriptor);
		return newSockfd;
}


int main (int argc, char* argv[]) {
	clientInformation currentClientInformation = proccessClientInfo(argc, argv);
	int newSockfd = establishSecondHandshake(currentClientInformation);
	connectNewServerSocket(newSockfd, currentClientInformation);
	//sendFileAndCloseConnection(newSockfd);	 


}
