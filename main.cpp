#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sys/mman.h>
#include <cstring>
#include "bucket.h"
#include "queue"
#include <string>
#include "circular_queue.h"
#include "src/circular_queue.h"
#include "src/queue.h"

struct Vec2 {
  Vec2() = default;
  Vec2(float x, float y) : x(x), y(y) {}
  float x;
  float y;
};

void testTemplated() {

  CQueue obj{"templated.bq", sizeof(Vec2), 10};

  for (int i = 0; i < 12; i++) {
    auto value = Vec2(i, i * i);
    printf("writing %f %f\n", value.x, value.y);
    obj.enqueue<Vec2>(&value, sizeof(Vec2));
  }

  Vec2 buf{};
  obj.printer(&buf, [](int idx, Vec2 *item) { PRINT("%d vec2(%.3f, %.3f)\n", idx, item->x, item->y); });
}

int main() {

  // testTemplated();
#if 1
  CQueue obj{"first.bq", 40, 10};
  uint8_t dq_buf[45]{0};
  uint8_t buf[] = "Q - bahre to dadam del o din ey sanama";

  for (int i = 0; i < 12; i++) {
    buf[0] = (char)((i + 48));
    printf("writing %s\n", buf);
    obj.enqueue((char *)buf, sizeof(buf));
  }

  char print_buf[40]{0};
  obj.printer(print_buf, [](int idx, char *item) { PRINT("eee %d %s\n", idx, item); });

  char test[3][40];
  PRINT("-------------------------------------- %s\n", "working or not");

  for (int i = 0; i < 3; i++) {
    if (obj.head((char *)test[i], sizeof(test[i]), true)) {
      PRINT("%s\n", test[i]);
    }
  }
  PRINT("--------------------------------------\n");
  for (int i = 0; i < 3; i++) {
    obj.enqueue(test[i], strlen(test[i]));
  }
  PRINT("--------------------------------------11\n");
  obj.printer(print_buf, [](int idx, char *item) { PRINT("%d %s\n", idx, item); });
  PRINT("--------------------------------------11\n");
  for (int i = 0; i < 5; i++) {
    if (obj.head((char *)dq_buf, sizeof(dq_buf), true)) {
      PRINT("%s\n", dq_buf);
    }
  }
  PRINT("--------------------------------------11\n");
  obj.printer(print_buf, [](int idx, char *item) { PRINT("%d %s\n", idx, item); });

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
