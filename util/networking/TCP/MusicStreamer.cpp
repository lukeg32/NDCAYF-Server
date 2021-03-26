#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <thread>

#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

#include "networkConfig.hpp"
#include "MusicStreamer.hpp"
#include "TCP.hpp"

Music::Music(int sock) : TCP(sock)
{
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &_w);
    _barWidth = _w.ws_col - 16;
    _songsPlayed = 0;
}

string getNextSong()
{
    for (const auto& entry : filesystem::directory_iterator(SONGPATH))
        cout << entry.path() << endl;

    string a = "asdf";
    return a;
}

void Music::run(atomic<bool>* isDead)
{
    if (!validate())
        printf("oh no!");

    string curSong = getNextSong();
    struct generalTCP& inBuf = getInBuf();
    struct generalTCP& outBuf = getOutBuf();

    struct musicHeader header;
    strcpy(header.name, curSong.c_str());

    char* theData = load_wav(curSong, header.channels, header.sampleRate, header.bitsPerSample, header.dataSize, header.format);

    printf("channel: %d, sampleRate: %d, bps %d, size: %d, name: %s\n", header.channels,
        header.sampleRate, header.bitsPerSample, header.dataSize, header.name);

    long cursor = 0;
    thread progressBar(progressBarThread, std::ref(cursor), std::ref(header.dataSize), _barWidth);

    printf("Starting\n");
    bool done = false;
    while (!done)
    {
        if (getFromPoll(true) == 0)
        {
            if (inBuf.protocol == STARTSTREAM)
            {
                cout << "\nsending header\n";

                memcpy(&outBuf.data, &header, sizeof(struct musicHeader));

                sendPTL(SONGHEADER);
            }
            else if (inBuf.protocol == MORESONG)
            {
                // math, to send as much as we can, but not the size
                // of file
                int ptl = MORESONG;
                int amount = SOCKET_BUFF;
                if((cursor + SOCKET_BUFF) > header.dataSize)
                {
                    // we are at the end
                    cout << "done\n";
                    amount = header.dataSize - cursor;
                    ptl = ENDSONG;
                }


                memcpy(&outBuf.data, &theData[cursor], amount);

                cursor += amount;
                outBuf.dataSize = cursor;
                outBuf.numObjects = amount;

                sendPTL(ptl);
            }
            else if (inBuf.protocol == ENDSONG)
            {
                done = true;
            }
        }
    }

    printf("\nExit\n");
    *isDead = true;
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
