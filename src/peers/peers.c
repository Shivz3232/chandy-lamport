#include "peers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include "../logger/logger.h"
#include "../config/config.h"

int numPeers = 0;
int connectedPeers = 0;

void* populatePeerInfo(struct Peer* peer) {
  struct addrinfo hints, *addr_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(peer->name, port, &hints, &addr_info) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  peer->addr_info = addr_info;

  int socket_fd;
  for (struct addrinfo* p = addr_info; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("socket");
      continue;
    }

    peer->chosen_addr_info = p;
    peer->socket_fd = socket_fd;
    
    break;
  }
  
  return NULL;
}

void freePeers(struct Peer* peers[]) {
  if (!peers) return;
  
  for (int i = 0; i < maxPeers; i++) {
      if (peers[i]) {
          free(peers[i]->name);
          free(peers[i]);
      }
  }
  
  free(peers);
}

struct Peer* getPredecessor(struct Peer* peers[]) {
  if (processId == 0) {
    return peers[numPeers - 1];
  }

  return peers[processId - 1];
}

struct Peer* getSuccessor(struct Peer* peers[]) {
  return peers[(processId + 1) % numPeers];
}
