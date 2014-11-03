/*
 * threadUtility.h
 *
 *  Created on: Oct 31, 2014
 *      Author: ravichandrasadineni
 */

#ifndef LIBS_THREADUTILITY_H_
#define LIBS_THREADUTILITY_H_

#include <pthread.h>
#include <semaphore.h>

#include "messageHeader.h"

struct clientWindowSegement {
	hdr header;
	int serverSeqNo;
	char data[488];
	int isReceived;
	struct clientWindowSegement *next;
};

typedef struct clientWindowSegement clientWindowSeg;

typedef struct
{

	clientWindowSeg*  head;
	clientWindowSeg*  tail;
	struct timeval tsThreadSlept;
	int currentWaitmilliSecs;
	int currentSize; // currentSize of the buffer
    int isDone;
    pthread_mutex_t mutex;
} sharedBuf;

void initializeBuffer(sharedBuf *buffer, int sliWindowsize);

#endif /* LIBS_THREADUTILITY_H_ */
