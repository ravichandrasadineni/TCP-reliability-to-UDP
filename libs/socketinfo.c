#include "socketinfo.h"
#define TRUE 1;
#define FALSE 0;


int isLoopbackAddr(struct sockaddr_in input)
{

	struct sockaddr_in loopbackaddr;
	inet_pton(AF_INET,"127.0.0.1",&loopbackaddr.sin_addr);
	if(input.sin_addr.s_addr == loopbackaddr.sin_addr.s_addr){
		return TRUE;
	}
	else
		return FALSE;
}




int isLocal(sockinfo addr,struct sockaddr_in input)
{
	struct in_addr newnetworkAddr;
	newnetworkAddr.s_addr= input.sin_addr.s_addr & addr.subnetMask.s_addr;
	if(newnetworkAddr.s_addr==addr.networkAddr.s_addr){
		return TRUE;
	}
	else
		return FALSE;
}




void sortNetworkMask(sockinfo* clientSocketInfo, int clientSocketInfoLength) {
	int i=0;
	int j=0;
	uint32_t var1;
	uint32_t var2;
	sockinfo tempSockInfo;
	for (i=0; i <clientSocketInfoLength; i++) {
		for(j=i+1; j <clientSocketInfoLength; j++) {
			var1 = ntohl(clientSocketInfo[i].subnetMask.s_addr);
			var2 = ntohl(clientSocketInfo[j].subnetMask.s_addr);
			if (var1 < var2 ) {
				tempSockInfo = clientSocketInfo[i];
				clientSocketInfo[i] = clientSocketInfo[j];
				clientSocketInfo[j] = tempSockInfo;
			}
		}
	}



}


