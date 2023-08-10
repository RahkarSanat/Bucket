#include <cassert>
#include <cstring>
#include "bucket.h"

// #define TEST_ENABLED 1

#define assertm(exp, msg) assert(((void)msg, exp))

void test_enqueu_dequeu() {
  Queue q{"test.txt"};
  q.enqueue("mara bebooooooos, maraaa beboooos", sizeof("mara bebooooooos, maraaa beboooos"));
  q.enqueue("dokhtare zibaa, emshab bar to mehmanam", sizeof("dokhtare zibaa, emshab bar to mehmanam"));

  size_t item_length = 0;
  q.dequeue(nullptr, &item_length);
  assertm(item_length == 34, "Item length retrieved incorrectly");

  char *data = new char[item_length];
  q.dequeue(data, nullptr);
  printf("0000000000000 %s\n", data);

  assertm(strcmp(data, "mara bebooooooos, maraaa beboooos") == 0, "data retrived correcly");
  delete[] data;
}