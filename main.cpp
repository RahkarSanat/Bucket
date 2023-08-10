#include <iostream>
#include <cstring>
#include "bucket.h"
#include "tests.h"

int main() {
#ifdef TEST_ENABLED
  run_tests();
#else
  // Queue q{"text.txt"};
  Bucket bucket;
  bucket.init("test/sq/v/a/trip");
  Queue q = bucket.getQueue("12");
  // q.enqueue("ahmad ori", sizeof("ahmad ori"));
  // q.enqueue("bar table shadane bkoob", sizeof("bar table shadane bkoob"));
  // q.enqueue("hala  k to dori o inja nisti", sizeof("hala  k to dori o inja nisti"));
  char buffer[40] = {0};
  // q.dequeue(buffer);
  // q.dequeue(buffer);
  // q.dequeue(buffer);
  // printf(":::::::::::::::::::::::::::::::::\n");
  // q.dequeue(buffer);

  char atData[100] = {0};
  bool result = q.at(15, atData, nullptr);
  if (result)
    printf("Data at item %d is :%s\n", 16, atData);
  else
    printf("Item with given id doesnt exist\n");

#endif
  // size_t t = 86;

  return 0;
}
