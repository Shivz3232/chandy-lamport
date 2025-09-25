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
#include "snapshot/snapshot.h"

struct Peer** peers;
struct pollfd* pollFds;
struct Snapshot* snapshots;

void* acceptConnections(void*);
void* startPolling();
void* startPollingV2();
void* step(int numEvents);

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

  struct addrinfo hints, *addrInfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, port, &hints, &addrInfo) < 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  debug("============================================\n");
  debug("Creating self socket.\n");
  int socketFd;
  if ((socketFd = bindToBest(addrInfo)) < 0) {
    perror("bindToBest");
    exit(2);
  }
  debug("Self socket created successfully\n");
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Trying to listen.\n");
  if (listen(socketFd, backlog) < 0) {
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
  pthread_create(&inboundConnectionsEstablishmentThread, NULL, acceptConnections, &socketFd);
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
  pollFds = setupPollFds(peers);
  debug("Successfully setup pollFds\n");
  debug("============================================\n\n\n\n");  

  if (starter) {
    debug("============================================\n");
    debug("Starting token forwarding thread\n");
    pthread_t tokenForwardingThread;
    pthread_create(&tokenForwardingThread, NULL, startTokenPassing, peers);
    pthread_detach(tokenForwardingThread);
    debug("Successfully started thread to forward token\n");
    debug("============================================\n\n\n\n");
  }
  
  // NOTE THIS IS A BLOCKING STEP
  debug("============================================\n");
  debug("Starting polling\n");
  startPollingV2();
  debug("Stopped polling\n");
  debug("============================================\n\n\n\n");  

  debug("============================================\n");
  debug("Wrapping up\n");
  freePollFds(pollFds);
  close(socketFd);
  freeaddrinfo(addrInfo);
  freePeers(peers);
  debug("Processs finished\n");
  debug("============================================\n\n\n\n");

  return 0;
}

void* acceptConnections(void* input) {
  int socketFd = *(int*)input;
  
  while (inboundConnections < numPeers - 1) {
    int peer_fd;
    struct sockaddr_storage peerAddr;
    socklen_t peerAddrLen = sizeof(peerAddr);

    if ((peer_fd = accept(socketFd, (struct sockaddr*)&peerAddr, &peerAddrLen)) < 0) {
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

void* startPollingV2() {
  while(1) {
    int numEvents = poll(pollFds, numPeers, -1);

    debug("Poll: received %d events\n", numEvents);

    if (numEvents == 0) {
      info("Poll timed out?!\n");
      return NULL;
    }

    step(numEvents);

    processSnapshots(&snapshots, peers);

    // Marker messages should be removed from any channel before this call.
    passToken(peers);
  }

  return NULL;
}

void* step(int numEvents) {
  for (int i = 0, seen = 0; seen <= numEvents && i < numPeers; i++) {
    if (i == processId || !(pollFds[i].revents & POLLIN)) continue;

    char* data = receivePacket(peers[i]->read_socket_fd);
    if (data == NULL) {
      debug("receivePacket: Failed!\n");
      continue;
    }

    enqueue(peers[i]->read_channel, data);
  }

  return NULL;
}
