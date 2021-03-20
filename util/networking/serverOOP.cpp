#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <glm/glm.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <string>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bit>

#include <thread>

using namespace std;

#include "networkConfig.hpp"
#include "server.hpp"
#include "serverOOP.hpp"

/**
 * makes a tcp socket
 * @return success or not
 */
int TCP::makeTCP(int port)
{
    int success = 1;
    struct sockaddr_in myaddr;
    if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("failed to make tcp listener");
        success = 0;
    }


    // so it doesn't fail on the binding
    int enable = 1;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    /*
    // to make it nonblocking, not sure what for?
    int on = 1;
    if (ioctl(listenSock, FIONBIO, (char *)&on) < 0)
    {
        perror("ioctl() failed");
        close(listenSock);
        exit(-1);
    }
    */


    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(port);

    if (bind(listenSock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Bind failed");
        success = 0;
    }

    listen(listenSock, 5);

    return success;
}

/**
 * the constructor for the tcp thing
 * @param ip server ip
 * @param type what we are doing with the tcp socket
 * @param file filename for uploading
 * TODO separate the construction from the running of the program
 */
TCP::TCP(int port)
{
    // amke the socket
    _port = port;
    makeTCP(port);
    setHostname();

    // try and connnect to the server
    /*
    if (!tcpConnect(ip, type))
    {
        printf("tcpError!\n");
        exit(EXIT_FAILURE);
    }
    */


}


void TCP::runSocket()
{
    struct sockaddr from;
    socklen_t lenaddr = sizeof(from);

    printf("listening on port: %d\n", _port);
    sockTCP = accept(listenSock, &from, &lenaddr);
    printf("got a connection\n");


    // universal stuff
    gettimeofday(&before, NULL);

    // for the poll
    pfd.fd = sockTCP;
    pfd.events = POLLIN | POLLHUP;
    pfd.revents = 0;

    charsRead = 0;
    charsProcessed = 0;

    count = 0;
    done = false;

    bufTSize = sizeof(struct generalTCP);

    // verify they are server
    if (!TCP::waitForKey())
    {
        perror("Failed to find key!\n");
        exit(EXIT_FAILURE);
    }

    // type specific set up and run
    if (_port == PORTTCP_UPLOAD)
    {
        printf("getting file!\n");
        fileRecieveMain();
        //exit(EXIT_FAILURE);
    }
    else if (_port == PORTTCP_DOWNLOAD)
    {
        fileSendInit();
        printf("sending file!\n");
        fileSendMain();
        exit(EXIT_FAILURE);
    }
    else if (_port == PORTTCP_MUSIC)
    {
        musicInit();
        printf("Streaming music\n");
        musicMain();
    }
    else if (_port == PORTTCP_VOICE)
    {
        printf("Voice channel\n");
        exit(EXIT_FAILURE);
    }
}

ofstream* TCP::fileGetInit()
{
    ofstream* myfile = new ofstream();
    bool gotIt = false;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    barWidth = w.ws_col - 8;

    while (!gotIt)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufT.protocol == SENDINGFILEHEADER)
            {
                memcpy(&information, &bufT.data, sizeof(struct aboutFile));
                printf("name %s\n", information.name);
                printf("type %d\n", information.type);
                printf("size %ld\n", information.lines);
                string dir;

                if (information.type == MAP)
                {
                    dir = "gamedata/maps/";
                }
                else if (information.type == GAMEMODE)
                {
                    dir = "gamedata/gameModes/";
                }

                dir += information.name;
                printf("%s\n", dir.c_str());
                myfile->open(dir);
                printf("Ready to recieve file\n");
                gotIt = true;
            }
        }
    }

    return myfile;
}

void TCP::fileRecieveMain()
{
    ofstream* output = fileGetInit();
    bool running = true;
    int count = 0;

    printf("Starting\n");
    while (running)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufT.protocol == SENDINGFILE)
            {
                count += bufT.numObjects;
                drawProgress((float)count / (float)information.lines, barWidth);

                *output << bufT.data;
                sendPTL(NEXTLINE);
            }
            if (bufT.protocol == ENDDOWNLOAD)
            {
                count += bufT.numObjects;
                drawProgress(1.0f, barWidth);

                // trim extra data off the end
                if (bufT.numObjects != sizeof(bufT.data))
                    bufT.data[bufT.numObjects] = '\0';

                *output << bufT.data;

                // confirm exit
                printf("\nWe have finished added %d bytes\n", count);
                output->close();
                sendPTL(ENDDOWNLOAD);
                running = false;
            }

        }
    }

    printf("we escaped\n");
}



