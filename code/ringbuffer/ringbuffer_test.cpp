#if TESTS_ENABLED
#include "ringbuffer/ringbuffer.h"

#include <thread>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

// Pins the calling thread to a specific logical processor.
static bool pinCallingThreadCore(int coreId)
{
    const DWORD_PTR mask = static_cast<DWORD_PTR>(1) << coreId;
    return SetThreadAffinityMask(GetCurrentThread(), mask) != 0;
}

// Uses 2 threads (producer and consumer) to push and pop integers to the ring buffer `numRuns` times.
// Returns true if every value was popped in order.
template <typename Ring_Buffer>
static bool runRingBuffer(int numItems)
{
    Ring_Buffer ringBuffer{};
    std::atomic<bool> ok{true};

    std::thread consumer([&]
    {
        pinCallingThreadCore(2);
        for (int i = 0; i < numItems; i++)
        {
            int value = 0;
            while (!ringBuffer.pop(value));

            if (value != i)
            {
                ok.store(false);
                return;
            }
        }
    });

    pinCallingThreadCore(3);

    bool aborted = false;
    for (int i = 0; i < numItems; i++)
    {
        while (!ringBuffer.push(i))
        {
            if (!ok.load())
            {
                aborted = true; // Consumer has aborted
                break;
            }
        }

        if (aborted) { break; }
    }

    consumer.join();

    return ok.load();
}

TEMPLATE_TEST_CASE("RingBuffer", "[ringbuffer][benchmark]",
    (Ring_Buffer_Lock<int, 100000>),
    (Ring_Buffer_LockFree<int, 100000>))
{
    constexpr int NUM_ITEMS = 1000000;

    BENCHMARK("push/pop 100,000 items")
    {
        return runRingBuffer<TestType>(NUM_ITEMS);
    };
}
#endif // TESTS_ENABLED
