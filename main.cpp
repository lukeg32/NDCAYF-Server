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

int main()
{
    struct MsgPacket test;
    char msg[] = "ndcayfclient$luke$6$2345656745$0$0.0,0.0,0.0&&1234";

    printf("running\n");
    printf("%d\n", processMsg(msg, &test));
    /*
     *
struct MsgPacket
{
    sockaddr_in addr;
    char name[128];
    int ptl;
    int id;
    char data[4048];
    unsigned long long time;
};
     *
     */
    printf("addr %s, name %s, ptl %d, clid %d, data %s, time %llu\n", inet_ntoa(test.addr.sin_addr), test.name, test.ptl, test.id, test.data, test.time);


    glm::vec3 testFront;
    char testmoves[10];
    int testid;

    getMovePoint(test, &testFront, testmoves, &testid);
    printf("id %d, move %s,  vec %.2f, %.2f, %.2f", testid, testmoves, testFront.x, testFront.y, testFront.z);

    return 0;

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

            if (type == CONNECT)
            {
                int id;
                // make a new player
                players[numPlayers].pos = spawns[nextSpawn].front;
                players[numPlayers].front = spawns[nextSpawn].front;

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
                    std:string moves;
                    int id;

                    /*
                    getMovePoint(lastPacket, &cameraFront, &moves, &id);
                    
                    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraFront));
                    glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

                    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                        cameraPos += cameraSpeed * cameraFront;
                    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                        cameraPos -= cameraSpeed * cameraFront;
                    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                        */

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
        if (time - start >= 100)
        {
            start = time;
            char temp[BUFSIZE] = "";
            for (int i = 0; i < numClients; i++)
            {
                sprintf(temp, "%s&%d&%d,%d,%d", temp, i,
                    players[clients[i].entity].pos.x, players[clients[i].entity].pos.y, players[clients[i].entity].pos.z);
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
