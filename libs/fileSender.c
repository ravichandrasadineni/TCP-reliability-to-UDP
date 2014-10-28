#include "fileSender.h"

sigjmp_buf jmpBuf;

static void sig_alarm(int signo) {
	printf("sigLongJump called \n");
	siglongjmp(jmpBuf, 1);
}

void sendFileAndCloseConnection(int sockfd,  clientInformation currentClientInfo, int currentClientSeqNumber, int currentServerSequenceNumber) {

	int clientWindowSize =  htons(currentClientInfo.clientInitialWindowSize);
	int serverWindowSize  = currentClientInfo.serverWindowSize;
	//struct sockaddr_in clientAddress = getClientSocketDetails(currentClientInfo);
	urtt_info urttInfo;
	urtt_init(&urttInfo);
	hdr currentClientHeader;
	int congestionWindow = 1000;
	int currentWaitingAckNumber =0;
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, congestionWindow );
	printf("current Minimum is %d \n", currentMinimum);
	int notDoneSending = 1;
	serverWindowSeg *head = NULL, *tail = NULL;
	createInitialServerBuffer(serverWindowSize,&head,&tail, &currentServerSequenceNumber);

	signal(SIGALRM, sig_alarm);

	while(notDoneSending) {
		serverWindowSeg* current = head;
		if(malarm(urtt_start(&urttInfo,0)) > 0 ) {
			printf("Timer on a Existing Timer \n");
			exit(3);
		}
		currentWaitingAckNumber = (current->header.seq)+1;
		while((currentMinimum  > 0) &&(current!= NULL )) {
			if(current->isSent) {
				continue;
			}
			else {
				current->isSent = 1;
				current->ts = urtt_ts(&urttInfo);
				sendMessage(sockfd,NULL,&(current->header),current->data);
				if(ntohs(current->header.finFlag) == 1) {
					notDoneSending =0;
					break;
				}
				currentMinimum--;
			}
			current = current->next;
		}
		do {
			printf("Before recv message \n");
			if(sigsetjmp(jmpBuf,1)!=0) {
				printf("sigsetjmp called \n");
				malarm(0);

				if(currentMinimum==0) {
					sendMessage(sockfd,NULL,&(head->header),head->data);
					malarm(2);
				}
				else {
					head->numOfAcks =0;
					sendMessage(sockfd,NULL,&(head->header),head->data);
					if(urtt_timeout(&urttInfo,&(head->numOfRtsm)) > 0) {
						printf("Tried 12 times reaching ipAddress for the following client Giving Up \n");
						printSocketDetailsforSocket(sockfd);
						exit(2);
					}
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
			}
			recvMessage(sockfd,NULL, &currentClientHeader, NULL);
			printf("After recv message \n");
			if(currentClientHeader.ack == currentWaitingAckNumber-1) {
				head->numOfAcks += 1;
				if(head->numOfAcks >=4) {
					malarm(0);
					head->numOfAcks =0;
					sendMessage(sockfd,NULL,&(head->header),head->data);
					head->numOfRtsm += 1;
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
			}
			else if(currentClientHeader.ack  >= currentWaitingAckNumber){
				malarm(0);
				unsigned int currentTime = urtt_ts(&urttInfo);
				urtt_stop(&urttInfo,currentTime-head->ts,head->numOfRtsm);
				currentMinimum = getMinimum(serverWindowSize,currentClientHeader.windowSize,congestionWindow);
				if(currentMinimum  == 0) {
					printf("Client window size is  0 \n");
					malarm(2000);
					continue;
				}
				else {
					int numOfAcks = currentClientHeader.ack-currentWaitingAckNumber+1;
					handleAck(numOfAcks,&head, &tail,&currentServerSequenceNumber);
				}
				if((ntohs(head->header.finFlag == 0)) && (ntohs(tail->header.finFlag==1))) {
					continue;
				}
				break;
			}
		}while(1);

	}

}


