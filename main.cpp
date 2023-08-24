#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include "bucket.h"
#include "queue"
// #include "tests.h"

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

    FILE *fd = fopen("test/sq/v/a/trip/12", "rb");
    struct stat st;
    if (fd != nullptr && stat("test/sq/v/a/trip/12", &st) == 0) {
      char *data = (char *)mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
      for (int i = 0; i < 100; i++)
        printf("%d", data[i]);
    }
  }
#endif
  // size_t t = 86;

  return 0;
}
