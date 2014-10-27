#include "unp.h"
#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H


typedef struct {
	int seq;
	int ack;
	int synFlag;
	int finFlag;	
	int windowSize; 
	int congestionSize;
} hdr;

hdr build_header(int seqNo, int ack,int synFlag, int finFlag, int windowSize, int congestionSize);

#endif

