#include <netdb.h>

#include "../config/config.h"

#ifndef PEERS_H
#define PEERS_H

extern int numPeers; // NOTE: THIS NUMBER INCLUDES SELF
extern int inboundConnections;
extern int outboundConnections;

struct Peer {
  int id;
  char* name;
  int read_socket_fd;
  struct addrinfo* read_addr_info;
  struct addrinfo* read_chosen_addr_info;
  int write_socket_fd;
  struct addrinfo* write_addr_info;
  struct addrinfo* write_chosen_addr_info;
  int connected;
};

void* populatePeerInfo(struct Peer* peer);
void freePeers(struct Peer* peers[]);
struct Peer* getPredecessor(struct Peer* peers[]);
struct Peer* getSuccessor(struct Peer* peers[]);
void* dialPeers(void*);

#endif
