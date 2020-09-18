#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <poll.h>

#include "networkConfig.hpp"
#include "server.hpp"
#include "serverEdit.hpp"

using namespace std;

int listenSock, readSock;
struct generalTCP bufT;
size_t bufTSize = sizeof(struct generalTCP);

bool makeTCPSocket()
{
    bool success = true;
    struct sockaddr_in myaddr;
    if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("failed to make tcp listener");
        success = false;
    }

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORTTCP);

    if (bind(listenSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        success = false;
    }

    listen(listenSock, 5);

    return success;
}


// makes a packet and fills in most of the data
struct generalTCP makeBasicTCPPack(int ptl)
{
    struct generalTCP pack;
    strcpy(pack.name, hostnameGet());
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}

bool getData()
{
    setHostname();
    makeTCPSocket();
    struct sockaddr from;
    socklen_t lenaddr = sizeof(from);

    readSock = accept(listenSock, &from, &lenaddr);

    printf("got a connection\n");
    int len;
    char buff[2048];
    char buf2[2000];
    char filename[2048];
    char filename2[2048];
    int sizeofFile;
    bool gotKey = false;

    struct pollfd pfd;
    pfd.fd = readSock;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;


    int peek;
    bool gotFile = false;
    bool notDone = true;
    bool gettingFile = false;
    struct generalTCP *get = new struct generalTCP;
    struct generalTCP nextLine = makeBasicTCPPack(NEXTLINE);
    ofstream myfile;
    int count = 0;
    while (!gotFile)
    {

        if (poll(&pfd, 1, 1000) > 0)
        {
            peek = recv(readSock, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
            printf("%d peek\n", peek);

            // they broke the connection
            if (peek == 0)
            {
                printf("they hung up\n");
                return false;
            }

            // error
            if (peek < 0)
            {
                perror("msg error\n");
                return false;
            }

            if (!gotKey)
            {
                len = recv(readSock, buff, 2048, 0);
                if (strcmp(buff, SUPERSECRETKEY_CLIENT) == 0)
                {
                    gotKey = true;
                    printf("got the key\n");
                    send(readSock, SUPERSECRETKEY_SERVER, sizeof(SUPERSECRETKEY_SERVER), 0);
                }
            }
            else
            {
                len = recv(readSock, &bufT, bufTSize, 0);

                printf("%d bytes: ptl %d\n", len, bufT.protocol);
                //printf("%d == %d\n", bufT.protocol, SENDINGFILE);
                if ((bufT.protocol == SENDINGFILE) && !gettingFile)
                {
                    struct aboutFile information;
                    memcpy(&information, &bufT.data, sizeof(struct aboutFile));
                    printf("name %s\n", information.name);
                    printf("type %d\n", information.type);
                    string dir;

                    if (information.type == MAP)
                    {
                        dir = "gamedata/maps/";
                    }
                    else if (information.type == GAMEMODE)
                    {
                        dir = "gamedata/gameModes/";
                    }

                    dir += information.name;
                    gettingFile = true;
                    myfile.open(dir);
                    printf("Ready to recieve file\n");
                }
                else if (bufT.protocol == ENDDOWNLOAD)
                {
                    gettingFile = false;
                    gotFile = true;
                    printf("we have finished\n");
                    myfile.close();
                    //close the file
                }
                else if ((bufT.protocol == SENDINGFILE) && gettingFile)
                {
                    printf("getting line\n");
                    struct lines theline;
                    memcpy(&theline, &bufT.data, sizeof(struct lines));
                    //printf("%d::::::[%s]", count, theline.aLine);
                    myfile << theline.aLine;
                    count++;
                    //printf("%d\n", nextLine.protocol);
                    send(readSock, (const void*)&nextLine, bufTSize, 0);

                    /*
                        printf("%s\n", filename);
                        while (notDone)
                        {
                            printf("get next\n");
                            len = recv(readSock, buf2, 2000, 0);
                            if (strcmp(buf2, "") == 0)
                            {
                                //printf("bad%s", buf2);
                            }
                            else if (strcmp(buf2, "next?") == 0)
                            {
                                send(readSock, "next", sizeof("next"), 0);
                            }
                            else
                            {

                                printf("%d::::::%s", count, buf2);
                                count++;
                                if (strcmp(buf2, "end") == 0)
                                {
                                    notDone = false;
                                    printf("exit successfully\n");
                                }
                                else
                                {
                                    myfile << buf2;
                                    buf2[0] = '\0';
                                    send(readSock, "next", sizeof("next"), 0);
                                }
                            }
                        }

                        myfile.close();

                        printf("Saved to %s\n", filename2);
                        gotFile = true;
                        //buff[0] = '\0';
                    }
                    else
                    {
                        //printf("%d bytes: %s\n", len, buf);
                    }
                    */
                }
            }
        }
    }
    printf("we escaped\n");
}


