#ifndef LISTENERS_H
#define LISTENERS_H
#include "TCPListener.hpp"

class UpListener: public TCPListener
{
    public:
        UpListener();
        void serveClients();
    private:
        int _temp;
};

class DnListener: public TCPListener
{
    public:
        DnListener();
        void serveClients();
    private:
        int _temp;
};

class MusicListener: public TCPListener
{
    public:
        MusicListener();
        void serveClients();
    private:
        int _temp;
};

#endif
