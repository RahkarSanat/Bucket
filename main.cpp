#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include "bucket.h"
#include "queue"
#include <string>
#include "circular_queue.h"

int main() {
#if 1
  CQueue obj{"first.bq", 40, 10};
  uint8_t dq_buf[45]{0};
  uint8_t buf[] = "b - bahre to dadam del o din ey sanam";

  for (int i = 0; i < 12; i++) {
    buf[0] = (char)((i + 48));
    obj.enqueue((char *)buf, sizeof(buf));
  }

  char test[3][40];
  printf("--------------------------------------\n");

  for (int i = 0; i < 3; i++)
    if (obj.head((char *)test[i], sizeof(test[i]), true)) {
      // strcpy(test[i], (char *)dq_buf);
      printf("%s\n", test[i]);
    }
  printf("--------------------------------------\n");

  for (int i = 0; i < 3; i++) {
    obj.enqueue(test[i], strlen(test[i]));
  }
  printf("--------------------------------------11\n");
  for (int i = 0; i < 10; i++)
    if (obj.head((char *)dq_buf, sizeof(dq_buf), true)) {
      // strcpy(dq_buf, (char *)dq_buf);
      printf("%s\n", dq_buf);
    }

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
