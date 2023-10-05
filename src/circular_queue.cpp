#include <inttypes.h>
#include "circular_queue.h"
#include <string.h>
#include "circular_queue.h"
#include "file_handle.h"

CQueue::CQueue(const char *name, uint16_t itemSize, uint16_t capacity, const char *path) {
  FILE *fd = nullptr;
  strcpy(this->name, name);

  if (path != nullptr)
    strcpy(this->path, path);
  char *validName = path == nullptr ? this->name : this->path;
  struct stat st;

  if (stat(validName, &st) == 0) {
    FileHandle file{validName, "rb"};
    fd = file.getFile();
    printf("%s circular queue existed! \n", validName);
    errno = 0;
    if (fd) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        if (this->mState.capacity != capacity || this->mState.itemSize != itemSize)
          printf("Warning: Queue exsited with different itemSize and/or capacity\n");
        printf(":::::::::::::::::::::::::::::::::::::%" PRIi32 " %" PRIi32 " %" PRIu16 "%" PRIu16 "\n", mState.head, mState.tail,
               mState.itemSize, mState.capacity);
        isAvailable = true;
      } else {
        printf("::::::::::::::::::::::::::::::::::::: %s\n", strerror(errno));
      }
    }
  } else if (itemSize != 0 && capacity != 0) {
    FileHandle file{validName, "wb"};
    fd = file.getFile();
    if (fd) {
      this->mState = (CQueueMetaData){// todo prevent updating itemSize
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
    printf("Error: Invalid size for item and capacity\n");
    return;
  }
}

CQueue::CQueue(const char *name, const char *path) {
  FILE *fd = nullptr;
  strcpy(this->name, name);
  if (path != nullptr)
    strcpy(this->path, path);
  char *validName = path == nullptr ? this->name : this->path;
  struct stat st;
  if (stat(validName, &st) == 0) {
    printf("your same circular queue existed!\n");
    FileHandle file{validName, "rb"};
    fd = file.getFile();
    if (fd) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    printf("Error: No such Circular Queue\n");
    return;
  }
}

CQueue::~CQueue() {}

bool CQueue::enqueue(const char *buffer, size_t buffer_len) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if (buffer_len > this->mState.itemSize) {
    printf("Error: buffer is larger than CQueue ItemSize\n");
    return false;
  }

  if ((this->mState.tail + 1) % this->mState.capacity == this->mState.head) {
    uint8_t data;
    this->dequeue();
  }

  if (this->mState.head == -1) {
    this->mState.head = this->mState.tail = 0;
  } else {
    this->mState.tail = (this->mState.tail + 1) % this->mState.capacity;
  }
  FileHandle file{validPath, "r+b"};
  fd = file.getFile();
  if (this->isAvailable && fd) {
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

void CQueue::updateState(FILE *f) {
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  FileHandle file{validPath, "r+b"};
  FILE *fd = file.getFile();

  if (fd) {
    printf("HERE %s %" PRIi32 " %" PRIi32 "\n", validPath, this->mState.tail, this->mState.head);
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(CQueueMetaData), 1, fd);
  } else {
    printf("Failed to update the queue status\n");
  }
}

bool CQueue::head(char *buffer, uint16_t bufferLen, bool dequeue) {
  FILE *fd = nullptr;
  errno = 0;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (buffer == nullptr) {
    printf("buffer is empty\n");
    return false;
  }
  if (bufferLen < this->mState.itemSize) {
    printf("Error: buffer len is smaller than CQueue itemsize\n");
    return false;
  }

  if (this->mState.head == -1) {
    printf("the circular queue is empty %d %" PRIi32 "\n", this->mState.itemSize, this->mState.tail);
    return false;
  }
  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  if (this->isAvailable && fd) {
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
    printf("the circular queue is empty, nothing to dequeue\n");
    return false;
  }
  FileHandle file{validPath, "rb"};
  fd = file.getFile();
  if (this->isAvailable && fd) {
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

CQueueMetaData CQueue::getState() const {
  // printf("Here2: %d %d %d %d\n", mState.capacity, mState.head, mState.itemSize, mState.tail);
  return this->mState;
}

// void CQueue::printer() {
//   for (int i = 0; i < CIRCULAR_QUEUE_CAPACITY; i++)
//     std::cout << (int)this->queue[i] << " ,";
//   std::cout << std::endl;
// }