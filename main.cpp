#include <iostream>
#include <cstring>
#include "bucket.h"
#include "queue"
#include "tests.h"

int main() {
#ifdef TEST_ENABLED
  run_tests();
#else
  Queue q{"text.txt"};
  {
    Bucket bucket;
    bucket.init("test/sq/v/a/trip");
    Queue q = bucket.getQueue("12");
    printf("queue is %s\n", q.isEmpty() ? "Empty" : "Not Empty");
    q.enqueue("First Item", sizeof("First Item"));
    q.enqueue("Second Item", sizeof("Second Item"));
    q.enqueue("Third Item", sizeof("Third Item"));

    Bucket bucket2;
    bucket2.init("test/sq/v/trip/");
    bool res = q.rename("13", &bucket2);
    printf("00000000000 %s\n", strerror(errno));
    auto test = q.getMetaData();
    printf("Queue head: %d\n\ttail: %d\n\titems: %d\n\tindex: %d\n", test->head, test->tail, test->count, test->index);
    // printf("removed file: %d\n", bucket.removeQueue("13"));
  }

  {
    Bucket bucket;
    bucket.init("test/sq/v/trip");
    Queue q = bucket.getQueue("13");
    printf("queue is %s\n", q.isEmpty() ? "Empty" : "Not Empty");
    char buffer[40] = {0};
    auto test = q.getMetaData();
    size_t item_length = 0;
    q.head(buffer, &item_length);
    printf("Queue head: %d\n\ttail: %d\n\titems: %d\n\tindex: %d\n\treadLength: %d\n", test->head, test->tail, test->count,
           test->index, item_length);
    q.head(buffer, &item_length);
    printf("Queue head: %d\n\ttail: %d\n\titems: %d\n\tindex: %d\n\treadLength: %d\n", test->head, test->tail, test->count,
           test->index, item_length);
    q.dequeue(item_length);
    q.head(buffer, &item_length);
    printf("Queue head: %d\n\ttail: %d\n\titems: %d\n\tindex: %d\n\treadLength: %d\n", test->head, test->tail, test->count,
           test->index, item_length);

    // printf("data is %s\n", buffer);
    // bool res = q.rename("14", &bucket);
    // printf("00000000000 %s\n", strerror(errno));
    // auto test = q.getMetaData();
    // printf("removed file: %d\n", bucket.removeQueue("13"));
  }

  // {
  //   Queue q = bucket.getQueue("14");
  //   printf("queue is %s\n", q.isEmpty() ? "Empty" : "Not Empty");
  //   Bucket newbucket;
  //   bucket.init("hello/world");
  //   q.move(&bucket);
  // }

#endif
  // size_t t = 86;

  return 0;
}
