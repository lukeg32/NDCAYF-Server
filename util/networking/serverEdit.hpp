#ifndef SERVEREDIT_H
#define SERVEREDIT_H
#include <glm/glm.hpp>


bool makeTCPSocket();
bool getData();
struct generalTCP makeBasicTCPPack(int ptl);

struct generalTCP
{
    char name[10];
    unsigned short int protocol;
    unsigned short int numObjects;
    struct timeval time;
    char data[1000];
};

struct aboutFile
{
    char name[30];
    int type;
};


struct lines
{
    char aLine[200];
};

#endif
