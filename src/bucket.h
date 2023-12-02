
#ifndef RS_BUCKET_BUCKET_H
#define RS_BUCKET_BUCKET_H

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#include "queue.h"

#define QUEUE_NAME_MAX_LENGTH 50

class Bucket {
public:
  void init(const char *path);
  Queue getQueue(const char *name);
  bool getExistingQueue(const char *name, Queue *queue);
  const char *getPath() const;
  bool removeQueue(const char *name);
  void list(bool showSize = false) const;

private:
  int mkdirp(const char *path, mode_t mode);
  char mBucketPath[QUEUE_NAME_MAX_LENGTH] = {0};
};

namespace bucket {
class Iterator {
public:
  Iterator(const char *dir_name);
  ~Iterator();
  virtual void from(uint32_t from);
  char *next();

private:
  DIR *dir;
  struct dirent *entry;
  char mDirectoryName[50] = {0};
};
} // namespace bucket
#endif // RS_BUCKET_BUCKET_H