#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"stringutility.h"
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






