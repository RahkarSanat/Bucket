#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include "bucket.h"
#include "queue"
#include "circular_queue.h"

int main() {
#if 1
  CQueue obj{"first.bq", nullptr, 45};
  uint8_t buf[40]{0};
  buf[0] = 'd';
  buf[25] = '1';
  buf[38] = 'F';
  obj.enqueue((char *)buf, sizeof(buf));
  // obj.enqueue((char *)buf, sizeof(buf));

  // std::cout << obj << std::endl;
  // obj.printer();

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
