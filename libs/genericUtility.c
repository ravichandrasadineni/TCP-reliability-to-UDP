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






