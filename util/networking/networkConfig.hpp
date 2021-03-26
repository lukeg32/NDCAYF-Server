#ifndef NETWORK_H
#define NETWORK_H
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>


#define PORT  12345
#define PORTTCP_UPLOAD  54321
#define PORTTCP_DOWNLOAD  54322
#define PORTTCP_MUSIC  54323
#define PORTTCP_VOICE  54324
#define SUPERSECRETKEY_SERVER "ndcayfser"
#define SUPERSECRETKEY_CLIENT "ndcayfcli"
#define PONG 1
#define PING 2
#define STATE 3
#define SPEED 4
#define CONNECT 5
#define MOVE 6
#define DUMP 7
#define INFO 8
#define SENDINGFILE 9
#define NEXTLINE 10
#define ENDDOWNLOAD 11
#define STARTSTREAM 12
#define MORESONG 13
#define SONGHEADER 14
#define ENDSONG 15
#define SENDINGFILEHEADER 16

#define UPLOADFILE 1
#define DOWNLOADFILE 2
#define STREAMMUSIC 3
#define STREAMVOICE 4

#define SONGPATH "songs"
#define MUSIC_BUFFERS 32
#define MIN_MUSIC_BUFFERS 2
#define START_MUSIC_BUFFERS 16
#define BUFFER_SIZE 60000
#define SOCKET_BUFF 60000

#define POLLOK 0
#define POLLHUNGUP 1
#define POLLBAD -1

#define MAP 1
#define GAMEMODE 2
#define OBJ 3
#define TEXTURE 4

#define MAXSERVERS 5
#define MAXPLAYERS 20

#define MAXMOVE 20


// not sure if this value is important enough
#define BUFSIZE 2048

struct generalPack
{
    char key[10];
    char name[10];
    int protocol;
    int numObjects;
    struct timeval time;
    char data[1000];
};


struct generalTCP
{
    char name[10];
    int protocol;
    int numObjects;
    size_t dataSize;
    struct timeval time;
    char data[SOCKET_BUFF];
};

struct musicHeader
{
    uint8_t channels;
    int32_t sampleRate;
    uint8_t bitsPerSample;
    ALsizei dataSize;
    ALenum format;
    char name[30];
};

struct aboutFile
{
    char name[30];
    int type;
    long size;
};

#endif
