#include "peers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include "../logger/logger.h"
#include "../config/config.h"

int numPeers = 0;
int outboundConnections = 0;
int inboundConnections = 0;

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

  peer->write_addr_info = addr_info;

  int socket_fd;
  for (struct addrinfo* p = addr_info; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("socket");
      continue;
    }

    peer->write_chosen_addr_info = p;
    peer->write_socket_fd = socket_fd;
    
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

void* dialPeers(void* input) {
  struct Peer** peers = input;
  
  for (int i = 0; i < numPeers; i++) {
    // Skip myself
    if (i == processId) {
      continue;
    }
    
    int retries = 0;
    int connected = 0;
    while (connected == 0 && retries < maxRetries) {
      struct addrinfo hints, *p;

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      
      if (getaddrinfo(peers[i]->name, port, &hints, &peers[i]->write_addr_info) < 0) {
        perror("getaddrinfo");
        retries += 1;
        sleep(backoffDuration);
        info("Retrying to connect to %s\n", peers[i]->name);
        continue;
      }

      int yes = 1;
      int socket_fd;
      for (p = peers[i]->write_addr_info; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
          perror("socket");
          continue;
        }

        info("Attempting connection to %s\n", peers[i]->name);

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) < 0) {
          perror("connect");
          close(socket_fd);
          continue;
        }

        // Use this connection only for writing.
        shutdown(socket_fd, SHUT_RD);

        info("Successfully connected to %s\n", peers[i]->name);
        
        break;
      }

      if (!p) {
        info("Failed to connect to %s. Tried %d times. Sleeping.\n", peers[i]->name, retries + 1);
        retries += 1;
        sleep(backoffDuration);
        info("Retrying to connect to %s\n", peers[i]->name);
        continue;
      }

      peers[i]->write_chosen_addr_info = p;

      outboundConnections += 1;

      connected = 1;
    }
  }
  
  return NULL;
}
