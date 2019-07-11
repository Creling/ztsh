/*
 * ZTsh Master side,;
 * this program is licensed under the GPL.
 */
#include "ztsh.h"


/* func declaration */
int ztsh_shell(int server, int in_socket, char *argv2);
void* wait_for_slave(); // 
void* wait_for_django(); // 
void* deal_with_django(void* socket);
int deal_with_slave(int dj_sock, int slave_sock);



/*  predefine */
int ret;
redisContext* re_base = NULL;
redisReply* reply = NULL;
pthread_mutex_t redis_mux;
int dj_serv_sock, dj_cli_sock, slave_serv_sock, slave_cli_sock;
struct sockaddr_un dj_serv_sock_ddr;
struct sockaddr_in slave_serv_sock_addr;
socklen_t addrlen;
pthread_t dj_t, slave_t;
int err;
unsigned char message[BUFSIZE + 1];
int temp_sock;

FILE* pLog;

int main()
{
    /* connect to redis */
    re_base = redisConnect("127.0.0.1", 6379);
    if (re_base == NULL || re_base->err) 
    {
        if (re_base) 
        {
            printf("Error: %s\n", re_base->errstr);
            return 1;
        } 
        else 
        {
            printf("Can't allocate redis context\n");
            return 1;
        }
    }
    fprintf(stderr, "%s\n", "redis connect success"); 
    reply = redisCommand(re_base, "Select %d", REDIS_DB);  // switch to DB_1
    reply = redisCommand(re_base, "flushall");  // flush old data

    err = pthread_create(&dj_t, NULL, wait_for_django, NULL);
    err = pthread_create(&slave_t, NULL, wait_for_slave, NULL);
    
    pthread_join(dj_t, NULL);
    pthread_join(slave_t, NULL);
    return 0;
}

void* wait_for_django()
{
    /* create domain socket */
    int dj_serv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    dj_serv_sock_ddr.sun_family = AF_UNIX;
    unlink(INFILEPATH);
    strcpy(dj_serv_sock_ddr.sun_path, INFILEPATH);
    if (bind(dj_serv_sock, (struct sockaddr*)&dj_serv_sock_ddr, sizeof(dj_serv_sock_ddr)))
    {
        perror("dj_serv_sock bind error");
        return NULL;
    }
    if (listen(dj_serv_sock, 1024) != 0)
    {
        perror("dj_serv_sock listen error");
        return NULL;
    } 

    addrlen = sizeof(dj_serv_sock_ddr);
    while (1)
    {
        fprintf(stderr, "Waiting for django client ...\n");
        dj_cli_sock = accept(dj_serv_sock, (struct sockaddr*)&dj_serv_sock_ddr, &addrlen);
        if (dj_cli_sock < 0)
        {
            perror("dj_cli_sock");
            return NULL;
        }
        err = pthread_create(&dj_t, NULL, deal_with_django, (void*)&dj_cli_sock);
        if (err != 0)
        {
            perror("create dj thread");
            return NULL;
        }
    }
}

void* deal_with_django(void* socket)  
{
    int dj_socket = *(int*)socket;
    char buff[16] = {'\0'};
    int recv = read(dj_socket, buff, 16);  // get target ip from dj_socket
    if (recv == 0)
    {
        fprintf(stderr, "%s\n", "connection ends");
        return NULL;
    }

    fprintf(stderr, "%s\n", buff);
    pthread_mutex_lock(&redis_mux);
        reply = redisCommand(re_base, "GET %s", buff);
    pthread_mutex_unlock(&redis_mux);
    if (reply->type == 4)
    {
        send(dj_socket, "No such IP", 10, 0);
        fprintf(stderr, "%s\n", "No such IP");
        shutdown(dj_socket, 2);
        pthread_exit(0);
    }
    
    int slave_sock = atoi(reply->str);  // get the target socket
    printf("slave_sock: %d\n", slave_sock);

    deal_with_slave(dj_socket, slave_sock);  // call a func to handle the soket
    
    fprintf(stderr, "%s\n", "a session ends");
    pthread_exit(0);
}

