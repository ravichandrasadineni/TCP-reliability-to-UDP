#include "bufferHandler.h"

char **buffer;
int currentReadingPosition = 0;
int initialRandomseqNumber =0;





void breakingfiletoBuffers(char filename){
	int size,n,i,c;
	FILE *fp;
	fp=fopen(filename,"r");
	if(fp==NULL)
	{
		printf("Unable to open file in read mode (or) The file %s is not present!!Please Check\n",filename);
		exit(0);
	}	
	fseek(fp,0L,SEEK_END);
	size=ftell(fp);
	rewind(fp);
	if(size%512==0){
		n=size/512;
	}
	else{
		n=size/512 + 1;
	}
	buffer = (char**)malloc(sizeof(char*)*(n));
	for(i=0;i<n;i++){
		buffer[i]=(char*)malloc(sizeof(char)*512);
	}
	for(i=0;i<n;i++){
		fread(buffer[i],512,1,fp);
	}
	close(fp);
}

void createInitialServerBuffer(int windowSegmentSize, serverWindowSeg* head, serverWindowSeg* tail) {
	// Intializing the random sequence number
	srand((unsigned) time(NULL));
	initialRandomseqNumber = rand()/10000 + 3;
	serverWindowSeg* serverWindow = (serverWindowSeg*)malloc(windowSegmentSize);
	serverWindow->next = NULL;
	head  = serverWindow;
	if(windowSegmentSize ==0) {
		printf("Common !!! Why 0 segement size ?");
		exit(2);
	}
	
	serverWindowSeg* currentSeg = head;
	int i;
	for (i =0; i< windowSegmentSize-1; i++) {
		currentSeg->header = buildmessageHeader(seqNo++);
		currentSeg->next = (serverWindowSeg*)malloc(windowSegmentSize);
		currentSeg->numOfRtsm = 0;
		currentSeg = currentReadingPosition->next;
		currentSeg->next = NULL;
	}
	tail = currentSeg;
}


void handleAck(int numOfAck, serverWindowSeg* head, serverWindowSeg* tail) {
	int i;
	serverWindowSeg *currentHead =NULL;
	serverWindowSeg *currentTail =NULL; 
	for( i=0; i < numOfAck; i++) {
		currentHead = head->next;
		free(head);
		head = currentHead;
	}
	
	for( i=0; i < numOfAck; i++) {
		currentTail = tail;
		tail =(serverWindowSeg*)malloc(windowSegmentSize);
		tail->next = NULL;
		currentTail->header = buildmessageHeader(seqNo++);
		currentTail->next= tail;
	}

}


msghdr buildmessageHeader(int seqNo) {
	int sizeOfBuffer = (sizeof(buffer)/sizeof(char*));
	struct msghdr msg;
	struct iovec iov[2];
	hdr currentHeader;
	currentHeader.seq = seqNo; 
	if(currentReadingPosition >= sizeOfBuffer) {
		return NULL;
	}
	else {
		msg.msg_name = NULL;
		msg.msg_Namelen = 0;
		msg.msg_iov = iov;
		msg.msg_iovlen = 2;
		iov[0].iov_base = &currentHeader;
		iov[0].iov_len = sizeof(currentHeader);
		iov[1].iov_base = buffer + currentReadingPosition;
		iov[1].iov_len = 512;
		//Incrementing the current position
		// Append null charcter at the client
		currentReadingPosition++;
	}
	return msghdr;
}






