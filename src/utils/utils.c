#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include "../config/config.h"
#include "../peers/peers.h"
#include "../logger/logger.h"

int bindToBest(struct addrinfo* addr_info) {
  int yes = 1;

  int socket_fd;
  
  for (struct addrinfo* p = addr_info; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("socket");
      continue;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, p->ai_addr, p->ai_addrlen) < 0) {
      close(socket_fd);
      perror("bind");
      continue;
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    getnameinfo(addr_info->ai_addr, addr_info->ai_addrlen, host, sizeof(host), service, sizeof(service), 0);
    debug("Successfully bound to %s Service: %s\n", host, service);

    return socket_fd;
  }

  return -1;
}

void* parseHostsfile(struct Peer* peers[]) {
  FILE* hostsfile = fopen(hostsFilePath, "r");
  if (hostsfile == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  char line[maxPeerNameSize];
  while(fgets(line, maxPeerNameSize, hostsfile) != NULL) {
    if (numPeers >= maxPeers) {
      info("Exeeded maximum number of supported peers.\n");
      fclose(hostsfile);
      exit(1);
    }

    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    if (strcmp(hostName, line) == 0) {
      processId = numPeers;
    }
    
    struct Peer* peer = malloc(sizeof(struct Peer));
    if (!peer) {
      perror("malloc");
      fclose(hostsfile);
      exit(EXIT_FAILURE);
    }

    peer->id = numPeers + 1;
    peer->name = strdup(line);

    peers[numPeers] = peer;
    populatePeerInfo(peer);
    
    numPeers += 1;
  }
  
  fclose(hostsfile);
  
  return NULL;
}

char* getNameInfo(struct sockaddr* peerAddr, socklen_t* peerAddrLen) {
  char peerName[NI_MAXHOST];
  if (!peerName) {
    perror("malloc");
    exit(EXIT_FAILURE);
}

  if (getnameinfo(peerAddr, *peerAddrLen, peerName, maxPeerNameSize, NULL, 0, NI_NAMEREQD) < 0) {
    perror("getnameinfo");
    exit(EXIT_FAILURE);
  }

  // Reduce FQDN by trimming at the first dot
  char *dot = strchr(peerName, '.');
  if (dot) {
    *dot = '\0';
  }

  return peerName;
}
