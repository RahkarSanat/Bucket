#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdint.h>
#include "file_handle.h"
#include "queue.h"

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

    const char *mode = dequeue ? "r+b" : "rb";
    FileHandle file{valid_path, mode};
    FILE *fd = file.getFile();

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
      updateState(fd);
    }
    return true;
  }

  template <typename T> bool enqueue(T *object, uint16_t objectLen) {
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
    FILE *fd = file.getFile();
    if (fd == nullptr) {
      PRINT("Error: fd == nullptr\n");
      return false;
    }
    if (fseek(fd, (mState.tail * mState.itemSize) + sizeof(CQueueMetaData), SEEK_SET) != 0) {
      PRINT("Error: fseek failed errno=%d\n", errno);
      return false;
    }
    size_t res = fwrite(object, objectLen, 1, fd);
    if (res != 1) {
      PRINT("Error: fwrite failed in enqueue\n");
      return false;
    }
    // if the written size is less than the queue initialized item size, then fill it with zero
    if (objectLen < mState.itemSize) {
      auto padding_size = mState.itemSize - objectLen;
      // write zeros in chunks to avoid VLA
      char zero_chunk[64] = {0};
      auto remaining = padding_size;
      while (remaining > 0) {
        auto chunk = remaining > sizeof(zero_chunk) ? sizeof(zero_chunk) : remaining;
        if (fwrite(zero_chunk, 1, chunk, fd) != chunk) {
          PRINT("Error: fwrite padding failed\n");
          return false;
        }
        remaining -= chunk;
      }
    }
    updateState(fd);
    return true;
  }

  template <typename T, typename Formatter> void printer(T *buffer, Formatter fmt) {
    errno = 0;
    const char *valid_path = resolvePath();
    if (valid_path == nullptr) {
      return;
    }

    FileHandle file{valid_path, "rb"};
    FILE *fd = file.getFile();
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
  bool updateState();
  bool updateState(FILE *fd);
};

#endif // CIRCULAR_QUEUE_H
