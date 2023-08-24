#ifndef RS_BUCKET_QUEUE_H
#define RS_BUCKET_QUEUE_H

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

class Bucket; // forward declaration

#define QUEUE_NAME_MAX_LENGTH 50

struct QueueMetaData {
  size_t head;
  size_t tail;
  uint16_t count;
  uint16_t index;
};

struct QueueItem {
  size_t bytesLen;
  uint16_t index;
  uint8_t check;
};

class Queue {
public:
  Queue();
  Queue(const char *name, const char *path = nullptr);
  ~Queue();
  void enqueue(const char *buffer, size_t buffer_len);
  void dequeue(const size_t itemLen);
  bool head(char *buffer, size_t *itemLen = nullptr, bool dequeue = false);
  void tail();
  bool at(uint16_t index, char *buffer, size_t *itemLen = nullptr);
  bool isEmpty() const;
  bool rename(const char *newName, const Bucket *bucket);
  bool move(const Bucket *other);
  const char *getName() const;
  const char *getPath() const;
  const QueueMetaData *const getMetaData() const;
  off_t byteSize() const;

private:
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  QueueMetaData mState;
  bool isAvailable = false;
  void updateState();
}; //

#endif // RS_BUCKET_QUEUE_H