void* wait_for_slave()
{
    slave_serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (slave_serv_sock < 0)
    {
        perror("slave_serv_sock");
        return NULL;
    }

    /* allow port multiplexing */
    int n = 1;
    ret = setsockopt(slave_serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&n, sizeof(n)); 
    if (ret < 0)
    {
        perror("setsockopt");
        return NULL;
    }
    
    /* create tcp socket */
    slave_serv_sock_addr.sin_family      = AF_INET;
    slave_serv_sock_addr.sin_port        = htons(SERVER_PORT);
    slave_serv_sock_addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(slave_serv_sock, (struct sockaddr *)&slave_serv_sock_addr,sizeof(slave_serv_sock_addr));
    if (ret < 0)
    {
        perror("bind");
        return NULL;
    }
    if (listen(slave_serv_sock, 1024) != 0)
    {
        perror("listen");
        return NULL;
    }

    addrlen = sizeof(slave_serv_sock_addr);
    while (1)
    {
        fprintf(stderr, "Waiting for the slave to connect...\n");
        fflush(stderr);
        slave_cli_sock = accept(slave_serv_sock, (struct sockaddr *)&slave_serv_sock_addr, &addrlen); 
        if(slave_cli_sock < 0)
        {
            perror("slave_cli_sock");
            return NULL;
        }
        temp_sock = dup(slave_cli_sock);
        pthread_mutex_lock(&redis_mux);
            reply = redisCommand(re_base, "GET %s", inet_ntoa(slave_serv_sock_addr.sin_addr));
            if (reply->type != 4)  // the ip is already in redis
            {
                pthread_mutex_unlock(&redis_mux);
                close(slave_cli_sock);
                continue;
            }
            reply = redisCommand(re_base, "SET %s %d", inet_ntoa(slave_serv_sock_addr.sin_addr), temp_sock);
            if (strcmp(reply->str, "OK"))  // something goes wrong
            {
                pthread_mutex_unlock(&redis_mux);
                perror("Add to redis");
                return NULL;
            }
        pthread_mutex_unlock(&redis_mux);
        close(slave_cli_sock);
        printf("temp_sock: %d\n", temp_sock);
        fprintf(stderr, "slave: %s connected.\n", inet_ntoa(slave_serv_sock_addr.sin_addr));
    }
}

int deal_with_slave(int dj_socket, int slave_sock)
{
    fprintf(stderr, "%s\n", "deal_with_slave");

    int tempret = ztsh_shell(slave_sock, dj_socket, "exec bash --login");
    fprintf(stderr, "%s %d\n", "ztsh_shell_return:", tempret);
    pLog = fopen("pel.log","a");
    write_log(pLog, "\n%s\n\n", "A Session Ends");
    fclose(pLog);
    return 0;
}

int ztsh_shell( int server, int in_socket, char *argv2 )
{
    fd_set rd;
    char *term;
    int ret, len;
    struct winsize ws;

    /* send the TERM environment variable */
    term = getenv( "TERM" );
    if( term == NULL )
    {
        term = "xterm";
    }

    len = strlen( term );

    ret = send_all(server, (unsigned char *) term, len);
    if( ret == 0 )
    {
        perror( "send_all" );
        return( 22 );
    }

    /* send the window size */
    ws.ws_row = 23;
    ws.ws_col = 124;

    message[0] = ( ws.ws_row >> 8 ) & 0xFF;
    message[1] = ( ws.ws_row      ) & 0xFF;
    message[2] = ( ws.ws_col >> 8 ) & 0xFF;
    message[3] = ( ws.ws_col      ) & 0xFF;

    ret = send_all( server, message, 4 );

    if( ret == 0 )
    {
        perror( "send_all" );
        return( 24 );
    }

    /* send the system command */
    len = strlen( argv2 );
    ret = send_all(server, (unsigned char *) argv2, len);

    /*  transfer betweem django and master */
    pLog = fopen("pel.log","a");
    write_log(pLog, "slave_sock: %d, dj_sock: %d\n", server, in_socket);
    fclose(pLog);

    while( 1 )
    {
        FD_ZERO( &rd );
        FD_SET( server, &rd );
        FD_SET( in_socket, &rd);

        int maxfds = (server > in_socket ? server : in_socket);

        if( select( maxfds + 1, &rd, NULL, NULL, NULL ) < 0 )
        {
            perror( "select" );
            ret = 28;
            break;
        }

        if( FD_ISSET( server, &rd ) )
        {
            ret = recv_all(server, message);
            message[ret] = '\0';
            pLog = fopen("pel.log","a");
            write_log(pLog, "receive from slave %d, len %d\n%s\n", server, ret, message);
            fclose(pLog);

            if( write( in_socket, message, ret ) != ret )
            {
                perror( "in_socket write" );
                ret = 30;
                break;
            }
        }

        if( FD_ISSET(in_socket, &rd) )
        {
            len = recv( in_socket, message, BUFSIZE, 0 );
            message[len] = '\0';
            pLog = fopen("pel.log","a");
            write_log(pLog, "receive from django %d, len %d\n%s\n", in_socket, len, message);
            fclose(pLog);

            if( len == 0 )
            {
                fprintf( stderr, "stdin: end-of-file\n" );
                ret = 31;
                break;
            }

            if( len < 0 )
            {
                perror( "read" );
                ret = 32;
                break;
            }

            ret = send_all(server, message, len);
        }
    }
    pLog = fopen("pel.log","a");
    write_log(pLog, "%s , sock: %d\n", "ztsh_shell out of while", server);
    fclose(pLog);

    return( ret );

}




/*
To do:
死亡线程的回收
非目标ip的判定
断线重连后跳过初始化步骤
两个大坑：一个是alarm定时退出
第二个 recv的len超出buffer长度之后会报资源不可用
 */
