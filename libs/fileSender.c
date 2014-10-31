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

int CWIN=40,SSThreshold=0;

sigjmp_buf jmpBuf;

static void sig_alarm(int signo) {
	printf("sigLongJump called \n");
	siglongjmp(jmpBuf, 1);
}
/*

int computeCWINandSSThreshold(int scenario,int serverWindowSize,int clientWindowSize){
	if(scenario==0){
		CWIN += 1;
		SSThreshold = 0;
	}
	else if(scenario==1){
		SSThreshold = CWIN/2;
		CWIN = CWIN/2;
	}
	else if(scenario == 2){
		SSThreshold = CWIN/2;
		CWIN = 1;
	}
	int currentMinimum = getMinimum(serverWindowSize,clientWindowSize, CWIN );
	return currentMinimum;
}
*/

void sendFileAndCloseConnection(int sockfd,  clientInformation currentClientInfo, int currentClientSeqNumber, int currentServerSequenceNumber) {
	printf("send message and close connection failed \n");
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
		//currentMinimum = computeCWINandSSThreshold(int scenario,int serverWindowSize,int clientWindowSize);
		while((currentMinimum  > 0) &&(current!= NULL )) {
			//printf("%d packets left to sent \n", currentMinimum);
			if(current->isSent) {
				continue;
			}
			else {
				current->isSent = 1;
				current->ts = urtt_ts(&urttInfo);
				sendMessage(sockfd,NULL,&(current->header),current->data);
				//printf("%s\n",current->data);
				if(ntohs(current->header.finFlag) == 1) {
					notDoneSending =0;
					break;
				}
				currentMinimum--;
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
			//currentMinimum=computeCWINandSSThreshold(0,serverWindowSize,clientWindowSize);
			printf("CWIN Doubling mode : currentMiminimum is %d CWIN is %d and SSThreshold is %d\n",currentMinimum,CWIN,SSThreshold);
			if(ntohs(currentClientHeader.ack) == currentWaitingAckNumber-1) {
				printf("Duplicate ACK \n");
				head->numOfAcks += 1;
				if(head->numOfAcks >=2) {
					malarm(0);
					head->numOfAcks =0;
					sendMessage(sockfd,NULL,&(head->header),head->data);
					head->numOfRtsm += 1;
					//currentMinimum=computeCWINandSSThreshold(1,serverWindowSize,clientWindowSize);
					printf("Fast Retransmit mode : currentMiminimum is %d CWIN is %d and SSThreshold is %d\n",currentMinimum,CWIN,SSThreshold);
					malarm(urtt_start(&urttInfo,head->numOfRtsm));
				}
			}
			else if(ntohs(currentClientHeader.ack)  >= currentWaitingAckNumber){
				malarm(0);
				unsigned int currentTime = urtt_ts(&urttInfo);
				urtt_stop(&urttInfo,currentTime-head->ts,head->numOfRtsm);
				currentMinimum = getMinimum(serverWindowSize,ntohs(currentClientHeader.windowSize),CWIN);
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
				}
				if((ntohs(head->header.finFlag) == 0) && (ntohs(tail->header.finFlag)==1)) {
					//printf("before calling continue \n");
					continue;
				}
				break;
			}
		}while(1);

	}
	printf("The child has exited \n");
}


