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
	printf("Timed Out \n");
	siglongjmp(jmpBuf, 1);
}


void printCurrentInFlight(int serverWindowSize, serverWindowSeg* head, serverWindowSeg* tail) {
	printf("current inFLight Segments :");
	serverWindowSeg* current = head;
	int i;
	for(i=0; i<serverWindowSize;i++) {
		if(current ->isSent== 1) {
			printf(" %d ", ntohs(current->header.seq));
			current = current->next;

		}

	}
	printf("\n");
}
int computeCWINandSSThreshold(int scenario,int serverWindowSize, serverWindowSeg* head, serverWindowSeg* tail,int clientWindowSize,int numberofACKs){
	int currentFull = 0, i;
	serverWindowSeg* current = head;
	for(i=0; i<serverWindowSize;i++) {
		if(current ->isSent== 1) {
			current = current->next;
			currentFull++;
		}
		else {
			break;
		}
	}
	serverWindowSize-= currentFull;
	numOfACKSReceived += numberofACKs;
	printf("Server Window Size : %d , Advertising window Size : %d\n",serverWindowSize,clientWindowSize);
	if(SSThreshold == 0){
		if(scenario==0){//for Normal ACK
			CWIN -= numOfACKSReceived;
			CWIN = CWIN + 2 * numOfACKSReceived;
			numOfACKSReceived = 0;

		}
		else if(scenario==1){//for Duplicate ACK's
			SSThreshold = CWIN/2;
			CWIN = CWIN/2;
			if(CWIN  < 1) {
				CWIN  = 1;
			}
			numOfACKSReceived = 0;
		}
		else if(scenario == 2){//for timeout
			SSThreshold = CWIN/2;
			CWIN = 1;
			numOfACKSReceived = 0;
		}
	}
	else{ // Additive Increase of threshold size
		if(scenario == 0  ){
			if(CWIN < SSThreshold){
				CWIN = CWIN - numOfACKSReceived;
				CWIN += 2 * numOfACKSReceived;
				if(CWIN > SSThreshold){
					CWIN = SSThreshold;
				}
				numOfACKSReceived = 0;
			}
			else{
				if(numOfACKSReceived == CWIN){
					CWIN ++;
					numOfACKSReceived = 0;
				}
			}
		}
		else if (scenario  == 1) {
			SSThreshold = CWIN/2;
			CWIN = CWIN/2;
			if(CWIN  < 1) {
				CWIN  = 1;
			}
			if(SSThreshold  <2 ) {
				SSThreshold =2;
			}
			numOfACKSReceived = 0;

		}
		else if (scenario == 2) {
			SSThreshold = CWIN/2;
			CWIN = 1;
			if(CWIN  < 1) {
				CWIN  = 1;
			}
			numOfACKSReceived = 0;
			if(SSThreshold  <2 ) {
				SSThreshold =2;
			}

		}
	}
	printCurrentInFlight(serverWindowSize,head, tail);
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, CWIN );
	if(scenario == 0 && SSThreshold == 0)
		printf("Doubling mode : CWIN is %d and SSThreshold is NOT SET\n",CWIN);
	else if(scenario == 0 && SSThreshold > 1)
		printf("Doubling mode : CWIN is %d and SSThreshold is %d\n",CWIN,SSThreshold);

	else if(scenario == 1 && SSThreshold == 0)
		printf("Fast Retransmit mode : CWIN is %d and SSThreshold is NOT SET\n",CWIN);
	else if(scenario == 1 && SSThreshold > 1)
		printf("Fast Retransmit mode : CWIN is %d and SSThreshold is %d\n",CWIN,SSThreshold);

	if(scenario == 2 && SSThreshold == 0)
		printf("Timeout mode : CWIN is %d and SSThreshold is NOT SET\n",CWIN);
	else if (scenario == 2 && SSThreshold > 1)
		printf("Timeout mode : CWIN is %d and SSThreshold is %d\n",CWIN,SSThreshold);
	return currentMinimum;
}


