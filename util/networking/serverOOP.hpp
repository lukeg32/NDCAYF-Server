#ifndef SERVEROOP_H
#define SERVEROOP_H
#include <string>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>


bool load_wav_file_header(std::ifstream& file, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size);
int32_t convert_to_int(char* buffer, size_t len);
char* load_wav(const std::string& filename, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size, ALenum& format);

struct generalTCP
{
    char name[10];
    int protocol;
    int numObjects;
    size_t dataSize;
    struct timeval time;
    char data[60000];
};

struct musicHeader
{
    uint8_t channels;
    int32_t sampleRate;
    uint8_t bitsPerSample;
    ALsizei dataSize;
    ALenum format;
};

struct aboutFile
{
    char name[30];
    int type;
    long lines;
};

struct lines
{
    char aLine[200];
};


class TCP {

  public:
    TCP(int port);
  private:
    int sockTCP, listenSock;
    struct sockaddr_in tcpServer;
    socklen_t addrlen;
    size_t bufTSize;

    int makeTCP(int port);
    bool tcpConnect(char ip[], int type);
    struct generalTCP makeBasicTCPPack(int ptl);
    int getLines(std::string file);
    void drawProgress(double percent, int width);
    void fileSendInit();
    int getFromPoll(bool waitForFill);
    bool waitForKey();
    void sendFileInfo(std::ifstream &myfile);
    bool sendNextLine(std::ifstream &myfile);
    bool fileSendMain();
    bool musicMain();
    void musicInit();
    bool musicGet();
    void sendPTL(int protocol);


    // packet we send
    struct generalTCP toSend;
    // packet we get
    struct generalTCP bufT;

    // info about the file
    struct aboutFile fileInfo;
    std::string fileName;
    long totalLine;

    struct lines bunch;

    // measure the length in time
    struct timeval before;
    struct timeval after;
    struct timeval diff;

    // for poll
    struct pollfd pfd;

    // for the progress bar
    struct winsize w;
    int barWidth;

    // figures
    int charsRead;
    long charsProcessed;

    // for tcp loop
    long count;
    bool done;
    int len;


    // file send specific
    bool sendingFile;
    bool waitingForTime;

    // for music streaming


};

#endif
