#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <cstddef>
#include "queue.h"
#include "bucket.h"

using QueueItemType = QueueItem;

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

Queue::Queue() {}

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
    QueueItem item = {.bytesLen = buffer_len, .index = static_cast<uint16_t>(this->mState.count + (1)), .check = 0};
    this->mState.tail += fwrite(&item, (sizeof(QueueItem)), 1, fd) * ((sizeof(QueueItem)));
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    fclose(fd);
    fd = nullptr;
    this->updateState();
  }
}

QueueItem Queue::head(char *buffer, size_t *itemLen, bool dequeue) {
  FILE *fd = nullptr;
  QueueItem item;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (this->isAvailable && (fd = fopen(validPath, "rb"))) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, ((sizeof(QueueItem))), 1, fd);
    if (this->mState.head == this->mState.tail) {
      fclose(fd);
      item.check = 2;
      return item;
    }
    if (itemLen) {
      *itemLen = item.bytesLen;
    }
    if (!buffer) {
      fclose(fd);
      return item;
    }
    fseek(fd, this->mState.head + ((sizeof(QueueItem))), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      fclose(fd);
      if (dequeue) {
        // setting the QueueItem.check
        fseek(fd, this->mState.head, SEEK_SET);
        item.check = 1;
        if (fwrite(&item, sizeof(QueueItem), 1, fd) == 1) {
          this->mState.head += item.bytesLen + ((sizeof(QueueItem)));
          ++this->mState.index;
          updateState();
        }
      }
      return item;
    } else
      printf("failed to read one item with len of: %d\n", item.bytesLen);
    fclose(fd);
  }
  item.check = 2;
  return item;
}

bool Queue::dequeue(const size_t itemLen) {
  QueueItem item;
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  errno = 0;
  if ((fd = fopen(validPath, "r+b"))) {
    // fseek(fd, this->mState.head, SEEK_SET);
    // if (fread(&item, ((sizeof(QueueItem))), 1, fd) == 1) {
    item.check = 1;
    fseek(fd, this->mState.head + offsetof(QueueItem, check), SEEK_SET);
    if (fwrite(&item.check, sizeof(item.check), 1, fd) == 1) {
      this->mState.head += itemLen + sizeof(QueueItem);
      this->mState.index++;
      updateState();
      fclose(fd);
      return true;
    }
    // }
  }
  printf("Failed to dequeue: cause: %s\n", strerror(errno));
  if (fd != nullptr) {
    fclose(fd);
  }
  return false;
}

QueueItem Queue::at(uint16_t index, char *buffer, size_t *itemLen) {
  QueueItem item;
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if ((fd = fopen(validPath, "rb"))) {
    // loop to find the index
    size_t currentPos = sizeof(QueueMetaData);
    while (currentPos <= this->mState.tail) {
      fseek(fd, currentPos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, ((sizeof(QueueItem))), 1, fd);
      if (item.index == index) {
        printf("Found the Item at %d %d \n", item.index, item.bytesLen);
        fread(buffer, item.bytesLen, 1, fd);
        fclose(fd);
        return item;
      }
      currentPos += item.bytesLen + ((sizeof(QueueItem)));
    }
    fclose(fd);
  } else {
    printf("Failed to read the queue\n");
  }

  item.check = 2;
  return item;
}

QueueItem Queue::at(uint16_t index) {
  QueueItem item;
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if ((fd = fopen(validPath, "rb"))) {
    // loop to find the index
    size_t currentPos = sizeof(QueueMetaData);
    while (currentPos <= this->mState.tail) {
      fseek(fd, currentPos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, ((sizeof(QueueItem))), 1, fd);
      if (item.index == index) {
        return item;
      }
      currentPos += item.bytesLen + ((sizeof(QueueItem)));
    }
    fclose(fd);
  } else {
    printf("No such Queue!\n");
  }
  printf("no Queue with provided index\n");
  item.check = 2;
  return item;
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
const char *Queue::getPath() const { return this->path; }

const QueueMetaData *const Queue::getMetaData() const { return &this->mState; }

off_t Queue::byteSize() const {
  struct stat st;
  if (stat(this->path, &st) == 0) {
    return st.st_size;
  }
  return -1;
}
