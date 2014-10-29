#include "messageHeader.h"


hdr build_header(int seqNo, int ack, int synFlag, int finFlag, int windowSize, int congestionSize) {
	hdr messageHeader; 
	messageHeader.seq = htons(seqNo);
	messageHeader.ack = htons(ack);
	messageHeader.synFlag = htons(synFlag);
	messageHeader.finFlag = htons(finFlag);
	messageHeader.windowSize = htons(windowSize);
	messageHeader.congestionSize = htons(congestionSize);
	return messageHeader;
} 

