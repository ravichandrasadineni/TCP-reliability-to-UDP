#include"genericUtility.h"
#define TRUE 1
#define FALSE 0
typedef int bool;

bool is_valid_int (char* integerString) {	
    while (*integerString)
    {
        if (isdigit (*integerString) ) {
            integerString++;
        }
        else {
            return FALSE;
        }
    }
    return TRUE;
            
}

int getRandomSequenceNumber(int base) {
	srand((unsigned) time(NULL));
	return rand()%base;
}



char *trimwhitespace(char *str)
{
  char *end;
  while(isspace(*str)) str++;
  if(*str == 0)  
    return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  *(end+1) = 0;
  return str;
}

unsigned int
malarm (unsigned int milliseconds)
{
  struct itimerval old, new;
  new.it_interval.tv_usec = 0;
  new.it_interval.tv_sec = 0;
  new.it_value.tv_usec = milliseconds*1000;
  new.it_value.tv_sec = 0;
  if (setitimer (ITIMER_REAL, &new, &old) < 0)
    return 0;
  else
    return old.it_value.tv_sec;
}


unsigned int
salarm (unsigned int seconds)
{
  struct itimerval old, new;
  new.it_interval.tv_usec = 0;
  new.it_interval.tv_sec = 0;
  new.it_value.tv_usec = 0;
  new.it_value.tv_sec = (long int) seconds;
  if (setitimer (ITIMER_REAL, &new, &old) < 0)
    return 0;
  else
    return old.it_value.tv_sec;
}


struct msghdr buildMessage(struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[512]) {
	struct msghdr msg;
	
	memset(&msg, 0, sizeof(msg));
	if(data == NULL) {
		struct iovec iov1[1];
		printf("message iov length is 1 \n");
		iov1[0].iov_base = messageHeader;
		iov1[0].iov_len = sizeof(*messageHeader);
		msg.msg_iov = iov1;
		msg.msg_iovlen = 1;
	}
	else {
		struct iovec iov2[2];
		iov2[0].iov_base = messageHeader;
		iov2[0].iov_len = sizeof(*messageHeader);
		iov2[1].iov_base = data;
		iov2[1].iov_len = 512;
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;	
	}
	
	if(msg_name == NULL) {
		 msg.msg_name = NULL;
		 msg.msg_namelen = 0;
	}
	else {
		 SA* saddress = (SA*)msg_name;
		 msg.msg_name = saddress;
		 msg.msg_namelen = sizeof(SA);
	}
	
	return msg;
}




int  sendMessage(int sockfd, struct sockaddr_in* msg_name,  hdr* messageHeader ,  char data[512]) {
	struct msghdr msg;
	
	memset(&msg, 0, sizeof(msg));
	if(data == NULL) {
		struct iovec iov1[1];
		printf("message iov length is 1 \n");
		iov1[0].iov_base = messageHeader;
		iov1[0].iov_len = sizeof(*messageHeader);
		msg.msg_iov = iov1;
		msg.msg_iovlen = 1;
	}
	else {
		struct iovec iov2[2];
		iov2[0].iov_base = messageHeader;
		iov2[0].iov_len = sizeof(*messageHeader);
		iov2[1].iov_base = data;
		iov2[1].iov_len = 512;
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;	
	}
	
	if(msg_name == NULL) {
		 msg.msg_name = NULL;
		 msg.msg_namelen = 0;
	}
	else {
		 SA* saddress = (SA*)msg_name;
		 msg.msg_name = saddress;
		 msg.msg_namelen = sizeof(SA);
	}
	
	return sendmsg(sockfd,&msg, 0);
}





