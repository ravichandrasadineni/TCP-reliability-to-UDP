#include "serverHandler.h"
#include "clientBufferHandler.h"

static sigjmp_buf jmpbuf;
struct itimerval timer;
int clientcurrentSeqNumber =0;
int serverseqNumber =0;
static void sig_alarm(int signo) {
	siglongjmp(jmpbuf,1);

}

int establishHandshake(int sockfd, struct sockaddr_in ipAddress, int sliWindowsize, char* filename, sockinfo* clientSocketInfo,float probability) {
	clientcurrentSeqNumber = getRandomSequenceNumber(10000);
	int currentRetransmissions =0,time=0;
	hdr initialHeader = build_header(clientcurrentSeqNumber, 0,1,0,sliWindowsize,0);
	hdr recvHeader;
	int returnValue=0;
	int newPortNumber;
	char port[488];
	Signal(SIGALRM, sig_alarm);

	sendagain :

	returnValue = clientsendMessage(sockfd,NULL,&initialHeader, filename,probability);


	if(returnValue == -1 ) {
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
	returnValue = clientrecvMessage(sockfd,NULL, &recvHeader, port,probability);

	if(returnValue == -1 ) {
		perror("failure while receiving first ACK :");
		exit(2);
	}
	salarm(0);
	serverseqNumber = ntohs(recvHeader.seq);
	int newPort = atoi(port);
	printf("the port number of new child socket is %d \n",newPort);
	connectAgain(sockfd,newPort);
	return sockfd;	
}


void handleServer(int sockfd, struct sockaddr_in ipAddress, int sliWindowsize, char* filename, sockinfo* clientSocketInfo, sharedBuf *buffer,float *probability) {
	float prob= *probability;
	hdr previousHeader,recvHeader,replyHeader;
	int shouldWait;
	int isDone =0;
	int returnValue =0;
	char stringMessage[488] ;

	sockfd = establishHandshake(sockfd,ipAddress,sliWindowsize,filename, clientSocketInfo,prob);
	previousHeader  = build_header(clientcurrentSeqNumber,++serverseqNumber,1,0,sliWindowsize,0);
	returnValue = clientsendMessage(sockfd,NULL,&previousHeader, NULL,prob);
	if(returnValue  == -1) {
		perror("sending  Message Failed : \n");
		exit(0);
	}
	while(1) {
		returnValue = clientrecvMessage(sockfd,NULL, &recvHeader, stringMessage,prob);
		if(returnValue  < 0) {
			perror("receiving  Message Failed : \n");
			exit(0);
		}
		if((serverseqNumber -1) == ntohs(recvHeader.seq)) {
			returnValue = clientsendMessage(sockfd,NULL,&previousHeader, NULL,prob);
		}
		else {
			replyHeader = populateClientBuffer(ntohs(previousHeader.ack),sliWindowsize,stringMessage,recvHeader, buffer,&isDone);
			previousHeader = replyHeader;
			if(isDone) {
				printf("Successfully received the file. Closing Connection \n");
				break;
			}
			clientsendMessage(sockfd,NULL,&replyHeader,NULL,prob);
		}
		if(returnValue  == -1) {
			perror("sending  Message Failed : \n");
			exit(0);
		}
	}
}



