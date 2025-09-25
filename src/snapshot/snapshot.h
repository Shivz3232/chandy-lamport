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

void* processSnapshots(struct Snapshot**, struct Peer**);
void* processSnapshot(struct Snapshot**, struct Snapshot*, struct Peer**);
struct Snapshot* newSnapshot(struct Peer*);

void insertAtHead(struct Snapshot**, struct Snapshot* s);
void insertAtTail(struct Snapshot**, struct Snapshot* s);
struct Snapshot* searchById(struct Snapshot*, const char*);
void deleteById(struct Snapshot**, const char*);
void freeSnapshotsList(struct Snapshot*);

#endif
