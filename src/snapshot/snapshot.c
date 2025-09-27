#include "snapshot.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../queue/queue.h"
#include "../peers/peers.h"
#include "../utils/utils.h"

void* checkForNewSnapshots(struct Snapshot** snapshots, struct Peer** peers) {
  for (int i = 0; i < numPeers; i++) {
    if (i == processId) continue;
    if (isEmpty(peers[i]->read_channel)) continue;

    char* front = peek(peers[i]->read_channel);
    if (strcmp(front, "token") == 0) continue;

    struct Snapshot* existingSnapshot = searchById(*snapshots, front);
    if (existingSnapshot != NULL) continue;

    debug("============================================\n");
    debug("Creating new snapshot obj with id %s\n", front);
    struct Snapshot* newSnapshot = createNewSnapshotObj(front);
    debug("Successfully created new snapshot obj with id %s\n", newSnapshot->id);
    debug("============================================\n\n\n\n");

    debug("============================================\n");
    debug("Inserting new snapshot %s to list\n", newSnapshot->id);
    insertAtTail(snapshots, newSnapshot);
    debug("Successfully inserted snapshot %s to list\n", newSnapshot->id);
    debug("============================================\n\n\n\n");

    debug("============================================\n");
    debug("Sleeping for %0.2f before broadcasting marker %s\n", markerDelay, newSnapshot->id);
    usleep(markerDelay * 1000000);
    debug("Woke up broadcasting marker %s\n", newSnapshot->id);
    debug("============================================\n\n\n\n");

    broadcastMarker(newSnapshot->id, peers);
  }

  return NULL;
}

struct Snapshot* createNewSnapshotObj(char* snapshotId) {
  struct Snapshot* snapshot = malloc(sizeof(struct Snapshot));

  snapshot->id = strdup(snapshotId);
  snapshot->processState = state;
  snapshot->channelStates = malloc(sizeof(struct ChannelState*) * numPeers - 1);
  snapshot->closedChannels = 0;

  for (int i = 0; i < numPeers; i++) {
    snapshot->channelStates[i] = malloc(sizeof(struct ChannelState));
    snapshot->channelStates[i]->channelSize = 0;
    snapshot->channelStates[i]->channelContent = malloc(sizeof(char*) * maxSnapshotChannelSize);
    snapshot->channelStates[i]->closed = 0;
  }

  return snapshot;
}

void* processExistingSnapshots(struct Snapshot* snapshots, struct Peer** peers) {
  struct Snapshot* temp = snapshots;
  while (temp != NULL) {
    debug("============================================\n");
    debug("Processing existing snapshot with id %s\n", temp->id);
    processExistingSnapshot(temp, peers);
    debug("Successfully processed existing snapshot with id %s\n", temp->id);
    debug("============================================\n\n\n\n");

    temp = temp->next;
  }
  
  return NULL;
}

void* processExistingSnapshot(struct Snapshot* snapshot, struct Peer** peers) {
  for (int i = 0; i < numPeers; i++) {
    if (i == processId) continue;
    if (isEmpty(peers[i]->read_channel)) continue;
    
    char* front = peek(peers[i]->read_channel);

    if (strcmp(front, snapshot->id) == 0) {
      snapshot->channelStates[i]->closed = 1;
      debug("processExistingSnapshot: closed channel %d for snapshot \"%s\"\n", i + 1, snapshot->id);
      info("{proc_id:%d, snapshot_id: %s, snapshot:\"channel closed\", channel:%d-%d, queue:[%s]}",
        processId + 1,
        snapshot->id,
        peers[i]->id + 1,
        processId + 1,
        joinStrings(
          snapshot->channelStates[i]->channelContent,
          snapshot->channelStates[i]->channelSize,
          ','
        )
      );

      continue;
    }

    snapshot->channelStates[i]->channelContent[
      snapshot->channelStates[i]->channelSize
    ] = strdup(front);
    debug("processExistingSnapshot: copied \"%s\" to peer %d channel in snaphsot \"%s\"\n", front, i + 1, snapshot->id);

    snapshot->channelStates[i]->channelSize += 1;
  }

  return NULL;
}

void* broadcastMarker(char* snapshotId, struct Peer** peers) {
  debug("============================================\n");
  debug("Broadcasting marker %s\n", snapshotId);
  
  char* markerPacket = createPacket(snapshotId);
  if (markerPacket == NULL) {
    info("broadcastMarker: failed to create marker packet!\n");
    return NULL;
  }

  for (int i = 0; i < numPeers; i++) {
    if (i == processId) continue;

    int numBytesSent;
    if ((numBytesSent = sendAll(peers[i]->write_socket_fd, markerPacket, packetHeaderSize + strlen(snapshotId))) < 0) {
      debug("broadcastMarker: Failed to send marker packet to peer %s. numBytesSent: %d\n", peers[i]->name, numBytesSent);
      return NULL;
    }

    if (numBytesSent == 0) {
      debug("broadcastMarker: Nothing was sent. numBytesSent is 0!!\n");
    } else if (numBytesSent < strlen(snapshotId)) {
      debug("broadcastMarker: Partial snapshotId was sent!!\n");
    } else {
      info("proc_id:%d, snapshot_id: %s, sender:%d, receiver:%d, msg:\"marker\", state:%d, has_token:(TODO)}",
        processId + 1,
        snapshotId,
        processId + 1,
        peers[i]->id + 1,
        state
      );
    }
  }
  
  free(markerPacket);

  debug("Successfully broadcasted marker %s\n", snapshotId);
  debug("============================================\n\n\n\n");
  
  return NULL;
}

struct Snapshot* initiateSnapshot(struct Snapshot** snapshots, struct Peer** peers) {
  debug("============================================\n");
  debug("Creating new snapshot obj with id %s\n", snapshotId);
  struct Snapshot* newSnapshot = createNewSnapshotObj(snapshotId);
  debug("Successfully created new snapshot obj with id %s\n", newSnapshot->id);
  debug("============================================\n\n\n\n");

  debug("============================================\n");
  debug("Inserting new snapshot %s to list\n", newSnapshot->id);
  insertAtTail(snapshots, newSnapshot);
  debug("Successfully inserted snapshot %s to list\n", newSnapshot->id);
  debug("============================================\n\n\n\n");

  broadcastMarker(newSnapshot->id, peers);
  
  return newSnapshot;
}

void insertAtHead(struct Snapshot** head, struct Snapshot* s) {
    s->next = *head;
    *head = s;
}

void insertAtTail(struct Snapshot** head, struct Snapshot* s) {
  s->next = NULL;
  if (*head == NULL) {
    *head = s;
    return;
  }
  struct Snapshot* temp = *head;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = s;
}

struct Snapshot* searchById(struct Snapshot* head, const char* snapshotId) {
  struct Snapshot* temp = head;
  while (temp != NULL) {
    if (strcmp(temp->id, snapshotId) == 0) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

void deleteById(struct Snapshot** head, const char* snapshotId) {
  struct Snapshot* temp = *head;
  struct Snapshot* prev = NULL;

  while (temp != NULL && strcmp(temp->id, snapshotId) != 0) {
    prev = temp;
    temp = temp->next;
  }

  if (temp == NULL) return;

  if (prev == NULL) {
    *head = temp->next;
  } else {
    prev->next = temp->next;
  }

  free(temp->id);

  free(temp);
}

void freeSnapshotsList(struct Snapshot* head) {
  while (head != NULL) {
    struct Snapshot* temp = head;
    head = head->next;

    free(temp->id);

    free(temp);
  }
}
