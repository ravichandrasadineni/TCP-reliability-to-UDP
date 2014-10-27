#include "fileUtility.h"

void readingClientInput(struct sockaddr_in *ipAddress,int *portNumber,char *filename,int *sliWindowsize,int *randSeed,float *prob,int *milliSec)
{
	int i,host;
	char* trimInput;	
	char input[1024],ipAddressString[INET_ADDRSTRLEN];
	struct in_addr ip;
    FILE *fp;
	fp=fopen("client.in","r");
	if (fp == NULL) 
	{
	  fprintf(stderr, "Can't open input file client.in!\n");
 	  exit(1);
	}
	for(i=1;fgets(input,sizeof(input),fp);i++)
	{
		if(i==8)
		{
		 printf("Input client.in file should have only 7 arguments\n\t1.IP address of server\n\t2.Port of Server\n\t3.filename to be transferred\n\t4.Receiving Sliding Window size\n\t5.Random generator seed value\n\t6.Probability of datagram loss(real number)\n\t7.Rate at which client reads data from receive buffer(milliseconds)\n");
		 exit(0);		
		}
		if(i==1)		
		{		
		 //input[strlen(input)-1]='\0';
		 trimInput = trimwhitespace(input);
		 if((inet_pton(AF_INET,trimInput,&(ip)))<=0)
		 {
			printf("inet_pton has failed\n");
			exit(0);
		 }	
		 ipAddress->sin_addr=ip;
		 
		}
		if(i==2)
		{
		 
		 if(is_valid_port(input)) {
				*portNumber=atoi(input);
		 }
		 else {
			printf("INVALID NUMBER FOR PORT\n");
			exit(0);
		}
		}
		if(i==3)
		{
			trimInput = trimwhitespace(input);
		 	strcpy(filename,trimInput);
		}
		if(i==4)
		{
		 trimInput = trimwhitespace(input);
			if(is_valid_int(trimInput)) {
				 *sliWindowsize=atoi(input);
			}
			else {
				printf("INVALID NUMBER FOR WINDOW SIZE\n");
				exit(0);
			}
		}	
		if(i==5)
		{
		 trimInput = trimwhitespace(input);
			if(is_valid_int(trimInput)) {
				 *randSeed=atof(input);
			}
			else {
				printf("INVALID NUMBER FOR RANDOM SEED\n");
				exit(0);
			}
		}
		if(i==6)
		{
		 *prob=atof(input);
			if(*prob < 0 || *prob > 1){
			printf("Probability value should be between 0.0 to 1.0\nPlease correct the value in client.in\nProgram will exit\n");
			exit(0);
		 	}
		}
		if(i==7)
		{
			trimInput = trimwhitespace(input);
			if(is_valid_int(trimInput)) {
				 *milliSec=atoi(input);
			}
			else {
				printf("INVALID NUMBER FOR Milliseconds\n");
				exit(0);
			}
		}
		memset(input,'\0',sizeof(input));
				
	}
	fclose(fp);
		
}

void readServerInput(int* portNumber, int* sliWindowSize)
{
	int i,port,buff_size;
	char input[1024];
	char* trimInput;
	FILE *fp; 	
	fp=fopen("server.in","r");
	if (fp == NULL) 
	{
		fprintf(stderr, "Can't open input file server.in!\n");
 	 	 exit(1);
	}
	for(i=1;fgets(input,sizeof(input),fp)>0;i++)
	{
		if(i==3)
		{
			 printf("Input server.in file should have only 2 arguments\n\t1.Wellknown Port\n\t2.Max sending window size\n");
			 exit(0);		
		}
		if(i==1)		
		{	
			if(is_valid_port(input)) {
				*portNumber=atoi(input);
			}
			else {
				printf("INVALID NUMBER FOR PORT\n");
				exit(0);
			}	
			 
		}
		if(i==2)
		{
			trimInput = trimwhitespace(input);
			if(is_valid_int(trimInput)) {
				*sliWindowSize=atoi(input);
			}
			else {
				printf("INVALID NUMBER FOR WINDOW SIZE\n");
				exit(0);
			}
		}
		memset(input,'\0',sizeof(input));
					
	}
	fclose(fp);
}	
