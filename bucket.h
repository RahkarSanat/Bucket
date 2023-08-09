
#include <stdio.h>
#include <stdint.h>

#define QUEUE_NAME_MAX_LENGTH 50

// #define echo(f, ...) printf(f, ...)

struct QueueMetaData {
  size_t head;
  size_t tail;
  uint16_t count;
  uint16_t index;
};

struct QueueItem {
  uint16_t index;
  size_t bytesLen;
};

class Queue {
public:
  Queue(const char *name);
  ~Queue();
  void enqueue(char *buffer, size_t buffer_len);
  void dequeue(char *buffer);
  void head();
  void tail();

private:
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  FILE *fd;
  QueueMetaData mState;
  bool isAvailable = false;
  void updateState();
};

class Bucket {
  bool init();
};
