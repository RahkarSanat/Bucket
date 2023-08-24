#include "bucket.h"
#include <cstring>
#include <errno.h>
#include <cstdlib>

int Bucket::mkdirp(const char *path, mode_t mode) {
  char *p = NULL;
  char *tmp = strdup(path);
  int len = strlen(tmp);
  // Iterate over each component of the path
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';

      // Create the directory if it doesn't exist
      if (mkdir(tmp, mode) == -1 && errno != EEXIST) {
        printf("::::::::::::::::::::::%s\n", strerror(errno));
        free(tmp);
        return -1;
      }

      *p = '/';
    }
  }

  // Create the final directory
  if (mkdir(tmp, mode) == -1 && errno != EEXIST) {
    free(tmp);
    return -1;
  }

  free(tmp);
  return 0;
}

Queue Bucket::getQueue(const char *name) {
  char fullPath[100] = {0};
  snprintf(fullPath, 100, "%s/%s", this->mBucketPath, name);
  return Queue{name, fullPath};
}

bool Bucket::getExistingQueue(const char *name, Queue *queue) {
  struct stat st;
  char fullPath[100] = {0};
  snprintf(fullPath, 100, "%s/%s", this->mBucketPath, name);
  if (stat(fullPath, &st) == 0) {
    *queue = Queue{name, fullPath};
    return true;
  }
  return false;
}

void Bucket::init(const char *path) {
  std::strcpy(this->mBucketPath, path);
  if (mkdirp(path, 0777) != 0) {
    printf("Failed to [ %s ] directory\n", path);
  }
}

bool Bucket::removeQueue(const char *name) {
  // check if queue exists
  char path[100] = {0};
  struct stat st;
  sprintf(path, "%s/%s", this->mBucketPath, name);
  if (stat(path, &st) == 0) {
    // remove the file
    errno = 0;
    return remove(path) == 0;
  }
  return false;
}
const char *Bucket::getPath() const { return this->mBucketPath; }
