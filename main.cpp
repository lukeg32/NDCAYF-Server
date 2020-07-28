#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <btBulletDynamicsCommon.h>
#include <string>
#include <iostream>

#include "util/bulletDebug/collisiondebugdrawer.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

// this is a horrible way to do this

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include "util/networking/networkConfig.hpp"
#include "util/networking/server.hpp"

using namespace std;
int clientNum;


int main()
{
    int sock;
    char buf[BUFSIZE];
    struct sockaddr_in fromAddr, *ptrAddr;
    int addrlen;
    int type;
    char extra[BUFSIZE];
    bool waitingForMsg = false;
    struct MsgPacket lastPacket, *ptrPacket;

    unsigned long long time;
    unsigned long long start = getMilliSeconds();

    struct timeval tv;

    clientNum = 0;
    ptrPacket = &lastPacket;

    struct SpawnPoint A;
    A.x = 0;
    A.y = 20;
    A.z = 0;


    struct SpawnPoint B;
    B.x = 0;
    B.y = -20;
    B.z = 0;

    struct SpawnPoint spawns[2] = {A, B};
    int nextSpawn = 0;

    struct Entity players[MAXPLAYERS];
    int numPlayers = 0;

    struct Client clients[MAXPLAYERS];
    int numClients = 0;

    if ((sock = makeSocket()) < 0)
    {
        perror("Failed to make a socket\n");
        return -1;
    }



    while(true){
        // check inbox
        time = getMilliSeconds();

        if (!waitingForMsg)
        {
            printf("Waiting on port %d\nTime: %llu\n", PORT, time);
            waitingForMsg = true;
        }

        if (recieve(buf, sock, &fromAddr) > 0)
        {
            waitingForMsg = false;

            printf("\tMsg %s\n", buf);
            printf("\tFrom %s\n", inet_ntoa(fromAddr.sin_addr));
            printf("\tAt %llu\n", getMilliSeconds());

            lastPacket.addr = fromAddr;

            type = processMsg(buf, ptrPacket);

            lastPacket.type = type;

            if (type == CONNECT)
            {
                int id;
                // make a new player
                players[numPlayers].x = spawns[nextSpawn].x;
                players[numPlayers].y = spawns[nextSpawn].y;
                players[numPlayers].z = spawns[nextSpawn].z;
                nextSpawn++;
                if (nextSpawn == 2)
                    nextSpawn = 0;

                // make a client
                clients[numClients].id = numClients; // useless
                clients[numClients].addr = fromAddr;
                clients[numClients].entity = numPlayers;

                numPlayers++;

                id = numClients;
                numClients++;
                //printf("%d\t%d\n", id, numClients);

                // i need to dump all entityes to the new client so it can make them
                sprintf(extra, "%d&", id);

                for (int i = 0; i < numClients; i++)
                {
                    sprintf(extra, "%s&%d&%d,%d,%d", extra, i,
                        players[clients[i].entity].x, players[clients[i].entity].y, players[clients[i].entity].z);
                }

                sendMsg(type, sock, fromAddr, extra);
            }
            else if (type == MOVE)
            {
                // add this to list of moves
            }
            else if (type == PONG)
            {
                sendMsg(type, sock, fromAddr);
            }

        }

        //check time
        if (time - start >= 100)
        {
            start = time;
            char temp[BUFSIZE];
            for (int i = 0; i < numClients; i++)
            {
                sprintf(temp, "%s&%d&%d,%d,%d", temp, i,
                    players[clients[i].entity].x, players[clients[i].entity].y, players[clients[i].entity].z);
            }

            printf("Dumping, %s\n", temp);

            for (int i = 0; i < numClients; i++)
            {
                sendMsg(DUMP, sock, clients[i].addr, temp);
            }
        }
    }

    close(sock);

    return 0;
}
