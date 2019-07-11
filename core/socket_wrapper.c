#include "socket_wrapper.h"

int wrapper_error;

/* max len = MAXSIZE */
int send_all(int sockfd, void* msg, unsigned int len)
{
    if (len > MAXSIZE)
    {
        /* len is too long */
        return 1;
    }

    size_t sum = 0;
    unsigned int temp = len;
    int n;
    char temp_offset[MAXSIZE + 4];
    char* offset = temp_offset;

    /* add len to the msg, avoid from sticky */
    offset[0] = *((char*)&temp + 0);
    offset[1] = *((char*)&temp + 1);
    offset[2] = *((char*)&temp + 2);
    offset[3] = *((char*)&temp + 3);
    
    memcpy(offset + 4, msg, len);

    while(sum < len + 4)
    {
        n = send(sockfd, offset, len + 4 - sum, 0);
        if (n < 0)
        {
            return 2;
        }
        sum += n;
        offset += n;
    }
    return len;
}

/* please make sure the size of buffer >= MAXSIZE */
int recv_all(int sockfd, void* buffer)
{
    size_t sum = 0;
    char* offset = buffer;
    int n;
    char temp[4];

    n = recv(sockfd, temp, 4, 0);
    if (n != 4)
    {
        return 3;
    }

    unsigned int len = *(unsigned int*)temp;

    while (sum < len)
    {
        n = recv(sockfd, offset, len - sum, 0);
        if (n == 0)
        {
            return 4;
        }
        if (n < 0)
        {
            return 5;
        }
        sum += n;
        offset += n;
    }
    return len;
}