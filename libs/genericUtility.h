#ifndef GENERIC_UTILITY_H
#define GENERIC_UTILITY_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "unp.h"
#include "messageHeader.h"
#include "threadUtility.h"
int is_legal_int (char* );
int getRandomSequenceNumber(int base);
unsigned int malarm (unsigned int milliseconds);
unsigned int salarm (unsigned int seconds);
int isFinSetForLastPacket(sharedBuf *buffer);
char *trimwhitespace(char* );
int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]);
int  recvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]);
int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488]);
int  recvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[512]);
void printCurrentBuffer(sharedBuf *buffer);
int  clientsendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488],float randomSeed);
int  clientrecvMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[488],float randomSeed);
struct timeval getTimeToWait(struct timeval base, int currentWaitMilliSecs);
void printfBuffer(sharedBuf *buffer);
#endif
