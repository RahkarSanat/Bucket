#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <cstddef>
#include "queue.h"
#include "bucket.h"
#include "file_handle.h"

inline bool iter_max(size_t counter) { return counter < 1000; }

Queue::Queue(const char *name, const char *path) {
  FILE *fd = nullptr;
  strncpy(this->name, name, QUEUE_NAME_MAX_LENGTH - 1);
  if (path != nullptr) {
    strncpy(this->path, path, sizeof(this->path) - 1);
  }
  const char *valid_name = resolvePath();
  if (valid_name == nullptr) {
    return;
  }
  struct stat st = {};
  if (stat(valid_name, &st) == 0) {
    FileHandle file{valid_name, "rb"};
    fd = file.getFile();
    if (fd != nullptr) {
      fseek(fd, 0L, SEEK_SET);
      if (fread(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    FileHandle file{valid_name, "wb"};
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

const char *Queue::resolvePath() const {
  const char *valid = (strlen(path) > 0) ? path : name;
  if (valid == nullptr || strlen(valid) == 0) {
    return nullptr;
  }
  return valid;
}

bool Queue::updateState() {
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return false;
  }
  FileHandle file{valid_path, "r+b"};
  FILE *fd = file.getFile();
  if (fd != nullptr) {
    fseek(fd, 0, SEEK_SET);
    if (fwrite(&this->mState, sizeof(QueueMetaData), 1, fd) == 1) {
      return true;
    }
  }
  return false;
}

bool Queue::updateState(FILE *fd) {
  if (fd == nullptr) {
    return false;
  }
  fseek(fd, 0, SEEK_SET);
  return fwrite(&this->mState, sizeof(QueueMetaData), 1, fd) == 1;
}

bool Queue::enqueue(const char *buffer, size_t buffer_len) {
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return false;
  }
  FileHandle file{valid_path, "r+b"};
  FILE *fd = file.getFile();
  if (this->isAvailable && fd != nullptr) {
    fseek(fd, mState.tail, SEEK_SET);
    QueueItem item = {.bytesLen = buffer_len, .index = static_cast<uint16_t>(this->mState.count + 1), .check = 0};
    this->mState.tail += fwrite(&item, sizeof(QueueItem), 1, fd) * sizeof(QueueItem);
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    this->mState.tail += res * buffer_len;
    this->mState.count += res;
    updateState(fd);
    return res == 1;
  }
  return false;
}

QueueItem Queue::head(char *buffer, size_t *itemLen, bool dequeue) {
  QueueItem item{};
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    item.check = 2;
    return item;
  }
  const char *mode = dequeue ? "r+b" : "rb";
  FileHandle file{valid_path, mode};
  FILE *fd = file.getFile();
  if (this->isAvailable && fd != nullptr) {
    fseek(fd, this->mState.head, SEEK_SET);
    fread(&item, sizeof(QueueItem), 1, fd);

    // return if the queue is empty with failed indication item.check = 2
    if (this->mState.head == this->mState.tail) {
      item.check = 2;
      return item;
    }
    // set the itemLen if it is not null, so it is asked
    if (itemLen != nullptr) {
      *itemLen = item.bytesLen;
    }

    // if the buffer is null, return
    if (buffer == nullptr) {
      return item;
    }
    // read actual data
    fseek(fd, this->mState.head + sizeof(QueueItem), SEEK_SET);
    if (fread(buffer, item.bytesLen, 1, fd) == 1) {
      if (dequeue) {
        // setting the QueueItem.check
        fseek(fd, this->mState.head, SEEK_SET);
        item.check = 1;
        if (fwrite(&item, sizeof(QueueItem), 1, fd) == 1) {
          this->mState.head += item.bytesLen + sizeof(QueueItem);
          ++this->mState.index;
          updateState(fd);
        }
      }
      return item;
    } else {
      printf("failed to read one item with len of: %zu\n", item.bytesLen);
    }
  }
  item.check = 2;
  return item;
}

bool Queue::dequeue(const size_t itemLen) {
  QueueItem item{};
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return false;
  }
  errno = 0;
  FileHandle file{valid_path, "r+b"};
  FILE *fd = file.getFile();
  if (fd != nullptr) {
    item.check = 1;
    fseek(fd, this->mState.head + offsetof(QueueItem, check), SEEK_SET);
    if (fwrite(&item.check, sizeof(item.check), 1, fd) == 1) {
      this->mState.head += itemLen + sizeof(QueueItem);
      this->mState.index++;
      updateState(fd);
      return true;
    }
  }
  printf("Failed to dequeue: cause: %d\n", errno);
  return false;
}

QueueItem Queue::at(uint16_t index, char *buffer, size_t *itemLen) {
  QueueItem item{};
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    item.check = 2;
    return item;
  }
  FileHandle file{valid_path, "rb"};
  FILE *fd = file.getFile();
  size_t iter_max_counter = 0;
  if (fd != nullptr) {
    size_t current_pos = sizeof(QueueMetaData);
    while (current_pos <= this->mState.tail && iter_max(iter_max_counter++)) {
      fseek(fd, current_pos, SEEK_SET);
      std::memset(&item, 0, sizeof(item));
      fread(&item, sizeof(QueueItem), 1, fd);
      if (item.index == index) {
        if (itemLen != nullptr) {
          *itemLen = item.bytesLen;
        }
        if (buffer != nullptr) {
          fread(buffer, item.bytesLen, 1, fd);
        }
        return item;
      }
      current_pos += item.bytesLen + sizeof(QueueItem);
    }
  }

  item.check = 2;
  return item;
}

QueueItem Queue::at(uint16_t index) { return at(index, nullptr, nullptr); }

bool Queue::isEmpty() const { return this->mState.count == 0; }

bool Queue::rename(const char *newName, const Bucket *bucket) {
  if (bucket == nullptr || newName == nullptr) {
    return false;
  }
  char dst[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return false;
  }

  snprintf(dst, sizeof(dst), "%s/%s", bucket->getPath(), newName);
  errno = 0;
  if (::rename(valid_path, dst) == 0) {
    strncpy(this->path, dst, sizeof(this->path) - 1);
    this->path[sizeof(this->path) - 1] = '\0';
    return true;
  }
  return false;
}

bool Queue::move(const Bucket *other) { return this->rename(this->getName(), other); }

const char *Queue::getName() const { return this->name; }
const char *Queue::getPath() const { return this->path; }

const QueueMetaData *const Queue::getMetaData() const { return &this->mState; }

off_t Queue::byteSize() const {
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return -1;
  }
  struct stat st;
  if (stat(valid_path, &st) == 0) {
    return st.st_size;
  }
  return -1;
}
