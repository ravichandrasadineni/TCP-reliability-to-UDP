#include "clientBufferHandler.h"


clientWindowSeg* createWindowSeg() {

	clientWindowSeg*  currentSegment = (clientWindowSeg*)malloc(sizeof(clientWindowSeg));
	currentSegment->isReceived=0;
	currentSegment->header=build_header(0,0,0,0,0,0);
	memset(currentSegment->data,0,488);
	currentSegment->serverSeqNo=0;
	currentSegment->next=NULL;
	return currentSegment;
}

void fillInTheMiddle(hdr recvHeader, char* Message, int diff, sharedBuf *buffer) {
	//printf("diff is %d \n", diff);
	clientWindowSeg * currentPosition;
	currentPosition = buffer->head ;
	int i;
	for(i=0; i<diff;i++) {
		currentPosition = currentPosition->next;
	}
	strncpy(currentPosition->data,Message,488)	;
	currentPosition->header= recvHeader;
	currentPosition->serverSeqNo = ntohs(recvHeader.seq);
	currentPosition->isReceived = 1;
}

void extendWindow(hdr recvHeader, char* Message, sharedBuf *buffer) {
	int currentSeqNo = ntohs(recvHeader.seq);

	int diff = currentSeqNo-(buffer->tail->serverSeqNo);
	int tailSeqNo = (buffer->tail->serverSeqNo);

	int i;
	for (i=0; i<diff;i++) {
		buffer->tail->next = createWindowSeg();
		buffer->tail = buffer->tail->next;
		buffer->tail->serverSeqNo = tailSeqNo+i+1;

	}
	strncpy(buffer->tail->data,Message,488);
	buffer->tail->header = recvHeader;
	buffer->tail->isReceived = 1;
	buffer->tail->serverSeqNo = ntohs(recvHeader.seq);
}

int findWindowSize(int slidingWindowSize, sharedBuf *buffer){
	clientWindowSeg *currentSeg;
	int size = slidingWindowSize,i;
	currentSeg = buffer->head ;
	for(i=1;i<=slidingWindowSize;i++){
		if((currentSeg != NULL )&&(currentSeg->isReceived == 1)){
			size--;
			currentSeg = currentSeg->next;
		}
		else
			break;
	}
	return size;
}

int findACK(int slidingWindowSize, sharedBuf *buffer){
	int ack=buffer->head ->serverSeqNo,i;
	clientWindowSeg *currentSeg;
	currentSeg = buffer->head ;
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

hdr populateClientBuffer(int previousAckNo,int slidingWindowSize,char *Message,hdr recvHeader, sharedBuf *buffer, int *isDone){

	hdr replyHeader;
	int diff,i,currentWindowSize,currentServerSeqNo,ack;
	currentServerSeqNo = ntohs(recvHeader.seq);
	//printf("\nThe current Server sequence number is %d\n",ntohs(recvHeader.seq));
	pthread_mutex_lock(&buffer->mutex);
	if(buffer->head == NULL) {
		buffer->head  =createWindowSeg();
		buffer->tail = buffer->head ;
		buffer->head ->serverSeqNo = previousAckNo;
	}
	if(buffer->head->serverSeqNo > currentServerSeqNo) {
		currentWindowSize = findWindowSize(slidingWindowSize, buffer);
		replyHeader = build_header(0,buffer->head->serverSeqNo,0,0,currentWindowSize,0);
		pthread_mutex_unlock(&buffer->mutex);
		return replyHeader;
	}
	diff = currentServerSeqNo-buffer->head ->serverSeqNo;
	if(diff > slidingWindowSize) {
		printf("diff and SlidingWindowSizes are %d %d \n",diff, slidingWindowSize);
		printf("Sever is Malfunctioning, sent packets more than the window size \n");
		exit(3);
	}
	currentWindowSize = buffer->tail->serverSeqNo - buffer->head ->serverSeqNo;
	if(diff<currentWindowSize ) {
		fillInTheMiddle(recvHeader, Message, diff, buffer);
	}
	else {
		extendWindow(recvHeader, Message, buffer);
	}

	currentWindowSize = findWindowSize(slidingWindowSize, buffer);
	ack = findACK(slidingWindowSize, buffer);
	replyHeader = build_header(0,ack,0,0,currentWindowSize,0);
	buffer->currentSize = slidingWindowSize-currentWindowSize;
	(*isDone) = isFinSetForLastPacket(buffer);
	printCurrentBuffer(buffer);
	pthread_mutex_unlock(&buffer->mutex);

	return replyHeader;
}
