/*
 * threadUtility.c
 *
 *  Created on: Oct 31, 2014
 *      Author: ravichandrasadineni
 */

#include "threadUtility.h"

#include <stddef.h>

void initializeBuffer(sharedBuf *buffer, int clientWindowSize) {
	buffer->head = NULL;
	buffer->tail = NULL;
	buffer->tsThreadSlept.tv_sec = 0;
	buffer->tsThreadSlept.tv_usec = 0;
	buffer->currentSize = 0;
	buffer->isDone = 0;
	pthread_mutex_init(&(buffer->mutex), NULL);
}