/**
 * inits file send specific vars and such
 * @param fileName the name of the file we are to open
 */
void TCP::fileSendInit()
{
    toSend = makeBasicTCPPack(SENDINGFILE);
    string dir = "obj/objects/";
    printf("%s..%s\n", dir.c_str(), fileName.c_str());
    fileName = dir + fileName;
    totalLine = getLines(fileName);

    ifstream in_file(fileName, ios::binary);
    in_file.seekg(0, ios::end);
    int fileSize = in_file.tellg();

    // file stuff
    fileInfo.lines = fileSize;
    strcpy(fileInfo.name, fileName.c_str());
    fileInfo.type = MAP;

    printf("name %s lines %ld\n", fileInfo.name, totalLine);


    // for the progress bar
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    barWidth = w.ws_col - 8;

    sendingFile = false;
    waitingForTime = false;
}


/**
 * stuff thats needed
 * @return
 */
void TCP::musicInit()
{
    toSend = makeBasicTCPPack(SENDINGFILE);

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    barWidth = w.ws_col - 8;
}

/**
 * waits for poll to trigger, then error checks, and sets the buf packet
 * 0 for fine, 1 for hung up and -1 for bad
 * @param waitForFill to wait for the buffer to fill up or not
 * @return int of what happened
 */
int TCP::getFromPoll(bool waitForFill)
{
    int peek;
    if (poll(&pfd, 1, 1000) > 0)
    {
        peek = recv(sockTCP, &bufT, bufTSize, MSG_PEEK | MSG_DONTWAIT);
        //printf("%d peek\n", peek);

        // they broke the connection
        if (peek == 0)
        {
            printf("they hung up\n");
            exit(EXIT_FAILURE);
            return POLLHUNGUP;
        }

        // error
        if (peek < 0)
        {
            perror("msg error");
            return POLLBAD;
        }

        if (waitForFill)
        {
            if (peek < sizeof(bufT))
            {
                //printf("waiting for it all\n");
                bufT.protocol = -1;
            }
            else
            {
                len = recv(sockTCP, &bufT, bufTSize, 0);
            }
        }
    }

    return POLLOK;
}

/**
 * waites for server to send the key
 * @return if we got one
 * TODO make this only try x amount of times before failing
 */
bool TCP::waitForKey()
{
    bool waiting = true;
    bool success = false;
    char buff[2048];
    printf("waiting for key\n");
    while (waiting)
    {
        if (getFromPoll(false) == 0)
        {
            len = recv(sockTCP, buff, 2048, 0);
            if (strcmp(buff, SUPERSECRETKEY_CLIENT) == 0)
            {
                success = true;
                waiting = false;
                printf("Got the key\n");
                send(sockTCP, SUPERSECRETKEY_SERVER, sizeof(SUPERSECRETKEY_SERVER), 0);
            }
        }

    }

    len = 0;
    return success;
}

/**
 * opens the file and gets the proper info
 * then makes aboutfile struct and sends it to server
 * @param myfile the file stream we are working with
 */
void TCP::sendFileInfo(ifstream &myfile)
{
    memcpy(&toSend.data, &fileInfo, sizeof(aboutFile));
    send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);
    printf("Sent the info about the map\n");
    myfile.open(fileName, ios::out);

    if (!myfile.is_open())
    {
        perror("Error opening the file:");
    }
}

/**
 * sends the next line
 * if the buffer fills up then nothing special
 * but if not then we send with ENDDOWNLOAD ptl
 * @return
 */
