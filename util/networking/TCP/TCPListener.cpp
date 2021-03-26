#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <vector>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <mutex>

using namespace std;
mutex mu;

#include "../networkConfig.hpp"
#include "TCPListener.hpp"
#include "TCP.hpp"

vector<std::thread*> TCPListener::pool;
vector<std::atomic<bool>*> TCPListener::isDead;

TCPListener::TCPListener(int port) : _port(port)
{
    makeTCP();
}


/**
 * takes the obj and makes a thread for it
 * has to lock it with mutex
 * @param connectionHandler the instance of TCP
 */
void TCPListener::makeThread(TCP* connectionHandler)
{
    atomic<bool>* dead = new atomic<bool>;
    *dead = false;
    thread* newThread = new thread(&TCP::run, connectionHandler, dead);

    mu.lock();
    pool.push_back(newThread);
    isDead.push_back(dead);
    mu.unlock();
}


/**
 * makes a listener socket
 */
int TCPListener::makeTCP()
{
    int success = 1;
    struct sockaddr_in myaddr;
    if ((_listenerSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to make tcp listener");
        success = 0;
    }


    // so it doesn't fail on the binding
    int enable = 1;
    if (setsockopt(_listenerSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }


    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(_port);

    if (bind(_listenerSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        success = 0;
    }

    listen(_listenerSock, 5);

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
    newSock = accept(_listenerSock, &from, &lenaddr);
    printf("Got a connection\n");

    fromUsefull = (struct sockaddr_in) fromUsefull;
    printf("From: %s::%d\n", inet_ntoa(fromUsefull.sin_addr), _port);

    return newSock;
}
