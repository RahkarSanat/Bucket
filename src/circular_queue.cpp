#include <iostream>
#include <circular_queue.h>
#include <string.h>

CQueue::CQueue(const char *name, const char *path, uint16_t itemSize, uint16_t capacity) {
  FILE *fd = nullptr;
  strcpy(this->name, name);
  if (path != nullptr)
    strcpy(this->path, path);
  char *validName = path == nullptr ? this->name : this->path;
  struct stat st;
  if (stat(validName, &st) == 0) {
    printf("your same circular queue existed!\n");
    if ((fd = fopen(validName, "rb"))) {
      fseek(fd, 0L, SEEK_SET); // at the file begining
      if (fread(&this->mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  } else {
    if ((fd = fopen(validName, "wb"))) {
      this->mState = (CQueueMetaData){// todo prevent updating itemSize
                                      .head = -1,
                                      .tail = -1,
                                      .itemSize = itemSize,
                                      .capacity = capacity};
      if (fwrite(&mState, sizeof(CQueueMetaData), 1, fd) == 1) {
        isAvailable = true;
      }
    }
  }
  if (fd != nullptr) {
    fclose(fd);
    fd = nullptr;
  }
}
CQueue::~CQueue() {}

bool CQueue::enqueue(const char *buffer, size_t buffer_len) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if ((this->mState.tail + 1) % this->mState.capacity == this->mState.head) {
    uint8_t data;
    this->dequeue();
  }

  if (this->mState.head == -1) {
    this->mState.head = this->mState.tail = 0;
  } else {
    this->mState.tail = (this->mState.tail + 1) % this->mState.capacity;
  }
  if (this->isAvailable && (fd = fopen(validPath, "r+b"))) {
    int reds = fseek(fd, this->mState.tail * this->mState.itemSize, SEEK_SET);
    size_t res = fwrite(buffer, buffer_len, 1, fd);
    if (buffer_len < this->mState.itemSize && res == 1) {
      char wasted = 0;
      int ddd = 0;
      if ((ddd = fwrite(&wasted, 1, this->mState.itemSize - buffer_len - 1, fd)) == this->mState.itemSize - buffer_len - 1) {
        // printf("Enqueu the rest %d %d\n", this->mState.itemSize - buffer_len - 1, ddd);
        // this->mState.tail++;
        fclose(fd);
        this->updateState();
        return true;
      }
    }
  }
  if (fd) {
    fclose(fd);
  }
  return false;
}

void CQueue::updateState() {
  FILE *fd;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;

  if ((fd = fopen(validPath, "r+b"))) {
    fseek(fd, 0, SEEK_SET);
    fwrite(&this->mState, sizeof(CQueueMetaData), 1, fd);
    fclose(fd);
  } else {
    printf("Failed to update the queue status\n");
  }
}

bool CQueue::head(char *buffer, bool dequeue) {
  FILE *fd = nullptr;
  char *validPath = strlen(this->path) == 0 ? this->name : this->path;
  if (buffer == nullptr) {
    printf("buffer is empty\n");
    return false;
  }
  if (this->mState.head == -1) {
    printf("the circular queue is empty\n");
    return false;
  }
  if (this->isAvailable && (fd = fopen(validPath, "rb"))) {
    fseek(fd, this->mState.head * this->mState.itemSize, SEEK_SET);
    fread(buffer, this->mState.itemSize, 1, fd);
    fclose(fd);
    if (dequeue) {
      if (this->mState.head == this->mState.tail) {
        this->mState.head = this->mState.head = -1;
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
  if (this->isAvailable && (fd = fopen(validPath, "rb"))) {
    // if (dequeue) {
    if (this->mState.head == this->mState.tail) {
      this->mState.head = this->mState.head = -1;
    } else {
      this->mState.head = (this->mState.head + 1) % this->mState.capacity;
    }
    updateState(); // must be called
    // }
    return true;
  }
  return false;
}

// void CQueue::printer() {
//   for (int i = 0; i < CIRCULAR_QUEUE_CAPACITY; i++)
//     std::cout << (int)this->queue[i] << " ,";
//   std::cout << std::endl;
// }