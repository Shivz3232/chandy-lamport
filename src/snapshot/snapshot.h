#include "../peers/peers.h"

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

struct ChannelState {
  char** channelContent;
  int channelSize;
  int closed;
};

// Linked list
struct Snapshot {
  char* id;
  int processState;
  struct ChannelState** channelStates;
  int closedChannels;
  struct Snapshot* next;
};

void* checkForNewSnapshots(struct Snapshot** snapshots, struct Peer** peers);
struct Snapshot* createNewSnapshotObj(char* snapshotId);
void* processExistingSnapshots(struct Snapshot* snapshots, struct Peer** peers);
void* processExistingSnapshot(struct Snapshot* snapshot, struct Peer** peers);

struct Snapshot* initiateSnapshot(struct Snapshot** head, struct Peer** peers);
void* broadcastMarker(char* snapshotId, struct Peer** peers);

void insertAtHead(struct Snapshot**, struct Snapshot* s);
void insertAtTail(struct Snapshot**, struct Snapshot* s);
struct Snapshot* searchById(struct Snapshot*, const char*);
void deleteById(struct Snapshot**, const char*);
void freeSnapshotsList(struct Snapshot*);

#endif
