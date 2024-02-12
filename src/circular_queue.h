#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stdint.h>
#include <ostream>
#include <array>
#include "queue.h"

#define CIRCULAR_QUEUE_CAPACITY 6

struct CQueueMetaData {
  int32_t head;
  int32_t tail;
  uint16_t itemSize;
  uint16_t capacity;
};

class CQueue {
public:
  CQueue() = default;
  CQueue(const char *name, const char *path = nullptr);
  CQueue(const char *name, uint16_t itemSize, uint16_t capacity, const char *path = nullptr);
  ~CQueue();
  bool head(char *buffer, uint16_t bufferLen, bool dequeue = false);

  bool enqueue(const char *buffer, size_t buffer_len);
  bool dequeue();
  void printer();
  CQueueMetaData getState() const;

private:
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  CQueueMetaData mState;
  bool isAvailable = false;
  void updateState();
};

#endif // CIRCULAR_QUEUE_H