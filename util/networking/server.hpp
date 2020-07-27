
int runServer();
struct sockaddr_in recieve(char buf[], int inSock);
int makeSocket();
void composeMsg(char msg[], char protocol[], char extra[] = "none");
int sendMsg(int type, int sock, struct sockaddr_in addr);
int processMsg(char msg[]);
