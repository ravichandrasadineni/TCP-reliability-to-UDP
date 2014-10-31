#include"libs/portutility.h"
#include"libs/genericUtility.h"
#include"libs/socketinfo.h"
#include"libs/fileUtility.h"
#include"libs/clientHandler.h"

int main(int argc,char *argv[])
{
	int portNumber=0, sliWindowSize=0, socketfd[100],numOfSockets =0;
	sockinfo serverSocketsInfo[100];
	srand((unsigned) time(NULL));
	readServerInput(&portNumber, &sliWindowSize);
	numOfSockets = getServerBindingSockets(portNumber, serverSocketsInfo);
	handleClients(serverSocketsInfo,numOfSockets,sliWindowSize);
	return (0);
}
