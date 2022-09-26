#include "../../common/lib.h"

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

int openfile() {
    int logfile;
    char *logfilename;
    time_t t;
    char *timestr;
    char logname[BUFSIZE] = "logs/log";

    time(&t);
    timestr = ctime(&t);
    logfilename = malloc(strlen(logname) + 3 + strlen(timestr) + 4 + 1);
    strcpy(logfilename, logname);
    strcat(logfilename, " - ");
    strcat(logfilename, timestr);
    strcat(logfilename, ".txt");
    CHECK(logfile = creat(logfilename, 0666), -1, "Error while opening log file")
    free(logfilename);
    return logfile;
}

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

void handler_stop(int signum)
{
    logwrite("SIGTERM received.");
    sock_closeall(); //sockets
    exit(0);
}

int main(int argc, char *argv[])
{
    char msg[BUFSIZE];
    char tmp[BUFSIZE];
    double *vect, *avg_vect;
    uint64_t len, avg_len;
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <dataMB>\n", argv[0]);
        return -1;
    }
    srand(time(NULL));
    // create vectors and vars
    // len = atoi(argv[2]) / sizeof(double);
    len = atoi(argv[2]) * 1024 * 1024 / sizeof(double);
    vect = calloc(len, sizeof(double));
    logwrite_int("Len in double: ", len);
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
        for (uint64_t i = 0; i < len; ++i)
            //vect[i] = i;
            vect[i] = (double)(rand() % 1024);
        logwrite("Ready to send");
        sock_send(sock_r, vect, len);
        //get responses
        avg_len = sock_rcv(sock_r, &avg_vect); //get data
        sock_send_str(sock_r, "ack"); //stupid, but functional
        sock_rcv_str(sock_r, msg);
        // close the socket
        sock_fin(sock_r);
        sock_r = -1;
        //send data to the file
        logwrite("Saving data to the file...");
        int f = openfile();
        for (uint64_t i = 0; i < avg_len; ++i) {
            sprintf(tmp, "%.3f ", avg_vect[i]);
            sprintf(tmp, "%lf ", avg_vect[i]);
            write(f, tmp, strlen(tmp));
        }
        write(f, "\n", 1);
        write(f, msg, strlen(msg));
        close(f);
        logwrite("Done.");
        free(avg_vect);
    }
    free(vect);
    return 0;
}