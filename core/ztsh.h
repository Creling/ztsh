#ifndef ZTSH_H
#define ZTSH_H

#define SERVER_PORT 7586
#define CONNECT_BACK_HOST  "localhost"
#define CONNECT_BACK_DELAY 30

#define INFILEPATH "/root/unix_to_ztsh.sock"
#define REDIS_DB 1

#define BUFSIZE 512

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/un.h>
#include <hiredis/hiredis.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <pty.h>

#include "write_log.h"
#include "socket_wrapper.h"

#endif 
