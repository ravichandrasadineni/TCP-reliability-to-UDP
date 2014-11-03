#include"clientHandler.h"
int clientsInfoLength = 0;
clientInfo clientsInformation[100];

void populateInputParams(sockinfo currentSocketInfo, struct sockaddr_in *cliAddr, hdr* header, char* filename, int sliWindowSize, char* argv[9]);


int isDuplicateClient(struct sockaddr_in cliAddr) {
	int i;
	char clientAddressString[INET_ADDRSTRLEN];
	// USE cliAddr for client Not currentClientAddress
	for( i =0; i < clientsInfoLength; i++ ) {
		if(clientsInformation[i].ipAddress.s_addr == cliAddr.sin_addr.s_addr) {		
			if(clientsInformation[i].port ==cliAddr.sin_port) {
				return 1;
			}
		}
	}
	
	return 0;
}

void closeOtherSockets(int sockfd,sockinfo* serverSocketsInfo, int size) {
	int i;
	for(i=0 ; i<size; i++) {
		if(sockfd != serverSocketsInfo[i].sockfd) {
			close(serverSocketsInfo[i].sockfd);
		}
	} 
}

void updateClientInformation(struct sockaddr_in cliAddr, int pid) {
	// USE cliAddr for client Not currentClientAddress
	sigset_t sigsetChild;
	Sigemptyset(&sigsetChild);
	Sigaddset(&sigsetChild, SIGCHLD);
	Sigprocmask(SIG_BLOCK, &sigsetChild, NULL);
	clientInfo newClientInfo;
	newClientInfo.ipAddress.s_addr = cliAddr.sin_addr.s_addr;

	newClientInfo.port = cliAddr.sin_port;
	newClientInfo.pid = pid;
	clientsInformation[clientsInfoLength] = newClientInfo;
	clientsInfoLength++;
	Sigprocmask(SIG_UNBLOCK, &sigsetChild, NULL);
}

void clearClientInformation(int signal) {
	pid_t pid;
	int stat;
	pid = wait(&stat);
	int i;
	int j;
	for(i=0; i < clientsInfoLength; i++) {
		if (clientsInformation[i].pid == pid) {
			for (j =i+1; j < clientsInfoLength; j++ ) {
				clientsInformation[j] = clientsInformation[j-1];
			}
			clientsInfoLength--;
			return;
		}		
	}
}



void handleClients(sockinfo* serverSocketsInfo, int size, int sliWindowSize) {
	fd_set rset;
	int maxfd=0;
	int recvCount;
	int pid;
	hdr header;
    char* recvBuffer;
    char filename[488];
    FD_ZERO(&rset);
    struct sockaddr_in clientAddress;
    signal(SIGCHLD, clearClientInformation);	
	while(1) {
		int i;
		
		maxfd =0;
		for (i=0; i < size; i++) {
			FD_SET(serverSocketsInfo[i].sockfd, &rset);
       		maxfd = max(maxfd,serverSocketsInfo[i].sockfd);
       	}
       	maxfd+=1;
       	if(select(maxfd, &rset, NULL, NULL,NULL) < 0) {
       		if(errno == EINTR) {
       			continue;
       		}
    		perror("Select system call failed : ");
    		exit(1);
   		}
   		int j;
       	for(j=0; j<size; j++) {
       		if(FD_ISSET(serverSocketsInfo[j].sockfd,&rset)) {
       			recvCount=recvMessage(serverSocketsInfo[j].sockfd,&clientAddress,&header,filename);
       			if(recvCount < 0) {
       				perror("recvMsg in the parent process failed :");
       				exit(2);
       			}
       			
       			if(isDuplicateClient(clientAddress)) {
       				continue;
       				
       			}  			
       			if((pid = fork() )== 0) {
       				 closeOtherSockets(serverSocketsInfo[j].sockfd,serverSocketsInfo,size);
       				 char **argv = (char **)(malloc(9*sizeof(char*)));
  
       				 populateInputParams(serverSocketsInfo[j],&clientAddress,&header,filename,sliWindowSize, argv);
       				 
       				  argv[8] = NULL;
       				 
					 execve("childServer",argv,NULL);
					 perror("exec failed : ");
					 exit(2);
       				 
       			}
       			else {
       				updateClientInformation(clientAddress,pid);
       	
       			}
       			
       		}	
       	}
    }
       			
}

void populateInputParams(sockinfo currentSocketInfo, struct sockaddr_in *cliAddr, hdr* header, char* filename, int sliWindowSize, char* argv[9]) {
	argv[0] = (char *)malloc(strlen(filename)) ;
	strcpy(argv[0],filename);
/*	printf("the value in argv[0] is %s \n",argv[0]);*/

	argv[1]=(char *)malloc(INET_ADDRSTRLEN);
	if(inet_ntop(AF_INET,&(cliAddr->sin_addr),argv[1],INET_ADDRSTRLEN)<=0)
		perror("inet_ntop failed");
/*	printf("the value in argv[1] is %s \n",argv[1]);*/
	
	argv[2]=(char *)malloc(10);
	snprintf(argv[2], 10,"%d",(int)ntohs(cliAddr->sin_port));
	
/*	printf("the value in argv[2] is %d \n",ntohs(cliAddr->sin_port));*/
	
	argv[3]=(char *)malloc(10);
	snprintf(argv[3], 10,"%d",currentSocketInfo.sockfd);
/*	printf("the value in argv[3] is %s \n",argv[3]);*/
	
	argv[4]=(char *)malloc(10);
	snprintf(argv[4], 10,"%d",ntohs(header->windowSize));
/*	printf("the value in argv[4] is %s \n",argv[4]);*/
	
	argv[5]=(char *)malloc(10);
	
	snprintf(argv[5], 10,"%d",sliWindowSize);
/*	printf("the value in argv[5] is %s \n",argv[5]);*/
	
	argv[6]=(char *)malloc(10);
	snprintf(argv[6], 10,"%d",ntohs(header->seq));
/*	printf("the value in argv[6] is %s \n",argv[6]);	*/
	
	argv[7]=(char *)malloc(INET_ADDRSTRLEN);
	if(inet_ntop(AF_INET,&(currentSocketInfo.subnetMask),argv[7],INET_ADDRSTRLEN)<=0){
		printf("Subnet Mask address inet_ntop conversion failed\n");
		exit(0);
	}
}

	
