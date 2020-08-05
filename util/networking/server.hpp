#ifndef SERVER_H
#define SERVER_H
#include <glm/glm.hpp>


struct Client
{
    int id;
    sockaddr_in addr;
    int entity;
};

struct Entity
{
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 oldPos;
    glm::vec3 oldFront;
    char moves[BUFSIZE * 2];
    int lastMv;
};

struct SpawnPoint
{
    glm::vec3 pos;
    glm::vec3 front;
};

struct MsgPacket
{
    sockaddr_in addr;
    char name[128];
    int ptl;
    int id;
    char data[BUFSIZE];
    unsigned long long time;
};

int recieve(char buf[], int inSock, struct sockaddr_in *fromAddr);
int makeSocket();
void composeMsg(char msg[], char protocol[], char extra[] = "none");
int sendMsg(int type, int sock, struct sockaddr_in addr, char extra[] = "none");
unsigned long long getMilliSeconds();
int processMsg(char msg[], struct MsgPacket *packet);
void getMovePoint(struct MsgPacket packet, glm::vec3 *front, char moves[], char frontstr[], int *id);
void getParts(std::string parts[], std::string raw, int amount, std::string deli);
void makeString(char result[], glm::vec3 pos, glm::vec3 front);

#endif
