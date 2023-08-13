#include "bucket.h"
#include <cstring>
#include <errno.h>
#include <cstdlib>

Queue::Queue(const char *name, const char *path) {
  FILE *fd = nullptr;
  strcpy(this->name, name);
  if (path != nullptr)
    strcpy(this->path, path);
  char *validName = path == nullptr ? this->name : this->path;
  struct stat st;
  if (stat(validName, &st) == 0) {
    printf("your same queue existed!\n");
    if ((fd = fopen(validName, "rb"))) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    if ((fd = fopen(validName, "wb"))) {
      this->mState = (QueueMetaData){.head = sizeof(QueueMetaData), .tail = sizeof(QueueMetaData), .count = 0, .index = 0};
      if (fwrite(&mState, sizeof(QueueMetaData), 1, fd)) {
        isAvailable = true;
      }
    }
  }
  if (fd != nullptr) {
    fclose(fd);
    fd = nullptr;
  }
}

Queue::~Queue() {}

void Queue::updateState() {
  FILE *fd;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if ((fd = fopen(validPath, "r+b"))) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(QueueMetaData), 1, fd);
    fclose(fd);
  } else {
    printf("Failed to update the queue status\n");
  }
}

void Queue::enqueue(const char *buffer, size_t buffer_len) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (this->isAvailable && (fd = fopen(validPath, "r+b"))) {
    int reds = fseek(fd, mState.tail, SEEK_SET);
    QueueItem item = {.bytesLen = buffer_len, .index = static_cast<uint16_t>(this->mState.count + (1))};
    this->mState.tail += fwrite(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd) * (sizeof(size_t) + sizeof(uint16_t));
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    fclose(fd);
    fd = nullptr;
    this->updateState();
  }
}

bool Queue::dequeue(char *buffer, size_t *itemLen) {
  FILE *fd = nullptr;
  QueueItem item;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (this->isAvailable && (fd = fopen(validPath, "rb"))) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd);
    if (this->mState.head == this->mState.tail) {
      fclose(fd);
      return false;
    }
    if (!buffer) {
      *itemLen = item.bytesLen;
      fclose(fd);
      return true;
    }
    fseek(fd, this->mState.head + (sizeof(size_t) + sizeof(uint16_t)), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      this->mState.head += item.bytesLen + (sizeof(size_t) + sizeof(uint16_t));
      fclose(fd);
      ++this->mState.index;
      updateState();
      return true;
    } else
      printf("failed to read one item\n");
    fclose(fd);
  }
  return false;
}

bool Queue::at(uint16_t index, char *buffer, size_t *itemLen) {
  QueueItem item;
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if ((fd = fopen(validPath, "rb"))) {
    // loop to find the index
    size_t currentPos = sizeof(QueueMetaData);
    while (currentPos <= this->mState.tail) {
      fseek(fd, currentPos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd);
      if (item.index == index) {
        printf("Found the Item at %d %d \n", item.index, item.bytesLen);
        fread(buffer, item.bytesLen, 1, fd);
        fclose(fd);
        return true;
      }
      // else {
      //   printf("not found yet %d %d %d\n", item.index, item.bytesLen, this->mState.tail);
      // }
      currentPos += item.bytesLen + (sizeof(size_t) + sizeof(uint16_t));
    }
    fclose(fd);
  } else {
    printf("Failed to read the queue\n");
  }

  return false;
}

bool Queue::isEmpty() const { return this->mState.count == 0; }

bool Queue::rename(const char *newName, const Bucket *bucket) {
  char dst[100] = {0};
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if (bucket) {
    sprintf(dst, "%s/%s", bucket->getPath(), newName);
  }
  errno = 0;
  if (::rename(validPath, dst) == 0) {
    std::strcpy(validPath, dst);
    return true;
  }
  return false;
}

bool Queue::move(const Bucket *other) {
  // is identical to queue rename but more classy
  // use the file name and the other bucket path to generate dst path
  return this->rename(this->getName(), other);
}

const char *Queue::getName() const { return this->name; }

const QueueMetaData *const Queue::getMetaData() const { return &this->mState; }

off_t Queue::byteSize() const {
  struct stat st;
  if (stat(this->path, &st) == 0) {
    return st.st_size;
  }
  return -1;
}

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
