#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <poll.h>

#include <netdb.h>

#include "../config/config.h"
#include "../peers/peers.h"
#include "../logger/logger.h"
#include "../queue/queue.h"

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
    
    peer->read_channel = malloc(sizeof(Queue));
    initializeQueue(peer->read_channel);

    peers[numPeers] = peer;
    populatePeerInfo(peer);
    
    numPeers += 1;
  }
  
  fclose(hostsfile);
  
  return NULL;
}

char* getNameInfo(struct sockaddr* peerAddr, socklen_t* peerAddrLen) {
  char* peerName = malloc(NI_MAXHOST);
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

void parseArgs(int argc, char* const argv[]) {
  int opt;
  int optionIndex = 0;

  static struct option longOptions[] = {
    {"name",     optional_argument, 0,  0 },
    {"network",  optional_argument, 0,  0 },
    {"hostname", optional_argument, 0,  0 },
    {0,          0,                 0,  0 }
  };

  while (1) {
      opt = getopt_long(argc, argv, "h:t:m:s:p:x", longOptions, &optionIndex);

      if (opt == -1) break;

      switch (opt) {
          case 0:
              if (strcmp(longOptions[optionIndex].name, "name") == 0) {
                  name = strdup(optarg);
              } else if (strcmp(longOptions[optionIndex].name, "network") == 0) {
                  network = strdup(optarg);
              } else if (strcmp(longOptions[optionIndex].name, "hostname") == 0) {
                  inputHostname = strdup(optarg);
              }
              break;

          case 'h':
              hostsFilePath = strdup(optarg);
              break;

          case 't':
              tokenDelay = atof(optarg);
              break;

          case 'm':
              markerDelay = atoi(optarg);
              break;

          case 's':
              snapshotDelay = atoi(optarg);
              break;

          case 'p':
              snapshotId = atoi(optarg);
              break;

          case 'x':
              starter = 1;
              break;

          default:
              break;
      }
  }

  debug("Arg name set to %s", name);
  debug("Arg network set to %s", network);
  debug("Arg inputHostname set to %s", inputHostname);
  debug("Arg hostsFilePath set to %s", hostsFilePath);
  debug("Arg markerDelay set to %d", markerDelay);
  debug("Arg snapshotDelay set to %d", snapshotDelay);
  debug("Arg tokenDelay set to %0.2f", tokenDelay);
  debug("Arg snapshotId set to %d", snapshotId);
  debug("Arg starter set to %d", starter);

}

struct pollfd* setupPollFds(struct Peer* peers[]) {
  struct pollfd* pollFds = malloc(sizeof(struct pollfd) * numPeers);
  if (pollFds == NULL) {
    return NULL;
  }

  for (int i = 0; i < numPeers; i++) {
    if (i == processId) {
      pollFds[i].fd = -1;
      pollFds[i].events = 0;
      pollFds[i].revents = 0;
    } else {
      pollFds[i].fd = peers[i]->read_socket_fd;
      pollFds[i].events = POLLIN | POLLHUP;
    }
  }

  return pollFds;
}

void* freePollFds(struct pollfd* pollFds) {
  if (pollFds) {
    free(pollFds);
    pollFds = NULL;
  }

  return NULL;
}