bool TCP::sendNextLine(ifstream &myfile)
{
    // fillup the buffer with info from the file
    if (myfile.read(toSend.data, sizeof(toSend.data)))
    {
        // it filled up correctly
        charsRead = myfile.gcount(); // should be 1000
        count += charsRead;
        toSend.numObjects = charsRead;
        drawProgress((double)count / (double)fileInfo.lines, barWidth);


        // send
        if (send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0) < 0)
        {
            perror("send wackiness");
        }
    }
    else
    {
        // the buffer did not fully fill, at the end

        // should be <1000
        charsRead = myfile.gcount();
        count += charsRead;
        toSend.numObjects = charsRead;

        drawProgress(1.0f, barWidth);

        //printf(" %lu%03lu.%03lu milliseconds \n", diff.tv_sec, diff.tv_usec / 1000 , diff.tv_usec % 1000);

        toSend.protocol = ENDDOWNLOAD;
        send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0);

        gettimeofday(&after, NULL);
        timersub(&after, &before, &diff);

        printf("\nSent all %ld byes in", count);
        printf(" %lu.%06lu seconds \n", diff.tv_sec, diff.tv_usec);

        myfile.close();

        waitingForTime = true;
        sendingFile = false;
    }
}


/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return honestly not necessary
 */
bool TCP::fileSendMain()
{
    ifstream myfile;
    bool done = false;

    // prep file and send
    sendFileInfo(myfile);

    bool first = true;
    sendingFile = true;

    printf("starting\n");
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            if (first)
            {
                sendNextLine(myfile);
                first = false;
            }
            // check that we are sending a file and that they want the next line
            if (sendingFile && (bufT.protocol == NEXTLINE))
            {
                sendNextLine(myfile);
            }
            else if (waitingForTime && (bufT.protocol == ENDDOWNLOAD))
            {
                timersub(&bufT.time, &before, &diff);
                printf("Server took %lu.%06lu seconds to finish\n", diff.tv_sec, diff.tv_usec);

                done = true;
                printf("exit\n");
                close(sockTCP);
            }
        }
    }

    return true;
}


/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return honestly not necessary
 */
bool TCP::musicGet()
{
    ofstream myfile;
    string thing("Fine.wav");
    myfile.open(thing, ios::binary);
    bool done = false;
    struct musicHeader header;

    bool first = true;
    bool firstSong = true;
    sendingFile = true;
    bool requested = false;
    send(sockTCP, SUPERSECRETKEY_SERVER, sizeof(SUPERSECRETKEY_SERVER), 0);

    printf("starting\n");
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            {
                // make audio player
                if (firstSong)
                {
                    //bufs.emplace();

                    myfile.write(bufT.data, bufT.numObjects);

                    firstSong = false;
                    memcpy(&header, &bufT.data, sizeof(struct musicHeader));
                    printf("channels %d, sampleRate %d, bps %d, size %d\n", header.channels, header.sampleRate, header.bitsPerSample, header.dataSize);
                }
                else
                {
                    // add buf to end
                    //bufs.emplace();
                }
                // do stuff
            }

            // check that we are sending a file and that they want the next line
            if (bufT.protocol == MORESONG)
            {
                //bufs.back().add(bufT.data, bufT.numObjects);

                /*
                if (bufs.back().needMore())
                {
                    sendPTL(MORESONG);
                    requested = true;
                }
                else
                {
                    // we will request more later
                    requested = false;
                }
                */

            }
            else if (bufT.protocol == ENDSONG)
            {
                // probs don't close socket
                //bufs.back().add(bufT.data, bufT.numObjects);
                //bufs.back().noMore();
                sendPTL(ENDSONG);
                done = true;
            }
        }
        // do music code

        // request more if we haven't,
        // once song finishes we won't recieve anymore for this song so this wont get called
        /*
        if (bufs.front().needMore() && !requested)
        {
            sendPTL(MORESONG);
            requested = true;
        }

        // and we need more
        if (bufs.front().isNextReady())
        {
            int size = bufs.front().getSize();
            char* theData = bufs.front().getData();
            myfile.write(theData, size);
        }
        */

        /*
        if (playing)
        {
            player.play();
        }
        else
        {
            // remove the first element, so the next in line becomes cur
            bufs.pop();

            // 
            AudioPlayer newPlayer(header, *bufs.front());
            player = newPlayer;
        }
        */
    }

    printf("exit\n");
    close(sockTCP);
    return true;
}


/**
 * main loop for sending files
 * assumes the key has been got
 * preps file and sends the first line before entering
 * @return honestly not necessary
 */
