#include "childServerUtility.h"

clientInformation  proccessClientInfo(int argc, char* argv[]) {
	
 	int i;
 	clientInformation currentclientInformation;
 	if(argc < 8) {
 		printf("Parent Server has not sent all the information required to the child \n");
 		exit(2);
 	} 
 	
 	strcpy(currentclientInformation.filename, argv[0]);
 	
 	if((inet_pton(AF_INET,argv[1],&(currentclientInformation.ipAddress)))<=0) {
			perror("inet_pton has failed for ipAddress in the childServer\n");
			exit(0);
	}
	
	if((inet_pton(AF_INET,argv[7],&(currentclientInformation.subnetMask)))<=0) {
			perror("inet_pton has failed for subnetMask in the childServer\n");
			exit(0);
	}
	
	currentclientInformation.port = atoi(argv[2]);
	currentclientInformation.currentSocketDiscriptor = atoi(argv[3]);
	currentclientInformation.clientInitialWindowSize = atoi(argv[4]);
	currentclientInformation.serverWindowSize = atoi(argv[5]); 
	currentclientInformation.clientSeqNumber = atoi(argv[6]);
 	return currentclientInformation;
}
