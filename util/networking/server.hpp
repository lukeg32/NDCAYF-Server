#ifndef SERVER_H
#define SERVER_H
#include <glm/glm.hpp>


struct Client
{
    sockaddr_in addr;
    int entity;
};


struct move
{
    glm::vec3 pos;
    glm::vec3 dir;
    char extraActions[5];
};


struct Entity
{
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 oldPos;
    glm::vec3 oldFront;
    struct move moves[64];
    int numMoves;
    int lastMv;
};

struct entityDump
{
    glm::vec3 pos;
    glm::vec3 dir;
    struct move moves[64];
    int numMoves;
};

struct SpawnPoint
{
    glm::vec3 pos;
    glm::vec3 front;
};

// remove
struct MsgPacket
{
    sockaddr_in addr;
    char name[128];
    int ptl;
    int id;
    char data[BUFSIZE];
    unsigned long long time;
};


struct generalPack
{
    char key[10];
    char name[10];
    unsigned short int protocol;
    unsigned short int numObjects;
    struct timeval time;
    char data[1000];
};


int makeSocket();
void close();

bool getClientID(sockaddr_in addr, int *numClients, struct Client *clients, int *id);

// remove
void composeMsg(char msg[], char protocol[], char extra[] = "none");
int processMsg(char msg[], struct MsgPacket *packet);
void getMovePoint(struct MsgPacket packet, glm::vec3 *front, char moves[], char frontstr[], int *id);
void getParts(std::string parts[], std::string raw, int amount, std::string deli);
void makeString(char result[], glm::vec3 pos, glm::vec3 front);
//

int recieveNew(struct generalPack *msgPack, struct sockaddr_in *fromAddr);
int sendNew(struct generalPack toSend, struct sockaddr_in toAddr);

struct generalPack makeBasicPack(int ptl);
void setHostname();

#endif
