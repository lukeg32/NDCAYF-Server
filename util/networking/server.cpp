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

void close()
{
    close(actualSock);
}

using namespace std;


void composeMsg(char msg[], char protocol[], char extra[])
{
    //unsigned long long millisecondsSinceEpoch = getMilliSeconds();

    //sprintf(msg, "%s$%s$%s$%llu$%s", SUPERSECRETKEY_SERVER, hostname, protocol, millisecondsSinceEpoch, extra);
}


void getParts(std::string parts[], std::string raw, int amount, std::string deli)
{
    size_t pos = 0;
    int cur = 0;
    std::string token;
    while ((pos = raw.find(deli)) != std::string::npos) {
        token = raw.substr(0, pos);
        parts[cur] = token;
        cur++;
        raw.erase(0, pos + deli.length());
    }
    parts[cur] = raw;
}

// changes id to be the client id
// returns true if addr was found in clients, id becomes that index
bool getClientID(sockaddr_in addr, int *numClients, struct Client *clients, int *id)
{
    // if first client then just give 0
    *id = 0;
    bool success = false;
    if (numClients != 0)
    {
        for (int i = 0; i < *numClients; i++)
        {
            //printf("========%s = %s\n", inet_ntoa(clients[i].addr.sin_addr), inet_ntoa(addr.sin_addr));
            if (clients[i].addr.sin_addr.s_addr == addr.sin_addr.s_addr && !success)
            {
                *id = i;
                success = true;
            }
        }

        if (!success)
        {
            // should be safe and only increment when a new addr is put in
            *id = *numClients;
            (*numClients)++;
        }
    }

    return success;
}


void getMovePoint(struct MsgPacket packet, glm::vec3 *front, char moves[], char frontstr[], int *id)
{
    std::string raw = packet.data;
    std::string parts[3];
    std::string partdeli("&");

    std::string floats[3];
    std::string subdeli(",");

    float x, y, z;


    // get the front raw
    getParts(parts, raw, 3, partdeli);
    /*
    for (int i = 0; i < 3; i++)
    {
        printf("%s\n", parts[i].c_str());
    }
    */


    // get the floats
    strcpy(frontstr, parts[0].c_str());
    getParts(floats, parts[0], 3, subdeli);

    //apply
    *front = glm::vec3(std::stod(floats[0]), std::stod(floats[1]), std::stod(floats[2]));

    // the moves
    strcpy(moves, parts[1].c_str());

    // get id
    *id = stoi(parts[2]);

    //printf("%s\n", moves);

}

void makeString(char result[], glm::vec3 pos, glm::vec3 front)
{
    sprintf(result, "%.3f,%.3f,%.3f&%.3f,%.3f,%.3f",
        pos.x, pos.y, pos.z, front.x, front.y, front.z);
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

// sees if this is a ping, or a client sending its state
// literlly useless
int processMsg(char msg[], struct MsgPacket *packet)
{
    char clientKey[128];
    char name[128];
    char protocol[128];
    char idstr[100];
    char data[BUFSIZE];
    int ptl;
    int clientID;
    char time[256];
    strcpy(clientKey, strtok(msg, "$"));


    if (strcmp(clientKey, SUPERSECRETKEY_CLIENT) == 0)
    {
        strcpy(name, strtok(NULL, "$"));

        // get protocol and make it a number
        strcpy(protocol, strtok(NULL, "$"));
        ptl = std::stoi(protocol);

        strcpy(time, strtok(NULL, "$"));

        // safe guard
        if (ptl == PING)
        {
            clientID = -1;
            strcpy(data, " ");;
        }
        else
        {
            strcpy(idstr, strtok(NULL, "$"));
            clientID = std::stoi(idstr);
            strcpy(data, strtok(NULL, "$"));
        }


        strcpy(packet->name, name);
        strcpy(packet->data, data);
        packet->ptl = ptl;
        packet->time = atoll(time);
        packet->id = clientID;

        if (ptl == PING)
        {
            printf("Send response\n");
            return PONG;
        }
        else if (ptl ==  CONNECT)
        {
            printf("\"Connecting\" client\n");
            return CONNECT;
        }
        else if (ptl == MOVE)
        {
            //printf("Got new move\n"); return MOVE;
            return MOVE;
        }
    }
    else
    {
        printf("Not a NDCAYF client\n");
        return -1;
    }
}


// send msg
/*
int sendMsg(int type, int sock, struct sockaddr_in addr, char extra[])
{
    char msg[BUFSIZE];
    char num[10];
    bool sendMsg = false;
    socklen_t addrlena = sizeof(addr);

    if (type == PONG)
    {

        sprintf(num, "%d", PONG);
        composeMsg(msg, num);
    }
    else if (type == CONNECT)
    {
        sprintf(num, "%d", CONNECT);

        composeMsg(msg, num, extra);
    }
    else if (type == DUMP)
    {
        sprintf(num, "%d", DUMP);

        composeMsg(msg, num, extra);
    }

    if (type != DUMP)
    {
        printf("Sending %s\n", msg);
    }

    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, addrlena) < 0)
    {
        perror("Failed to send a msg\n");
    }
}
*/
