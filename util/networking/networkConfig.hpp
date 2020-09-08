#ifndef NETWORK_H
#define NETWORK_H
/*
 * 
 * store all network constants here,
 * or elsewhere once integrated into
 * the main code, ikc
 *
 */

#define PORT  12345
#define SUPERSECRETKEY_SERVER "ndcayfser"
#define SUPERSECRETKEY_CLIENT "ndcayfcli"

// protocols
/*
#define PING "ping"
#define PONG "pong"
#define CONNECT "connect"
#define MOVE "imove"
*/

// helpers
#define PONG 1
#define PING 2
#define STATE 3
#define SPEED 4
#define CONNECT 5
#define MOVE 6
#define DUMP 7

#define MAXSERVERS 5
#define MAXPLAYERS 20


// not sure if this value is important enough
#define BUFSIZE 2048

// a universal key map that all clients will understand 
#define UNI_FD "w"
#define UNI_BK "s"
#define UNI_RT "d"
#define UNI_LT "a"

#endif
