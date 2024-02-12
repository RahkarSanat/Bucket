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
  char *validName = path == nullptr ? static_cast<char *>(this->name) : static_cast<char *>(this->path);
  struct stat st {};

  if (stat(validName, &st) == 0) {
    FileHandle file{validName, "rb"};
    fd = file.getFile();
    PRINT("%s circular queue existed! \n", validName);
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
    FileHandle file{validName, "wb"};
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
  char *validName = path == nullptr ? static_cast<char *>(this->name) : static_cast<char *>(this->path);
  struct stat st {};
  if (stat(validName, &st) == 0) {
    PRINT("your same circular queue existed!\n");
    FileHandle file{validName, "rb"};
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

bool CQueue::enqueue(const char *buffer, size_t buffer_len) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if (buffer_len > this->mState.itemSize) {
    PRINT("Error: buffer is larger than CQueue ItemSize\n");
    return false;
  }

  if ((this->mState.tail + 1) % this->mState.capacity == this->mState.head) {
    this->dequeue();
  }

  if (this->mState.head == -1) {
    this->mState.head = this->mState.tail = 0;
  } else {
    this->mState.tail = (this->mState.tail + 1) % this->mState.capacity;
  }
  FileHandle file{validPath, "r+b"};
  fd = file.getFile();
  if (this->isAvailable && fd != nullptr) {
    int reds = fseek(fd, (this->mState.tail * this->mState.itemSize) + sizeof(CQueueMetaData), SEEK_SET);
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    if (buffer_len < this->mState.itemSize && res == 1) {
      char wasted = 0;
      int ddd = 0;
      if ((ddd = fwrite(&wasted, 1, this->mState.itemSize - buffer_len - 1, fd)) == this->mState.itemSize - buffer_len - 1) {
        file.explicitClose();
        this->updateState();
        return true;
      }
    }
  }
  return false;
}

void CQueue::updateState() {
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "r+b"};
  FILE *fd = file.getFile();

  if (fd != nullptr) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(CQueueMetaData), 1, fd);
  } else {
    PRINT("Failed to update the queue status\n");
  }
}

bool CQueue::head(char *buffer, uint16_t bufferLen, bool dequeue) {
  FILE *fd = nullptr;
  errno = 0;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (buffer == nullptr) {
    PRINT("buffer is empty\n");
    return false;
  }
  if (bufferLen < this->mState.itemSize) {
    PRINT("Error: buffer len is smaller than CQueue itemsize\n");
    return false;
  }

  if (this->mState.head == -1) {
    PRINT("the circular queue is empty %d %" PRIi32 "\n", this->mState.itemSize, this->mState.tail);
    return false;
  }

  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  if (this->isAvailable && fd != nullptr) {
    fseek(fd, this->mState.head * this->mState.itemSize + sizeof(CQueueMetaData), SEEK_SET);
    fread(buffer, this->mState.itemSize, 1, fd);
    if (dequeue) {
      if (this->mState.head == this->mState.tail) {
        this->mState.head = this->mState.tail = -1;
      } else {
        this->mState.head = (this->mState.head + 1) % this->mState.capacity;
      }
      updateState(); // must be called
    }
    return true;
  }
  return false;
}

bool CQueue::dequeue() {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (this->mState.head == -1) {
    PRINT("the circular queue is empty, nothing to dequeue\n");
    return false;
  }
  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  if (this->isAvailable && fd != nullptr) {
    // if (dequeue) {
    if (this->mState.head == this->mState.tail) {
      this->mState.head = this->mState.tail = -1;
    } else {
      this->mState.head = (this->mState.head + 1) % this->mState.capacity;
    }
    // fclose(fd);
    updateState(); // must be called
    return true;
  }
  return false;
}

CQueueMetaData CQueue::getState() const { return this->mState; }

void CQueue::printer() {
  FILE *fd = nullptr;
  errno = 0;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  PRINT("asdfasdfasdfasfdasdfasfdsadfasf %d %d %p\n", this->mState.head, this->isAvailable, fd);
  // head == -1 means queue is empty
  if (this->mState.head == -1) {
    PRINT("the circular queue is empty %d %" PRIi32 "\n", this->mState.itemSize, this->mState.tail);
  }

  if (this->isAvailable && fd != nullptr) {
    auto state = this->getState();
    char buffer[state.itemSize]{0};
    while (state.head != -1 || state.tail != -1) {
      fseek(fd, state.head * state.itemSize + sizeof(CQueueMetaData), SEEK_SET);
      fread(static_cast<char *>(buffer), state.itemSize, 1, fd);
      PRINT("%d %s\n", state.head, buffer);
      if (state.head == state.tail) {
        state.head = state.tail = -1;
      } else {
        state.head = (state.head + 1) % state.capacity;
      }
    }
  }
}