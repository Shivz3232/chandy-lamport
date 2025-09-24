#include <poll.h>

#include "../peers/peers.h"

#ifndef UTILS_H
#define UTILS_H

int bindToBest(struct addrinfo* addr_info);
void* parseHostsfile(struct Peer* peers[]);
char* getNameInfo(struct sockaddr*, socklen_t*);
void parseArgs(int, char* const*);

struct pollfd* setupPollFds(struct Peer* peers[]);
void* freePollFds(struct pollfd*);

#endif
