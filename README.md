# Bucket
**in-file** bucket of queue

####  create a new Queue:

````
#include "bucket.h"
#include "tests.h"

int main() {
  Queue q{"text.txt"};
  q.enqueue("Item1", sizeof("Item1"));
  q.enqueue("Item2", sizeof("Item2"));
  q.enqueue("Item3", sizeof("Item3"));

  char buffer[40] = {0};
  q.dequeue(buffer); // get item 1
  q.dequeue(buffer); // get item 2
  q.dequeue(buffer); // get item 3

  assert(q.dequeue(buffer) == false); // result is false, no Item

  return 0;
}
```````
#### Get Item at [ index ]: 

````
char atData[100] = {0};
  bool result = q.at(2, atData, nullptr);
  if (result)
    printf("Data at item %d is :%s\n", 2, atData);
  else
    printf("Item with given id doesnt exist\n");
````
