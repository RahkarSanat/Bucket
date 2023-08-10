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
      printf("Queue initialized\n");
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
        printf("queue metadata is : %d %d :::\n", mState.head, mState.tail);
        isAvailable = true;
      }
    }
  } else {
    if ((fd = fopen(name, "wb"))) {
      this->mState = (QueueMetaData){.head = sizeof(QueueMetaData), .tail = sizeof(QueueMetaData), .count = 0, .index = 0};
      if (fwrite(&mState, sizeof(QueueMetaData), 1, fd)) {
        printf("create new Queue with metadata\n");
        isAvailable = true;
      }
    }
  }
  fclose(fd);
}

Queue::~Queue() {
  if (!fd) {
    fclose(fd);
    fd = nullptr;
  }
}

void Queue::updateState() {
  FILE *fd;
  if ((fd = fopen(this->name, "r+b"))) {
    printf("trying to write state here: %d %d\n", this->mState.head, this->mState.tail);
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
    printf("))))))) %d\n", ftell(fd));
    QueueItem item = {.bytesLen = buffer_len, .index = mState.count + 1};
    this->mState.tail += fwrite(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd) * (sizeof(size_t) + sizeof(uint16_t));
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    printf("%d ^^^^^^^^^^^^^^^^^^^^^^ %d %d\n", item.bytesLen, item.index, this->mState.tail);
    this->updateState();
    fclose(fd);
  }
}

void Queue::dequeue(char *buffer, size_t *itemLen) {
  // open file
  QueueItem item;
  if (this->isAvailable && (fd = fopen(this->name, "rb"))) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, (sizeof(size_t) + sizeof(uint16_t)), 1, fd);
    if (!buffer) {
      *itemLen = item.bytesLen;
      fclose(fd);
      return;
    }
    printf("item %d size is : %d\n", item.index, item.bytesLen);
    fseek(fd, this->mState.head + (sizeof(size_t) + sizeof(uint16_t)), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      printf("data for item %d is: %s\n", item.index, buffer);
      this->mState.head += item.bytesLen + (sizeof(size_t) + sizeof(uint16_t));
      printf("head at: %d\n", this->mState.head);
      fclose(fd);
      updateState();
    } else
      printf("failed to read one item\n");
  }
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
        printf("::::::::::::::::::::::\n");
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

Queue Bucket::init(const char *path) {
  if (mkdirp(path, 777) == 0) {
  }
}
