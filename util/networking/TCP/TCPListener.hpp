#ifndef TCPListener_H
#define TCPListener_H
#include <atomic>
#include <thread>
#include <vector>

#include "TCP.hpp"

class TCPListener
{
    public:
        TCPListener(int port);
        int makeTCP();
        void makeThread(TCP* connectionHandler);
        int getConnection();
        virtual void serveClients() = 0;

        static std::vector<std::thread*> pool;
        static std::vector<std::atomic<bool>*> isDead;
    private:
        int _listenerSock;
        int _port;
};

#endif
