#ifndef LISTENERS_H
#define LISTENERS_H

class UpListener: public TCPListener
{
    public:
        UpListener(int sock);
        void serveClients();
    private:
        int _temp;
}

class DnListener: public TCPListener
{
    public:
        DnListener(int sock);
        void serveClients();
    private:
        int _temp;
}

class MusicListener: public TCPListener
{
    public:
        MusicListener(int sock);
        void serveClients();
    private:
        int _temp;
}

#endif
