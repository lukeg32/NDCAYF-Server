#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "networkConfig.hpp"
#include "server.hpp"

using namespace std;

char hostname[128];
Client clients[MAXPLAYERS];
struct generalPack buf;
size_t bufSize = sizeof(struct generalPack);
socklen_t addrSize = sizeof(struct sockaddr_in);

int actualSock;

void setHostname()
{
    gethostname(hostname, 128);
}

char* hostnameGet()
{
    return hostname;
}

void close()
{
    close(actualSock);
}

// changes id to be the client id
// returns true if addr was found in clients, id becomes that index
bool getClientID(sockaddr_in addr, int *numClients, struct Client *clients, int *id)
{
    // if first client then just give 0
    *id = 0;
    bool success = false;
    int freeSlot = -1;
    if (numClients != 0)
    {
        for (int i = 0; i < *numClients; i++)
        {
            // finds the first match that isn't disconnect
            if ((clients[i].addr.sin_addr.s_addr == addr.sin_addr.s_addr) && !success && !clients[i].disconnected)
            {
                *id = i;
                success = true;
            }

            // finds the first disconnected player
            if (clients[i].disconnected && (freeSlot == -1))
            {
                freeSlot = i;
            }
        }

        // gets an id for this new player
        if (!success)
        {
            // if no freeslot then goes to the next one
            if (freeSlot == -1)
            {
                *id = *numClients;
                (*numClients)++;
            }
            else
            {
                // uses freeslot
                *id = freeSlot;

            }
        }
    }

    return success;
}

// gets the difference between two points and checks if the largest distance is too big
// true for too far, false for not
bool isMovingTooFar(glm::vec3 *lastGoodPos, glm::vec3 *toBeYou)
{
    bool success = false;
    glm::vec3 theDiff = abs(*lastGoodPos - *toBeYou);
    float bigBoi = std::max(theDiff.x, std::max(theDiff.y, theDiff.z));

    if (isnan(bigBoi) || (bigBoi > MAXMOVE))
    {
        printf("ERROR caught haxer or trash values: [%.2f, %.2f, %.2f](%.2f)=================\n", toBeYou->x, toBeYou->y, toBeYou->z, bigBoi);
        success = true;
    }

    return success;

}

// returns true if this is a client that is connected
bool findClient(sockaddr_in addr, int *numClients, struct Client *clients)
{
    bool success = false;
    if (numClients > 0)
    {
        for (int i = 0; i < *numClients; i++)
        {
            //printf("========%s == %s && %s && %s\n", inet_ntoa(clients[i].addr.sin_addr), inet_ntoa(addr.sin_addr), !success ? "True" : "False", !clients[i].disconnected ? "True" : "False");
            if ((clients[i].addr.sin_addr.s_addr == addr.sin_addr.s_addr) && !success && !clients[i].disconnected)
            {
                success = true;
            }
        }

    }

    return success;
}


int makeSocket()
{
    int success = 0;
    int inSock;
    struct sockaddr_in myaddr;
    struct timeval tv;

    // gets the hostname and sets it to the var
    gethostname(hostname, 128);

    // make a socket UDP
    if ((inSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Cannot create socket\n");
        success = -1;
    }

    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    // set a timeout, so it doesn't halt the process
    if (setsockopt(inSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("Time out option failed\n");
        success = -1;
    }

    // create the address we listen to
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);

    // bind to address, and so that recvfrom works without having to send any
    if (bind(inSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        success = -1;
    }

    actualSock = inSock;

    return success;
}



int recieveNew(struct generalPack *msgPack, struct sockaddr_in *fromAddr)
{
    int recvlen = -1;
    int success = -1;

    recvlen = recvfrom(actualSock, &buf, bufSize, 0, (struct sockaddr *)fromAddr, &addrSize);

    if (recvlen > 0) {
        //printf("Received %d bytes\n", recvlen);
        success = 1;
        *msgPack = buf;

        //printf("%s, %s, %d, %ld, %ld\n", msgPack->key, msgPack->name, msgPack->protocol, msgPack->time.tv_sec, msgPack->time.tv_usec);
        //printf("From: %s\n", inet_ntoa(fromAddr->sin_addr));
    }

    return success;
}

int sendNew(struct generalPack toSend, struct sockaddr_in toAddr)
{
    int success = 0;

    // set the timestamp
    gettimeofday(&(toSend.time), NULL);
    printf("%lu %lu\n", toSend.time.tv_sec, toSend.time.tv_usec);

    // send
    if (sendto(actualSock, (const void *)&toSend, bufSize, 0, (struct sockaddr *)&toAddr, addrSize) < 0)
    {
        perror("Failed to send\n");
        success = -1;
    }

    return success;
}

struct generalPack makeBasicPack(int ptl)
{
    struct generalPack pack;
    strcpy(pack.key, SUPERSECRETKEY_SERVER);
    strcpy(pack.name, hostname);
    pack.protocol = ptl;

    return pack;
}

int getCurClients(struct Client *clients, int *num)
{
    int actual = 0;
    for (int i = 0; i < *num; i++)
    {
        if (!clients[i].disconnected)
        {
            actual++;
        }
    }

    return actual;
}
