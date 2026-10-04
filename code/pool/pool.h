#pragma once
#include <cstddef>
#include <new>
#include <utility>

template <typename T>
struct Pool
{
    // Ensure that the type is large enough to hold our free-list pointer
    static_assert(sizeof(T) >= sizeof(uintptr_t));

    Pool(size_t size) : m_capacity{size}
    {
        m_buffer = new std::byte[m_capacity * sizeof(T)];

        // Below, we build a free list of same sized blocks. This way, our allocated buffer is physically an
        // array of raw bytes, but logically, it is a connected linked list of pointers waiting to be consumed.

        // The head starts at the very first block
        m_head = reinterpret_cast<Node*>(m_buffer);
        Node* current = m_head;

        // Link each block to the one sequentially ahead of it
        for (size_t i = 0; i < m_capacity - 1; i++)
        {
            std::byte* nextBlockAddress = m_buffer + ((i + 1) * sizeof(T));
            current->m_next = reinterpret_cast<Node*>(nextBlockAddress);
            current = current->m_next;
        }

        current->m_next = nullptr;
    }

    ~Pool()
    {
        if (m_buffer)
        {
            delete[] m_buffer;
        }
    }

    // Non-copyable
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    // Returns the first free block of memory from the free list. If no blocks are free, returns nullptr.
    void* allocate()
    {
        if (m_head == nullptr)
        {
            return nullptr;
        }

        Node* freeBlock = m_head;
        m_head = m_head->m_next;

        return freeBlock;
    }

    void free(void* deadObject)
    {
        if (deadObject == nullptr)
        {
            return;
        }

        Node* recycledBlock = reinterpret_cast<Node*>(deadObject);
        recycledBlock->m_next = m_head;
        m_head = recycledBlock;
    }

    template <typename... Args>
    T* spawn(Args&&... args)
    {
        void* memory = allocate();
        if (!memory)
        {
            return nullptr;
        }

        return new (memory) T(std::forward<Args>(args)...);
    }

    void despawn(T* instance)
    {
        if (!instance)
        {
            return;
        }

        // Call destructor to clean up the object
        instance->~T();

        free(instance);
    }

private:
    struct Node
    {
        Node* m_next;
    };

    std::byte* m_buffer;
    size_t m_capacity;
    Node* m_head{nullptr};
};
