#include "peers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>

#include "../logger/logger.h"

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

void* markPeerAsConnected(struct Peer* peers[], struct sockaddr_in* peer_addr, socklen_t* peer_addr_len) {
  char peer_name[maxPeerNameSize];
  
  if (getnameinfo((struct sockaddr*)peer_addr, *peer_addr_len, peer_name, maxPeerNameSize, NULL, 0, NI_NAMEREQD) < 0) {
    perror("getnameinfo");
    exit(EXIT_FAILURE);
  }

  // Reduce FQDN by trimming at the first dot
  char *dot = strchr(peer_name, '.');
  if (dot) {
    *dot = '\0';
  }
  
  for (int i = 0; i < numPeers; i++) {
    if (strcmp(peers[i]->name, peer_name) == 0 && peers[i]->connected == 0) {
      peers[i]->connected = 1;
      connectedPeers += 1;

      info("Marked peer %s as connected\n", peer_name);
      
      return NULL;
    }
  }

  info("Recieived message from an unknown peer: %s\n", peer_name);
  
  return NULL;
}
