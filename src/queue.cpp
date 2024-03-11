#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <cstddef>
#include "queue.h"
#include "bucket.h"
#include "file_handle.h"

inline bool iter_max(size_t counter) { return counter < 1000; }

using QueueItemType = QueueItem;

Queue::Queue(const char *name, const char *path) {
  FILE *fd = nullptr;
  strcpy(this->name, name);
  if (path != nullptr) {
    strcpy(this->path, path);
  }
  char *validName = path == nullptr ? this->name : this->path;
  struct stat st = {0};
  if (stat(validName, &st) == 0) {
    FileHandle file{validName, "rb"};
    fd = file.getFile();
    printf("your same queue existed!\n");
    if (fd != nullptr) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    FileHandle file{validName, "wb"};
    fd = file.getFile();
    if (fd != nullptr) {
      this->mState = (QueueMetaData){.head = sizeof(QueueMetaData), .tail = sizeof(QueueMetaData), .count = 0, .index = 0};
      if (fwrite(&mState, sizeof(QueueMetaData), 1, fd)) {
        isAvailable = true;
      }
    }
  }
}

Queue::Queue() {}

Queue::~Queue() {}

void Queue::updateState() {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  FileHandle file{validPath, "r+b"};
  fd = file.getFile();
  if (fd != nullptr) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(QueueMetaData), 1, fd);
  } else {
    printf("Failed to update the queue status\n");
  }
}

void Queue::enqueue(const char *buffer, size_t buffer_len) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "r+b"};
  fd = file.getFile();
  if (this->isAvailable && fd) {
    int reds = fseek(fd, mState.tail, SEEK_SET);
    QueueItem item = {.bytesLen = buffer_len, .index = static_cast<uint16_t>(this->mState.count + (1)), .check = 0};
    this->mState.tail += fwrite(&item, (sizeof(QueueItem)), 1, fd) * ((sizeof(QueueItem)));
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    file.explicitClose();
    fd = nullptr;
    this->updateState();
  }
}

QueueItem Queue::head(char *buffer, size_t *itemLen, bool dequeue) {
  FILE *fd = nullptr;
  QueueItem item{};
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  if (this->isAvailable && fd) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, ((sizeof(QueueItem))), 1, fd);

    // return if the queue is empty with failed indication item.check = 2
    if (this->mState.head == this->mState.tail) {
      item.check = 2;
      return item;
    }
    // set the itemLen if it is not null, so it is asked
    if (itemLen) {
      *itemLen = item.bytesLen;
    }

    // if the buffer is null, return
    if (!buffer) {
      return item;
    }
    // read actual data
    fseek(fd, this->mState.head + ((sizeof(QueueItem))), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      if (dequeue) {
        // setting the QueueItem.check
        fseek(fd, this->mState.head, SEEK_SET);
        item.check = 1;
        if (fwrite(&item, sizeof(QueueItem), 1, fd) == 1) {
          this->mState.head += item.bytesLen + ((sizeof(QueueItem)));
          ++this->mState.index;
          file.explicitClose();
          fd = nullptr;
          updateState();
        }
      }
      return item;
    } else
      printf("failed to read one item with len of: %d\n", item.bytesLen);
  }
  item.check = 2;
  return item;
}

bool Queue::dequeue(const size_t itemLen) {
  QueueItem item{};
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  errno = 0;
  FileHandle file{validPath, "r+b"};
  fd = file.getFile();
  if (fd != nullptr) {
    item.check = 1;
    fseek(fd, this->mState.head + offsetof(QueueItem, check), SEEK_SET);
    if (fwrite(&item.check, sizeof(item.check), 1, fd) == 1) {
      this->mState.head += itemLen + sizeof(QueueItem);
      this->mState.index++;
      file.explicitClose();
      fd = nullptr;
      updateState();
      return true;
    }
    // }
  }
  printf("Failed to dequeue: cause: %s\n", strerror(errno));
  return false;
}

QueueItem Queue::at(uint16_t index, char *buffer, size_t *itemLen) {
  QueueItem item{};
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "rb"};
  FILE *fd = file.getFile();
  size_t iterMaxCounter = 0;
  if (fd != nullptr) {
    // loop to find the index
    size_t currentPos = sizeof(QueueMetaData);
    while (currentPos <= this->mState.tail && iter_max(iterMaxCounter++)) {
      fseek(fd, currentPos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, ((sizeof(QueueItem))), 1, fd);
      if (item.index == index) {
        printf("Found the Item at %d %d \n", item.index, item.bytesLen);
        fread(buffer, item.bytesLen, 1, fd);
        return item;
      }
      currentPos += item.bytesLen + ((sizeof(QueueItem)));
    }
  } else {
    printf("Failed to read the queue\n");
  }

  item.check = 2;
  return item;
}

QueueItem Queue::at(uint16_t index) {
  QueueItem item{};
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "rb"};
  FILE *fd = file.getFile();
  size_t iterMaxCounter = 0;
  if (fd != nullptr) {
    // loop to find the index
    size_t currentPos = sizeof(QueueMetaData);
    while (currentPos <= this->mState.tail && iter_max(iterMaxCounter++)) {
      fseek(fd, currentPos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, ((sizeof(QueueItem))), 1, fd);
      if (item.index == index) {
        return item;
      }
      currentPos += item.bytesLen + ((sizeof(QueueItem)));
    }
  } else {
    printf("No such Queue!\n");
  }
  printf("no Queue with provided index\n");
  item.check = 2;
  return item;
}

// bool Queue::update(const QueueItem &itemProp) {
//   // find the item in the queue with its itemProp.index

//   // replace this itemProp with the old one
// }
// void Queue::update(const QueueItem &itemProp, const char *data) {
//   // Not implemented yet
// }

bool Queue::isEmpty() const { return this->mState.count == 0; }

bool Queue::rename(const char *newName, const Bucket *bucket) {
  char dst[100] = {0};
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if (bucket != nullptr) {
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
