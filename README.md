# Parallel Queue
A generic concurrent FIFO queue implemented with C's threads.h library.
The queue lets you enqueue and dequeue from many threads simultaneously, in a deadlock-proof implementation, while remaining the order of both the enqueuing and dequeuing threads.

## implementation
The implementation of the queue is with 2 different linked lists, first one contains items held in the list, the second one contains symbolic theads.

## features
* initQueue -
  Initialize a new queue, if a queue is already initialized, the function will not do nothing
* destroyQueue -
  deletes the only queue that is being used, after calling destroyQueue u will be able to run initQueue once again.
* enqueue -
  Adds an item to the queue
* dequeue -
  Removes an item from the queue and returns it, if the queue is empty from items, the thread who called dequeue will wait for an item to enter the queue.
* tryDequeue -
  Removes an item from the queue and returns it, only if number of item is the list is more than threads waiting for removing an item
* size -
  Returns the number of items in the queue
* waiting -
  Returns the number of waiting threads
* visited -
  Returns the number of items the entered the queue and removed, both by dequeue and tryDequeue
