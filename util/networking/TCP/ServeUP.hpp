#ifndef FILEDN_H
#define FILEDN_H
#include <iostream>
#include <atomic>

#include "TCP.hpp"

class ServeUP: public TCP {
    public:
        ServeUP(int sock);
        void run(std::atomic<bool>* isDead);
    private:
        bool getHeader();
        std::string getDir(int type, char* fileName);

        struct aboutFile _fileInfo;


};

#endif