int getClientBindingSocket(struct sockaddr_in *ipAddress,int port,sockinfo* clientSocketInfo)
{
	sockinfo *cli=clientSocketInfo;
	struct sockaddr_in	*sa,servaddr,cliaddr,peeraddr,sockaddr;
	int sockfd,noOfrecords=0,i,on=1,doaliases=1,addrFamily=AF_INET,loop_back;
	char networkAddrString[INET_ADDRSTRLEN], ipAddressString[INET_ADDRSTRLEN], subnetMaskString[INET_ADDRSTRLEN];
	char serveripAddressString[INET_ADDRSTRLEN];
	struct ifi_info	*ifi, *ifihead;
	socklen_t len=INET_ADDRSTRLEN;


	for (ifihead = ifi = Get_ifi_info_plus(addrFamily, doaliases); ifi != NULL; ifi = ifi->ifi_next) {
		printf("%s: \n", ifi->ifi_name);
		if ( (sa = (struct sockaddr_in *)ifi->ifi_addr) != NULL) {
			printf("\tIP addr: %s\n",Sock_ntop_host((struct sockaddr *)sa, sizeof(*sa)));
			clientSocketInfo->sockfd=0;
			clientSocketInfo->ipAddr = sa->sin_addr;
		}
		if ( (sa = (struct sockaddr_in *)ifi->ifi_ntmaddr) != NULL) {
			printf("\tnetwork mask: %s\n", Sock_ntop_host((struct sockaddr *)sa, sizeof(*sa)));
			clientSocketInfo->subnetMask = sa->sin_addr;
		}
		clientSocketInfo++;
		noOfrecords++;
	}	
	sortNetworkMask(cli,noOfrecords);
	if((loop_back=isLoopbackAddr(*ipAddress))==1){
		bzero(&cliaddr,sizeof(cliaddr));
		bzero(&servaddr,sizeof(servaddr));
		bzero(&peeraddr,sizeof(peeraddr));
		bzero(&sockaddr,sizeof(sockaddr));
		sockfd=socket(AF_INET,SOCK_DGRAM,0);
		if(sockfd<0){
			perror("Error while creating socket :");
			exit(2);
		}	
		Setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on));
		cliaddr.sin_family=addrFamily;
		cliaddr.sin_port=htons(0);
		cliaddr.sin_addr=ipAddress->sin_addr;
		Bind(sockfd, (SA*)&cliaddr, INET_ADDRSTRLEN);
		getsockname(sockfd,(SA*)&sockaddr,&len);
		printf("After Binding, Selected Values:\nClient address is:%s and Ephemeral port assigned to client by Kernel is %d\n",inet_ntoa(sockaddr.sin_addr),(int)ntohs(sockaddr.sin_port));
		servaddr.sin_family=addrFamily;
		servaddr.sin_port = htons(port);
		servaddr.sin_addr=ipAddress->sin_addr;
		servaddr.sin_port=htons(port);
		connect(sockfd, (SA*)&servaddr,sizeof(servaddr));
		getpeername(sockfd,(SA*)&peeraddr,&len);
		printf("After Connecting,Selected Values:\nThe server address is: %s and port number is: %d\n",inet_ntoa(peeraddr.sin_addr),(int)ntohs(peeraddr.sin_port));
		free(ifihead);
		return sockfd;	
	}		
	for(i=1;i<=noOfrecords;i++){
		cli->networkAddr.s_addr = cli->ipAddr.s_addr & cli->subnetMask.s_addr;
		inet_ntop(AF_INET, &(cli->ipAddr), ipAddressString, INET_ADDRSTRLEN);
		//inet_ntop(AF_INET, &(cli->subnetMask), subnetMaskString, INET_ADDRSTRLEN);
		//inet_ntop(AF_INET, &(cli->networkAddr), networkAddrString, INET_ADDRSTRLEN);
		//printf("Socket fd : %d , IP addr: %s , SubnetMask addr: %s, Network Addr:%s\n",cli->sockfd,ipAddressString,subnetMaskString, networkAddrString);
		int status=isLocal(*cli,*ipAddress);
		if(status==1){
			printf("The Server is local\n");
			inet_ntop(AF_INET,&(ipAddress->sin_addr),serveripAddressString,INET_ADDRSTRLEN);
			printf("Choosen Values :\nServer address is: %s\nport number is: %d\nClient Address is:%s\n", serveripAddressString, port, ipAddressString);
			sockfd = socket(AF_INET, SOCK_DGRAM,0);
			if(sockfd<0){
				perror("Error while creating socket :");
				exit(2);
			}
			bzero(&cliaddr,sizeof(cliaddr));
			bzero(&servaddr,sizeof(servaddr));
			bzero(&peeraddr,sizeof(peeraddr));
			bzero(&sockaddr,sizeof(sockaddr));

			cli->sockfd=sockfd;
			Setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on));
			cliaddr.sin_family=addrFamily;
			cliaddr.sin_addr=cli->ipAddr;
			cliaddr.sin_port=htons(0);
			Bind(sockfd, (SA*)&cliaddr, INET_ADDRSTRLEN);
			getsockname(sockfd,(SA*)&sockaddr,&len);
			printf("After Binding, Selected Values:\nClient address is:%s and Ephemeral port assigned to client by Kernel is %d\n",inet_ntoa(sockaddr.sin_addr),(int)ntohs(sockaddr.sin_port));
			servaddr.sin_family=addrFamily;
			servaddr.sin_addr=ipAddress->sin_addr;
			servaddr.sin_port=htons(port);
			connect(sockfd, (SA*)&servaddr,sizeof(servaddr));
			getpeername(sockfd,(SA*)&peeraddr,&len);
			printf("The server address is: %s and port number is: %d\n",inet_ntoa(peeraddr.sin_addr),(int)ntohs(peeraddr.sin_port));
			free(ifihead);
			return sockfd;
		}
		cli++;
	}
	//if the server is not local then
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd<0){
		perror("Error while creating socket :");
		exit(2);
	}
	struct sockaddr_in isloopbackaddr;
	inet_pton(AF_INET,"127.0.0.1",&isloopbackaddr.sin_addr);
	if(isloopbackaddr.sin_addr.s_addr==((struct sockaddr_in *)ifihead->ifi_addr)->sin_addr.s_addr){
		ifihead=ifihead->ifi_next;
	}
	bzero(&cliaddr,sizeof(cliaddr));
	bzero(&servaddr,sizeof(servaddr));
	bzero(&peeraddr,sizeof(peeraddr));
	bzero(&sockaddr,sizeof(sockaddr));

	cliaddr.sin_family=addrFamily;
	cliaddr.sin_addr=((struct sockaddr_in *)ifihead->ifi_addr)->sin_addr;
	cliaddr.sin_port=htons(0);
	printf("After Binding, Selected Values:\nClient address is:%s and Ephemeral port assigned to client by Kernel is %d\n",inet_ntoa(cliaddr.sin_addr),(int)ntohs(cliaddr.sin_port));
	Bind(sockfd,(SA*)&cliaddr,INET_ADDRSTRLEN);
	getsockname(sockfd,(SA*)&sockaddr,&len);
	printf("After Binding, Selected Values:\nClient address is:%s and Ephemeral port assigned to client by Kernel is %d\n",inet_ntoa(sockaddr.sin_addr),(int)ntohs(sockaddr.sin_port));
	servaddr.sin_family=addrFamily;
	servaddr.sin_addr=ipAddress->sin_addr;
	servaddr.sin_port=htons(port);
	connect(sockfd, (SA*)&servaddr,sizeof(servaddr));
	getpeername(sockfd,(SA*)&peeraddr,&len);
	printf("The server address is: %s and port number is: %d\n",inet_ntoa(peeraddr.sin_addr),(int)ntohs(peeraddr.sin_port));
	free(ifihead);
	return sockfd;
}

void connectAgain(int sockfd, int port) {
	int len = INET_ADDRSTRLEN;
	struct sockaddr_in peeraddr;
	getpeername(sockfd,(SA*)&peeraddr,&len);
	peeraddr.sin_port=htons(port);
	printSocketDetails(peeraddr);
	if(connect(sockfd, (SA*)&peeraddr,sizeof(peeraddr)) < 0) {
		perror("unable to connect again to the new port ERROR :");
		exit(1);
	}
}


