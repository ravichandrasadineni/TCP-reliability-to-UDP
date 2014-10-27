#ifndef GENERIC_UTILITY_H
#define GENERIC_UTILITY_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "unp.h"
#include "messageHeader.h"
int is_legal_int (char* );
int getRandomSequenceNumber(int base);
unsigned int malarm (unsigned int milliseconds);
unsigned int salarm (unsigned int seconds);
char *trimwhitespace(char* );
int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[512]);
#endif
