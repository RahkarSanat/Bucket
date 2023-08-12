
#ifndef BUCKET_QUEUE_H
#define BUCKET_QUEUE_H

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#define QUEUE_NAME_MAX_LENGTH 50

// #define echo(f, ...) printf(f, ...)

struct QueueMetaData {
  size_t head;
  size_t tail;
  uint16_t count;
  uint16_t index;
};

struct QueueItem {
  size_t bytesLen;
  uint16_t index;
};

class Bucket;
class Queue {
public:
  Queue(const char *name, const char *path = nullptr);
  ~Queue();
  void enqueue(char *buffer, size_t buffer_len);
  bool dequeue(char *buffer, size_t *itemLen = nullptr);
  void head();
  void tail();
  bool at(uint16_t index, char *buffer, size_t *itemLen = nullptr);
  bool isEmpty() const;
  bool rename(const char *newName, const Bucket *bucket);
  bool move(const Bucket *other);
  const char *getName() const;
  const QueueMetaData *const getMetaData() const;

private:
  FILE *fd;
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  QueueMetaData mState;
  bool isAvailable = false;
  void updateState();
}; //

class Bucket {
public:
  void init(const char *path);
  Queue getQueue(const char *name);
  const char *getPath() const;
  bool removeQueue(const char *name);

private:
  int mkdirp(const char *path, mode_t mode);
  char mBucketPath[QUEUE_NAME_MAX_LENGTH] = {0};
};
#endif // BUCKET_QUEUE_H