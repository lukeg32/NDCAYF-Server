#ifndef NETWORK_H
#define NETWORK_H
/*
 * 
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, idc
 *
 */

#define PORT  12345
#define PORTTCP  54321
#define SUPERSECRETKEY_SERVER "ndcayfser"
#define SUPERSECRETKEY_CLIENT "ndcayfcli"

// protocols
/*
#define PING "ping"
#define PONG "pong"
#define CONNECT "connect"
#define MOVE "imove"
*/

// helpers
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

#endif
