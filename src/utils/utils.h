#include <poll.h>

#include "../peers/peers.h"

#ifndef UTILS_H
#define UTILS_H

char* getNameInfo(struct sockaddr*, socklen_t*);
int bindToBest(struct addrinfo* addr_info);
char* createPacket(char*);
int sendAll(int, char*, int);
char* receivePacket(int);
int receiveAll(int, char*, int);

void* parseHostsfile(struct Peer* peers[]);
void parseArgs(int, char* const*);

struct pollfd* setupPollFds(struct Peer* peers[]);
void* freePollFds(struct pollfd*);

#endif
