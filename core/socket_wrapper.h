#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 4096

int send_all(int sockfd, void* msg, unsigned int len);

int recv_all(int sockfd, void* buffer);
