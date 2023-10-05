#include <catch2/catch_all.hpp>
#include "../src/queue.h"
#include "../src/circular_queue.h"
#include "../src/bucket.h"

TEST_CASE("Test Enqueue one item check update status correctly") {

  SECTION("testSec") {
    Queue q{"name"};
    const char *test_string = "FIRST ITEM";
    q.enqueue(test_string, sizeof(test_string));
    auto state = q.getMetaData();
    REQUIRE(state->count == 1);
    REQUIRE(state->head == sizeof(QueueMetaData));
    REQUIRE(state->tail == sizeof(QueueMetaData) + sizeof(test_string) + sizeof(QueueItem));
    REQUIRE(state->index == 0);
  }

  SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("Test for Enqueue and Dequeue correctly") {

  SECTION("testSec") {
    Queue q{"name"};
    char test_string[] = "FIRST ITEM TEST";
    q.enqueue(test_string, sizeof(test_string));
    auto state = q.getMetaData();
    char buffer[sizeof(test_string)] = {0};
    size_t itemlen = 0;
    q.head(buffer, &itemlen);
    REQUIRE(itemlen == sizeof(test_string));
    REQUIRE(strcmp(buffer, test_string) == 0);
  }

  SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("Test for Enqueue and Dequeue With strlen() correctly") {

  SECTION("testSec") {
    Queue q{"name"};
    const char *test_string = "FIRST ITEM";
    q.enqueue(test_string, strlen(test_string) + 1);
    auto state = q.getMetaData();
    char buffer[strlen(test_string) + 1] = {0};
    size_t itemlen = 0;
    q.head(buffer, &itemlen);
    REQUIRE(itemlen == strlen(test_string) + 1);
    REQUIRE(strcmp(buffer, test_string) == 0);
  }

  SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("test_rename_change_internal_path_in_bucket") {

  SECTION("testSec") {
    Bucket bucket;
    bucket.init("test/rename/queue/path/");
    Bucket bucket2;
    bucket2.init("test/rename/queue/path/2");
    Queue q = bucket.getQueue("name");
    q.move(&bucket2);
    printf("%s\n", q.getPath());
    REQUIRE(strcmp(q.getPath(), "test/rename/queue/path/2/name") == 0);
  }

  SECTION("Tear Down") { REQUIRE(remove("test/rename/queue/path/2/name") == 0); }
}

TEST_CASE("Test for Enqueue with different sizes") {

  SECTION("testSec") {
    Queue q{"name"};
    int numberOfItem = 100;
    char buffer[numberOfItem] = {0};
    for (int i = 1; i < numberOfItem; i++) {
      for (int j = 0; j < i; j++) {
        buffer[j] = 'a';
      }
      q.enqueue(buffer, i);
    }
    REQUIRE(q.getMetaData()->count == (numberOfItem - 1));
    // std::memset(buffer, 0, 100);

    size_t itemLength = 0;
    const QueueMetaData *tempStatus = nullptr;
    for (int i = 1; i < numberOfItem; i++) {
      q.head(buffer, &itemLength);
      buffer[itemLength] = '\0';
      REQUIRE(i == strlen(buffer));
      tempStatus = q.getMetaData();
      q.dequeue(itemLength);
      REQUIRE(i == tempStatus->index);
    }

    REQUIRE(tempStatus->head == tempStatus->tail);
  }
  SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("Test bucket creation") {

  SECTION("testSec") {
    Bucket bucket;
    bucket.init("test/bucket/created");
    struct stat st;
    REQUIRE(stat("test/bucket/created", &st) == 0);
  }
  //   SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("test_queue_item_check_option_working") {

  SECTION("testSec") {
    Queue q{"name"};
    const char *test_str = "A RANDOM ITEM IN QUEUE";
    char buffer[100] = {0};
    size_t itemlen = 0;
    q.enqueue(test_str, strlen(test_str) + 1);
    QueueItem item = q.head(buffer, &itemlen);
    REQUIRE(item.check == 0);
    REQUIRE(q.dequeue(itemlen) == true);
    REQUIRE(q.at(1).check == 1);
    q.enqueue(test_str, strlen(test_str) + 1);
    item = q.at(2);
    REQUIRE(item.check == 0);
  }
  SECTION("Tear Down") { REQUIRE(remove("name") == 0); }
}

TEST_CASE("Test getting existing queue") {

  SECTION("testSec") {
    Bucket bucket;
    bucket.init("test/bucket/test/getExist");
    bucket.getQueue("13");
    Queue temp;
    REQUIRE(bucket.getExistingQueue("13", &temp) == true);
    REQUIRE(bucket.getExistingQueue("14", &temp) == false);
  }
}

TEST_CASE("Test Persistant circular queue state") {

  int number = 40;
  SECTION("testSec") {
    {
      CQueue q{"test1.cq", 100, 100};
      const char *item = "Test to See if CQueue is persisited";
      for (int i = 0; i < number; i++)
        q.enqueue(item, strlen(item));
      CQueueMetaData meta = q.getState();
    }
    {
      CQueue q{"test1.cq", 100, 100};
      CQueueMetaData meta = q.getState();
      printf("data is %d %d %d %d", meta.head, meta.tail, meta.capacity, meta.itemSize);

      REQUIRE(meta.head == 0);
      REQUIRE(meta.tail == number - 1);
    }
    SECTION("Tear Down") { REQUIRE(remove("test1.cq") == 0); }
  }
}

// TEST_CASE("test_") {

//   int number = 40;
//   SECTION("testSec") {
//     {
//       CQueue q{"test1.cq", 100, 100};
//       const char *item = "Test to See if CQueue is persisited";
//       for (int i = 0; i < number; i++)
//         q.enqueue(item, strlen(item));
//       CQueueMetaData meta = q.getState();
//     }
//     {
//       CQueue q{"test1.cq", 100, 100};
//       CQueueMetaData meta = q.getState();
//       REQUIRE(meta.head == 0);
//       REQUIRE(meta.tail == number - 1);
//     }
//     SECTION("Tear Down") { REQUIRE(remove("test1.cq") == 0); }
//   }
// }