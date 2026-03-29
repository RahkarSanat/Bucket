#include <catch2/catch_all.hpp>
#include <cstring>
#include <cstdio>
#include <inttypes.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include "../src/queue.h"
#include "../src/circular_queue.h"
#include "../src/bucket.h"
#include "../src/file_handle.h"

// ──────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────

static int rmCallback(const char *fpath, const struct stat * /*sb*/, int /*typeflag*/, struct FTW * /*ftwbuf*/) {
  return remove(fpath);
}

static void rmrf(const char *path) { nftw(path, rmCallback, 64, FTW_DEPTH | FTW_PHYS); }

static void cleanFile(const char *path) { remove(path); }

// ──────────────────────────────────────────────
// FileHandle tests
// ──────────────────────────────────────────────

TEST_CASE("FileHandle: open valid file") {
  const char *fname = "fh_test_valid";
  // create the file first
  { FileHandle creator{fname, "wb"}; }

  FileHandle fh{fname, "rb"};
  REQUIRE(fh.getFile() != nullptr);
  cleanFile(fname);
}

TEST_CASE("FileHandle: open non-existent file for read returns null") {
  FileHandle fh{"fh_does_not_exist_xyz", "rb"};
  REQUIRE(fh.getFile() == nullptr);
}

TEST_CASE("FileHandle: explicitClose returns true and nullifies file") {
  const char *fname = "fh_test_close";
  { FileHandle creator{fname, "wb"}; }

  FileHandle fh{fname, "r+b"};
  REQUIRE(fh.getFile() != nullptr);
  REQUIRE(fh.explicitClose() == true);
  REQUIRE(fh.getFile() == nullptr);
  cleanFile(fname);
}

TEST_CASE("FileHandle: double close safety") {
  const char *fname = "fh_test_dblclose";
  { FileHandle creator{fname, "wb"}; }

  FileHandle fh{fname, "r+b"};
  REQUIRE(fh.explicitClose() == true);
  // second explicitClose should return false (already null)
  REQUIRE(fh.explicitClose() == false);
  cleanFile(fname);
}

TEST_CASE("FileHandle: construct from existing FILE*") {
  const char *fname = "fh_test_fromptr";
  FILE *raw = fopen(fname, "wb");
  REQUIRE(raw != nullptr);
  {
    FileHandle fh{raw};
    REQUIRE(fh.getFile() == raw);
  } // destructor closes raw
  cleanFile(fname);
}

// ──────────────────────────────────────────────
// Queue tests
// ──────────────────────────────────────────────

TEST_CASE("Queue: isEmpty on new queue") {
  Queue q{"q_empty_test"};
  REQUIRE(q.isEmpty() == true);
  cleanFile("q_empty_test");
}

TEST_CASE("Queue: isEmpty false after enqueue") {
  Queue q{"q_notempty"};
  REQUIRE(q.enqueue("data", 5) == true);
  REQUIRE(q.isEmpty() == false);
  cleanFile("q_notempty");
}

TEST_CASE("Queue: enqueue one item updates metadata") {
  Queue q{"q_meta"};
  const char *test_string = "FIRST ITEM";
  q.enqueue(test_string, sizeof(test_string));
  auto state = q.getMetaData();
  REQUIRE(state->count == 1);
  REQUIRE(state->head == sizeof(QueueMetaData));
  REQUIRE(state->tail == sizeof(QueueMetaData) + sizeof(test_string) + sizeof(QueueItem));
  REQUIRE(state->index == 0);
  cleanFile("q_meta");
}

TEST_CASE("Queue: enqueue and head reads back data") {
  Queue q{"q_headread"};
  char test_string[] = "FIRST ITEM TEST";
  q.enqueue(test_string, sizeof(test_string));
  char buffer[sizeof(test_string)] = {0};
  size_t item_len = 0;
  q.head(buffer, &item_len);
  REQUIRE(item_len == sizeof(test_string));
  REQUIRE(strcmp(buffer, test_string) == 0);
  cleanFile("q_headread");
}

TEST_CASE("Queue: head with null buffer returns item metadata only") {
  Queue q{"q_headnull"};
  const char *data = "hello";
  q.enqueue(data, strlen(data) + 1);
  size_t item_len = 0;
  QueueItem item = q.head(nullptr, &item_len);
  REQUIRE(item.check == 0);
  REQUIRE(item_len == strlen(data) + 1);
  cleanFile("q_headnull");
}

