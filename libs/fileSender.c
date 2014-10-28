#include "fileSender.h"



sigjmp_buf jmpBuf;

static void sig_alarm(int signo) {

	siglongjmp(jmpBuf, 1);
}

void sendFileAndCloseConnection(int sockfd,  int clientWindowSize, int serverWindowSize, int currentClientSeqNumber, int currentServerSequenceNumber) {
	urtt_info urttInfo;
	urtt_init(&urttInfo);
	hdr currentClientHeader;
	int congestionWindow = 1000;
	int currentWaitingAckNumber =0;
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, congestionWindow );
	int notDoneSending = 1;
	serverWindowSeg *head = NULL, *tail = NULL;
	createInitialServerBuffer(serverWindowSize,head,tail, &currentServerSequenceNumber);
	Signal(SIGALRM, sig_alarm);
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
				if(current->header.finFlag == 1) {
					notDoneSending =0;
					break;
				}
				currentMinimum--;
			}
			current = current->next;
		}
		do {
			recvMessage(sockfd,NULL, &currentClientHeader, NULL);
			if(currentClientHeader.ack == currentWaitingAckNumber-1) {
				head->numOfAcks += 1;
				if(head->numOfAcks >=4) {

				}

			}
			else if(currentClientHeader.ack  >= currentWaitingAckNumber){
				malarm(0);
				unsigned int currentTime = urtt_ts(&urttInfo);
				urtt_stop(&urttInfo,currentTime-head->ts,head->numOfRtsm);
				int numOfAcks = currentClientHeader.ack-currentWaitingAckNumber+1;
				handleAck(numOfAcks,head, tail,&currentServerSequenceNumber);
				currentMinimum = getMinimum(serverWindowSize,currentClientHeader.windowSize,congestionWindow);
				if((head->header.finFlag == 0) && (tail->header.finFlag==1)) {
					continue;
				}
				break;
			}

		}while(1);




	}

}


