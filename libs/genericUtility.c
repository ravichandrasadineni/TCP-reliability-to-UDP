#include "genericUtility.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>

#include "unp.h"
#include "messageHeader.h"

#define TRUE 1
#define FALSE 0
typedef int bool;
int previousSleepingTime =0;
int  recvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]);
int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]);

bool is_valid_int (char* integerString) {	
	while (*integerString)
	{
		if (isdigit (*integerString) ) {
			integerString++;
		}
		else {
			return FALSE;
		}
	}
	return TRUE;

}


int getRandomSequenceNumber(int base) {
	int returnValue = rand()%base;
	return returnValue;
}

struct timeval getTimeToWait(struct timeval base, int currentWaitMilliSecs) {
	struct timeval	tv;
	struct timeval diff;
	struct timeval currentWaitTime;
	currentWaitTime.tv_sec =currentWaitMilliSecs/1000;
	currentWaitMilliSecs= currentWaitMilliSecs%1000;
	currentWaitTime.tv_usec= currentWaitMilliSecs*1000;
	currentWaitTime.tv_usec += 1000;
	Gettimeofday(&tv, NULL);
	diff.tv_sec = (tv.tv_sec - base.tv_sec);
	diff.tv_usec = (tv.tv_usec - base.tv_usec);
	currentWaitTime.tv_sec  -= 	diff.tv_sec;
	currentWaitTime.tv_usec -= diff.tv_usec;

	return(currentWaitTime);
}


int getSleepingTime(int milliSeconds) {
	double r = ((double) rand() / (RAND_MAX));
	milliSeconds  = (int)(-1 *log(r) *milliSeconds);
	previousSleepingTime = milliSeconds;
	return milliSeconds;
}


// USE THIS FUNCTION TO CHECK WETHER THE PACKET HAS TO BE DISCARDED
int shouldDiscard(float probability) {
	//printf("The probability value in shouldDiscard function is %f\n",probability);
	if(( rand()%100 ) <= (probability*100)) {

		return 1;
	}

	return 0;


}

char *trimwhitespace(char *str)
{
	char *end;
	while(isspace(*str)) str++;
	if(*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	*(end+1) = 0;
	return str;
}

unsigned int
malarm (unsigned int milliseconds)
{

	struct itimerval old, new;
	new.it_interval.tv_usec = 0;
	new.it_interval.tv_sec = 0;
	new.it_value.tv_usec = 0;
	new.it_value.tv_sec = 0;
	if(milliseconds >= 1000 ) {
		new.it_value.tv_sec = milliseconds/1000;
	}
	new.it_value.tv_usec = (milliseconds%1000)*1000;
	if (setitimer (ITIMER_REAL, &new, &old) < 0) {
		perror("setitimer failed ErrorNo :");
		exit(2);
	}
	else
		return old.it_value.tv_sec;
}


unsigned int
salarm (unsigned int seconds)
{
	struct itimerval old, new;
	new.it_interval.tv_usec = 0;
	new.it_interval.tv_sec = 0;
	new.it_value.tv_usec = 0;
	new.it_value.tv_sec = (long int) seconds;
	if (setitimer (ITIMER_REAL, &new, &old) < 0)
		return 0;
	else
		return old.it_value.tv_sec;
}


struct msghdr buildMessage(struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]) {
	struct msghdr msg;

	memset(&msg, 0, sizeof(msg));
	if(data == NULL) {
		struct iovec iov1[1];
		printf("message iov length is 1 \n");
		iov1[0].iov_base = messageHeader;
		iov1[0].iov_len = sizeof(*messageHeader);
		msg.msg_iov = iov1;
		msg.msg_iovlen = 1;
	}
	else {
		struct iovec iov2[2];
		iov2[0].iov_base = messageHeader;
		iov2[0].iov_len = sizeof(*messageHeader);
		iov2[1].iov_base = data;
		iov2[1].iov_len = 488;
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;	
	}

