#include "lib.h"

struct in_addr lookup_host(const char *host)
{
    struct addrinfo hints, *res;
    int errcode;
    void *ptr;
    char buf[1024];
    struct in_addr retval;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    CHECK(getaddrinfo(host, NULL, &hints, &res), -1, "Can't retrieve address info")

    printf("Host: %s\n", host);
    retval = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    //here it should be usable for sockets
    inet_ntop(res->ai_family, &retval, buf, BUFSIZE - 1);
    printf("IPv4 address: %s\n", buf);
    freeaddrinfo(res);
    return retval;
}

int sock_r = -1;

int main(int argc, char *argv[])
{    
    int ret;
    struct sockaddr_in sa_srv;
    int len;
    int *vect = calloc(BUFSIZE, sizeof(int));
    
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_address/hostname> <server_port>\n", argv[0]);
        return -1;
    }
    sa_srv.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &sa_srv.sin_addr))
        printf("Address is a valid IP\n");
    else
    {
        printf("Address is a hostname... maybe\n");
        sa_srv.sin_addr = lookup_host(argv[1]);
    }
    sa_srv.sin_port = htons(atoi(argv[2]));
    printf("Connecting...\n");
    sock_r = socket(PF_INET, SOCK_STREAM, 0);
    sa_srv.sin_family = AF_INET;
    //check error <=0
    CHECK(connect(sock_r, (struct sockaddr *)&sa_srv, sizeof(sa_srv)), -1, "Error while connecting")
    //read the data
    len = sock_rcv(sock_r, vect);
    sock_send(sock_r, vect, len);
    shutdown(sock_r, SHUT_WR);
    return 0;
}