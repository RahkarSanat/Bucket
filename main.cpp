#include <iostream>
#include <cstring>
#include "bucket.h"
#include "tests.h"

int main() {
#ifdef TEST_ENABLED
  test_enqueu_dequeu();
#else
  // Queue q{"text.txt"};
  Bucket bucket;
  bucket.init("trip/v/sq");
  Queue q = bucket.getQueue("12");
  q.enqueue("ahmad ori", sizeof("ahmad ori"));
  q.enqueue("bar table shadane bkoob", sizeof("bar table shadane bkoob"));
  q.enqueue("hala  k to dori o inja nisti", sizeof("hala  k to dori o inja nisti"));
  char buffer[40] = {0};
  q.dequeue(buffer);
  q.dequeue(buffer);
  q.dequeue(buffer);
#endif
  // size_t t = 86;

  return 0;
}