	if(msg_name == NULL) {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	else {
		SA* saddress = (SA*)msg_name;
		msg.msg_name = saddress;
		msg.msg_namelen = sizeof(SA);
	}

	return msg;
}


void printSocketDetailsforSocket(int sockfd) {
	struct sockaddr_in socketDetails;
	int len = INET_ADDRSTRLEN;
	getpeername(sockfd,(SA*)&socketDetails,&len);
	char clientAddressString[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&(socketDetails.sin_addr),clientAddressString,INET_ADDRSTRLEN);
	printf("the peer socket address is %s \n", clientAddressString);
	printf("the peer socket port is %d\n", ntohs(socketDetails.sin_port));

}





int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]) {
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	if(data == NULL) {
		struct iovec iov1[1];
		iov1[0].iov_base = messageHeader;
		iov1[0].iov_len = sizeof(*messageHeader);
		msg.msg_iov = iov1;
		msg.msg_iovlen = 1;
	}
	else {
		struct iovec iov2[2];
		iov2[0].iov_base = messageHeader;
		iov2[0].iov_len = sizeof(*messageHeader);
		iov2[1].iov_base = data;
		iov2[1].iov_len = 488;
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;	
	}

	if(msg_name == NULL) {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	else {
		SA* saddress = (SA*)msg_name;
		msg.msg_name = saddress;
		msg.msg_namelen = sizeof(SA);
	}
	int returnValue =  sendmsg(sockfd,&msg, 0);
	if(returnValue  < 0) {
		perror("Unable to send message Error :");
		exit(3);
	}
	return returnValue;
}

int  clientsendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488],float randomSeed) {
	int discardthePacket = shouldDiscard(randomSeed);
	int count;
	if(discardthePacket == 1){
		printf("segment with acknowledgment Number  %d  and WindowSize is  %d Discarded  While sending  \n",ntohs(messageHeader->ack),ntohs(messageHeader->windowSize) );
		return -2;
	}
	printf("segment with acknowledgment number  %d  and WindowSize %d sent successfully \n",ntohs(messageHeader->ack),ntohs(messageHeader->windowSize));
	count = sendMessage(sockfd,msg_name,messageHeader,data);
	return count;
}

int  recvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]) {
	struct msghdr msg;
	char data2[488];
	socklen_t clientAddressLength = INET_ADDRSTRLEN;
	memset(&msg, 0, sizeof(msg));
	if(data == NULL) {
		struct iovec iov1[1];
		iov1[0].iov_base = messageHeader;
		iov1[0].iov_len = sizeof(*messageHeader);
		msg.msg_iov = iov1;
		msg.msg_iovlen = 1;
	}
	else {
		struct iovec iov2[2];
		iov2[0].iov_base = messageHeader;
		iov2[0].iov_len = sizeof(*messageHeader);
		iov2[1].iov_base = data;
		iov2[1].iov_len = 488;
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;	
	}

	if(msg_name == NULL) {
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
	}
	else {
		SA* saddress = (SA*)msg_name;
		msg.msg_name = saddress;
		msg.msg_namelen = clientAddressLength;

	}
	int returnValue =  recvmsg(sockfd,&msg, 0);
	if(returnValue  < 0) {
		exit(3);
	}
	return returnValue;
}

void printfBuffer(sharedBuf *buffer) {
	int i, size =0;
	clientWindowSeg* pointer = buffer->head;
	for(i=0; i<buffer->currentSize;i++ ) {
		clientWindowSeg* currentSegment = pointer;
		if(ntohs(pointer ->header.finFlag) == 1) {
			buffer->isDone = 1;
			printf("File printed successfully \n");
			break;
		}
		printf("%s \n",pointer->data);

		pointer = pointer->next;
		if(pointer != NULL)
			free(currentSegment);
	}
	buffer->head=pointer;
	buffer->currentSize =0;

}


void  printCurrentBuffer(sharedBuf *buffer) {
	clientWindowSeg* pointer = buffer->head;
	printf ("current client sliding Window contents :");
	for(pointer=buffer->head; pointer!=NULL;pointer= pointer->next) {
		if(pointer->isReceived) {
			printf(" %d ", pointer->serverSeqNo);
		}
		else {
			printf (" X ");
		}
	}
	printf("\n");
}

int isFinSetForLastPacket(sharedBuf *buffer) {
	int i, size =0;
	clientWindowSeg* pointer = buffer->head;
	for(i=0; i<buffer->currentSize;i++ ) {
		clientWindowSeg* currentSegment = pointer;
		if(ntohs(pointer ->header.finFlag) == 1) {
			return 1;
		}
		pointer = pointer->next;
	}
	return 0;
}


int  clientrecvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488],float randomSeed) {
	int discardthePacket,returnValue;

	while(1) {
		returnValue = recvMessage(sockfd,msg_name,messageHeader,data);
		if(returnValue  <0) {
			perror(" Client Recv Message Failed :");
		}
		discardthePacket = shouldDiscard(randomSeed);
		if(discardthePacket == 1){
			printf("segment with server sequence number %d is discarded while receiving \n",ntohs(messageHeader->seq));
			continue;

		}
		else {
			printf("segment with server sequence number %d is received \n",ntohs(messageHeader->seq));
			break;
		}
	}
	return  returnValue;
	//return recvMessage(sockfd,msg_name,messageHeader,data);
}