TEST_CASE("Queue: head on empty queue returns check=2") {
  Queue q{"q_headempty"};
  char buffer[32] = {0};
  size_t item_len = 0;
  QueueItem item = q.head(buffer, &item_len);
  REQUIRE(item.check == 2);
  cleanFile("q_headempty");
}

TEST_CASE("Queue: head with dequeue=true advances head and reads next") {
  Queue q{"q_headdeq"};
  const char *s1 = "first";
  const char *s2 = "second";
  q.enqueue(s1, strlen(s1) + 1);
  q.enqueue(s2, strlen(s2) + 1);

  char buffer[32] = {0};
  size_t item_len = 0;

  // head with dequeue=true should read data and advance head
  QueueItem item = q.head(buffer, &item_len, true);
  REQUIRE(item.check == 1);
  REQUIRE(strcmp(buffer, s1) == 0);
  REQUIRE(item_len == strlen(s1) + 1);

  // next head should return second item
  memset(buffer, 0, sizeof(buffer));
  item = q.head(buffer, &item_len);
  REQUIRE(strcmp(buffer, s2) == 0);
  REQUIRE(item_len == strlen(s2) + 1);
  cleanFile("q_headdeq");
}

TEST_CASE("Queue: at with invalid index returns check=2") {
  Queue q{"q_at_invalid"};
  q.enqueue("item", 5);
  QueueItem item = q.at(999);
  REQUIRE(item.check == 2);
  cleanFile("q_at_invalid");
}

TEST_CASE("Queue: at with buffer reads data correctly") {
  cleanFile("q_at_buf"); // ensure no stale file
  Queue q{"q_at_buf"};
  const char *s1 = "alpha";
  const char *s2 = "bravo";
  q.enqueue(s1, strlen(s1) + 1);
  q.enqueue(s2, strlen(s2) + 1);

  // Verify first item has index=1
  QueueItem item1 = q.at(1);
  REQUIRE(item1.check == 0);
  REQUIRE(item1.index == 1);

  // Read second item with buffer (index=2)
  char buffer[32] = {0};
  size_t item_len = 0;
  QueueItem item2 = q.at(2, buffer, &item_len);
  REQUIRE(item2.check == 0);
  REQUIRE(item2.index == 2);
  REQUIRE(item_len == strlen(s2) + 1);
  REQUIRE(strcmp(buffer, s2) == 0);
  cleanFile("q_at_buf");
}

TEST_CASE("Queue: dequeue marks check=1") {
  Queue q{"q_deq_check"};
  const char *data = "A RANDOM ITEM IN QUEUE";
  q.enqueue(data, strlen(data) + 1);

  char buffer[100] = {0};
  size_t item_len = 0;
  QueueItem item = q.head(buffer, &item_len);
  REQUIRE(item.check == 0);
  REQUIRE(q.dequeue(item_len) == true);
  REQUIRE(q.at(1).check == 1);
  cleanFile("q_deq_check");
}

TEST_CASE("Queue: multiple enqueue/dequeue with varying sizes") {
  Queue q{"q_multi"};
  int num_items = 100;
  char buffer[100] = {0};
  for (int i = 1; i < num_items; i++) {
    for (int j = 0; j < i; j++) {
      buffer[j] = 'a';
    }
    q.enqueue(buffer, i);
  }
  REQUIRE(q.getMetaData()->count == (num_items - 1));

  size_t item_length = 0;
  const QueueMetaData *temp_status = nullptr;
  for (int i = 1; i < num_items; i++) {
    q.head(buffer, &item_length);
    buffer[item_length] = '\0';
    REQUIRE(i == (int)strlen(buffer));
    temp_status = q.getMetaData();
    q.dequeue(item_length);
    REQUIRE(i == temp_status->index);
  }
  REQUIRE(temp_status->head == temp_status->tail);
  cleanFile("q_multi");
}

