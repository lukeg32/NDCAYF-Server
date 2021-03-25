#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>

using namespace std;

#include "networkConfig.hpp"
#include "TCP.hpp"

TCPListener::TCPListener(int port) : _port(port)
{

}


/**
 * takes the obj and makes a thread for it
 * TODO implement a thread pool
 * right now its just gonna work
 * @param connectionHandler the instance of TCP
 */
void TCPListener::makeThread(TCP& connectionHandler)
{
    _numThreads++;
    thread* newThread = new thread(&TCP::run, connectionHandler);
}


/**
 * makes a listener socket
 */
int TCPListener::makeTCP()
{
    int success = 1;
    struct sockaddr_in myaddr;
    if ((_listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to make tcp listener");
        success = 0;
    }


    // so it doesn't fail on the binding
    int enable = 1;
    if (setsockopt(_listenSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }


    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(_port);

    if (bind(_listenSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        success = 0;
    }

    listen(_listenSock, 5);

    return success;
}


/**
 * accepts tcp connections
 * @return the new socket
 */
int TCPListener::getConnection()
{
    int newSock;
    struct sockaddr from;
    socklen_t lenaddr = sizeof(from);
    struct sockaddr_in fromUsefull;

    printf("Listening on port: %d\n", _port);
    newSock = accept(_listenSock, &from, &lenaddr);
    printf("Got a connection\n");

    fromUsefull = (struct sockaddr_in) fromUsefull;
    printf("From: %s::%d\n", inet_ntoa(fromUsefull.sin_addr), _port);

    return newSock;
}


/**
 * must be overriden
 * in here the specific class will take the
 * newSock from getConnection and do something
 * with it
 */
void serveClients()
{
    cout << "oh no!" << endl;
}

