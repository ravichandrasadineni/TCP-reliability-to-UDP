#include "serverHandler.h"
#include "clientBufferHandler.h"
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
	hdr recvHeader;
	int returnValue=0;
	int newPortNumber;
	char port[512];
	Signal(SIGALRM, sig_alarm);

	sendagain :

	returnValue = sendMessage(sockfd,NULL,&initialHeader, filename);
	if(returnValue < 0 ) {	
		perror("failure while sending first SYN :");
		exit(2);
	}
	if(salarm(INITIAL_TIME_OUT)>0) {
		printf("timer already set somewhere \n");
		exit(2);
	}	
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
		returnValue = recvMessage(sockfd,NULL, &recvHeader, port);
	}while((returnValue <0));

	if(returnValue < 0 ) {
		perror("failure while recieving first ACK :");
		exit(2);
	}
	salarm(0);
	serverseqNumber = ntohs(initialHeader.seq);
	int newPort = atoi(port);
	printf("the port number of new child socket is %d \n",newPort);
	connectAgain(sockfd,newPort);
	return sockfd;	
}


void handleServer(int sockfd, struct sockaddr_in ipAddress, int sliWindowsize, char* filename, sockinfo* clientSocketInfo) {
	hdr initialHeader,recvHeader,replyHeader;
	int returnValue =0;
	char stringMessage[512] ;
	memset(stringMessage,0,512);
	sockfd = establishHandshake(sockfd,ipAddress,sliWindowsize,filename, clientSocketInfo);

	initialHeader  = build_header(clientcurrentSeqNumber,++serverseqNumber,1,0,sliWindowsize,0);
	printf("THe initial ACK Number %d \n", serverseqNumber);
	printf("waiting on send Message \n");
	returnValue = sendMessage(sockfd,NULL,&initialHeader, NULL);

	//createClientSlidingWindow(&head,sliWindowsize);

	while(1) {
		printf("waiting on recv Message \n");

		returnValue = recvMessage(sockfd,NULL, &recvHeader, stringMessage);
		replyHeader = populateClientBuffer(ntohs(initialHeader.ack),sliWindowsize,stringMessage,recvHeader);
		returnValue=sendMessage(sockfd,NULL,&replyHeader,NULL);
		if(ntohs(recvHeader.finFlag)) {
			printf("Successfully  transfered the file \n");
			break;
		}
	}
}



