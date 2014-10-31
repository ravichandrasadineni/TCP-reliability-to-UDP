#include "fileSender.h"

#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include<math.h>

#include "bufferHandler.h"
#include "childServerUtility.h"
#include "genericUtility.h"
#include "messageHeader.h"
#include "socketinfo.h"
#include "urtt.h"

int CWIN=1,SSThreshold=0,numOfACKSReceived=0;

sigjmp_buf jmpBuf;

static void sig_alarm(int signo) {
	printf("sigLongJump called \n");
	siglongjmp(jmpBuf, 1);
}


int computeCWINandSSThreshold(int scenario,int serverWindowSize,int clientWindowSize,int numberofACKs){
	numOfACKSReceived += numberofACKs;
	if(SSThreshold == 0){
		if(scenario==0){//for Normal ACK
			CWIN = CWIN - numOfACKSReceived;
			CWIN = 2 * numOfACKSReceived;
			numOfACKSReceived = 0;
		}
		else if(scenario==1){//for Duplicate ACK's
			SSThreshold = CWIN/2;
			CWIN = CWIN/2;
			numOfACKSReceived = 0;
		}
		else if(scenario == 2){//for timeout
			SSThreshold = CWIN/2;
			CWIN = 1;
		}
	}
	else{ // Additive Increase of threshold size
		if(scenario == 0 || scenario == 1 || scenario == 2){
			if(CWIN < SSThreshold){
				CWIN = CWIN - numOfACKSReceived;
				CWIN = 2 * numOfACKSReceived;
				if(CWIN > SSThreshold){
					CWIN = SSThreshold;
				}
				numOfACKSReceived = 0;
			}
			else{
				if(numOfACKSReceived == CWIN + 1){
					CWIN ++;
					numOfACKSReceived = 0;
				}
				else{
					printf("The number of ACK's received is greater than CWIN.....Debug\n");
					exit(0);
				}
			}
		}
	}
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, CWIN );
	return currentMinimum;
}


void sendFileAndCloseConnection(int sockfd,  clientInformation currentClientInfo, int currentClientSeqNumber, int currentServerSequenceNumber) {
	int clientWindowSize =  currentClientInfo.clientInitialWindowSize;
	int serverWindowSize  = currentClientInfo.serverWindowSize;
	int isZeroWindowSize = 0;
	//struct sockaddr_in clientAddress = getClientSocketDetails(currentClientInfo);
	//printf("current Sequence number %d \n", currentServerSequenceNumber);
	urtt_info urttInfo;
	urtt_init(&urttInfo);
	hdr currentClientHeader;
	int currentWaitingAckNumber =0;
	int numOfProbesSent = 0;
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, CWIN );
	if(currentMinimum  == 0) {
		isZeroWindowSize = 1;
	}
	//printf("current Minimum is %d \n", currentMinimum);
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
		currentWaitingAckNumber = ntohs(current->header.seq)+1;
		while((currentMinimum  > 0) &&(current!= NULL )) {
			if(current->isSent) {
				currentMinimum--;
				continue;
			}
			else {
				current->isSent = 1;
				current->ts = urtt_ts(&urttInfo);
				sendMessage(sockfd,NULL,&(current->header),current->data);
				if(ntohs(current->header.finFlag) == 1) {
					notDoneSending =0;
					currentMinimum--;
					break;
				}

			}
			current = current->next;
		}
		do {
			//printf("Before recv message \n");
			if(sigsetjmp(jmpBuf,1)!=0) {
				printf("sigsetjmp called \n");
				malarm(0);
				if(isZeroWindowSize) {
					if(numOfProbesSent == 12) {
						printf("Client has  Locked its window. No point Probing More than 12 times. Giving Up \n");
						exit(2);
					}
					sendMessage(sockfd,NULL,&(head->header),head->data);
					malarm(2000);
					numOfProbesSent++;
				}
				else {
					head->numOfAcks =0;
					currentMinimum=computeCWINandSSThreshold(2,serverWindowSize,clientWindowSize,0);
					sendMessage(sockfd,NULL,&(head->header),head->data);
					if((urtt_timeout(&urttInfo,&(head->numOfRtsm)) > 0)) {
						printf("Tried 12 times reaching ipAddress for the following client Giving Up \n");
						printSocketDetailsforSocket(sockfd);
						exit(2);
					}
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
			}
			recvMessage(sockfd,NULL, &currentClientHeader, NULL);
			if(ntohs(currentClientHeader.ack) == currentWaitingAckNumber-1) {
				printf("Duplicate ACK \n");
				head->numOfAcks += 1;
				if(head->numOfAcks >=2) {
					malarm(0);
					head->numOfAcks =0;
					sendMessage(sockfd,NULL,&(head->header),head->data);
					head->numOfRtsm += 1;
					currentMinimum=computeCWINandSSThreshold(1,serverWindowSize,ntohs(currentClientHeader.windowSize),3);
					printf("Fast Retransmit mode : currentMiminimum is %d CWIN is %d and SSThreshold is %d\n",currentMinimum,CWIN,SSThreshold);
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
			}
			else if(ntohs(currentClientHeader.ack)  >= currentWaitingAckNumber){
				currentMinimum=computeCWINandSSThreshold(0,serverWindowSize,ntohs(currentClientHeader.windowSize),ntohs(currentClientHeader.ack)-currentWaitingAckNumber+1);
				printf("Doubling mode : currentMiminimum is %d CWIN is %d and SSThreshold is %d\n",currentMinimum,CWIN,SSThreshold);
				malarm(0);
				unsigned int currentTime = urtt_ts(&urttInfo);
				urtt_stop(&urttInfo,currentTime-head->ts,head->numOfRtsm);
				if(currentMinimum  == 0) {
					isZeroWindowSize = 1;
					printf("Client window size is  0 \n");
					malarm(2000);
					continue;
				}
				else {
					isZeroWindowSize  = 0;
					numOfProbesSent = 0;
					int numOfAcks = ntohs(currentClientHeader.ack)-currentWaitingAckNumber+1;

					handleAck(numOfAcks,&head, &tail,&currentServerSequenceNumber);
					currentWaitingAckNumber = ntohs(head->header.seq)+1;
					printf("currentWaitingAckNumber is %d\n",currentWaitingAckNumber);
				}
				if((ntohs(head->header.finFlag) == 0) && (ntohs(tail->header.finFlag)==1)) {
					printf("before calling continue \n");
					continue;
				}
				printf("Before Break \n");
				break;
			}
		}while(1);

	}
	printf("The child has exited \n");
}


