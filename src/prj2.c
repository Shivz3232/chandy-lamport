#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <netdb.h>

#include <pthread.h>

#include "config/config.h"
#include "peers/peers.h"
#include "utils/utils.h"
#include "logger/logger.h"
#include "token/token.h"
#include "queue/queue.h"

struct Peer** peers;
struct pollfd* pollFds;

void* acceptConnections(void*);

void* setupPollFds();
void* startPolling();
void* freePollFds();

int main(int argc, char const* argv[]) {
  info("Process started\n");

  parseArgs(argc, (char* const*)argv);

  initializeEnvVariables();
  debug("Successfully initialized config parameters.\n");
  debug("============================================\n\n\n\n");

  peers = malloc(maxPeers * sizeof(struct Peer*));

  debug("============================================\n");
  debug("Parsing hosts file.\n");
  parseHostsfile(peers);
  debug("Successfully parsed hosts file.\n");
  debug("============================================\n\n\n\n");

  if (numPeers == 0) {
    info("Hostsfile empty, no peers found.\n");
    return 0;
  }

  info(
    "{proc_id: %d, state: %d, predecessor: %s, successor: %s}\n\n\n\n",
    processId + 1,
    state,
    getPredecessor(peers)->name,
    getSuccessor(peers)->name
  );

  struct addrinfo hints, *addr_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, port, &hints, &addr_info) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  debug("============================================\n");
  debug("Creating self socket.\n");
  int socket_fd;
  if ((socket_fd = bindToBest(addr_info)) < 0) {
    perror("bindToBest");
    exit(2);
  }
  debug("Self socket created successfully\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Trying to listen.\n");
  if (listen(socket_fd, backlog) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  debug("Listening successfully\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  pthread_t outboundConnectionsEstablishmentThread;
  pthread_create(&outboundConnectionsEstablishmentThread, NULL, dialPeers, peers);
  pthread_detach(outboundConnectionsEstablishmentThread);
  debug("Successfully started thread to establish outbound connections\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  pthread_t inboundConnectionsEstablishmentThread;
  pthread_create(&inboundConnectionsEstablishmentThread, NULL, acceptConnections, &socket_fd);
  pthread_detach(inboundConnectionsEstablishmentThread);
  debug("Successfully started thread to establish inbound connections\n");
  debug("============================================\n\n\n\n");

  while (outboundConnections != numPeers - 1 || inboundConnections != numPeers - 1) {
    info("All connections not yet formed. outbound: %d, inbound: %d. Main thread going to sleep.\n", outboundConnections, inboundConnections);
    sleep(backoffDuration);
  }

  info("READY! Establishied inbound and outbound connections with all peers\n");

  debug("============================================\n");
  debug("Setting up pollFds\n");
  setupPollFds();
  debug("Successfully setup pollFds\n");
  debug("============================================\n\n\n\n");  

  

  if (starter) {
    debug("============================================\n");
    debug("Starting token forwarding thread");
    pthread_t tokenForwardingThread;
    pthread_create(&tokenForwardingThread, NULL, startTokenPassing, peers);
    pthread_detach(tokenForwardingThread);
    debug("Successfully started thread to forward token\n");
    debug("============================================\n\n\n\n");
  }
  
  // NOTE THIS IS A BLOCKING STEP
  debug("============================================\n");
  debug("Starting polling\n");
  startPolling();
  debug("Stopped polling\n");
  debug("============================================\n\n\n\n");  

  debug("============================================\n");
  debug("Wrapping up\n");
  freePollFds();
  close(socket_fd);
  freeaddrinfo(addr_info);
  freePeers(peers);
  debug("Processs finished\n");
  debug("============================================\n\n\n\n");

  return 0;
}

void* acceptConnections(void* input) {
  int socket_fd = *(int*)input;
  
  while (inboundConnections < numPeers - 1) {
    int peer_fd;
    struct sockaddr_storage peerAddr;
    socklen_t peerAddrLen = sizeof(peerAddr);

    if ((peer_fd = accept(socket_fd, (struct sockaddr*)&peerAddr, &peerAddrLen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    char* peerName = getNameInfo((struct sockaddr*)&peerAddr, &peerAddrLen);

    int found = 0;
    for (int i = 0; i < numPeers && found == 0; i++) {
      if (strcmp(peers[i]->name, peerName) == 0) {
        found = 1;
        
        peers[i]->read_socket_fd = peer_fd;

        peers[i]->read_addr_info = malloc(peerAddrLen);
        memcpy(peers[i]->read_addr_info, &peerAddr, peerAddrLen);

        peers[i]->read_addr_info_len = peerAddrLen;

        break;
      }
    }

    if (found == 0) {
      info("Received connection from an unknown peer %s\n", peerName);
    } else {
      info("Successful inbound connection from %s\n", peerName);
    }

    free(peerName);

    inboundConnections += 1;
  }

  return NULL;
}

void* setupPollFds() {
  pollFds = malloc(sizeof(struct pollfd) * numPeers);

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

  return NULL;
}

void* startPolling() {
  while (1) {
    int numEvents = poll(pollFds, numPeers, -1);

    debug("Poll: received %d events\n", numEvents);

    if (numEvents == 0) {
      info("Poll timed out?!\n");
      return NULL;
    } else {
      for (int j = 0, seen = 0; seen == numEvents || j < numPeers; j++) {
        if (pollFds[j].revents == 0) continue;

        if (j == processId) {
          debug("Ignoring poll event from self\n");
          continue;
        }

        if (j < numPeers && (pollFds[j].revents & POLLIN)) {
          debug("Poll: POLLIN event for process %d\n", j + 1);
          int numBytes;
          char* buf = malloc(maxMessageSize);

          if((numBytes = recv(peers[j]->read_socket_fd, buf, maxMessageSize - 1, 0)) < 0) {
            debug("recv: Failed! numBytes: %d\n", numBytes);
          }

          if (numBytes == 0) {
            debug("Peer %d closed the connection. numBytes: 0!!\n", j);
          }

          buf[numBytes] = '\0';

          debug("Peer %d says: %s\n", peers[j]->name, buf);

          enqueue(peers[j]->read_channel, buf);

          debug("============================================\n");
          debug("Starting token forwarding thread");
          pthread_t tokenForwardingThread;
          pthread_create(&tokenForwardingThread, NULL, passToken, peers);
          pthread_detach(tokenForwardingThread);
          debug("Successfully started thread to forward token\n");
          debug("============================================\n\n\n\n");
        }

        if (pollFds[j].revents & POLLHUP) {
          debug("Poll: POLLHUP event for process %d\n", j + 1);
          // handle closed socket
        }

        seen += 1;
      }
    }
  }

  return NULL;
}

void* freePollFds() {
  if (pollFds) {
    free(pollFds);
    pollFds = NULL;
  }

  return NULL;
}

