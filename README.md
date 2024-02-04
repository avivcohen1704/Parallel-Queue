# Parallel-Queue

## Description

The `Parallel-Queue` project implements a generic, thread-safe, First-In-First-Out (FIFO) queue in C, utilizing the `threads.h` library to enable simultaneous enqueue and dequeue operations across multiple threads. Designed to avoid deadlocks and maintain the order of operations, this project showcases advanced concurrency control, synchronization techniques, and a thorough understanding of parallel computing principles in C.

## Technologies Used

- Programming Language: C
- Libraries: threads.h for threading support
- Tools: Git for version control

## Features

- **Thread-Safe Operations**: Ensures safe concurrent access by multiple threads without causing deadlocks.
- **Maintains Order**: Preserves the order of enqueuing and dequeuing operations, critical for FIFO queues.
- **Dynamic Data Structure**: Utilizes linked lists to store queue items and manage thread waitlists dynamically.
- **Blocking and Non-Blocking Operations**: Supports both blocking (`dequeue`) and non-blocking (`tryDequeue`) dequeue operations.
- **Utility Functions**: Includes `initQueue`, `destroyQueue`, `enqueue`, `dequeue`, `tryDequeue`, `size`, `waiting`, and `visited` functions for comprehensive queue management.

## Getting Started

### Prerequisites

- A C compiler like GCC or Clang.
- Make sure `threads.h` is supported on your system.

### Installation

Clone the repository to your local machine:

```bash
git clone https://github.com/[YourGitHubUsername]/Parallel-Queue.git
cd Parallel-Queue
```
## Compilation
Compile the project with your C compiler, for example:
```bash
gcc -o parallel_queue main.c -lpthread
```
Replace main.c with the actual name of your source file that includes the main function.
## Running the Project
Run the compiled program:
```bash
./parallel_queue
```
## Contributing
Contributions are welcome! If you have improvements or bug fixes, please fork the repository and submit a pull request.



