#include"libs/portutility.h"
#include"libs/genericUtility.h"
#include"libs/fileUtility.h"
#include "libs/socketinfo.h"
#include "libs/threadUtility.h"
sharedBuf sharedBuffer;
int portNumber=0,sliWindowsize=0,randSeed=0,milliSec=0;
float prob=0;
char filename[488];
struct sockaddr_in ipAddress;

// producer thread
void *produce(void *arg) {
	sockinfo clientSocketInfo[100];
	int sockfd;
	char ipAddrString[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&(ipAddress.sin_addr),ipAddrString,INET_ADDRSTRLEN);
	sockfd=getClientBindingSocket(&ipAddress,portNumber,clientSocketInfo);
	handleServer(sockfd,ipAddress, sliWindowsize, filename, clientSocketInfo, &sharedBuffer, &prob);


	return NULL;
}


// consumer thread
void *consume(void *arg) {
	clientWindowSeg* currentHead;
	while(1) {
		pthread_mutex_lock(&(sharedBuffer.mutex));
		if(sharedBuffer.currentSize >0) {

			printfBuffer(&sharedBuffer);
		}
		if(sharedBuffer.isDone == 1) {
			pthread_mutex_unlock(&sharedBuffer.mutex);
			break;
		}
		pthread_mutex_unlock(&sharedBuffer.mutex);
		int currentMilliSec = getSleepingTime(milliSec);
		if(currentMilliSec > 1000) {
			sleep(currentMilliSec/1000);
		}
		else {
			usleep(currentMilliSec*1000);
		}
	}

	return NULL;
}


int main(int argc,char *argv[])
{
	pthread_t producer, consumer;
	readingClientInput(&ipAddress,&portNumber,filename,&sliWindowsize,&randSeed,&prob,&milliSec); //in fileUtility.c
	srand((unsigned) randSeed);
	initializeBuffer(&sharedBuffer,sliWindowsize);

	//pthread_create(&producer, NULL, (void*)&produce, NULL);
	pthread_create(&consumer, NULL, (void*)&consume, NULL);
	produce(NULL);
	//pthread_join(producer, NULL);
	pthread_join(consumer, NULL);
	return(0);
}

