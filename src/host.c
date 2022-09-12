#include "lib.h"

int port;
int port_changed = 0;

int sock_r = -1;
int sock_l; //listening socket

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//prototypes
//sock
int sock_listen(int port);
int sock_accept(int sock_l);
void sock_closeall();

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
//SOCKET FUNCTIONS

int sock_listen(int port)
{
    int sock;
    struct sockaddr_in sa;

    CHECK(sock = socket(PF_INET, SOCK_STREAM, 0), -1, "Error while creating socket")
    fcntl(sock, F_SETFL/*, O_NONBLOCK*/ | fcntl(sock_l, F_GETFL));
    sa.sin_family = AF_INET;
    if (port < 1024)
        logwrite("WARNING: Trying to use root-only socket");
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;
    CHECK(bind(sock, (struct sockaddr *)&sa, sizeof(sa)), -1, "Error while binding socket")
    listen(sock, MAXLISTEN);
    logwrite("Connect socket opened.");
    return sock;
}

int sock_accept(int sock_l)
{
    struct sockaddr_in sa_r;
    int sa_r_len;

    if ((sock_r = accept(sock_l, (struct sockaddr *)&sa_r, (socklen_t *)&sa_r_len)) == -1)
    {
        CHECK_SPECIFIC("Error while accepting connection", EAGAIN)
        return 0;
    }
    logwrite_int("Connection accepted:", sock_r);
    //connection accepted, all good
    fcntl(sock_r, F_SETFL/*, O_NONBLOCK*/ | fcntl(sock_l, F_GETFL));
    return 1;
}

void sock_closeall()
{
    sock_fin(sock_r);
    logwrite("Client sockets closed");
    sock_fin(sock_l);
    logwrite("Listen-socket closed");
}

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void handler_stop(int signum)
{
    logwrite("SIGTERM received.");
    //ending phase: delete all created stuff.
    sock_closeall(); //sockets
    //closing log
    logwrite("Server stopped. Closing the log...");
    exit(0);
}

int main(int argc, char *argv[])
{
    int *vect = calloc(BUFSIZE, sizeof(int));
    logwrite("Before listen");
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <dataMB>\n", argv[0]);
        return -1;
    }
    //create vectors and vars
    int len = atoi(argv[2]) / sizeof(int);
    logwrite_int("Len in int: ", len);
    // int len = atoi(argv[2]) * 1024 * 1024 / sizeof(int);
    //  open connect socket
    sock_l = sock_listen(atoi(argv[1]));
    //prepare interrupt handlers
    signal(SIGTERM, handler_stop);
    signal(SIGINT, handler_stop);
    while (1) {
        //wait for client
        while (sock_r < 0)
            sock_accept(sock_l);
        //send the vector
        for (int i = 0; i < len; ++i)
            vect[i] = i;
        sock_send(sock_r, vect, len);
        //get responses until closed
        sock_rcv(sock_r, vect);
        logwrite("closing?");
        sock_fin(sock_r);
        sock_r = -1;
    }
    return 0;
}