#include "../peers/peers.h"

#ifndef UTILS_H
#define UTILS_H

int bindToBest(struct addrinfo* addr_info);
void* parseHostsfile(struct Peer* peers[]);

#endif
