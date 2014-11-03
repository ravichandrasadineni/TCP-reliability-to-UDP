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
	printf("the current sequence number is %d \n",clientcurrentSeqNumber);
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
	returnValue = recvMessage(sockfd,NULL, &recvHeader, port);

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
	printf("The probability value in handle server function is %f\n",prob);
	hdr previousHeader,recvHeader,replyHeader;
	int shouldWait;
	int returnValue =0;
	char stringMessage[488] ;
	memset(stringMessage,0,488);
	sockfd = establishHandshake(sockfd,ipAddress,sliWindowsize,filename, clientSocketInfo,prob);
	previousHeader  = build_header(clientcurrentSeqNumber,++serverseqNumber,1,0,sliWindowsize,0);
	returnValue = clientsendMessage(sockfd,NULL,&previousHeader, NULL,prob);
	printf("current Client Sequence Number and Server Sequence number %d %d \n", clientcurrentSeqNumber, serverseqNumber);
	while(1) {
		returnValue = clientrecvMessage(sockfd,NULL, &recvHeader, stringMessage,prob);
		if(returnValue  < 0) {
			perror("receiving  Message Failed : \n");
			exit(0);
		}
		printf("current Client Sequence Number and Server Sequence number %d %d \n", serverseqNumber, ntohs(recvHeader.seq));
		if((serverseqNumber -1) == ntohs(recvHeader.seq)) {
			returnValue = clientsendMessage(sockfd,NULL,&previousHeader, NULL,prob);
		}
		else {
			replyHeader = populateClientBuffer(ntohs(previousHeader.ack),sliWindowsize,stringMessage,recvHeader, buffer);
			//printfBuffer(buffer);
			returnValue=clientsendMessage(sockfd,NULL,&replyHeader,NULL,prob);
			if(ntohs(recvHeader.finFlag)) {
				printf("Successfully  transfered the file \n");
				break;
			}
		}
		if(returnValue  == -1) {
			perror("sending  Message Failed : \n");
			exit(0);
		}
		printf(" current Size of the buffer is %d \n",buffer->currentSize );
				pthread_mutex_lock(&buffer->mutex);
				if(buffer->currentSize == sliWindowsize) {
					shouldWait = 1;
				}
				pthread_mutex_unlock(&buffer->mutex);
				if(shouldWait !=0) {
					int selectReturnValue =0;
					while(1) {
						fd_set rset;
						int maxfd=0;
						FD_ZERO(&rset);
						FD_SET(sockfd, &rset);
						maxfd= sockfd +1;
						struct timeval timeToWait = getTimeToWait(buffer->tsThreadSlept,buffer->currentWaitmilliSecs );
						if((selectReturnValue = select(maxfd, &rset, NULL, NULL,&timeToWait)) < 0) {
							if(selectReturnValue == 0) {
								shouldWait=0;
								break;
							}
							else {
								returnValue=sendMessage(sockfd,NULL,&replyHeader,NULL);
								if(returnValue  < 0) {
									perror("sending  Message Failed : \n");
								}
							}
						}
					}
				}
			//previousHeader = recvHeader;


	}
}