TEST_CASE("Queue: persistence across instances") {
  const char *fname = "q_persist";
  const char *data = "persistent data";
  {
    Queue q{fname};
    q.enqueue(data, strlen(data) + 1);
    REQUIRE(q.getMetaData()->count == 1);
  }
  {
    Queue q{fname};
    REQUIRE(q.getMetaData()->count == 1);
    char buffer[64] = {0};
    size_t item_len = 0;
    QueueItem item = q.head(buffer, &item_len);
    REQUIRE(item.check == 0);
    REQUIRE(strcmp(buffer, data) == 0);
  }
  cleanFile(fname);
}

TEST_CASE("Queue: byteSize returns file size") {
  Bucket bucket;
  bucket.init("test_bytesize_dir");
  Queue q = bucket.getQueue("q_bytesize");

  // empty queue should have at least the metadata
  off_t initial_size = q.byteSize();
  REQUIRE(initial_size == (off_t)sizeof(QueueMetaData));

  q.enqueue("some data", 10);
  off_t after_size = q.byteSize();
  REQUIRE(after_size > initial_size);
  rmrf("test_bytesize_dir");
}

TEST_CASE("Queue: enqueue returns bool") {
  Queue q{"q_ret_bool"};
  REQUIRE(q.enqueue("hello", 6) == true);
  REQUIRE(q.getMetaData()->count == 1);
  cleanFile("q_ret_bool");
}

TEST_CASE("Queue: byteSize works with name-only queue") {
  cleanFile("q_bytesize_name");
  Queue q{"q_bytesize_name"};
  off_t size = q.byteSize();
  REQUIRE(size == (off_t)sizeof(QueueMetaData));
  q.enqueue("test", 5);
  REQUIRE(q.byteSize() > size);
  cleanFile("q_bytesize_name");
}

TEST_CASE("Queue: rename with null bucket returns false") {
  Queue q{"q_rename_null"};
  q.enqueue("data", 5);
  REQUIRE(q.rename("newname", nullptr) == false);
  cleanFile("q_rename_null");
}

// ──────────────────────────────────────────────
// CQueue tests
// ──────────────────────────────────────────────

TEST_CASE("CQueue: empty queue state") {
  const char *fname = "cq_empty.cq";
  CQueue q{fname, 64, 10};
  CQueueMetaData meta = q.getState();
  REQUIRE(meta.head == -1);
  REQUIRE(meta.tail == -1);
  REQUIRE(meta.itemSize == 64);
  REQUIRE(meta.capacity == 10);
  cleanFile(fname);
}

