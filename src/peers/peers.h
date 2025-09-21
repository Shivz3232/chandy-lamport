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
void freePeers(struct Peer* peers[]);
struct Peer* getPredecessor(struct Peer* peers[]);
struct Peer* getSuccessor(struct Peer* peers[]);

#endif
