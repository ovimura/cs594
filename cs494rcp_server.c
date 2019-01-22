#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


struct Request {
  int sockfd;
  struct sockaddr_in cliaddr;
  int len;
  char* hello;
  char buf[1024];
};

long PORT = 8080;

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

void *server_process(void *vargs)
{ struct Request *req = (struct Request*)vargs;
  printf("Client: %s\n", req->buf);
  sendto(req->sockfd, (const char *)req->hello, strlen(req->hello), MSG_CONFIRM, (const struct sockaddr *)&req->cliaddr, req->len);
  printf("Hello message sent.\n");
  pthread_exit(NULL);
}



int main(int argc, char** argv)
{

  int sockfd;
  char buffer[1024];
  char *hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  // Create socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  int len, n;
  n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  printf("%d %s %ld, %p, %d\n", sockfd, (const char*)hello, strlen(hello), (const struct sockaddr*)&cliaddr, len);
  //  printf("Client: %s\n", buffer);
//  sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
//  printf("Hello message sent.\n");

/*  
  pthread_t thread_id;
  if (argc != 2)
    printf("Usage: %s <port>\n", argv[0]);
  else if (argc == 2) {    
    if(isNumber(argv[1]))
    {
      PORT = (long)argv[1];
      pthread_create(&thread_id, NULL, server_process, &sockfd);
      pthread_join(thread_id, NULL);
    }
  }*/
pthread_t thread_id;

  while (1) {
	struct Request *request = (struct Request*)malloc(sizeof(struct Request));
	request->len = len;
	memcpy(request->buf, buffer, 1024);
	request->cliaddr = cliaddr;
	request->sockfd = sockfd;
	request->hello = hello;
    pthread_create(&thread_id, NULL, server_process, (void*)request);
    pthread_join(thread_id, NULL);
    exit(0);
  }
  return 0;
}
