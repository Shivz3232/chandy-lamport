#include "snapshot.h"

#include <stdlib.h>
#include <string.h>

#include "../config/config.h"
#include "../logger/logger.h"
#include "../queue/queue.h"
#include "../peers/peers.h"
#include "../snapshot/snapshot.h"

void* processSnapshots(struct Snapshot** snapshots, struct Peer** peers) {
  // Check if there are any marker messages.
  struct Snapshot* temp = *snapshots;
  while (temp != NULL) {
    processSnapshot(snapshots, temp, peers);
    temp = temp->next;
  }

  // Check for new snapshots
  for (int i = 0; i < numPeers; i++) {
    if (i == processId) continue;
    if (isEmpty(peers[i]->read_channel)) continue;
    if (strcmp(peek(peers[i]->read_channel), "token") == 0 ) continue;

    struct Snapshot* snapshot = newSnapshot(peers[i]);
    
    insertAtTail(snapshots, snapshot);

    processSnapshot(snapshots, snapshot, peers);
  }

  return NULL;
}

void* processSnapshot(struct Snapshot** head, struct Snapshot* snapshot, struct Peer** peers) {
  debug("processSnapshot: Processing snapshot %s\n", snapshot->id);
  
  for (int i = 0; i < numPeers; i++) {
    int markerSeen = 0;
    
    if (snapshot->channelStates[i]->closed) continue;
    if (isEmpty(peers[i]->read_channel)) continue;

    // COPY EVERYTHING FROM THE QUEUE TO THE CHANNEL
    for (int j = 0; j < peers[i]->read_channel->rear; j++) {
      if (strcmp(snapshot->id, peers[i]->read_channel->items[j]) == 0) {
        markerSeen = 1;
        break;
      }

      snapshot->channelStates[i]->channelContent[
        snapshot->channelStates[i]->channelSize
      ] = strdup(peers[i]->read_channel->items[j]);
    }

    if (markerSeen) {
      snapshot->channelStates[i]->closed = 1;
      snapshot->closedChannels += 1;
    }
  }

  if (snapshot->closedChannels == numPeers) {
    info("{proc_id:%d, snapshot_id: %d, snapshot:\"complete\"}\n", processId + 1, snapshot->id);
    deleteById(head, snapshot->id);
  }

  return NULL;
}

struct Snapshot* newSnapshot(struct Peer* peer) {
  struct Snapshot* snapshot = malloc(sizeof(struct Snapshot));

  snapshot->id = strdup(peek(peer->read_channel));
  snapshot->processState = state;
  snapshot->channelStates = malloc(sizeof(struct ChannelState*) * numPeers);

  for (int i = 0; i < numPeers; i++) {
    snapshot->channelStates[i] = malloc(sizeof(struct ChannelState));
    snapshot->channelStates[i]->channelSize = 0;
    snapshot->channelStates[i]->channelContent = malloc(sizeof(char*) * maxSnapshotChannelSize);
    snapshot->closedChannels = 0;

    if (i == peer->id) {
      snapshot->channelStates[i]->closed = 1;
      snapshot->closedChannels += 1;

      info("{proc_id:%d, snapshot_id: %s, snapshot:\"channel closed\", channel:%d-%d, queue:[]}",
        processId + 1,
        snapshot->id,
        peer->id + 1,
        processId + 1
      );
    } else {
      snapshot->channelStates[i]->closed = 0;
    }
  }

  dequeue(peer->read_channel);

  debug("newSnapshot: Created new snapshot %s\n", snapshot->id);

  return snapshot;
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