bool TCP::musicMain()
{
    ifstream myfile;
    string thing("songs/bee.wav");
    struct musicHeader header;
    char* theData = load_wav(thing, header.channels, header.sampleRate, header.bitsPerSample, header.dataSize, header.format);
    printf("channel: %d, sampleRate: %d, bps %d, size: %d\n", header.channels,
    header.sampleRate, header.bitsPerSample, header.dataSize);
    bool done = false;

    myfile.open(thing, ios::binary);
    size_t sizeHeader = 44;
    char *headerRaw = new char[sizeHeader];
    myfile.read(headerRaw, sizeHeader);
    myfile.close();

    long cursor = 0;

    bool firstSong = true;
    sendingFile = true;
    bool requested = false;
    printf("sending key\n");
    std::thread progressBar(progressBarThread, std::ref(cursor), std::ref(header.dataSize), barWidth);

    printf("starting\n");
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            if (bufT.protocol == STARTSTREAM)
            {
                std::cout << "\nsending header\n";

                toSend.protocol = SONGHEADER;
                toSend.dataSize = sizeHeader;
                memcpy(&toSend.data, &header, sizeof(struct musicHeader));
                memcpy(&toSend.data[sizeof(struct musicHeader)], headerRaw, sizeHeader);
                /*
                std::cout << "size: " << sizeof(toSend) << std::endl;
                std::cout << "name: " << toSend.name << std::endl;
                std::cout << "ptl: " << toSend.protocol << std::endl;
                std::cout << "numObjects: " << toSend.numObjects << std::endl;
                std::cout << "dataSize: " << toSend.dataSize << std::endl;
                std::cout << "data: " << toSend.data << std::endl;



                char *string_ptr = (char *)&toSend;
                size_t kk = sizeof(struct generalTCP);
                int k = 0;

                while(kk--)
                {
                    if (k == 16)
                    {
                        k = 0;
                        printf("\n");
                    }
                    printf("%hhx ", *string_ptr++);
                    k++;
                }
                */

                send(sockTCP, (const void*)&toSend, sizeof(toSend), 0);
            }
            else if (bufT.protocol == MORESONG)
            {
                // math, to send as much as we can, but not the size
                // of file
                toSend.protocol = MORESONG;
                int amount = SOCKET_BUFF;
                if((cursor + SOCKET_BUFF) > header.dataSize)
                {
                    // we are at the end
                    std::cout << "done\n";
                    amount = header.dataSize - cursor;
                    toSend.protocol = ENDSONG;
                }


                memcpy(&toSend.data, &theData[cursor], amount);

                cursor += amount;
                toSend.dataSize = cursor;
                toSend.numObjects = amount;

                send(sockTCP, (const void*)&toSend, sizeof(toSend), 0);
            }
            else if (bufT.protocol == ENDSONG)
            {
                std::cout << "ending" << "\r";
                std::cout.flush();
                // open new file, get info
                // create new header, send it
                done = true;
            }
        }
    }

    printf("exit\n");
    close(sockTCP);
    return true;
}

void TCP::sendPTL(int protocol)
{
    toSend.protocol = protocol;

    // send
    if (send(sockTCP, (const void*)&toSend, sizeof(struct generalTCP), 0) < 0)
    {
        perror("send wackiness");
    }
}


/**
 * makes a tcp struct with the hostname and ptl
 * and returns, for ease of use
 * @param ptl what protocol this packet is
 * @return a struct with its header filled, mostly
 */
struct generalTCP TCP::makeBasicTCPPack(int ptl)
{
    struct generalTCP pack;
    strcpy(pack.name, hostnameGet());
    pack.protocol = ptl;
    pack.numObjects = 1;

    return pack;
}


/**
 * gets the lines of a file by brute force
 * @param file file we are opening
 * @return the number of lines in file
 */
int TCP::getLines(string file)
{
    ifstream fileToCount;
    int count = 0;
    string line;

    fileToCount.open(file);

    while (getline(fileToCount, line))
    {
        count++;
    }

    fileToCount.close();

    return count;
}

void progressBarThread(long& top, int& bottom, int width)
{
    while (top != bottom)
    {
        drawProgress((float) top / (float) bottom, width);
    }
}

/**
 * makes the pretty progress bar
 * stolen from stack overflow btw
 * @param percent how far along we want this
 * @param width the max width
 */
