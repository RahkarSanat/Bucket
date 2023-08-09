#include <iostream>
#include "bucket.h"
#include "cstring"

int main() {
  Queue q{"text4.txt"};
  q.enqueue("ahmad ori", sizeof("ahmad ori"));
  char buffer[40] = {0};
  // q.dequeue(buffer);
  // q.dequeue(buffer);
  // q.dequeue(buffer);
  size_t t = 86;

  return 0;
}