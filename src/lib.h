#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFSIZE 8388608 //32MB max
#define MAXLISTEN 256

#define CHECK(fun, errval, msg) \
    {                           \
        if ((fun) == errval)    \
        {                       \
            perror(msg);        \
            exit(errno);        \
        }                       \
    }

#define CHECK_SPECIFIC(msg, errval) \
    {                               \
        if (errno != errval)        \
        {                           \
            perror(msg);            \
            exit(errno);            \
        }                           \
    }

int logwrite(const char *str)
{
    time_t t;
    char *timestr;

    //write to log
    time(&t);
    timestr = ctime(&t);
    printf(timestr);
    printf(" | ");
    printf(str);
    printf("\n");
    return 1;
}

int logwrite_int(const char *str, long num)
{
    char outstr[1024];
    sprintf(outstr, "%s %ld", str, num);
    logwrite(outstr);
}

int logwrite_arr(const char *str, const int *arr, int size)
{
    char outstr[1024];
    char tmp[1024];
    strcpy(outstr, str);
    for (int i = 0; i < size; ++i)
    {
        sprintf(tmp, " %d", arr[i]);
        strcat(outstr, tmp);
    }
    logwrite(outstr);
}



void sock_fin(int sock)
{
    shutdown(sock, SHUT_RDWR); //say bye-bye to the socket and now close it (it's ALREADY FIN'd by client)
    close(sock);
}

int sock_send(int sock_r, const int *msg, int len)
{
    int ret;
    //sanity check
    if (sock_r < 0)
    {
        logwrite_int("ERROR: Tried to send message to closed socket:", sock_r);
        return -1;
    }
    logwrite_int("Sending to socket ", sock_r);
    logwrite_arr("Sent:", msg, len);
    ret = send(sock_r, msg, len * sizeof(int), 0);
    return ret;
}

int sock_rcv(int sock_r, int *msg)
{
    int rcv_sz;

    //receive message
    rcv_sz = recv(sock_r, msg, BUFSIZE * sizeof(int), 0); //-1 so \0 will be conserved
    if (rcv_sz == -1)
    {
        //CHECK_SPECIFIC("Error while receiving data from socket", EAGAIN)
        perror("");
        return 0;
    }
    else if (rcv_sz == 0)
    {
        logwrite_int("Socket down: ", sock_r);
        sock_fin(sock_r);
        return -2;
    }
    rcv_sz /= sizeof(int);
    logwrite_int("Received from socket: ", sock_r);
    //logwrite(msg);
    logwrite_arr("Rcv:", msg, rcv_sz);
    logwrite_int("size: ", rcv_sz);
    return rcv_sz;
}