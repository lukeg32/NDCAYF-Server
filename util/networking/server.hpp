#ifndef SERVER_H
#define SERVER_H
#include <glm/glm.hpp>


struct SpawnPoint
{
    glm::vec3 pos;
    glm::vec3 front;
};


struct Client
{
    sockaddr_in addr;
    struct timeval lastResponse;
    bool disconnected;
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
    struct move moves[64];
    unsigned short numMoves;
    unsigned int lastMv;
};

struct infoStruct
{
    unsigned short maxPlayers;
    unsigned short curPlayers;
    char mapName[25];
    char gameType[25];
    bool isCustom;
};


int makeSocket();
void close();

int recieveNew(struct generalPack *msgPack, struct sockaddr_in *fromAddr);
int sendNew(struct generalPack toSend, struct sockaddr_in toAddr);

// validating
bool isMovingTooFar(glm::vec3 *lastGoodPos, glm::vec3 *toBeYou);
bool findClient(sockaddr_in addr, int *numClients, struct Client *clients);

bool getClientID(sockaddr_in addr, int *numClients, struct Client *clients, int *id);
int getCurClients(struct Client *clients, int *numClients);

struct generalPack makeBasicPack(int ptl);
void setHostname();
char* hostnameGet();

#endif
