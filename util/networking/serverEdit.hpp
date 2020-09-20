#ifndef SERVEREDIT_H
#define SERVEREDIT_H
#include <glm/glm.hpp>
#include <string>


bool makeTCPSocket();
bool getData();
struct generalTCP makeBasicTCPPack(int ptl);
void drawProgress(double percent, int width);

struct generalTCP
{
    char name[10];
    int protocol;
    int numObjects;
    struct timeval time;
    char data[2000];
};

struct aboutFile
{
    char name[30];
    int type;
    long lines;
};


struct lines
{
    char aLine[200];
};

#endif
