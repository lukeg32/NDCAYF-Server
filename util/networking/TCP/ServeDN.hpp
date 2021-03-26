#ifndef FILEUP_H
#define FILEUP_H
#include <iostream>
#include <atomic>

#include "TCP.hpp"

class ServeDN: public TCP {
    public:
        ServeDN(int sock);
        void run(std::atomic<bool>* isDead);
    private:
        void makeHeader(std::string file);

        struct aboutFile _fileInfo;
        std::string _path;


};

#endif
