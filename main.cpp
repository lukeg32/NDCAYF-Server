#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <btBulletDynamicsCommon.h>

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

int clientNum;


int main()
{
    int sock;
    char buf[BUFSIZE];
    struct sockaddr_in fromAddr, *ptrAddr;
    int addrlen;
    int type;
    bool waitingForMsg = false;
    struct MsgPacket lastPacket, *ptrPacket;

    struct timeval tv;

    clientNum = 0;
    ptrPacket = &lastPacket;

    struct SpawnPoint A;
    A.x = 0;
    A.y = 20;
    A.z = 0;


    struct SpawnPoint B;
    B.x = 20;
    B.y = 20;
    B.z = 0;

    struct SpawnPoint spawns[2] = {A, B};
    int nextSpawn = 0;

    struct Entity players[MAXPLAYERS];
    int numPlayers = 0;

    if ((sock = makeSocket()) < 0)
    {
        perror("Failed to make a socket\n");
        return -1;
    }


    while(true){
        // check inbox

        if (!waitingForMsg)
        {
            printf("Waiting on port %d\nTime: %llu\n", PORT, getMilliSeconds());
            waitingForMsg = true;
        }

        if (recieve(buf, sock, &fromAddr) > 0)
        {
            waitingForMsg = false;

            printf("\tMsg %s\n", buf);
            printf("\tFrom %s\n", inet_ntoa(fromAddr.sin_addr));
            printf("\tAt %llu\n", getMilliSeconds());

            lastPacket.addr = ptrAddr;
            type = processMsg(buf, ptrPacket);

            if (type == CONNECTME)
            {
                // make new player
                struct Entity player;
                player.x = spawns[nextSpawn].x;
                player.y = spawns[nextSpawn].y;
                player.z = spawns[nextSpawn].z;
                nextSpawn++;

                players[numPlayers] = player;

                struct Client aClient;
                aClient.id = clientNum;
                aClient.addr = fromAddr;
                aClient.entity = numPlayers;

                numPlayers++;

            }

            sendMsg(type, sock, fromAddr);
        }

        //check time
    }

    close(sock);

    return 0;
}
