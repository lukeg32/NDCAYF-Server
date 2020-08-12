#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <btBulletDynamicsCommon.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

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
const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;

bool test_nw = false;

int main()
{
    if (test_nw)
    {


        return 0;
    }

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
    A.pos = glm::vec3(0, 20, 0);
    A.front = glm::vec3(0, 0, 0);


    struct SpawnPoint B;
    B.pos = glm::vec3(0, -20, 0);
    B.front = glm::vec3(10, 10, 10);

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



    printf("Starting on port %d\nTime: %llu\n", PORT, start);
    while(true){
        // check inbox
        time = getMilliSeconds();

        if (!waitingForMsg)
        {
            //printf("Waiting on port %d\nTime: %llu\n", PORT, time);
            waitingForMsg = true;
        }

        if (recieve(buf, sock, &fromAddr) > 0)
        {
            waitingForMsg = false;

            /*
            printf("\tMsg %s\n", buf);
            printf("\tAt %llu\n", getMilliSeconds());
            */

            lastPacket.addr = fromAddr;

            type = processMsg(buf, ptrPacket);
            //printf("\tFrom %s %d\n", inet_ntoa(fromAddr.sin_addr), type);

            if (type == CONNECT)
            {
                int id;
                // make a new player
                players[numPlayers].pos.x = spawns[nextSpawn].pos.x;
                players[numPlayers].pos.y = spawns[nextSpawn].pos.y;
                players[numPlayers].pos.z = spawns[nextSpawn].pos.z;
                players[numPlayers].front = spawns[nextSpawn].front;
                //strcpy(players[numPlayers].moves, "h");

                // use all the spawns
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

                // just tell the client its start, then the client will wait for the dump to actually start
                // i need to dump all entityes to the new client so it can make them
                sprintf(extra, "%d", id);

                for (int i = 0; i < numClients; i++)
                {
                    sprintf(extra, "%s&%d&%.2f,%.2f,%.2f", extra, i,
                        players[clients[i].entity].pos.x, players[clients[i].entity].pos.y, players[clients[i].entity].pos.z);
                }

                sendMsg(type, sock, fromAddr, extra);
            }
            else if (type == MOVE)
            {
                // validate, too lazy to do rn
                if (true)
                {
                    //calculate the position when the packet info is applied
                    glm::vec3 cameraFront;
                    char moves[10];
                    char frontstr[100];
                    char both[110];
                    int mvID;
                    int id = lastPacket.id;


                    getMovePoint(lastPacket, &cameraFront, moves, frontstr, &mvID);
                    players[clients[id].entity].lastMv = mvID;
                    //printf("Process move [%s]:[%s]\n", moves, frontstr);

                    sprintf(both, "%s&%s&", frontstr, moves);
                    //printf("%s\n", both);
                    strcat(players[clients[id].entity].moves, both);

                    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraFront));
                    glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

                    players[clients[id].entity].front = cameraFront;

                    for (int i = 0; i < strlen(moves); i++)
                    {

                        if (moves[i] == *UNI_FD)
                            players[clients[id].entity].pos += cameraSpeed * cameraFront;

                        if (moves[i] == *UNI_BK)
                            players[clients[id].entity].pos -= cameraSpeed * cameraFront;

                        if (moves[i] == *UNI_LT)
                            players[clients[id].entity].pos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

                        if (moves[i] == *UNI_RT)
                            players[clients[id].entity].pos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

                    }
                    //printf("Process done\n");


                }
                else
                {
                    printf("Invalid, ignoring\n");
                }
            }
            else if (type == PONG)
            {
                sendMsg(type, sock, fromAddr);
            }

        }

        //check time
        if (time - start >= SECOND)
        {
            start = time;
            // for each client
                // dump each client move with oldPos and oldfront to begin, 
                // except this one, dump the newpos, and lastmove
            for (int i = 0; i < numClients; i++)
            {
                char temp[BUFSIZE] = "";
                for (int j = 0; j < numClients; j++)
                {
                    char start[40];
                    if (i == j)
                    {
                        makeString(start, players[clients[j].entity].pos, players[clients[j].entity].front);
                        sprintf(temp, "%s(%d&%s&%d", temp, j, start, players[clients[j].entity].lastMv);
                    }
                    else
                    {
                        makeString(start, players[clients[j].entity].oldPos, players[clients[j].entity].oldFront);
                        //                         other  id  startpos/front    moves
                        sprintf(temp, "%s(%d&%s&%s", temp, j, start, players[clients[j].entity].moves);
                    }


                }
               /*
               sprintf(temp, "%s&%d&%.2f,%.2f,%.2f", temp, i,
                    players[clients[i].entity].pos.x, players[clients[i].entity].pos.y, players[clients[i].entity].pos.z);
               sprintf(temp, "%s&%d&%s", temp, i, players[clients[i].entity].moves);
               */

               // bring the old pos to the current
               // the old pos is where the other clients start this client, then they use the moves to get to the pos
               players[clients[i].entity].oldPos = players[clients[i].entity].pos;
               players[clients[i].entity].oldFront = players[clients[i].entity].front;


               // reset the moves list
               strcpy(players[clients[i].entity].moves, "");

               printf("================Dumping to %d, [%s]==================\n", i, temp);
               sendMsg(DUMP, sock, clients[i].addr, temp);
            }


        }

    }

    close(sock);

    return 0;
}
