# Ring Buffer

## What is it?
A Ring Buffer is a FIFO data structure where a producer thread pushes items into a limited-sized queue and a
consumer thread (which lags behind the producer thread) pops and reads the items. If the queue becomes full,
then the producer thread pauses its operation until the consumer has freed some slots.

The included code shows an example of a Single Producer, Single Consumer (SPSC) Ring Buffer.
- `Ring_Buffer_Lock` uses naive locks to protect the internal head and tail pointers. This works but using
coarse locks like this causes massive contention on repeated runs and results in a huge performance hit with
unpredictable performance.
- `Ring_Buffer_LockFree` uses atomics and no locks to ensure that both threads see each other's work in the
right order. It also uses specific memory ordering (`relaxed`, `acquire`, `relaxed`) over the default `seq_cst`
for an extra performance boost. Overall, this is significantly faster than the lock version. On my PC, the lock-free
version is about 7 to 8 times faster than the lock version.

## Use cases
- Logging. Since logging is kind of a "fire-and-forget" operationm, a Ring Buffer can be used to process temporary
data (such as string interpolation for a log) and discard/overwrite it for the next log.
