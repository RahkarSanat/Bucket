#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <memory>
#include <stdlib.h>
#include "bucket.h"

int Bucket::mkdirp(const char *path, mode_t mode) {
  char *p = NULL;
  using DeleterT = void (*)(void *);
  std::unique_ptr<char, DeleterT> tmp = std::unique_ptr<char, DeleterT>{strdup(path), [](void *ptr) { free(ptr); }};

  int len = strlen(tmp.get());
  // Iterate over each component of the path
  for (p = tmp.get() + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';

      // Create the directory if it doesn't exist
      if (mkdir(tmp.get(), mode) == -1 && errno != EEXIST) {
        return -1;
      }
      *p = '/';
    }
  }
  // Create the final directory
  if (mkdir(tmp.get(), mode) == -1 && errno != EEXIST) {
    return -1;
  }

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

void Bucket::list() const {
  char *entry = nullptr;
  bucket::Iterator iter{this->getPath()};
  printf("listing bucket in: %s :------>\n", this->getPath());
  while ((entry = iter.next())) {
    printf("- %s \n", entry);
  }
}

namespace bucket {

Iterator::Iterator(const char *dir_name) {
  if (dir_name != nullptr) {
    memcpy(mDirectoryName, dir_name, strlen(dir_name) + 1);
  }
  dir = opendir(mDirectoryName);
  if (dir == nullptr) {
    printf("Unable to open directory\n");
  }
}

void Iterator::from(uint32_t from) {
  char *entry_name = nullptr;
  char queueName[15] = {0};
  sprintf(queueName, "%08" PRIx32, from);
  while ((entry_name = this->next())) {
    if (strcmp(entry_name, queueName) == 0) {
      long location = telldir(dir);
      if (location > 0)
        seekdir(dir, location - 1);
      return;
    }
  }
  printf("Didn't find the entry you wanted to iterate from it\n");
}

char *Iterator::next() {
  if (!dir) {
    printf("Directory is not open\n");
    return nullptr;
  }
  entry = readdir(this->dir);
  if (entry != nullptr) {
    return entry->d_name;
  }
  return nullptr;
}

Iterator::~Iterator() {
  if (dir) {
    closedir(dir);
  }
}

} // namespace bucket