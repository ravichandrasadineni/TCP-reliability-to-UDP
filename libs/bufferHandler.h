#ifndef BUFFER_HANDLER_H
#define BUFFER_HANDLER_H
#include "unp.h"
#include "messageHeader.h"

 struct serverWindowSegement {
 	 hdr header;
 	char *data;
 	unsigned int ts;
 	int numOfRtsm;
 	int numOfAcks;
 	int isSent;
 	struct serverWindowSegement *next;
 	 
 } ; 
 
 typedef struct serverWindowSegement serverWindowSeg;
 void breakfiletoBuffers(char* filename);
 void createInitialServerBuffer(int windowSegmentSize, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber);
 void handleAck(int numOfAck, serverWindowSeg** head, serverWindowSeg** tail, int* currentServerSequenceNumber);
 
 
#endif
