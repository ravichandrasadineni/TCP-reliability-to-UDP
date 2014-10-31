#include "clientBufferHandler.h"


clientWindowSeg *head = NULL, *tail=NULL;
clientWindowSeg* createWindowSeg() {

	clientWindowSeg*  currentSegment = (clientWindowSeg*)malloc(sizeof(clientWindowSeg));
	currentSegment->isReceived=0;
	currentSegment->header=build_header(0,0,0,0,0,0);
	currentSegment->data=NULL;
	currentSegment->serverSeqNo=0;
	currentSegment->next=NULL;
	return currentSegment;
}

void fillInTheMiddle(hdr recvHeader, char* Message, int diff) {
	//printf("diff is %d \n", diff);
	clientWindowSeg * currentPosition;
	currentPosition = head;
	int i;
	for(i=0; i<diff;i++) {
		currentPosition = currentPosition->next;
	}
	currentPosition->data=Message;
	currentPosition->header= recvHeader;
	currentPosition->serverSeqNo = ntohs(recvHeader.seq);
	currentPosition->isReceived = 1;
}

void extendWindow(hdr recvHeader, char* Message) {
	int currentSeqNo = ntohs(recvHeader.seq);

	int diff = currentSeqNo-(tail->serverSeqNo);

	int i;
	for (i=0; i<diff;i++) {
		tail->next = createWindowSeg();
		tail = tail->next;
	}
	tail->data = Message;
	tail->header = recvHeader;
	tail->isReceived = 1;
	tail->serverSeqNo = ntohs(recvHeader.seq);
}

int findWindowSize(int slidingWindowSize){
	clientWindowSeg *currentSeg;
	int size = slidingWindowSize,i;
	currentSeg = head;
	for(i=1;i<=slidingWindowSize;i++){
		if((currentSeg != NULL )&&(currentSeg->isReceived == 1)){
			size--;
			currentSeg = currentSeg->next;
		}
		else
			break;
	}
	//printf("current windowsize is %d \n",size);
	return size;
}

int findACK(int slidingWindowSize){
	int ack=head->serverSeqNo,i;
	clientWindowSeg *currentSeg;
	currentSeg = head;
	for(i=0;i<slidingWindowSize;i++){
		if((currentSeg != NULL )&&(currentSeg->isReceived == 1)){
			ack++;
			currentSeg = currentSeg->next;
		}
		else
			break;
	}
	//printf("current ack is %d \n",ack);
	return ack;
}

hdr populateClientBuffer(int previousAckNo,int slidingWindowSize,char *Message,hdr recvHeader){
	printf("%s",Message);
	hdr replyHeader;
	int diff,i,currentWindowSize,currentServerSeqNo,ack;
	currentServerSeqNo = ntohs(recvHeader.seq);
	//printf("\nThe current Server sequence number is %d\n",ntohs(recvHeader.seq));
	if(head == NULL) {
		head =createWindowSeg();
		tail = head;
		head->serverSeqNo = previousAckNo;
	}
	diff = currentServerSeqNo-head->serverSeqNo;
	if(diff > slidingWindowSize) {
		printf("diff and SlidingWindowSizes are %d %d \n",diff, slidingWindowSize);
		printf("Sever is Malfunctioning, sent packets more than the window size \n");
		exit(3);
	}
	currentWindowSize = tail->serverSeqNo - head->serverSeqNo;
	if(diff<currentWindowSize ) {
		fillInTheMiddle(recvHeader, Message, diff);
	}
	else {
		extendWindow(recvHeader, Message);
	}
	currentWindowSize = findWindowSize(slidingWindowSize);
	ack = findACK(slidingWindowSize);
	replyHeader = build_header(0,ack,0,0,currentWindowSize,0);
	return replyHeader;
}
