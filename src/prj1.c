#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>

#include "config/config.h"
#include "peers/peers.h"
#include "utils/utils.h"
#include "logger/logger.h"

int state = 0;
struct Peer** peers;

void* acceptConnections(void*);

int main(int argc, char const* argv[]) {
  info("Process started\n");

  initializeEnvVariables();
  debug("Successfully initialized environment Variables.\n");
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
  debug("Successfully started thread to establish outbound connections\n");
  debug("============================================\n\n\n\n");

  

  while (outboundConnections != numPeers - 1 && inboundConnections != numPeers - 1) {
    info("All connections not yet formed. outbound: %d, inbound: %d. Main thread going to sleep.\n", outboundConnections, inboundConnections);
    sleep(backoffDuration);
  }

  debug("============================================\n");
  debug("Wrapping up\n");
  close(socket_fd);
  freeaddrinfo(addr_info);
  freePeers(peers);
  debug("Processs finished\n");
  debug("============================================\n\n\n\n");

  return 0;
}

void* acceptConnections(void* input) {
  return NULL;
}