void sendFileAndCloseConnection(int sockfd,  clientInformation currentClientInfo, int currentClientSeqNumber, int currentServerSequenceNumber) {
	int clientWindowSize =  currentClientInfo.clientInitialWindowSize;
	int serverWindowSize  = currentClientInfo.serverWindowSize;
	int isZeroWindowSize = 0;
	urtt_info urttInfo;
	urtt_init(&urttInfo);
	hdr currentClientHeader;
	int currentWaitingAckNumber =0;
	int numOfProbesSent = 0;
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, CWIN );
	if(currentMinimum  == 0) {
		isZeroWindowSize = 1;
	}
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
				current = current->next;
				continue;
			}
			else {
				current->isSent = 1;
				current->ts = urtt_ts(&urttInfo);
				printf("SEQ NO %d sent \n",ntohs(current->header.seq));
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
			if(sigsetjmp(jmpBuf,1)!=0) {
				head->numOfAcks =0;
				currentMinimum=computeCWINandSSThreshold(2,serverWindowSize,head,tail,clientWindowSize,0);
				printf("SEQ NO %d sent \n",ntohs(current->header.seq));
				sendMessage(sockfd,NULL,&(head->header),head->data);
				if((urtt_timeout(&urttInfo,&(head->numOfRtsm)) > 0)) {
					printf("Tried 12 times reaching ipAddress for the below client Giving Up \n");
					printSocketDetailsforSocket(sockfd);
					exit(2);
				}
				malarm(urtt_start(&urttInfo,head->numOfRtsm));
			}
			recvMessage(sockfd,NULL, &currentClientHeader, NULL);
			printf("ACK %d received \n", ntohs(currentClientHeader.ack));
			if(ntohs(currentClientHeader.ack) == currentWaitingAckNumber-1) {
				head->numOfAcks += 1;
				if(head->numOfAcks >=2) {
					printf("FAST RETRANSMIT \n");
					malarm(0);
					head->numOfAcks =0;
					printf("SEQ NO %d sent \n",ntohs(current->header.seq));
					sendMessage(sockfd,NULL,&(head->header),head->data);
					head->numOfRtsm += 1;
					currentMinimum=computeCWINandSSThreshold(1,serverWindowSize,head,tail,ntohs(currentClientHeader.windowSize),3);
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
				else {
					malarm(2000);
				}
			}
			else if(ntohs(currentClientHeader.ack)  >= currentWaitingAckNumber){
				currentMinimum=computeCWINandSSThreshold(0,serverWindowSize,head,tail,ntohs(currentClientHeader.windowSize),ntohs(currentClientHeader.ack)-currentWaitingAckNumber+1);
				malarm(0);
				unsigned int currentTime = urtt_ts(&urttInfo);
				urtt_stop(&urttInfo,currentTime-head->ts,head->numOfRtsm);
				if(currentMinimum  == 0) {
					numOfProbesSent++;
					if(numOfProbesSent >= 12) {
						printf("Client has  Locked its window. No point Probing More than 12 times. Giving Up \n");
						exit(0);
					}
					if((!notDoneSending) &&(ntohs(head->header.finFlag)==1)) {
						exit(0);
					}
					isZeroWindowSize = 1;
					printf("Client window size is  0 \n");
					malarm(2000);
					sleep(2);
					continue;
				}
				else {
					isZeroWindowSize  = 0;
					numOfProbesSent = 0;
					int numOfAcks = ntohs(currentClientHeader.ack)-currentWaitingAckNumber+1;
					handleAck(numOfAcks,&head, &tail,&currentServerSequenceNumber);
					currentWaitingAckNumber = ntohs(head->header.seq)+1;
				}

				if((!notDoneSending) &&(ntohs(head->header.finFlag)!=1)) {
					continue;
				}
				break;
			}
		}while(1);

	}
	printf("File Transferred Successfully. Child has exited\n");
}


