
class TCPListener
{
    public:
        TCPListener::TCPListener(int port);
        int makeTCP();
        void makeThread(TCP& connectionHandler);
        int getConnection();
        virtual void serveClients();
    private:
        int _listenerSock;
}
