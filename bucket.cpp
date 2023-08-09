#include "bucket.h"
#include <cstring>
#include <sys/stat.h>

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
        printf("create new Queue with metadata");
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
  if ((fd = fopen(this->name, "w+b"))) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(QueueMetaData), 1, fd);
  } else {
    printf("Failed to update the queue status");
  }
}

void Queue::enqueue(char *buffer, size_t buffer_len) {
  // enqueu data
  if (this->isAvailable && (fd = fopen(this->name, "w+b"))) {
    int reds = fseek(fd, mState.tail, SEEK_SET);
    printf("))))))) %d ", ftell(fd));
    QueueItem item = {.index = mState.count + 1, .bytesLen = buffer_len};
    this->mState.tail += fwrite(&item, sizeof(QueueItem), 1, fd) * sizeof(QueueItem);
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    this->updateState();
    fclose(fd);
  }
  // adjust the queue status variables
  // close
}

void Queue::dequeue(char *buffer) {
  // open file
  QueueItem item;
  if (this->isAvailable && (fd = fopen(this->name, "rb"))) {
    // loads states : states are up to date anywhere
    // read data
    fseek(fd, this->mState.head, SEEK_SET);
    printf("fseek result is : %d", fread(&item, sizeof(QueueItem), 1, fd));
    printf("item %d size is : %d\n", item.index, item.bytesLen);
    // fseek(fd, this->mState.head + sizeof(QueueItem), SEEK_SET);
    // if (fread(buffer, item.bytesLen, 1, fd) == 1) {
    //   printf("data for item %d is\n%s", item.index, buffer);
    //   this->mState.head += item.bytesLen + sizeof(QueueItem);
    //   updateState();
    // } else
    //   printf("failed to read one item\n");
    fclose(fd);
  }
  // remove data optionally
  // close file
}
