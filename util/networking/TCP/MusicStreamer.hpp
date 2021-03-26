#ifndef STREAMER_H
#define STREAMER_H
#include <sys/ioctl.h>
#include <atomic>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <fstream>

#include "TCP.hpp"

class Music: public TCP {
    public:
        Music(int sock);
        void run(std::atomic<bool>* isDead);
    private:
        struct winsize _w;
        int _barWidth;
        int _songsPlayed;

};

char* load_wav(const std::string& filename, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size, ALenum& format);
bool load_wav_file_header(std::ifstream& file, uint8_t& channels, int32_t& sampleRate, uint8_t& bitsPerSample, ALsizei& size);
int32_t convert_to_int(char* buffer, size_t len);

#endif
