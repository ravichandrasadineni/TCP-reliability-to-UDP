#include "unp.h"
#include "socketinfo.h"
#include "messageHeader.h"
#include "childServerUtility.h"
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
	
	 socklen_t len=INET_ADDRSTRLEN;
	 struct sockaddr_in	sockaddr;
	 int currentRetransmissions =0,time=0;
	 getsockname(currentClientInformation.currentSocketDiscriptor,(SA*)&sockaddr,&len);
	 int newSockfd =getNewSocket(sockaddr, currentClientInformation);
	 getsockname(newSockfd,(SA*)&sockaddr,&len);
	 currentSequenceNumber = getRandomSequenceNumber(10000);
	 currentAckNumber = currentClientInformation.clientSeqNumber +1;
	 hdr initialHeader = build_header(currentSequenceNumber, currentAckNumber,1,0,currentClientInformation.serverWindowSize,0);
	 hdr revcHeader;
	 int returnValue=0;
	 char port[512];
	 
	 snprintf(port, 10,"%d",ntohs(sockaddr.sin_port));
	 struct msghdr sendMsg, sendMsg2, recvMsg;
	 memset(&sendMsg, 0, sizeof(sendMsg));
	 memset(&sendMsg2, 0, sizeof(sendMsg));
	 memset(&recvMsg, 0, sizeof(recvMsg)); 
	 struct iovec iovsend[2], iovRecv[1];
	 sendMsg.msg_name = NULL;
	 sendMsg.msg_namelen = 0;
	 sendMsg.msg_iov = iovsend;
	 sendMsg.msg_iovlen = 2;
	 iovsend[0].iov_base = &initialHeader;
	 iovsend[0].iov_len = sizeof(initialHeader);
	 iovsend[1].iov_base = port;
	 iovsend[1].iov_len = 512;
	 
	 struct sockaddr_in clientAddress = getClientSocketDetails(currentClientInformation);
	 sendMsg2.msg_name = (SA*)&clientAddress;
	 sendMsg2.msg_namelen = sizeof(clientAddress);
	 sendMsg2.msg_iov = iovsend;
	 sendMsg2.msg_iovlen = 2;
	 
	 recvMsg.msg_name = NULL;
     recvMsg.msg_namelen = 0;
	 recvMsg.msg_iov =iovRecv;
	 recvMsg.msg_iovlen = 1;
	 
	 iovRecv[0].iov_base = &revcHeader;
	 iovRecv[0].iov_len = sizeof(revcHeader);
	 
	 Signal(SIGALRM, sig_alarm);
	 
	 sendagain :
	 	returnValue = sendmsg(newSockfd,&sendMsg, 0);
		if(returnValue < 0 ) {	
			//perror("failure while sending first SYN for new socket :");
			//exit(2);
		}
		
		returnValue = sendmsg(currentClientInformation.currentSocketDiscriptor,&sendMsg2, 0);
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
