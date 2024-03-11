#ifndef RS_BUCKET_QUEUE_H
#define RS_BUCKET_QUEUE_H

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

class Bucket; // forward declaration

#define QUEUE_NAME_MAX_LENGTH 50

struct QueueMetaData {
  size_t head;
  size_t tail;
  uint16_t count;
  uint16_t index;
};

struct QueueItem {
  size_t bytesLen;
  uint16_t index;
  /**
   * \details check could have three values
   *          0: the item is uncheckd: default value
   *          1: the item is checked: the value will be checked after its item dequeu gets dequeue
   *          2: invalid Queue item struct, this value indicates an error
   */
  uint8_t check;
};

class Queue {
public:
  Queue();
  Queue(const char *name, const char *path = nullptr);
  ~Queue();
  void enqueue(const char *buffer, size_t buffer_len);
  bool dequeue(const size_t itemLen);
  QueueItem head(char *buffer, size_t *itemLen = nullptr, bool dequeue = false);
  void tail();
  QueueItem at(uint16_t index, char *buffer, size_t *itemLen = nullptr);
  QueueItem at(uint16_t index);
  bool isEmpty() const;
  bool rename(const char *newName, const Bucket *bucket);
  bool move(const Bucket *other);
  // /**
  //  * @brief update the Queue Item properties of an item in the queue
  //  * @param[in] itemProp: new item prop to be replaced
  //  *
  //  * @return: true if the operation was succesfull, false otherwise
  //  */
  // bool update(const QueueItem &itemProp);
  // void update(const QueueItem &itemProp, const char *data);
  const char *getName() const;
  const char *getPath() const;
  const QueueMetaData *const getMetaData() const;
  off_t byteSize() const;

private:
  char name[QUEUE_NAME_MAX_LENGTH] = {0};
  char path[2 * QUEUE_NAME_MAX_LENGTH] = {0};
  QueueMetaData mState;
  bool isAvailable = false;
  void updateState();
}; //

#ifdef __linux__
template <typename... Args> void PRINT(Args... args) { printf(args...); }
#else
template <typename... Args> void PRINT(Args... args) { ESP_LOGI(args...); }
#endif

#endif // RS_BUCKET_QUEUE_H