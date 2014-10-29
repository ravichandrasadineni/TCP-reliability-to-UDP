#ifndef CLIENT_BUFFER_HANDLER_H
#define CLIENT_BUFFER_HANDLER_H
#include "unp.h"
#include "messageHeader.h"
#include "bufferHandler.h"
struct clientWindowSegement {
	hdr header;
	int serverSeqNo;
	char *data;
	int isReceived;
	struct clientWindowSegement *next;
}; 

typedef struct clientWindowSegement clientWindowSeg;
//void createClientSlidingWindow(clientWindowSeg **head,int slidingWindowSize);
hdr populateClientBuffer(int previousSeqNo,int slidingWindowSize,char *Message,hdr recvHeader);
//void breakfiletoBuffers(char* filename);
//void createInitialServerBuffer(int windowSegmentSize, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber);
//void handleAck(int numOfAck, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber);
#endif
