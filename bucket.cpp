#include "bucket.h"
#include <cstring>
#include <errno.h>
#include <cstdlib>

Queue::Queue(const char *name) {
  strcpy(this->name, name);
  struct stat st;
  if (stat(this->name, &st) == 0) {
    printf("your same queue existed!\n");
    if ((fd = fopen(name, "rb"))) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    if ((fd = fopen(name, "wb"))) {
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

Queue::~Queue() {
  if (fd) {
    fclose(fd);
    fd = nullptr;
  }
}

void Queue::updateState() {
  FILE *fd;
  if ((fd = fopen(this->name, "r+b"))) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(QueueMetaData), 1, fd);
    fclose(fd);
  } else {
    printf("Failed to update the queue status\n");
  }
}

void Queue::enqueue(char *buffer, size_t buffer_len) {
  // enqueu data
  if (this->isAvailable && (fd = fopen(this->name, "r+b"))) {
    int reds = fseek(fd, mState.tail, SEEK_SET);
    QueueItem item = {.bytesLen = buffer_len, .index = mState.count + 1};
    this->mState.tail += fwrite(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd) * (sizeof(size_t) + sizeof(uint16_t));
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    this->updateState();
    fclose(fd);
    fd = nullptr;
  }
}

bool Queue::dequeue(char *buffer, size_t *itemLen) {
  // open file
  QueueItem item;
  if (this->isAvailable && (fd = fopen(this->name, "rb"))) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd);
    if (this->mState.head == this->mState.tail) {
      return false;
    }
    if (!buffer) {
      *itemLen = item.bytesLen;
      fclose(fd);
      fd = nullptr;
      return true;
    }
    fseek(fd, this->mState.head + (sizeof(size_t) + sizeof(uint16_t)), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      this->mState.head += item.bytesLen + (sizeof(size_t) + sizeof(uint16_t));
      fclose(fd);
      fd = nullptr;
      updateState();
      return true;
    } else
      printf("failed to read one item\n");
    return false;
  }
}

bool Queue::at(uint16_t index, char *buffer, size_t *itemLen) {
  QueueItem item;
  FILE *fd = nullptr;
  if ((fd = fopen(this->name, "rb"))) {
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
      } else {
        printf("not found yet %d %d %d\n", item.index, item.bytesLen, this->mState.tail);
      }
      currentPos += item.bytesLen + (sizeof(size_t) + sizeof(uint16_t));
    }
    fclose(fd);
  } else {
    printf("Failed to read the queue\n");
  }

  return false;
}

int Bucket::mkdirp(const char *path, mode_t mode) {
  char *p = NULL;
  char *tmp = strdup(path);
  int len = strlen(tmp);
  int temp = umask(0);
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
    printf("::::::::::::::::::::::\n");
    free(tmp);
    return -1;
  }

  free(tmp);
  return 0;
}

Queue Bucket::getQueue(const char *name) {
  char fullPath[100] = {0};
  snprintf(fullPath, 100, "%s/%s", this->mBucketPath, name);
  return Queue{fullPath};
}

void Bucket::init(const char *path) {
  std::strcpy(this->mBucketPath, path);
  if (mkdirp(path, 0777) != 0) {
    printf("Failed to create active trip directory\n");
  }
}
