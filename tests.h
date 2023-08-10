#include <cassert>
#include <cstring>
#include "bucket.h"

// #define TEST_ENABLED 1

#define assertm(exp, msg) assert(((void)msg, exp))

void test_enqueu_dequeu() {
  Queue q{"testfiles/test_enqueu_dequeu.txt"};
  q.enqueue("mara bebooooooos, maraaa beboooos", sizeof("mara bebooooooos, maraaa beboooos"));
  q.enqueue("dokhtare zibaa, emshab bar to mehmanam", sizeof("dokhtare zibaa, emshab bar to mehmanam"));

  size_t item_length = 0;
  q.dequeue(nullptr, &item_length);
  assertm(item_length == 34, "Item length retrieved incorrectly");

  char *data = new char[item_length];
  q.dequeue(data, nullptr);

  assertm(strcmp(data, "mara bebooooooos, maraaa beboooos") == 0, "data retrived correcly");
  delete[] data;
}

void test_dequeue_too_much() {
  Queue q{"testfiles/test_dequeue_too_much.txt"};
  q.enqueue("mara bebooooooos, maraaa beboooos", sizeof("mara bebooooooos, maraaa beboooos"));
  q.enqueue("dokhtare zibaa, emshab bar to mehmanam", sizeof("dokhtare zibaa, emshab bar to mehmanam"));

  bool result = false;
  size_t item_length = 0;
  q.dequeue(nullptr, &item_length);
  char *data = new char[item_length];
  result = q.dequeue(data);
  delete[] data;
  assertm(result, "Failed to dequeue item");

  q.dequeue(nullptr, &item_length);
  data = new char[item_length];
  result = q.dequeue(data);
  delete[] data;
  assertm(result, "Failed to dequeue item");

  q.dequeue(nullptr, &item_length);
  data = new char[item_length];
  result = q.dequeue(data);
  delete[] data;
  assertm(result == false, "dequeue item incorrectly");
}

void test_bucket_creation() {
  Bucket bucket{};
  bucket.init("testfiles/test/bucket/created");
  struct stat st;
  assertm(stat("testfiles/test/bucket/created", &st) == 0, "Directory doesnt get created");
}

void test_bucket_get_queue() {
  Bucket bucket{};
  bucket.init("testfiles/test_bucket_get_queue/bucket/created");
  struct stat st;
  assertm(stat("testfiles/test_bucket_get_queue/bucket/created", &st) == 0, "Directory doesnt get created");

  Queue q = bucket.getQueue("0000123");
  char test_data[] = "zade shole dar chaman";
  q.enqueue(test_data, sizeof(test_data));

  size_t item_length = 0;
  q.dequeue(nullptr, &item_length);
  assertm(item_length == sizeof(test_data), "Item length is not true");
  char *data = new char[item_length];
  bool result = q.dequeue(data);
  printf(">>>>>>>>>>>> %s\n", data);

  assertm(strcmp(data, test_data) == 0, "Items are not equal");
  assertm(result, "Result value is not ok");
  delete[] data;
  data = nullptr;

  result = q.dequeue(data);
  assertm(result == false, "Result value is not ok");

  char another_str[] = "The umask is used by open(2), mkdir(2), and other system calls that create files to modify the "
                       "permissions placed on newly created files or  directories.   Specifically,  permis‐"
                       "sions in the umask"
                       "are turned off from the mode argument to open(2) and mkdir(2)";
  q.enqueue(another_str, sizeof(another_str));

  item_length = 0;
  q.dequeue(nullptr, &item_length);
  assertm(item_length == sizeof(another_str), "Item length is not true");
  data = new char[item_length];
  result = q.dequeue(data);
  assertm(result, "Result value is not ok");
  printf(">>>>>>>>>>>> %s\n", data);
}

void run_tests() {
  test_enqueu_dequeu();
  test_dequeue_too_much();
  test_bucket_creation();
  test_bucket_get_queue();
}