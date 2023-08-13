
#ifndef RS_BUCKET_BUCKET_H
#define RS_BUCKET_BUCKET_H

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "queue.h"

#define QUEUE_NAME_MAX_LENGTH 50

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
#endif // RS_BUCKET_BUCKET_H