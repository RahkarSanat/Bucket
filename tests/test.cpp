#include <catch2/catch_all.hpp>
#include "../src/queue.h"
#include "../src/bucket.h"

TEST_CASE("Test Enqueue three item check update status correctly") {

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

    size_t itemLength = 0;
    const QueueMetaData *tempStatus = nullptr;
    for (int i = 1; i < 100; i++) {
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