int getServerBindingSockets( int port, sockinfo* serverSocketsInfo)
{
	struct ifi_info *serInterfaces;
	int sockfd;
	int numOfSockets =0;
	char networkAddrString[INET_ADDRSTRLEN], ipAddressString[INET_ADDRSTRLEN], subnetMaskString[INET_ADDRSTRLEN];
	const int on =1;
	struct sockaddr_in	*sa,sockaddr;
	struct ifi_info	*ifi, *ifihead;
	int addrFamily=AF_INET;
	int doaliases=1;
	socklen_t len=INET_ADDRSTRLEN;
	printf("Interfaces : \n");
	for (ifihead = ifi = Get_ifi_info_plus(addrFamily, doaliases); ifi != NULL; ifi = ifi->ifi_next) {
		sockfd = socket(AF_INET, SOCK_DGRAM,0);
		if(sockfd<0){
			perror("Error while creating socket :");
			exit(2);
		}
		Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if ( (sa = (struct sockaddr_in *)ifi->ifi_addr) != NULL) {
			sa->sin_family = addrFamily;
			sa->sin_port = htons(port);
			Bind(sockfd, (SA*)sa, sizeof(*sa));
			serverSocketsInfo->sockfd = sockfd;
			serverSocketsInfo->ipAddr = sa->sin_addr;
			numOfSockets++;
		}
		if ( (sa = (struct sockaddr_in *)ifi->ifi_ntmaddr) != NULL) {
			serverSocketsInfo->subnetMask = sa->sin_addr;
		}
		serverSocketsInfo->networkAddr.s_addr = serverSocketsInfo->ipAddr.s_addr & serverSocketsInfo->subnetMask.s_addr;
		inet_ntop(AF_INET, &(serverSocketsInfo->ipAddr), ipAddressString, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(serverSocketsInfo->subnetMask), subnetMaskString, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(serverSocketsInfo->networkAddr), networkAddrString, INET_ADDRSTRLEN);
		printf("IP Address is : %s, Subnet Mask is : %s, Network Address is : %s   \n",ipAddressString,subnetMaskString, networkAddrString);

		serverSocketsInfo++;

	}
	printf("\n");
	getsockname(sockfd,(SA*)&sockaddr,&len);

	free(ifihead);
	return numOfSockets;
}


void printSocketDetails(struct sockaddr_in socketDetails) {
	char clientAddressString[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&(socketDetails.sin_addr),clientAddressString,INET_ADDRSTRLEN);
	printf("the client address is %s \n", clientAddressString);
	printf("the client port is %d\n", ntohs(socketDetails.sin_port));
	return;
}




struct sockaddr_in getClientSocketDetails(clientInformation currentClientInformation) {
	struct sockaddr_in ClientSocketDetails;
	ClientSocketDetails.sin_family= AF_INET;
	ClientSocketDetails.sin_addr=currentClientInformation.ipAddress;
	ClientSocketDetails.sin_port =htons(currentClientInformation.port);
	char clientAddressString[INET_ADDRSTRLEN];
	printSocketDetails(ClientSocketDetails);
	return ClientSocketDetails;
}



int getNewSocket(struct sockaddr_in	sockaddr, clientInformation currentClientInformation) {

	int newSockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(newSockfd<0){
		perror("Error while creating socket :\n");
		exit(2);
	}
	int on = 1;
	sockinfo serverSockinfo;
	struct sockaddr_in newServerSocket, ClientSocketDetails ;
	newServerSocket.sin_family= AF_INET;
	newServerSocket.sin_addr= sockaddr.sin_addr;
	newServerSocket.sin_port=htons(0);
	ClientSocketDetails = getClientSocketDetails(currentClientInformation);
	serverSockinfo.ipAddr = sockaddr.sin_addr;
	serverSockinfo.subnetMask = currentClientInformation.subnetMask;
	serverSockinfo.networkAddr.s_addr = sockaddr.sin_addr.s_addr & currentClientInformation.subnetMask.s_addr;
	if (isLocal(serverSockinfo,ClientSocketDetails)) {
		int returnValue = setsockopt(newSockfd, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on));
		if(returnValue  < 0) {
			perror(" Setting Do Not Route Option Failed Error :");
			exit(2);
		}
		if(!isLoopbackAddr(ClientSocketDetails)){
			printf("Client is local to the server. Setting DONOTROUTE \n");
		}
		else {
			printf("Client is on  loopback.setting DONOTROUTE. \n");
		}


	}

	if(bind(newSockfd, (SA*)&newServerSocket, INET_ADDRSTRLEN)<0) {
		perror("\n bind  on new server socket failed ");
		exit(2);
	}

	return newSockfd;
}

void connectNewServerSocket(int newSockfd, clientInformation currentClientInformation) {
	struct sockaddr_in  ClientSocketDetails;
	ClientSocketDetails = getClientSocketDetails(currentClientInformation);
	if(connect(newSockfd, (SA*)&ClientSocketDetails,sizeof(ClientSocketDetails)) <0) {
		perror("\n connect  call on new server socket failed");
		exit(2);
	}


}








