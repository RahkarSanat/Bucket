#include <cstdio>
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
  char fullPath[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  snprintf(fullPath, sizeof(fullPath), "%s/%s", this->mBucketPath, name);
  return Queue{name, fullPath};
}

bool Bucket::getExistingQueue(const char *name, Queue *queue) {
  struct stat st = {};
  char fullPath[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  snprintf(fullPath, sizeof(fullPath), "%s/%s", this->mBucketPath, name);
  if (stat(fullPath, &st) == 0) {
    *queue = Queue{name, fullPath};
    return true;
  }
  return false;
}

void Bucket::init(const char *path) {
  strncpy(this->mBucketPath, path, QUEUE_NAME_MAX_LENGTH - 1);
  this->mBucketPath[QUEUE_NAME_MAX_LENGTH - 1] = '\0';
  if (mkdirp(path, 0777) != 0) {
    printf("Failed to [ %s ] directory\n", path);
  }
}

bool Bucket::removeQueue(const char *name) {
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  struct stat st = {};
  snprintf(path, sizeof(path), "%s/%s", this->mBucketPath, name);
  if (stat(path, &st) == 0) {
    errno = 0;
    return remove(path) == 0;
  }
  return false;
}
const char *Bucket::getPath() const { return this->mBucketPath; }

void Bucket::list(bool showSize) const {
  char *entry = nullptr;
  bucket::Iterator iter{this->getPath()};
  printf("listing bucket in: %s :------>\n", this->getPath());
  while ((entry = iter.next())) {
    if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0) {
      continue;
    }
    if (showSize) {
      struct stat st = {};
      char path[320] = {};
      snprintf(path, sizeof(path), "%s/%s", this->getPath(), entry);
      if (stat(path, &st) == 0) {
        printf("- %s (%ld bytes)\n", entry, (long)st.st_size);
      } else {
        printf("- %s \n", entry);
      }
    } else {
      printf("- %s \n", entry);
    }
  }
}

size_t Bucket::getDirSize() const {
  char *entry = nullptr;
  struct stat st = {};
  bucket::Iterator iter{this->getPath()};
  size_t size = 0;
  char path[320] = {};
  while ((entry = iter.next())) {
    if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0) {
      continue;
    }
    snprintf(path, sizeof(path), "%s/%s", this->getPath(), entry);
    stat(path, &st);
    size += st.st_size;
  }
  return size;
}

namespace bucket {

Iterator::Iterator(const char *dir_name) {
  if (dir_name != nullptr) {
    strncpy(mDirectoryName, dir_name, sizeof(mDirectoryName) - 1);
    mDirectoryName[sizeof(mDirectoryName) - 1] = '\0';
  }
  dir = opendir(mDirectoryName);
  if (dir == nullptr) {
    printf("Unable to open directory\n");
  }
  entry = nullptr;
}

bool Iterator::from(uint32_t from) {
  char *entry_name = nullptr;
  char queueName[15] = {0};
  snprintf(queueName, sizeof(queueName), "%08" PRIx32, from);
  while ((entry_name = this->next())) {
    if (strcmp(entry_name, queueName) == 0) {
      long location = telldir(dir);
      if (location > 0) {
        seekdir(dir, location - 1);
      }
      return true;
    }
  }
  printf("Didn't find the entry you wanted to iterate from it\n");
  return false;
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
