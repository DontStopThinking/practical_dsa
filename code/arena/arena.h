#pragma once
#include <bit>
#include <cassert>
#include <cstddef>
#include <new>

struct Arena
{
    Arena(size_t size)
        : m_capacity{size}
    {
        if (size > 0)
        {
            m_buffer = new std::byte[size];
        }
    }

    ~Arena()
    {
        if (m_buffer)
        {
            delete[] m_buffer;
        }
    }

    // Non-copyable
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        if (!m_buffer)
        {
            return nullptr;
        }

        /**
            Alignment: CPUs prefer it when a data type is placed at a memory address that is a multiple of the
            size of the data type. For e.g. a 4-byte "int" should start at an address that is a multiple of 4.
            An 8-byte "double" must start at a memory address divisible by 8. Otherwise, the CPU will need to do
            multiple fetches and then reconstruct that type using the relevant bits from each cache line, which
            is slow. On some rigid architectures, this might not even be allowed and will crash.
         */

        assert(std::has_single_bit(alignment) && "alignment must be a power of 2");
        // NOTE(sbalse): Could also silently correct alignment to the next available power of 2, using:
        // alignment = std::bit_ceil(alignment); ?

        const size_t alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        const size_t newOffset = alignedOffset + size;

        if (newOffset > m_capacity)
        {
            return nullptr;
        }

        void* result = m_buffer + alignedOffset;

        m_offset = newOffset;

        return result;
    }

    void reset()
    {
        m_offset = 0;
    }

// private:
    std::byte* m_buffer{nullptr};
    size_t m_capacity{0};
    size_t m_offset{0};
};
