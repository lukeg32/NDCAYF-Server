#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>

using namespace std;

#include "../networkConfig.hpp"
#include "TCP.hpp"
#include "TCPListener.hpp"
#include "Listeners.hpp"
#include "ServeUP.hpp"
#include "ServeDN.hpp"
#include "MusicStreamer.hpp"

UpListener::UpListener(): TCPListener(PORTTCP_UPLOAD)
{

}

void UpListener::serveClients()
{
    ServeUP* newUp;
    while (true)
    {
        int sock = getConnection();
        newUp = new ServeUP(sock);
        makeThread(newUp);
    }
}

DnListener::DnListener(): TCPListener(PORTTCP_DOWNLOAD)
{

}

void DnListener::serveClients()
{
    ServeDN* newUp;
    while (true)
    {
        int sock = getConnection();
        newUp = new ServeDN(sock);
        makeThread(newUp);
    }
}


MusicListener::MusicListener(): TCPListener(PORTTCP_MUSIC)
{

}

void MusicListener::serveClients()
{
    Music* newUp;
    while (true)
    {
        int sock = getConnection();
        newUp = new Music(sock);
        makeThread(newUp);
    }
}
