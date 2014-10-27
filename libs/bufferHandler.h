#ifndef BUFFER_HANDLER_H
#define BUFFER_HANDLER_H
#include "unp.h"
#include "messageHeader.h"

typedef struct {
 	msghdr *header;
 	int ts;
 	int numOfRtsm;
 	int numOfAcks;
 	windowSegment *next;
 	 
 } serverWindowSeg; 
#endif