void drawProgress(double percent, int width)
{
    cout << "[";
    int pos = width * percent;
    for (int i = 0; i < width; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(percent * 100.0) << " %\r";
    cout.flush();
}



int32_t convert_to_int(char* buffer, size_t len)
{
    int32_t a = 0;
    if(endian::native == endian::little)
        memcpy(&a, buffer, len);
    else
        for(size_t i = 0; i < len; ++i)
            reinterpret_cast<char*>(&a)[3 - i] = buffer[i];
    return a;
}

//http://soundfile.sapp.org/doc/WaveFormat/
//read this and this function makes sense
bool load_wav_file_header(ifstream& file, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size)
{
    char buffer[4];
    if(!file.is_open())
        return false;

    // the RIFF
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read RIFF" << endl;
        return false;
    }


    if(strncmp(buffer, "RIFF", 4) != 0)
    {
        cerr << "ERROR: file is not a valid WAVE file (header doesn't begin with RIFF)" << endl;
        return false;
    }

    // the size of the file
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read size of file" << endl;
        return false;
    }

    // the WAVE
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read WAVE" << endl;
        return false;
    }
    if(strncmp(buffer, "WAVE", 4) != 0)
    {
        cerr << "ERROR: file is not a valid WAVE file (header doesn't contain WAVE)" << endl;
        return false;
    }

    // "fmt/0"
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read fmt/0" << endl;
        return false;
    }

    // this is always 16, the size of the fmt data chunk
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read the 16" << endl;
        return false;
    }

    // PCM should be 1?
    if(!file.read(buffer, 2))
    {
        cerr << "ERROR: could not read PCM" << endl;
        return false;
    }

    // the number of channels
    if(!file.read(buffer, 2))
    {
        cerr << "ERROR: could not read number of channels" << endl;
        return false;
    }
    channels = convert_to_int(buffer, 2);

    // sample rate
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read sample rate" << endl;
        return false;
    }
    sampleRate = convert_to_int(buffer, 4);

    // (sampleRate * bitsPerSample * channels) / 8
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read (sampleRate * bitsPerSample * channels) / 8" << endl;
        return false;
    }

    // ?? dafaq
    if(!file.read(buffer, 2))
    {
        cerr << "ERROR: could not read dafaq" << endl;
        return false;
    }

    // bitsPerSample
    if(!file.read(buffer, 2))
    {
        cerr << "ERROR: could not read bits per sample" << endl;
        return false;
    }
    bitsPerSample = convert_to_int(buffer, 2);
    //cerr << bitsPerSample << endl;

    // data chunk header "data"
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read data chunk header" << endl;
        return false;
    }

    //cerr << buffer << endl;
    //file.read(buffer, 
    if(strncmp(buffer, "data", 4) != 0)
    {
        cerr << "ERROR: file is not a valid WAVE file (doesn't have 'data' tag)" << endl;
        return false;
    }

    // size of data
    if(!file.read(buffer, 4))
    {
        cerr << "ERROR: could not read data size" << endl;
        return false;
    }
    size = convert_to_int(buffer, 4);

    /* cannot be at the end of file */
    if(file.eof())
    {
        cerr << "ERROR: reached EOF on the file" << endl;
        return false;
    }
    if(file.fail())
    {
        cerr << "ERROR: fail state set on the file" << endl;
        return false;
    }

    return true;
}

char* load_wav(const string& filename, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size, ALenum& format)
{
    ifstream in(filename, ios::binary);
    if(!in.is_open())
    {
        cerr << "ERROR: Could not open \"" << filename << "\"" << endl;
        return nullptr;
    }
    if(!load_wav_file_header(in, channels, sampleRate, bitsPerSample, size))
    {
        cerr << "ERROR: Could not load wav header of \"" << filename << "\"" << endl;
        return nullptr;
    }

    char* data = new char[size];

    in.read(data, size);

    if(channels == 1 && bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if(channels == 1 && bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if(channels == 2 && bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if(channels == 2 && bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr
            << "ERROR: unrecognised wave format: "
            << channels << " channels, "
            << bitsPerSample << " bps" << std::endl;
        exit(EXIT_FAILURE);
    }

    return data;
}
