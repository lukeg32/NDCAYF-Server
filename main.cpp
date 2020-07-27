#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <btBulletDynamicsCommon.h>

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


int main()
{
    int sock;
    char buf[BUFSIZE];
    struct sockaddr_in fromAddr;
    int addrlen;
    int type;

    if ((sock = makeSocket()) < 0)
    {
        perror("Failed to make a socket\n");
        return -1;
    }


    while(true){
        fromAddr = recieve(buf, sock);

        type = processMsg(buf);

        sendMsg(type, sock, fromAddr);
    }

    close(sock);

    return 0;
}
