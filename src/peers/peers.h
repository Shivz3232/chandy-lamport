#include <netdb.h>

#include "../config/config.h"

#ifndef PEERS_H
#define PEERS_H

extern int numPeers;
extern int connectedPeers;

struct Peer {
  int id;
  char* name;
  int socket_fd;
  struct addrinfo* addr_info;
  struct addrinfo* chosen_addr_info;
  int connected;
};

void* populatePeerInfo(struct Peer* peer);
void* markPeerAsConnected(struct Peer* peers[], struct sockaddr_in* peer_addr, socklen_t* peer_addr_len);

#endif
