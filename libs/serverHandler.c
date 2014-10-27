#include "serverHandler.h"

static sigjmp_buf jmpbuf;
struct itimerval timer;
int serverCurrentSeqNumber = 0;
int clientcurrentSeqNumber =0;
int serverseqNumber =0;


static void sig_alarm(int signo) {
	siglongjmp(jmpbuf,1);

}

int establishHandshake(int sockfd, struct sockaddr_in ipAddress, int sliWindowsize, char* filename, sockinfo* clientSocketInfo) {
	clientcurrentSeqNumber = getRandomSequenceNumber(10000);
	int currentRetransmissions =0,time=0;
	printf("the current sequence number is %d \n",clientcurrentSeqNumber);
	hdr initialHeader = build_header(clientcurrentSeqNumber, 0,1,0,sliWindowsize,0);
	hdr revcHeader;
	int returnValue=0;
	int newPortNumber;
	char port[512];
	struct msghdr sendMsg, recvMsg;
	memset(&sendMsg, 0, sizeof(sendMsg));
	memset(&recvMsg, 0, sizeof(recvMsg)); 
	struct iovec iovsend[2], iovRecv[2];
	sendMsg.msg_name = NULL;
	sendMsg.msg_namelen = 0;
	sendMsg.msg_iov = iovsend;
	sendMsg.msg_iovlen = 2;
	iovsend[0].iov_base = &initialHeader;
	iovsend[0].iov_len = sizeof(initialHeader);
	iovsend[1].iov_base = filename;
	iovsend[1].iov_len = 512;
	recvMsg.msg_name = NULL;
	recvMsg.msg_namelen = 0;
	recvMsg.msg_iov =iovRecv;
	recvMsg.msg_iovlen = 2;
	iovRecv[0].iov_base = &revcHeader;
	iovRecv[0].iov_len = sizeof(revcHeader);
	iovRecv[1].iov_base = port;
	iovRecv[1].iov_len = 512;
	
	Signal(SIGALRM, sig_alarm);

  sendagain :
	
	 returnValue = sendmsg(sockfd,&sendMsg, 0);
	if(returnValue < 0 ) {	
		perror("failure while sending first SYN :");
		exit(2);
	}
	timer.it_value.tv_sec = INITIAL_TIME_OUT;
	timer.it_value.tv_usec = 0;
	
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	
	
	setitimer (ITIMER_REAL, &timer, NULL);
	
	
	if (sigsetjmp(jmpbuf,1)!=0) {
		if(currentRetransmissions >=NUMBER_OF_RETRANSMITS) {
			printf("Tried sending Syn for 12 times but failed\n");
			exit(1);
		}
		else {
			currentRetransmissions++;
			goto sendagain; 
		}
	}	
	do {
		returnValue = recvmsg(sockfd,&recvMsg, 0);
	}while((returnValue <0));
	
	if(returnValue < 0 ) {
			perror("failure while recieving first ACK :");
			exit(2);
		}
	
	timer.it_value.tv_sec =0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer (ITIMER_REAL, &timer, NULL);
	serverseqNumber = initialHeader.seq;
	int newPort = atoi(port);
	printf("the port number of new client socket is %d \n",newPort);	
	close(sockfd);
	
	sockfd = getClientBindingSocket(&ipAddress,newPort,clientSocketInfo);
	
	
	return sockfd;
	
}


int handleServer(int sockfd, struct sockaddr_in ipAddress, int sliWindowsize, char* filename, sockinfo* clientSocketInfo) {
	hdr initialHeader,revcHeader;
	int returnValue =0;
	char stringMessage[512];
	memset(stringMessage,0,512);
	sockfd = establishHandshake(sockfd,ipAddress,sliWindowsize,filename, clientSocketInfo);	
	printf("establishHandshake called for the first time \n");
	struct msghdr sendMsg, recvMsg;
	memset(&sendMsg, 0, sizeof(sendMsg));
	memset(&recvMsg, 0, sizeof(recvMsg)); 
	struct iovec iovsend[1], iovRecv[2];
	sendMsg.msg_name = NULL;
	sendMsg.msg_namelen = 0;
	sendMsg.msg_iov = iovsend;
	sendMsg.msg_iovlen = 1;
	iovsend[0].iov_base = &initialHeader;
	iovsend[0].iov_len = sizeof(initialHeader);
	recvMsg.msg_name = NULL;
	recvMsg.msg_namelen = 0;
	recvMsg.msg_iov =iovRecv;
	recvMsg.msg_iovlen = 2;
	iovRecv[0].iov_base = &revcHeader;
	iovRecv[0].iov_len = sizeof(revcHeader);
	iovRecv[1].iov_base = stringMessage;
	iovRecv[1].iov_len = 512;
	
	while(1) {
		initialHeader  = build_header(clientcurrentSeqNumber,serverseqNumber+1,1,0,sliWindowsize,0);
		serverseqNumber++;
		iovsend[0].iov_base = &initialHeader;
		iovsend[0].iov_len = sizeof(initialHeader);
		sendmsg(sockfd,&sendMsg, 0);
		returnValue = recvmsg(sockfd,&recvMsg, 0);
		printf("\n%s\n",stringMessage);
		
	}
	
}
		
	
