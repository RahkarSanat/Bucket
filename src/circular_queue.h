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
    if (fread(buffer, mState.itemSize, 1, fd) != 1) {
      PRINT("Error: fread failed - errno=%d\n", errno);
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
      return false;
    }

    FileHandle file{valid_path, "r+b"};
    fd = file.getFile();
    if (fd == nullptr) {
      return false;
    }
    if (fseek(fd, (mState.tail * mState.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
      PRINT("Error: fseek failed errno=%d\n", errno);
      return false;
    }
    size_t res = fwrite(object, objectLen, 1, fd);
    // if the written size is less than the queue initialized item size, then fill it with zero
    if (objectLen < mState.itemSize && res == 1) {
      char wasted = 0;
      auto expected_size = mState.itemSize - objectLen - 1;
      if (fwrite(&wasted, 1, expected_size, fd) == expected_size) {
        file.explicitClose();
        updateState();
        return true;
      }
    }
    return false;
  }

  template <typename T, typename Formatter> void printer(T *buffer, Formatter fmt) {
    FILE *fd = nullptr;
    errno = 0;
    const char *valid_path = resolvePath();
    if (valid_path == nullptr) {
      return;
    }

    FileHandle file{valid_path, "rb"};
    fd = file.getFile();
    PRINT("---- Stats: %d %d %p\n", mState.head, isAvailable, fd);
    // head == -1 means queue is empty
    if (mState.head == -1) {
      PRINT("the circular queue is empty %d %" PRIi32 "\n", mState.itemSize, mState.tail);
    }

    if (isAvailable && fd != nullptr) {
      auto state = getState();

      while (state.head != -1 || state.tail != -1) {
        if (fseek(fd, (static_cast<long>(state.head) * state.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
          return;
        }
        if (fread(buffer, state.itemSize, 1, fd) != 1) {
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
  void updateState();
};

#endif // CIRCULAR_QUEUE_H
