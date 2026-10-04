# Pool Data Structure

## What is a Pool?
A `Pool` is a data structure that holds a linked list of fixed-sized blocks of memory. Under the hood the total
memory of the linked list is actually contiguous, thus allowing greater potential for the memory to be held in
CPU cache.

Similar to an `Arena`, a `Pool` pre-allocates a chunk of memory at once and does not allocate again during
program execution. Unlike an `Arena`, which only grows linearly and does not allow reusing dead memory until
all newer allocations are freed, a `Pool` maintains a "free list" and allows reclaiming dead memory, even if
it is "behind" the newest allocation.

## Use cases
- Frequently spawning and despawning high volumes of dynamic objects, e.g. bullets, particles, projectiles etc.
- Managing network connections sockets, worker threads, database connections.

## Advantages
- Deterministic $O(1)$ performance of spawning and despawning.
- No external dynamic memory fragmentation, since allocated blocks are identical in size.
- Improved CPU cache locality.

## Limitations
- Fixed size. Works best when you have an idea of what size of `Pool` you need beforehand.
- Minimal size constraint. Since the underlying memory is conceptually a linked list, individual elements must
be at least large enough to store a pointer (i.e. 8 bytes on a 64-bit architecture).
