#include <inttypes.h>
#include "circular_queue.h"
#include <string.h>
#include "circular_queue.h"
#include "file_handle.h"

CQueue::CQueue(const char *name, uint16_t itemSize, uint16_t capacity, const char *path) {
  FILE *fd = nullptr;
  strcpy(static_cast<char *>(this->name), name);

  if (path != nullptr) {
    strcpy(static_cast<char *>(this->path), path);
  }
  char *valid_name = path == nullptr ? static_cast<char *>(this->name) : static_cast<char *>(this->path);
  struct stat st{};

  if (stat(valid_name, &st) == 0) {
    FileHandle file{valid_name, "rb"};
    fd = file.getFile();
    PRINT("%s circular queue existed! \n", valid_name);
    errno = 0;
    if (fd != nullptr) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        if (this->mState.capacity != capacity || this->mState.itemSize != itemSize) {
          PRINT("Warning: Queue exsited with different itemSize and/or capacity\n");
        }
        isAvailable = true;
      }
    }
  } else if (itemSize != 0 && capacity != 0) {
    FileHandle file{valid_name, "wb"};
    fd = file.getFile();
    if (fd != nullptr) {
      this->mState = (CQueueMetaData){// TODO prevent updating itemSize
                                      .head = -1,
                                      .tail = -1,
                                      .itemSize = itemSize,
                                      .capacity = capacity};
      if (fwrite(&mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        this->isAvailable = true;
      }
    }
  } else {
    this->isAvailable = false;
    PRINT("Error: Invalid size for item and capacity\n");
    return;
  }
}

CQueue::CQueue(const char *name, const char *path) {
  FILE *fd = nullptr;
  strcpy(static_cast<char *>(this->name), name);
  if (path != nullptr) {
    strcpy(static_cast<char *>(this->path), path);
  }
  char *valid_name = path == nullptr ? static_cast<char *>(this->name) : static_cast<char *>(this->path);
  struct stat st{};
  if (stat(valid_name, &st) == 0) {
    PRINT("your same circular queue existed!\n");
    FileHandle file{valid_name, "rb"};
    fd = file.getFile();
    if (fd != nullptr) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    PRINT("Error: No such Circular Queue\n");
    return;
  }
}

CQueue::~CQueue() {}

void CQueue::updateState() {
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return;
  }
  FileHandle file{valid_path, "r+b"};
  FILE *fd = file.getFile();

  if (fd != nullptr) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(CQueueMetaData), 1, fd);
  } else {
    PRINT("Failed to update the queue status\n");
  }
}

bool CQueue::dequeue() {
  FILE *fd = nullptr;
  const char *valid_path = resolvePath();
  if (valid_path == nullptr) {
    return false;
  }
  if (mState.head == -1) {
    PRINT("the circular queue is empty, nothing to dequeue\n");
    return false;
  }
  FileHandle file{valid_path, "rb"};
  fd = file.getFile();
  if (isAvailable && fd != nullptr) {
    if (mState.head == mState.tail) {
      mState.head = mState.tail = -1;
    } else {
      mState.head = (mState.head + 1) % mState.capacity;
    }
    updateState(); // must be called
    return true;
  }
  return false;
}

CQueueMetaData CQueue::getState() const { return this->mState; }

const char *CQueue::resolvePath() const {
  const char *valid = (path != nullptr && strlen(path) > 0) ? path : name;
  if (valid == nullptr || strlen(valid) == 0) {
    PRINT("Error: no valid file path available!\n");
    return nullptr;
  }
  return valid;
}
