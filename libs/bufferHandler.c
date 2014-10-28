#include "bufferHandler.h"

char **buffer;
int currentReadingPosition = 0;
int segmentsInFile = 0;

void buildHeaderAndData(hdr messageHeader, char* data, int currentServerSequenceNumber);
void breakfiletoBuffers(char* filename){
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
	segmentsInFile = n;
	buffer = (char**)malloc(sizeof(char*)*(n));
	for(i=0;i<n;i++){
		buffer[i]=(char*)malloc(sizeof(char)*512);
	}
	for(i=0;i<n;i++){
		fread(buffer[i],512,1,fp);
	}
	fclose(fp);
}

void createInitialServerBuffer(int windowSegmentSize, serverWindowSeg* head, serverWindowSeg* tail, int* currentServerSequenceNumber) {
	// Intializing the random sequence number
	head  = NULL;
	if(windowSegmentSize ==0) {
		printf("0 segement size , can't send packets'");
		exit(2);
	}
	
	serverWindowSeg* currentSeg = head;
	int i;
	for (i =0; i< windowSegmentSize; i++) {
		if(currentSeg == NULL) {
			currentSeg = (serverWindowSeg*)malloc(sizeof(serverWindowSeg));
		}
		else {
			currentSeg->next = (serverWindowSeg*)malloc(sizeof(serverWindowSeg));
			currentSeg = currentSeg->next;
			buildHeaderAndData(currentSeg->header, currentSeg->data, (*currentServerSequenceNumber));
			*currentServerSequenceNumber = *currentServerSequenceNumber +1;
			currentSeg->numOfRtsm = 0;
			currentSeg->isSent = 0;
			currentSeg->next = NULL;
		}
	}
	tail = currentSeg;
}


void handleAck(int numOfAck, serverWindowSeg* head, serverWindowSeg* tail, int* currentServerSequenceNumber) {
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
		tail =(serverWindowSeg*)malloc(sizeof(serverWindowSeg));
		tail->next = NULL;
		tail->numOfRtsm = 0;
		tail->isSent = 0;
		buildHeaderAndData(tail->header,tail->data,(*currentServerSequenceNumber) );
		*currentServerSequenceNumber = *currentServerSequenceNumber +1;
		currentTail->next= tail;
	}

}



void buildHeaderAndData(hdr messageHeader, char* data, int currentServerSequenceNumber) {
	 hdr currentHeader;
	
	
	 if(currentReadingPosition >= segmentsInFile) {
		currentHeader = build_header(currentServerSequenceNumber, 0,0, 1,0,0);
		data = NULL;
	}
	else {
		currentHeader = build_header(currentServerSequenceNumber, 0,0, 0,0,0);
		data = buffer[currentReadingPosition++];
	}

}







