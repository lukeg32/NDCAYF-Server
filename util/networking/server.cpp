#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "networkConfig.hpp"

char hostname[128];


void composeMsg(char msg[], char protocol[], char extra[] = "none")
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    unsigned long long millisecondsSinceEpoch =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;

    sprintf(msg, "%s$%s$%s$%llu$%s", SUPERSECRETKEY_SERVER, hostname, protocol, millisecondsSinceEpoch, extra);
}

int makeSocket()
{
    struct sockaddr_in myaddr;    /* our address */
    int inSock;
    char lanReply[128];

    if ((inSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        return -1;
    }


    gethostname(hostname, 128);
    //sprintf(lanReply, "%s$%s", SUPERSECRETKEY_SERVER, hostname);

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);

    if (bind(inSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    return inSock;
}


struct sockaddr_in recieve(char buf[], int inSock)
{
    int recvlen;                                /* # bytes received */
    struct sockaddr_in fromAddr;
    socklen_t addrlen = sizeof(fromAddr);        /* length of addresses */

    printf("Waiting on port %d\n", PORT);
    recvlen = recvfrom(inSock, buf, BUFSIZE, 0, (struct sockaddr *)&fromAddr, &addrlen);
    printf("Received %d bytes\n", recvlen);
    if (recvlen > 0) {
        buf[recvlen] = 0;
        printf("Received message: \"%s\"\n", buf);
    }

    return fromAddr;
}

/*
 * msg format rn:
 * delimited by $
 * key
 * name
 * protocol
 * time
 *
 * delimited by &
 * example state
 * objid&x&y&z&rotation&ect
 *
 */

// sees if this is a ping, or a client sending its state
int processMsg(char msg[])
{
    char clientKey[128];
    char name[128];
    char protocol[128];
    strcpy(clientKey, strtok(msg, "$"));

    if (strcmp(clientKey, SUPERSECRETKEY_CLIENT) == 0)
    {
        strcpy(name, strtok(NULL, "$"));
        strcpy(protocol, strtok(NULL, "$"));


        if (strcmp(protocol, PING) == 0)
        {
            printf("Send response\n");
            return SENDPONG;
        }
    }
    else
    {
        printf("Not a NDCAYF client\n");
        return -1;
    }
}


// send msg
int sendMsg(int type, int sock, struct sockaddr_in addr)
{
    char msg[BUFSIZE];
    bool sendMsg = false;
    socklen_t addrlen = sizeof(addr);

    if (type == SENDPONG)
    {
        composeMsg(msg, PONG);
    }

    printf("Sending %s\n", msg);

    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, addrlen) < 0)
    {
        perror("Failed to send a msg\n");
    }
}
