#include"genericUtility.h"
#include <math.h>
#define TRUE 1
#define FALSE 0
typedef int bool;

int previousSleepingTime =0;
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
	printf("The return Value is %d \n", returnValue);
	return returnValue;
}


int getSleepingTime(int milliSeconds) {
	milliSeconds  = (int)(-1 *log(1.0/rand()) *milliSeconds);
	previousSleepingTime = milliSeconds;
	printf("sleeping Time of the thread is %d \n", previousSleepingTime);
	return milliSeconds;
}


// USE THIS FUNCTION TO CHECK WETHER THE PACKET HAS TO BE DISCARDED
int shouldDiscard(float probability) {
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
	if(milliseconds > 1000 ) {
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
	printf("the client address is %s \n", clientAddressString);
	printf("the client port is %d\n", ntohs(socketDetails.sin_port));

}



int  clientsendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]) {

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



int  recvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]) {
	struct msghdr msg;
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
		perror("Unable to recv message Error :");
		exit(3);
	}
	return returnValue;
}





