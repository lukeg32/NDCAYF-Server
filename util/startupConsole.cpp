#include "startupConsole.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

void printMessage(string messageString, const char* str,  messageType type){
    printType(type);
    printf("%s %s", messageString, str);
}

void printMessage(string messageString, const char* str, const char* str2, messageType type){
    printType(type);
    printf("%s %s %s\n", messageString, str, str2);
}

void printType(messageType type)
{

    switch (message.type){
    case ERROR:
        printf("ERROR");
        break;
    case WARNING:
        printf("WARNING");
        break;
    case SUCCESS:
        printf("SUCCESS");
        break;
    case STATUS:
        printf("> ");
        break;
    case FATALERROR:
        printf("FATALERROR");
        break;
    case NONE:
        break;
    }
}

void printMessage(string messageString, messageType type){
    printType(type);
    printf("%s\n", messageString.c_str());
}

void updateMessage(string messageString){
    printf(messageType.c_str());
}

void updateMessageType(messageType type){}

void waitToCloseError(){
    exit();
}
