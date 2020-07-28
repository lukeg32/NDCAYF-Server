/*
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, ikc
 *
 */

#define PORT  12345
#define SUPERSECRETKEY_SERVER "ndcayfserver"
#define SUPERSECRETKEY_CLIENT "ndcayfclient"

// protocols
#define PING "ping"
#define PONG "pong"
#define CONNECT "connect"

// helpers
#define SENDPONG 1
#define SENDPING 2
#define ISSTATE 3
#define SPEED 4
#define CONNECTME 5

#define MAXSERVERS 5
#define MAXPLAYERS 20

// not sure if this value is important enough
#define BUFSIZE 2048
