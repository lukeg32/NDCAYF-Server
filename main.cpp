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
#include "util/networking/serverEdit.hpp"

using namespace std;
int nextSpawn = 0;

bool test_nw = true;

int getSpawn();

int getSpawn()
{
    int spawn = nextSpawn;
    nextSpawn++;
    if (nextSpawn == 2)
    {
        nextSpawn = 0;
    }
    return spawn;
}

int main()
{
    if (test_nw)
    {
        bool one = true;
        bool two = true;
        printf("%d\n", one + two);
        printf("waiting for commands\n");
        getData();

        return 0;
    }

    loadModels("scratchpadWorld.json");

    // socket inputs
    struct generalPack *msgPack = new struct generalPack;
    struct sockaddr_in fromAddr;

    // probably only run on fast and medium
    struct timeval FAST;
    FAST.tv_sec = 0;
    FAST.tv_usec = 17000;
    struct timeval MEDIUM;
    MEDIUM.tv_sec = 0;
    MEDIUM.tv_usec = 250000;
    struct timeval SLOW;
    SLOW.tv_sec = 1;
    SLOW.tv_usec = 0;

    struct timeval TIMEOUT;
    TIMEOUT.tv_sec = 10;
    TIMEOUT.tv_usec = 0;

    struct timeval cur;
    struct timeval last;
    struct timeval timeouthelper;
    gettimeofday(&cur, NULL);

    timeradd(&cur, &FAST, &last);


    struct Entity objects[MAXPLAYERS];
    int numObjects = 0;

    struct Client clients[MAXPLAYERS];
    int numClients = 0;


    struct SpawnPoint A;
    A.pos = glm::vec3(20, 20, 0);
    A.front = glm::vec3(0, 0, 0);


    struct SpawnPoint B;
    B.pos = glm::vec3(0, 20, 20);
    B.front = glm::vec3(10, 10, 10);

    struct SpawnPoint spawns[2] = {A, B};

    int movesRecieved = 0;

    if (makeSocket() < 0)
    {
        perror("Failed to make a socket\n");
        return -1;
    }

    // pre built packets
    struct generalPack pingPack = makeBasicPack(PING);
    struct generalPack pongPack = makeBasicPack(PONG);
    struct generalPack connectPack = makeBasicPack(CONNECT);
    struct generalPack dumpPack = makeBasicPack(DUMP);
    struct generalPack infoPack = makeBasicPack(INFO);

    int msgCounter = 0;
    while(true)
    {
        gettimeofday(&cur, NULL);
        // check inbox
        if (recieveNew(msgPack, &fromAddr) > 0)
        {
            msgCounter++;
            //printf("%s, %s, %d, %ld, %ld, %d\n", msgPack->key, msgPack->name, msgPack->protocol, msgPack->time.tv_sec, msgPack->time.tv_usec, got);
            if (msgPack->protocol == PING)
            {
                sendNew(pongPack, fromAddr);
            }
            else if (msgPack->protocol == INFO)
            {
                struct infoStruct aboutMe;
                aboutMe.maxPlayers = MAXPLAYERS;
                aboutMe.curPlayers = getCurClients(clients, &numClients);
                strcpy(aboutMe.mapName, "your mum");
                strcpy(aboutMe.gameType, "floaters");
                aboutMe.isCustom = true;


                memcpy(&infoPack.data, &aboutMe, sizeof(struct infoStruct));
                sendNew(infoPack, fromAddr);
            }
            else if (msgPack->protocol == MOVE)
            {
                struct move movePoint;
                unsigned int mvID;
                int id;

                movePoint = *((struct move*)(msgPack->data));
                memcpy(&mvID, &msgPack->data[sizeof(struct move)], sizeof(unsigned int));
                //printf("%s\n", movePoint.extraActions);
                //printf("%.2f %.2f %.2f\n", movePoint.pos.x, movePoint.pos.y, movePoint.pos.z);
                //printf("%d\n", mvID);


                if (findClient(fromAddr, &numClients, clients))
                {
                    // get the id
                    getClientID(fromAddr, &numClients, clients, &id);
                    // validate that its not a jump to infinity
                    if (!isMovingTooFar(&objects[clients[id].entity].pos, &movePoint.pos))
                    {
                        movesRecieved++;
                        printf("%.2f %.2f %.2f %d ", movePoint.pos.x, movePoint.pos.x,  movePoint.pos.z, id);

                        // update data
                        objects[clients[id].entity].lastMv = mvID;
                        objects[clients[id].entity].front = movePoint.dir;
                        objects[clients[id].entity].pos = movePoint.pos;

                        // update the last response data
                        gettimeofday(&clients[id].lastResponse, NULL);
                        clients[id].disconnected = false;

                        // add move to list
                        objects[clients[id].entity].moves[objects[clients[id].entity].numMoves] = movePoint;
                        objects[clients[id].entity].numMoves++;
                    }
                    else
                    {
                        printf("nanananananan\n");
                    }

                }
            }
            else if (msgPack->protocol == CONNECT)
            {
                int id;

                // check if this is an old client
                if (getClientID(fromAddr, &numClients, clients, &id))
                {
                    // old client, should be useless
                    printf("This is an old client\n");
                    int spawn = getSpawn();

                    printf("==SPAWN==%.2f %.2f %.2f\n", spawns[spawn].pos.x, spawns[spawn].pos.y,  spawns[spawn].pos.z);
                    objects[clients[id].entity].pos = spawns[spawn].pos;
                    objects[clients[id].entity].front = spawns[spawn].front;

                    struct move temp;
                    temp.pos = spawns[spawn].pos;
                    temp.dir = spawns[spawn].front;
                    objects[numObjects].moves[0] = temp;
                    objects[numObjects].numMoves++;

                    gettimeofday(&clients[id].lastResponse, NULL);
                    clients[id].disconnected = false;
                    clients[id].entity = numObjects;
                    clients[id].addr = fromAddr;
                    objects[clients[id].entity].lastMv = 0;

                }
                else
                {
                    int spawn = getSpawn();

                    // make a new player
                    objects[numObjects].pos = spawns[spawn].pos;
                    objects[numObjects].front = spawns[spawn].front;
                    printf("==SPAWN==%.2f %.2f %.2f\n", spawns[spawn].pos.x, spawns[spawn].pos.y,  spawns[spawn].pos.z);

                    // sets the spawn b/c .pos and .front are not used rn
                    struct move temp;
                    temp.pos = spawns[spawn].pos;
                    temp.dir = spawns[spawn].front;
                    objects[numObjects].moves[0] = temp;
                    objects[numObjects].numMoves++;

                    // make a client
                    clients[id].addr = fromAddr;
                    clients[id].entity = numObjects;
                    gettimeofday(&clients[id].lastResponse, NULL);
                    clients[id].disconnected = false;
                    objects[clients[id].entity].lastMv = 0;

                    numObjects++;

                    printf("This is an new client %d\n", id);

                }
                strcpy(connectPack.data, (char *)&id);

                printf("%s, %s, %d, %ld, %ld, %d\n", connectPack.key, connectPack.name, connectPack.protocol, connectPack.time.tv_sec, connectPack.time.tv_usec);

                sendNew(connectPack, fromAddr);
            }
        }


        //TODO add timing
        if (timercmp(&cur, &last, >=))
        {
            // all moves will be printed on one line
            if (movesRecieved > 0)
            {
                movesRecieved = 0;
                printf("\n");
            }
            gettimeofday(&cur, NULL);
            timeradd(&cur, &FAST, &last);

            // for each client
                // dump each client move with oldPos and oldfront to begin,
                // except this one, dump the newpos, and lastmove
            for (int i = 0; i < numClients; i++)
            {
                // don't do anything for this player
                if (!clients[i].disconnected)
                {
                    int buf = 0;
                    int counter = 0;
                    for (int j = 0; j < numClients; j++)
                    {
                        // don't get the disconnected players data
                        if (!clients[j].disconnected)
                        {
                            counter++;
                            if (i == j)
                            {
                                //sprintf(temp, "%s(%d&%s&%d", temp, j, start, objects[clients[j].entity].lastMv);
                                // send the latest data
                                //printf("%d\n", objects[clients[j].entity].numMoves);
                                int bestMove = objects[clients[j].entity].numMoves - 1;
                                if (bestMove < 0)
                                {
                                    bestMove = 0;
                                }

                                memcpy(&dumpPack.data[buf], &objects[clients[j].entity].moves[bestMove], sizeof(struct move));
                                buf += sizeof(struct move);

                                memcpy(&dumpPack.data[buf], &objects[clients[j].entity].lastMv, sizeof(unsigned int));
                                buf += sizeof(unsigned int);
                            }
                            else
                            {
                                //printf("%d\n", objects[clients[j].entity].numMoves);
                                memcpy(&dumpPack.data[buf], &objects[clients[j].entity].numMoves, sizeof(unsigned short));
                                buf += sizeof(unsigned short);

                                for (int k = 0; k < objects[clients[j].entity].numMoves; k++)
                                {
                                    memcpy(&dumpPack.data[buf], &objects[clients[j].entity].moves[k], sizeof(struct move));
                                    buf += sizeof(struct move);
                                }
                            }

                            if (buf >= 1000)
                            {
                                printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhhhhhhhhhhhhhhh\n");
                            }
                        }
                    }


                    // send data
                    dumpPack.numObjects = counter;

                    printf("================Dumping to %d, numObjects %d, bytes %d==================", i, counter, buf);
                    printf("IP: %s ", inet_ntoa(clients[i].addr.sin_addr));
                    /////////sendMsg(DUMP, sock, clients[i].addr, temp);
                    sendNew(dumpPack, clients[i].addr);
                }
            }

            for (int i = 0; i < numClients; i++)
            {
                // reset the moves list
                objects[clients[i].entity].numMoves = 0;

                // checks if a player hasn't sent data in a while
                timeradd(&clients[i].lastResponse, &TIMEOUT, &timeouthelper);
                if (!clients[i].disconnected && timercmp(&cur, &timeouthelper, >=))
                {
                     printf("Disconnecting player %d\n", i);
                     clients[i].disconnected = true;
                     int thisone = clients[i].entity;
                     objects[thisone].numMoves = 0;
                }
            }
        }
    }




    //printf("Starting on port %d\nTime: %llu\n", PORT, start);
    /*
    while(true){
        // check inbox
        //time = getMilliSeconds();

        if (!waitingForMsg)
        {
            //printf("Waiting on port %d\nTime: %llu\n", PORT, time);
            waitingForMsg = true;
        }

        //if (recieveNew(buf, &fromAddr) > 0)
        {
            waitingForMsg = false;

            
            printf("\tMsg %s\n", buf);
            printf("\tAt %llu\n", getMilliSeconds());
            

            lastPacket.addr = fromAddr;

            type = processMsg(buf, ptrPacket);
            //printf("\tFrom %s %d\n", inet_ntoa(fromAddr.sin_addr), type);

            if (type == CONNECT)
            {
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
                    getClientID(lastPacket.addr, &numClients, clients, &id);


                    getMovePoint(lastPacket, &cameraFront, moves, frontstr, &mvID);
                    players[clients[id].entity].lastMv = mvID;
                    printf("Process move [%s]:[%s], %d", moves, frontstr);
                    printf(" old[%.3f,%.3f,%.3f]", players[clients[id].entity].pos.x, players[clients[id].entity].pos.y, players[clients[id].entity].pos.z);

                    sprintf(both, "%s&%s&", frontstr, moves);
                    //printf("%s\n", both);
                    //strcat(players[clients[id].entity].moves, both);

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
                    printf(" new [%.3f,%.3f,%.3f] %d\n", players[clients[id].entity].pos.x, players[clients[id].entity].pos.y, players[clients[id].entity].pos.z, mvID);


                }
                else
                {
                    printf("Invalid, ignoring\n");
                }
            }
            else if (type == PONG)
            {
                //////////sendMsg(type, sock, fromAddr);
            }

        }

        //check time
        //if (time - start >= NORMAL)
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

               printf("================Dumping to %d, [%s]==================\n", i, temp);
               //printf("addr %s\n", inet_ntoa(clients[i].addr.sin_addr));
               /////////sendMsg(DUMP, sock, clients[i].addr, temp);
            }

            for (int i = 0; i < numClients; i++)
            {
               // reset the moves list
               strcpy(players[clients[i].entity].moves, "");


               // bring the old pos to the current
               // the old pos is where the other clients start this client, then they use the moves to get to the pos
               players[clients[i].entity].oldPos = players[clients[i].entity].pos;
               players[clients[i].entity].oldFront = players[clients[i].entity].front;
            }


        }

    }
    */

    close();

    return 0;
}