TEST_CASE("CQueue: single enqueue sets head=0, tail=0") {
  const char *fname = "cq_single.cq";
  {
    CQueue q{fname, 32, 10};
    // enqueue with size < itemSize so it returns true (padding path)
    char data[16] = "hello cqueue";
    REQUIRE(q.enqueue(data, 16) == true);
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 0);
    REQUIRE(meta.tail == 0);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: multiple enqueues advance tail") {
  const char *fname = "cq_multi.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "item";
    q.enqueue(data, 16);
    q.enqueue(data, 16);
    q.enqueue(data, 16);
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 0);
    REQUIRE(meta.tail == 2);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: dequeue single item resets to empty") {
  const char *fname = "cq_deq_single.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "item";
    q.enqueue(data, 16);
    REQUIRE(q.dequeue() == true);
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == -1);
    REQUIRE(meta.tail == -1);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: dequeue advances head") {
  const char *fname = "cq_deq_adv.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "item";
    q.enqueue(data, 16);
    q.enqueue(data, 16);
    q.enqueue(data, 16);
    REQUIRE(q.dequeue() == true);
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 1);
    REQUIRE(meta.tail == 2);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: dequeue on empty returns false") {
  const char *fname = "cq_deq_empty.cq";
  {
    CQueue q{fname, 32, 10};
    REQUIRE(q.dequeue() == false);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: overflow auto-dequeues oldest") {
  const char *fname = "cq_overflow.cq";
  cleanFile(fname);
  uint16_t capacity = 5;
  {
    CQueue q{fname, 32, capacity};
    char data[16] = {0};
    // fill to capacity - 1 items (capacity slots, but overflow triggers at capacity)
    for (int i = 0; i < capacity; i++) {
      snprintf(data, sizeof(data), "item%d", i);
      q.enqueue(data, 16);
    }
    // At this point head=0, tail=capacity-1, queue is full.
    // Enqueue one more should auto-dequeue oldest.
    snprintf(data, sizeof(data), "overflow");
    q.enqueue(data, 16);

    CQueueMetaData meta = q.getState();
    // head advanced once from auto-dequeue when queue was full
    REQUIRE(meta.head == 1);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: head with buffer too small returns false") {
  const char *fname = "cq_small_buf.cq";
  {
    CQueue q{fname, 64, 10};
    char data[32] = "test data";
    q.enqueue(data, 32);

    char small_buf[16] = {0};
    REQUIRE(q.head(small_buf, 16) == false);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: head on empty queue returns false") {
  const char *fname = "cq_head_empty.cq";
  {
    CQueue q{fname, 32, 10};
    char buffer[32] = {0};
    REQUIRE(q.head(buffer, 32) == false);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: head with null buffer returns false") {
  const char *fname = "cq_head_null.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "item";
    q.enqueue(data, 16);
    REQUIRE(q.head<char>(nullptr, 32) == false);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: enqueue with exact itemSize returns true") {
  const char *fname = "cq_exact.cq";
  cleanFile(fname);
  {
    CQueue q{fname, 16, 10};
    char data[16] = "exact size fit";
    REQUIRE(q.enqueue(data, 16) == true);

    char buffer[16] = {0};
    REQUIRE(q.head(buffer, 16) == true);
    REQUIRE(strcmp(buffer, "exact size fit") == 0);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: enqueue with object larger than itemSize returns false") {
  const char *fname = "cq_too_big.cq";
  {
    CQueue q{fname, 16, 10};
    char big_data[32] = "this is way too big for the cq";
    REQUIRE(q.enqueue(big_data, 32) == false);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: templated enqueue/head with struct") {
  struct TestStruct {
    int32_t x;
    int32_t y;
    char label[8];
  };

  const char *fname = "cq_struct.cq";
  cleanFile(fname);
  {
    uint16_t item_size = sizeof(TestStruct);
    CQueue q{fname, item_size, 10};
    TestStruct input = {.x = 42, .y = -7, .label = "point"};
    REQUIRE(q.enqueue(&input, sizeof(TestStruct)) == true);

    TestStruct output = {};
    REQUIRE(q.head(&output, item_size) == true);
    REQUIRE(output.x == 42);
    REQUIRE(output.y == -7);
    REQUIRE(strcmp(output.label, "point") == 0);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: head with dequeue=true removes item") {
  const char *fname = "cq_head_deq.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "item1";
    q.enqueue(data, 16);
    snprintf(data, sizeof(data), "item2");
    q.enqueue(data, 16);

    char buffer[32] = {0};
    REQUIRE(q.head(buffer, 32, true) == true);
    REQUIRE(strncmp(buffer, "item1", 5) == 0);

    // After dequeue, head should point to second item
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 1);
    REQUIRE(meta.tail == 1);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: printer iterates all items") {
  const char *fname = "cq_printer.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = {0};
    for (int i = 0; i < 3; i++) {
      snprintf(data, sizeof(data), "print%d", i);
      q.enqueue(data, 16);
    }

    int count = 0;
    char buffer[32] = {0};
    q.printer(static_cast<char *>(buffer), [&count](int32_t /*idx*/, char *buf) {
      REQUIRE(strlen(buf) > 0);
      count++;
    });
    REQUIRE(count == 3);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: wrap-around behavior") {
  const char *fname = "cq_wrap.cq";
  uint16_t capacity = 5;
  {
    CQueue q{fname, 32, capacity};
    char data[16] = {0};

    // Fill 3 items
    for (int i = 0; i < 3; i++) {
      snprintf(data, sizeof(data), "w%d", i);
      q.enqueue(data, 16);
    }
    // Dequeue 2
    q.dequeue();
    q.dequeue();

    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 2);
    REQUIRE(meta.tail == 2);

    // Enqueue 3 more — tail should wrap around
    for (int i = 3; i < 6; i++) {
      snprintf(data, sizeof(data), "w%d", i);
      q.enqueue(data, 16);
    }
    meta = q.getState();
    REQUIRE(meta.head == 2);
    // tail should have wrapped: 2+3 = 5, wraps at capacity 5 → positions 3, 4, 0
    REQUIRE(meta.tail == 0);

    // Verify we can read the head item
    char buffer[32] = {0};
    REQUIRE(q.head(buffer, 32) == true);
    REQUIRE(strncmp(buffer, "w2", 2) == 0);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: persistence across instances") {
  const char *fname = "cq_persist.cq";
  int num = 40;
  {
    CQueue q{fname, 100, 100};
    const char *item = "Test to See if CQueue is persisted";
    for (int i = 0; i < num; i++) {
      q.enqueue(item, strlen(item));
    }
  }
  {
    CQueue q{fname, 100, 100};
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 0);
    REQUIRE(meta.tail == num - 1);
    REQUIRE(meta.itemSize == 100);
    REQUIRE(meta.capacity == 100);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: reopen with name-only constructor") {
  const char *fname = "cq_reopen.cq";
  {
    CQueue q{fname, 32, 10};
    char data[16] = "reopen test";
    q.enqueue(data, 16);
  }
  {
    // use the 2-arg constructor (name only, loads existing)
    CQueue q{fname};
    CQueueMetaData meta = q.getState();
    REQUIRE(meta.head == 0);
    REQUIRE(meta.tail == 0);
    REQUIRE(meta.itemSize == 32);
    REQUIRE(meta.capacity == 10);
  }
  cleanFile(fname);
}

TEST_CASE("CQueue: name-only constructor on non-existent file") {
  CQueue q{"cq_nonexist_xyz.cq"};
  CQueueMetaData meta = q.getState();
  // should not be available — state remains zero-initialized
  REQUIRE(meta.head == 0);
  REQUIRE(meta.tail == 0);
}

// ──────────────────────────────────────────────
// Bucket tests
// ──────────────────────────────────────────────

TEST_CASE("Bucket: init creates nested directories") {
  Bucket bucket;
  bucket.init("test_b/nested/deep/path");
  struct stat st;
  REQUIRE(stat("test_b/nested/deep/path", &st) == 0);
  REQUIRE(S_ISDIR(st.st_mode));
  rmrf("test_b");
}

TEST_CASE("Bucket: getQueue creates new queue file") {
  Bucket bucket;
  bucket.init("test_b_getq");
  Queue q = bucket.getQueue("myqueue");
  struct stat st;
  REQUIRE(stat("test_b_getq/myqueue", &st) == 0);
  REQUIRE(q.isEmpty() == true);
  rmrf("test_b_getq");
}

TEST_CASE("Bucket: getQueue loads existing queue") {
  Bucket bucket;
  bucket.init("test_b_getq_exist");
  {
    Queue q = bucket.getQueue("q1");
    q.enqueue("data", 5);
  }
  {
    Queue q = bucket.getQueue("q1");
    REQUIRE(q.getMetaData()->count == 1);
  }
  rmrf("test_b_getq_exist");
}

TEST_CASE("Bucket: getExistingQueue returns false for non-existent") {
  Bucket bucket;
  bucket.init("test_b_noexist");
  Queue temp;
  REQUIRE(bucket.getExistingQueue("nope", &temp) == false);
  rmrf("test_b_noexist");
}

TEST_CASE("Bucket: getExistingQueue returns true for existing") {
  Bucket bucket;
  bucket.init("test_b_exist");
  bucket.getQueue("found");
  Queue temp;
  REQUIRE(bucket.getExistingQueue("found", &temp) == true);
  REQUIRE(temp.isEmpty() == true);
  rmrf("test_b_exist");
}

TEST_CASE("Bucket: removeQueue deletes file") {
  Bucket bucket;
  bucket.init("test_b_rmq");
  bucket.getQueue("todelete");
  struct stat st;
  REQUIRE(stat("test_b_rmq/todelete", &st) == 0);
  REQUIRE(bucket.removeQueue("todelete") == true);
  REQUIRE(stat("test_b_rmq/todelete", &st) != 0);
  rmrf("test_b_rmq");
}

TEST_CASE("Bucket: removeQueue returns false for non-existent") {
  Bucket bucket;
  bucket.init("test_b_rmq_no");
  REQUIRE(bucket.removeQueue("ghost") == false);
  rmrf("test_b_rmq_no");
}

TEST_CASE("Bucket: getDirSize returns correct total size") {
  rmrf("test_b_dirsize"); // clean start
  Bucket bucket;
  bucket.init("test_b_dirsize");
  Queue q1 = bucket.getQueue("a");
  Queue q2 = bucket.getQueue("b");
  q1.enqueue("some data here", 15);
  q2.enqueue("more data", 10);

  size_t dir_size = bucket.getDirSize();
  // getDirSize now skips . and .., so it should match the sum of file sizes
  struct stat st1, st2;
  stat("test_b_dirsize/a", &st1);
  stat("test_b_dirsize/b", &st2);
  REQUIRE(dir_size == (size_t)(st1.st_size + st2.st_size));
  rmrf("test_b_dirsize");
}

TEST_CASE("Bucket: list smoke test and skips dot entries") {
  rmrf("test_b_list");
  Bucket bucket;
  bucket.init("test_b_list");
  bucket.getQueue("x");
  bucket.getQueue("y");
  // just verify it doesn't crash; list() now skips . and ..
  REQUIRE_NOTHROW(bucket.list());
  REQUIRE_NOTHROW(bucket.list(true));
  rmrf("test_b_list");
}

TEST_CASE("Bucket: move queue between buckets") {
  Bucket b1, b2;
  b1.init("test_b_mv1");
  b2.init("test_b_mv2");
  Queue q = b1.getQueue("moveme");
  q.enqueue("data", 5);
  q.move(&b2);
  REQUIRE(strcmp(q.getPath(), "test_b_mv2/moveme") == 0);
  // original should be gone
  struct stat st;
  REQUIRE(stat("test_b_mv1/moveme", &st) != 0);
  REQUIRE(stat("test_b_mv2/moveme", &st) == 0);
  rmrf("test_b_mv1");
  rmrf("test_b_mv2");
}

// ──────────────────────────────────────────────
// Iterator tests
// ──────────────────────────────────────────────

TEST_CASE("Iterator: iterate empty directory skips . and ..") {
  mkdir("test_iter_empty", 0777);
  bucket::Iterator iter{"test_iter_empty"};
  int count = 0;
  char *entry = nullptr;
  while ((entry = iter.next())) {
    // should only see . and ..
    bool is_dot = (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0);
    REQUIRE(is_dot);
    count++;
  }
  REQUIRE(count == 2);
  rmrf("test_iter_empty");
}

TEST_CASE("Iterator: iterate directory with files") {
  Bucket bucket;
  bucket.init("test_iter_files");
  bucket.getQueue("file1");
  bucket.getQueue("file2");
  bucket.getQueue("file3");

  bucket::Iterator iter{"test_iter_files"};
  int count = 0;
  char *entry = nullptr;
  while ((entry = iter.next())) {
    if (strcmp(entry, ".") != 0 && strcmp(entry, "..") != 0) {
      count++;
    }
  }
  REQUIRE(count == 3);
  rmrf("test_iter_files");
}

TEST_CASE("Iterator: from with non-existent ID returns false") {
  Bucket bucket;
  bucket.init("test_iter_from");
  bucket.getQueue("aabbccdd");

  bucket::Iterator iter{"test_iter_from"};
  // from() formats the ID as %08x hex, so 0xDEADBEEF
  REQUIRE(iter.from(0xDEADBEEF) == false);
  rmrf("test_iter_from");
}

TEST_CASE("Iterator: from with valid hex ID positions correctly") {
  Bucket bucket;
  bucket.init("test_iter_fromv");
  // Create files named as hex IDs (from() uses %08x format)
  // We need to create a file named "0000000a" (hex for 10)
  {
    char path[64];
    snprintf(path, sizeof(path), "test_iter_fromv/%08x", 10);
    FileHandle creator{path, "wb"};
  }
  {
    char path[64];
    snprintf(path, sizeof(path), "test_iter_fromv/%08x", 20);
    FileHandle creator{path, "wb"};
  }

  bucket::Iterator iter{"test_iter_fromv"};
  REQUIRE(iter.from(10) == true);
  rmrf("test_iter_fromv");
}

TEST_CASE("Iterator: null directory") {
  bucket::Iterator iter{"nonexistent_dir_xyz"};
  REQUIRE(iter.next() == nullptr);
}
