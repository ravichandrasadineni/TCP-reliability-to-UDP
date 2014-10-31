#include "bufferHandler.h"

char **buffer;
int currentReadingPosition = 0;
int segmentsInFile = 0;

void buildHeaderAndData(hdr* messageHeader, char** data, int currentServerSequenceNumber);
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
	if(size%488==0){
		n=size/488;
	}
	else{
		n=size/488 + 1;
	}
	segmentsInFile = n;
	buffer = (char**)malloc(sizeof(char*)*(n));
	for(i=0;i<n;i++){
		buffer[i]=(char*)malloc(sizeof(char)*488);
	}
	for(i=0;i<n;i++){
		fread(buffer[i],488,1,fp);
	}


	fclose(fp);
}

void createInitialServerBuffer(int windowSegmentSize, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber) {
	// Intializing the random sequence number
	if(windowSegmentSize ==0) {
		printf("0 segement size , can't send packets'");
		exit(2);
	}

	serverWindowSeg* currentSeg = NULL;
	int i;
	for (i =0; i< windowSegmentSize; i++) {
		if(currentSeg == NULL) {
			currentSeg = (serverWindowSeg*)malloc(sizeof(serverWindowSeg));
			*head =  currentSeg;
			*currentServerSequenceNumber = *currentServerSequenceNumber +1;
			buildHeaderAndData(&(currentSeg->header), &(currentSeg->data), (*currentServerSequenceNumber));
			currentSeg->numOfRtsm = 0;
			currentSeg->isSent = 0;
			currentSeg->next = NULL;
		}
		else {
			currentSeg->next = (serverWindowSeg*)malloc(sizeof(serverWindowSeg));
			currentSeg = currentSeg->next;
			*currentServerSequenceNumber = *currentServerSequenceNumber +1;
			buildHeaderAndData(&(currentSeg->header), &(currentSeg->data), (*currentServerSequenceNumber));
			currentSeg->numOfRtsm = 0;
			currentSeg->isSent = 0;
			currentSeg->next = NULL;
		}
	}
	*tail = currentSeg;
	printf("The head Pointer is pointing to %d \n", ntohs((*head)->header.seq));
	printf("The tail Pointer is pointing to %d \n", ntohs((*tail)->header.seq));
}


void handleAck(int numOfAck, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber) {
	printf("Num of Acks is %d \n", numOfAck);
	int i;
	serverWindowSeg* headPointer = *head;
	serverWindowSeg* tailPointer = *tail;
	serverWindowSeg *currentHead =NULL;
	serverWindowSeg *currentTail =NULL; 
	for( i=0; i < numOfAck; i++) {
		currentHead = headPointer->next;
		free(headPointer);
		headPointer = currentHead;
	}

	for( i=0; i < numOfAck; i++) {
		currentTail = tailPointer;
		tailPointer =(serverWindowSeg*)malloc(sizeof(serverWindowSeg));
		tailPointer->next = NULL;
		tailPointer->numOfRtsm = 0;
		tailPointer->isSent = 0;
		*currentServerSequenceNumber = *currentServerSequenceNumber +1;
		buildHeaderAndData(&(tailPointer->header),&(tailPointer->data),(*currentServerSequenceNumber) );
		currentTail->next= tailPointer;
		printf("The number of segments in file is %d\n",segmentsInFile);

	}
	*head = headPointer;
	*tail = tailPointer;
	printf("The head Pointer is pointing to %d \n", ntohs((*head)->header.seq));
	printf("The tail Pointer is pointing to %d \n", ntohs((*tail)->header.seq));
}



void buildHeaderAndData(hdr* messageHeader, char** data, int currentServerSequenceNumber) {

	printf("currentReadingPosition is %d \n", currentReadingPosition);
	if(currentReadingPosition >= segmentsInFile) {
			printf("is a Fin Packet \n");
			(*messageHeader) = build_header(currentServerSequenceNumber, 0,0, 1,0,0);
			(*data) = NULL;
	}
	else {
		(*messageHeader) = build_header(currentServerSequenceNumber, 0,0, 0,0,0);
		(*data) = buffer[currentReadingPosition++];
	}

}







