#include "packet.h"

void usage(char *a)
{
  printf("Usage: %s <server_ip> <port> <file>\n", a);
}

int isNumber(char* s)
{
  int len = strlen(s);
  for (int i=0; i<len; i++)
    if (!isdigit(s[i]))
    {
      printf("error: the entered <port> is not a number\n");
      exit(1);
    }
}

int validateFilePath(const char *f)
{
  FILE *file;
  if(file = fopen(f, "r"))
  {
    fclose(file);
    return 1;
  } else return 0;
}




int isServerIpFormat(char *ip)
{ char delim[] = ".";
  char *ptr = strtok(ip, delim);
  int i = 0;
  while(ptr != NULL)
  {
    ptr = strtok(NULL, delim);
    i++;
  }
  if(i==4)
    return 1;
  else
    return 0;
}


