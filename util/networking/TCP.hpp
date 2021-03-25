#ifndef TCP_H
#define TCP_H
#include <poll.h>

class TCP {
    public:
        bool waitForKey();
        bool validate();
        void sendPTL(int protocol);
        int getFromPoll(bool waitForFill);
        struct generalTCP& getInBuf();
        struct generalTCP& getOutBuf();
        bool tcpConnect();
        TCP(int sock);
        virtual void run();

    private:
        struct generalTCP makeBasicTCPPack(int ptl);

        // the in out buffers
        struct generalTCP _toSend;
        struct generalTCP _toRecieve;

        // socket stuff
        struct pollfd _pfd;
        int _theSock;

};

void progressBarThread(long& top, int& bottom, int width);
void progressBarWithBufThread(long& top, int& bottom, int width, int& numBuffs);
void drawProgress(double percent, int width);
void drawProgressWithBufCount(double percent, int width, int numBuffs);
void drawProgressRaw(double percent, int width);

#endif
