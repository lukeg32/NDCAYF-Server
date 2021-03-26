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
#include "FileGet.hpp"
#include "FileUpload.hpp"
#include "MusicStreamer.hpp"

UpListener::UpListener(): TCPListener(PORTTCP_UPLOAD)
{

}

void UpListener::serveClients()
{
    Upload* newUp;
    while (true)
    {
        int sock = getConnection();
        newUp = new Upload(sock);
        makeThread(newUp);
    }
}

DnListener::DnListener(): TCPListener(PORTTCP_DOWNLOAD)
{

}

void DnListener::serveClients()
{
    FileGet* newUp;
    while (true)
    {
        int sock = getConnection();
        newUp = new FileGet(sock);
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
