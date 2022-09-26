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

#define BUFSIZE 1024
#define PKTSIZE 128
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

void logwrite(const char *str)
{
    time_t t;
    char *timestr;

    //write to log
    time(&t);
    timestr = ctime(&t);
    printf("%s", timestr);
    printf(" | ");
    printf("%s", str);
    printf("\n");
}

void logwrite_int(const char *str, long num)
{
    char outstr[1024];
    sprintf(outstr, "%s %ld", str, num);
    logwrite(outstr);
}

void logwrite_arr(const char *str, const double *arr, uint64_t size)
{
    char outstr[1024];
    char tmp[1024];
    strcpy(outstr, str);
    for (uint64_t i = 0; i < size; ++i)
    {
        sprintf(tmp, " %lf", arr[i]);
        strcat(outstr, tmp);
    }
    logwrite(outstr);
}



void sock_fin(int sock)
{
    shutdown(sock, SHUT_RDWR); //say bye-bye to the socket and now close it (it's ALREADY FIN'd by client)
    close(sock);
}

int sock_send(int sock_r, const double *msg, uint64_t len)
{
    int ret;
    uint64_t total = 0;
    const double *ptr = msg;
    uint64_t cur_len;
    //sanity check
    if (sock_r < 0)
    {
        logwrite_int("ERROR: Tried to send message to closed socket:", sock_r);
        return -1;
    }
    logwrite_int("Sending to socket ", sock_r);
    send(sock_r, &len, sizeof(uint64_t), 0); //send length
    while (total < len) {
        cur_len = len - total > PKTSIZE ? PKTSIZE : len - total;
        //logwrite_arr("Sent:", ptr, cur_len);
        ret = send(sock_r, ptr, cur_len * sizeof(double), 0);
        if (ret < 0) {
            perror("");
            return ret;
        }
        total += cur_len;
        ptr += cur_len;
    }
    return ret;
}

int sock_rcv(int sock_r, double **msg)
{
    uint64_t len;
    uint64_t rcv_total = 0;
    uint64_t rcv_sz;

    //read length
    rcv_sz = recv(sock_r, &len, sizeof(uint64_t), 0);
    logwrite_int("Total length: ", len);
    *msg = calloc(len, sizeof(double));
    double *ptr = *msg;
    // receive message
    logwrite_int("Received from socket: ", sock_r);
    while (rcv_total < len && rcv_sz > 0) {
        rcv_sz = recv(sock_r, ptr, PKTSIZE * sizeof(double), 0);
        rcv_sz /= sizeof(double);
        //logwrite_int("size: ", rcv_sz);
        //logwrite_arr("Rcv:", ptr, rcv_sz);
        rcv_total += rcv_sz;
        ptr += rcv_sz; //shift pointer to the end
    }
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
    return len;
}

int sock_send_str(int sock_r, const char *msg)
{
    int ret;
    //sanity check
    if (sock_r < 0)
    {
        logwrite_int("ERROR: Tried to send message to closed socket:", sock_r);
        return -1;
    }
    logwrite_int("Sending to socket ", sock_r);
    //logwrite(msg);
    ret = send(sock_r, msg, strlen(msg), 0);
    return ret;
}

int sock_rcv_str(int sock_r, char *msg)
{
    int rcv_sz;

    //receive message
    rcv_sz = recv(sock_r, msg, BUFSIZE - 1, 0); //-1 so \0 will be conserved
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
    msg[rcv_sz] = '\0';
    logwrite_int("Received from socket: ", sock_r);
    //logwrite(msg);
    logwrite(msg);
    logwrite_int("size: ", rcv_sz);
    return rcv_sz;
}