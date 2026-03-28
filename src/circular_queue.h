#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdint.h>
#include <ostream>
#include <array>
#include "file_handle.h"
#include "queue.h"

#define CIRCULAR_QUEUE_CAPACITY 6

struct CQueueMetaData {
  int32_t head;
  int32_t tail;
  uint16_t itemSize;
  uint16_t capacity;
};

class CQueue {
public:
  CQueue() = default;
  CQueue(const char *name, const char *path = nullptr);
  CQueue(const char *name, uint16_t itemSize, uint16_t capacity, const char *path = nullptr);
  ~CQueue();

  template <typename T> bool head(T *buffer, uint16_t bufferLen, bool dequeue = false) {

    errno = 0;
    const char *valid_path = resolvePath();
    if (valid_path == nullptr) {
      return false;
    }

    if (buffer == nullptr) {
      PRINT("buffer is empty\n");
      return false;
    }

    if (bufferLen < mState.itemSize) {
      PRINT("Error: buffer len is smaller than CQueue itemsize\n");
      return false;
    }

    if (mState.head == -1) {
      PRINT("Error: CQueue is empty %d %" PRIi32 "\n", mState.itemSize, mState.tail);
      return false;
    }

    if (!isAvailable) {
      return false;
    }

    FILE *fd = nullptr;
    FileHandle file{valid_path, "rb"};
    fd = file.getFile();

    if (fd == nullptr) {
      return false;
    }
    if (fseek(fd, (static_cast<long>(mState.head) * mState.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
      PRINT("Error: fseek failed errno=%d\n", errno);
      return false;
    }
    size_t readCount = fread(buffer, mState.itemSize, 1, fd);
    if (readCount != 1) {
      if (feof(fd)) {
        PRINT("Error: fread failed — reached end of file unexpectedly\n");
        PRINT("       file pos=%ld itemSize=%" PRIi32 "\n", ftell(fd), mState.itemSize);
      } else if (ferror(fd)) {
        PRINT("Error: fread failed — file error errno=%d\n", errno);
      } else {
        PRINT("Error: fread failed — partial read, readCount=%zu\n", readCount);
      }
      return false;
    }
    if (dequeue) {
      if (mState.head == mState.tail) {
        mState.head = mState.tail = -1;
      } else {
        mState.head = (mState.head + 1) % mState.capacity;
      }
      updateState(); // must be called
    }
    return true;
  }

  template <typename T> bool enqueue(T *object, uint16_t objectLen) {
    FILE *fd = nullptr;
    const char *valid_path = resolvePath();
    if (valid_path == nullptr) {
      return false;
    }

    if (objectLen > mState.itemSize) {
      PRINT("Error: buffer is larger than CQueue ItemSize\n");
      return false;
    }

    if ((mState.tail + 1) % mState.capacity == mState.head) {
      this->dequeue();
    }

    if (mState.head == -1) {
      mState.head = mState.tail = 0;
    } else {
      mState.tail = (mState.tail + 1) % mState.capacity;
    }

    if (!isAvailable) {
      PRINT("Error: not Available\n");
      return false;
    }

    FileHandle file{valid_path, "r+b"};
    fd = file.getFile();
    if (fd == nullptr) {
      PRINT("Error: fd == nullptr\n");
      return false;
    }
    if (fseek(fd, (mState.tail * mState.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
      PRINT("Error: fseek failed errno=%d\n", errno);
      return false;
    }
    size_t res = fwrite(object, objectLen, 1, fd);
    // if the written size is less than the queue initialized item size, then fill it with zero
    if (res != 1) {
      PRINT("Error: fhat the wuck\n");
    }
    if (objectLen < mState.itemSize && res == 1) {
      char wasted = 0;
      auto expected_size = mState.itemSize - objectLen;
      printf("%d %d %d\n", mState.itemSize, objectLen, expected_size);
      if (fwrite(&wasted, 1, expected_size, fd) == expected_size) {
        file.explicitClose();
        updateState();
        return true;
      }
    }
    return false;
  }

  // void CQueue::printer() {
  //   FILE *fd = nullptr;
  //   errno = 0;
  //   char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  //   FileHandle file{validPath, "rb"};
  //   fd = file.getFile();
  //   PRINT("asdfasdfasdfasfdasdfasfdsadfasf %d %d %p\n", this->mState.head, this->isAvailable, fd);
  //   // head == -1 means queue is empty
  //   if (this->mState.head == -1) {
  //     PRINT("the circular queue is empty %d %" PRIi32 "\n", this->mState.itemSize, this->mState.tail);
  //   }
  //
  //   if (this->isAvailable && fd != nullptr) {
  //     auto state = this->getState();
  //     char buffer[state.itemSize]{0};
  //     while (state.head != -1 || state.tail != -1) {
  //       fseek(fd, state.head * state.itemSize + sizeof(CQueueMetaData), SEEK_SET);
  //       fread(static_cast<char *>(buffer), state.itemSize, 1, fd);
  //       PRINT("%d %s\n", state.head, buffer);
  //       if (state.head == state.tail) {
  //         state.head = state.tail = -1;
  //       } else {
  //         state.head = (state.head + 1) % state.capacity;
  //       }
  //     }
  //   }
  // }
  template <typename T, typename Formatter> void printer(T *buffer, Formatter fmt) {
    FILE *fd = nullptr;
    errno = 0;
    const char *valid_path = resolvePath();
    if (valid_path == nullptr) {
      printf("FAILED TO RESOLVE path\n");
      return;
    }

    FileHandle file{valid_path, "rb"};
    fd = file.getFile();
    // head == -1 means queue is empty
    if (mState.head == -1) {
      PRINT("the circular queue is empty %d %" PRIi32 "\n", mState.itemSize, mState.tail);
    }

    if (isAvailable && fd != nullptr) {
      auto state = getState();

      while (state.head != -1 || state.tail != -1) {
        // PRINT("---- Stats: %d %d %p\n", state.head, state.tail, isAvailable, fd);
        if (fseek(fd, (static_cast<long>(state.head) * state.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
          return;
        }
        // fread(buffer, state.itemSize, 1, fd);
        if (fread(buffer, state.itemSize, 1, fd) != 1) {
          // PRINT("End is %d %d\n", state.head, state.tail);
          return;
        }
        fmt(state.head, buffer);
        if (state.head == state.tail) {
          state.head = state.tail = -1;
        } else {
          state.head = (state.head + 1) % state.capacity;
        }
      }
    }
  }

  bool dequeue();
  CQueueMetaData getState() const;
  const char *resolvePath() const;

private:
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  CQueueMetaData mState{};
  bool isAvailable = false;
  bool updateState();
};

#endif // CIRCULAR_QUEUE_H
