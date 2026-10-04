#pragma once
#include <array>
#include <atomic>
#include <mutex>

// This is a SPSC RingBuffer

template <typename T, size_t N>
struct Ring_Buffer_Lock
{
    bool push(const T& value)
    {
        std::scoped_lock lock(m_mutex);
        size_t newHead = m_head + 1;
        if (newHead == m_buffer.size()) [[unlikely]]
        {
            newHead = 0; // Wrap around
        }

        if (newHead == m_tail) [[unlikely]]
        {
            return false; // Full
        }

        m_buffer[m_head] = value;
        m_head = newHead;
        return true;
    }

    bool pop(T& value)
    {
        std::scoped_lock lock(m_mutex);
        if (m_head == m_tail) [[unlikely]]
        {
            return false; // Empty
        }
        value = m_buffer[m_tail];

        size_t nextTail = m_tail + 1;
        if (nextTail == m_buffer.size()) [[unlikely]]
        {
            nextTail = 0; // Wrap around
        }
        m_tail = nextTail;
        return true;
    }

    std::array<T, N> m_buffer{};
    size_t m_head{0};
    size_t m_tail{0};
    mutable std::mutex m_mutex;
};

template <typename T, size_t N>
struct Ring_Buffer_LockFree
{
    bool push(const T& value)
    {
        // `relaxed` indicates no memory ordering. This is okay here since only the single producer thread
        // ever writes to this variable.
        const size_t head = m_head.load(std::memory_order_relaxed);
        size_t nextHead = head + 1;
        if (nextHead == m_buffer.size()) [[unlikely]]
        {
            nextHead = 0; // Wrap around
        }

        // Since, we're reading the consumer thread's data here, `acquire` ensures that this read happens
        // after `release`.
        if (nextHead == m_tail.load(std::memory_order_acquire)) [[unlikely]]
        {
            return false; // Full
        }

        m_buffer[head] = value;
        // Using `release` here lets the consumer thread know that it can use acquire to read `m_head` in order.
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }

    bool pop(T& value)
    {
        const size_t tail = m_tail.load(std::memory_order_relaxed);
        if (tail == m_head.load(std::memory_order_acquire)) [[unlikely]]
        {
            return false; // Empty
        }

        value = m_buffer[tail];

        size_t nextTail = tail + 1;
        if (nextTail == m_buffer.size()) [[unlikely]]
        {
            nextTail = 0; // Wrap around
        }

        m_tail.store(nextTail, std::memory_order_release);
        return true;
    }

    std::array<T, N> m_buffer{};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> m_head{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> m_tail{0};
};
