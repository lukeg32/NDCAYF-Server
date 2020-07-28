#ifndef SERVER_H
#define SERVER_H


struct Client
{
    int id;
    sockaddr_in addr;
    int entity;
};

struct Entity
{
    int x;
    int y;
    int z;
};

struct SpawnPoint
{
    int x;
    int y;
    int z;
};

struct MsgPacket
{
    sockaddr_in addr;
    char name[128];
    int ptl;
    int type;
    unsigned long long time;
};

int recieve(char buf[], int inSock, struct sockaddr_in *fromAddr);
int makeSocket();
void composeMsg(char msg[], char protocol[], char extra[] = "none");
int sendMsg(int type, int sock, struct sockaddr_in addr, char extra[] = "none");
unsigned long long getMilliSeconds();
int processMsg(char msg[], struct MsgPacket *packet);

#endif
